// --------------------- client.cpp -----------------------------------------
//
// Siqi Zheng, CSS 503
// Created:         May. 27, 2020
// Last Modified:   Jun. 2, 2020
// --------------------------------------------------------------
// Purpose: implement a client-server model on TCP/IP protocol. 
// A client process establishes a connection to a server, sends data or requests, 
// and closes the connection.  
// The server accepts the connection and creates a thread to service the request 
// and then wait for another connection on the main thread. 
// What the client side do is to: (1) establish a connection to a server, (2) send 
// a message to the server letting it know the number of iterations of the test it 
// will perform, (3)perform the appropriate number of tests with the server (measure 
// the time this takes), (4) receive from the server a message with the number read() 
// system calls it performed, (5) print information about the test, (6)close the socket.
// ---------------------------------------------------------------------------

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <chrono>
#include <ratio>
using namespace std;

const int BUFFSIZE=1500;

int main(int argc, char *argv[])
{
    using namespace std::chrono;

    char *serverName;
    char *port;
    char *repetition;
    int nbufs;
    int bufsize;
    int type;

    char * serverResult = new char[100];

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int clientSD = -1;


    /*
     *  Argument validation
    1. serverName:  the name of the server
    2. port: the IP port number used by server (use the last 5 digits of your student id)
    3. repetition: the repetition of sending a set of data buffers
    4. nbufs: the number of data buffers
    5. bufsize: the size of each data buffer (in bytes)
    6. type: the type of transfer scenario: 1, 2, or 3
     */
    if (argc != 7)
    {
       cerr << "please provide all arguments" << endl;
       return -1;
    }

    /*
     * Use getaddrinfo() to get addrinfo structure corresponding to serverName / Port
	 * This addrinfo structure has internet address which can be used to create a socket too
     */
    serverName = argv[1];
    port = argv[2];
    repetition = argv[3];
    nbufs = atoi(argv[4]);
    bufsize = atoi(argv[5]);
    type = atoi(argv[6]);

    char databuf[nbufs][bufsize];
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;					/* Allow IPv4 or IPv6*/
    hints.ai_socktype = SOCK_STREAM;					/* TCP */
    hints.ai_flags = 0;							/* Optional Options*/
    hints.ai_protocol = 0;						/* Allow any protocol*/

    int rc = getaddrinfo(serverName, port, &hints, &result);
    if (rc != 0)
    {
       cerr << "ERROR: " << gai_strerror(rc) << endl;
       exit(EXIT_FAILURE);
    }

    /*
     * Iterate through addresses and connect
     */
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (clientSD == -1)
		{
            continue;
        }
		/*
		* A socket has been successfully created
		*/
        rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0)
        {
            cerr << "Connection Failed" << endl;
            close(clientSD);
            return -1;
        }
        else	//success
        {
            break;
        }
    }

    if (rp == NULL)
    {
        cerr << "No valid address" << endl;
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(result);

    // Tell server the repetion number, use "#" to mark the end of number
    char endchar = '#';
    int writeLength = write(clientSD, repetition, strlen(repetition));
    write(clientSD, &endchar, 1);

    /*
     *  init data to be written
     */
    for (int i = 0; i < nbufs ; i++)
    {
        for(int j = 0; j< bufsize; j++)
        {
            databuf[i][j] = 'z';
        }
    }

     struct iovec vector[nbufs];
     for (int j = 0; j < nbufs; j++) 
     {
        vector[j].iov_base = databuf[j];
        vector[j].iov_len = bufsize;
     }

    int repetitionCount =  atoi(repetition);

    // Depends on the type, write data to server
    steady_clock::time_point start = steady_clock::now();//record the start time

    if(type == 1) 
    {    //Multiple writes: invokes the write() system call for each data buffer, 
        //thus resulting in calling as many write()s as the number of data buffers, (i.e., nbufs).
        for(int i = 0; i< repetitionCount; i++)
        {   
            for (int j = 0; j < nbufs; j++)
            {
                write(clientSD, databuf[j], bufsize);  
            }
        }
    }
    else if(type == 2)
    {    //writev: allocates an array of iovec data structures, each having its *iov_base field 
        // point to a different data buffer as well as storing the buffer size in its iov_len field; 
        //and thereafter calls writev() to send all data buffers at once. 
        for(int i = 0; i< repetitionCount; i++)
        {   
            writev(clientSD, vector, nbufs); 
        }
    }
    else if (type ==3)
    {   //single write: allocates an nbufs-sized array of data buffers, and thereafter calls 
        // write() to send this array, (i.e., all data buffers) at once.
        for(int i = 0; i< repetitionCount; i++)
        {   
            write(clientSD, databuf, nbufs * bufsize);
        } 
    }
    else
    {
        cerr << "Type need to be 1, 2 or 3" << endl;
        exit(EXIT_FAILURE);
    }
    

    int bytesRead = read(clientSD, serverResult, 99);
    serverResult[bytesRead] = '\0';
    
    steady_clock::time_point end = steady_clock::now(); //record the finish time

    duration<double> time_span = duration_cast<duration<double>>(end - start); //caculate the time slot
    
    printf("Test (%d): time = %f sec, #reads = %d, throughput %f Gbps \n", 
        type, time_span.count(), atoi(serverResult), 
        repetitionCount* nbufs * bufsize * 8/ double(1024*1024*1024)/time_span.count()); //throughput
    
    close(clientSD);
    return 0;
}

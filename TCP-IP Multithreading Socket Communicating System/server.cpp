// --------------------- server.cpp -----------------------------------------
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
// What the server do in detail is to: (1) allocate dataBuf[BUFSIZE], where BUFSIZE = 1500 
// to read in data being sent by client, (2) receive a message by the client with the number 
// of iterations to perform, (3) read from the client the appropriate number of iterations 
// of BUFSIZE amounts of data (4) send the number of read() calls made as an acknowledgment 
// to the client (5)close this connection. (6) terminate the thread.
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

const int BUFFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

/*thread function
*/
void * socketThread(void *arg)
{
    int repetition;
    int readCount = 0;
    char* repetitionbuf = new char[32]; //used to send the repetition number
    char c = '0';

    char databuf[BUFFSIZE];
    bzero(databuf, BUFFSIZE);

    int newSD = *((int*)arg); //client socket descriptor

    int index = 0;
    while (read(newSD, &c, 1) && c != '#')//'#' is a flag that the client tells server the end of number
    {
        repetitionbuf[index++] = c;
    }

    repetitionbuf[index] = '\0';
    repetition = atoi(repetitionbuf);

    for (int i = 0; i < repetition; i++) //in this program 20000 repetition
    {
        int bytesRead = 0;
        while (bytesRead < BUFFSIZE)
        {
            bytesRead += read(newSD, databuf + bytesRead, BUFFSIZE - bytesRead);
            readCount++;
        }
    }

    const char* result = to_string(readCount).c_str();

    int bytesWritten = write(newSD, result, strlen(result));
    
    printf("Exit socketThread \n");

    close(newSD);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    char *serverPort;

    char databuf[BUFFSIZE];
    bzero(databuf, BUFFSIZE);

    pthread_t tid[60];

    /* 
     * Read arguments
     */
    if (argc != 2)
    {
       cerr << "please provide all arguments" << endl;
       return -1;
    }
   
    serverPort = argv[1];

    /*
     * Build address
     */
    int port = stoi(serverPort);
    sockaddr_in acceptSocketAddress;
    bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    acceptSocketAddress.sin_family = AF_INET;
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    acceptSocketAddress.sin_port = htons(port);

    /*
     *  Open socket and bind
     */
    int serverSD = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
   
    int rc = bind(serverSD, (sockaddr *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    if (rc < 0)
    {
        cerr << "Bind Failed" << endl;
    }

    /*
     *  listen and accept
     */
    listen(serverSD, NUM_CONNECTIONS);       //setting number of pending connections
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);

    int i = 0;
    while(1)
    {
      int newSD = accept(serverSD, (sockaddr *) &newSockAddr, &newSockAddrSize);

      if (pthread_create(&tid[i++], NULL, socketThread, &newSD) != 0)
      {
          cerr<<"Failed to create thread"<<endl;
      }
            
    }

    close(serverSD);

    return 0;
}
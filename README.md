# TCP/IP-Multithreading-Socket-Communicating-System

In this program we will use the client-server model where a client process establishes a connection to a server, sends data or requests, and closes the connection.  
The server will accept the connection and create a thread to service the request and then wait for another connection on the main thread.  Servicing the request consists of (1) reading the number of iterations the client will perform, (2) reading the data sent by the client, and (3) sending the number of reads which the server performed.   

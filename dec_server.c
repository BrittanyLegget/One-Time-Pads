#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}


void SendReceive(int conn, char* buffer, int size, char* type)
{
    //Number of characters remaining to be sent or received
    int remaining = size;
    //How many have been received so far
    int received = 0;
    //Hold number of characters received in each run
    int chars;
    //Determine if this is the first run, if so, we send/receive the length of data to be expected first.
    int len = 0;
    //Holds the number of characters to be expected
    int exp;

    while (remaining > 0)
    {
        //confirm correct connection
        if (strcmp(type, "src") == 0) {
            //printf("Confirming Souce\n");
            recv(conn, buffer, size, 0);
            remaining = 0;
        }
        //connection source declined, notify
        if (strcmp(type, "dec") == 0) {
            send(conn, buffer, size, 0);
            remaining = 0;
        }
        //Send Data
        if (strcmp(type, "send") == 0) {

            // First send the length to be expected
            if (len == 0) {
                send(conn, &size, sizeof(size), 0);
                len = 1;
            }
            //Then send the data
            chars = send(conn, buffer + received, remaining, 0);
        }
        //Receive Data
        if (strcmp(type, "recv") == 0) {

            // First receive the length we should expect
            if (len == 0) {
                recv(conn, &exp, sizeof(exp), 0);
                remaining = exp;
                len = 1;
            }
            // Then receive the specified length
            chars = recv(conn, buffer + received, remaining, 0);
        }
        //if error, exit
        if (chars == -1)
        {
            exit(1);
        }
        //Increment number of characters received
        received += chars;
        //Decrement the number of characters left to receive
        remaining -= chars;
    }
}

int main(int argc, char* argv[]) {

    int connectionSocket, charsRead;
    char buffer[10000];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    // Create buffers
    char textBuffer[5000];
    char keyBuffer[5000];
    char sendBuffer[10000];
    char receiveBuffer[5000];
    char source[4];
    char sendSource[4] = "dec";

    int childStatus;
    // Check usage & args
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // make ports reusable
    //Source: https://stackoverflow.com/questions/48263191/socket-programming-c-setsockopt
    const int reuse = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket,
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    //Child prcess ID
    pid_t spawnPid;

    // Accept a connection, blocking if one is not available until one connects
    while (1) {

        //Fork a new process
        spawnPid = fork();

        switch (spawnPid) {
        case -1:
            perror("fork()\n");
            break;
        case 0:

            // Clear out the buffer arrays
            memset(textBuffer, '\0', sizeof(textBuffer));
            memset(keyBuffer, '\0', sizeof(keyBuffer));
            memset(sendBuffer, '\0', sizeof(sendBuffer));
            memset(receiveBuffer, '\0', sizeof(receiveBuffer));

            // Accept the connection request which creates a connection socket
            connectionSocket = accept(listenSocket,
                (struct sockaddr*)&clientAddress,
                &sizeOfClientInfo);
            if (connectionSocket < 0) {
                error("ERROR on accept");
            }

           // printf("SERVER: Connected to client running at host %d port %d\n",
             /*   ntohs(clientAddress.sin_addr.s_addr),
                ntohs(clientAddress.sin_port));*/

            /********************************************************
                CONFIRM CORRECT CONNECTION & RECEIVE DATA
             *********************************************************/
             //First confirm source connection
            SendReceive(connectionSocket, source, 3, "src");

            //Send back results of connection check
            SendReceive(connectionSocket, sendSource, 3, "dec");

            //If correct connection, receive Data
            SendReceive(connectionSocket, receiveBuffer, 1, "recv");
            //printf("SERVER: I received this from the client: \"%s\"\n", receiveBuffer);


            /********************************************************
                PARSE RECEIVED STRING (Delimiter is ':' )
            *********************************************************/
            char* delimiter;
            int leftover;
            //string up until ++ if found
            delimiter = strstr(receiveBuffer, ":");

            //find length to ++ occurance
            int len = delimiter - receiveBuffer;

            //Copy string up until ++
            strncpy(textBuffer, receiveBuffer, len);

            //length of leftover string
            leftover = strlen(receiveBuffer) - len;

            //copy key located after delimiter into keyBuffer
            strncpy(keyBuffer, delimiter + 1, leftover);

            /********************************************************
                ENCRYPT STRING

            Sources:
            https://en.wikipedia.org/wiki/One-time_pad
            https://en.wikipedia.org/wiki/Modular_arithmetic
            *********************************************************/
            //Valid char reference set (same from keygen)
            char keySet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
            //To store each character for processing
            char textChar;
            char keyChar;
            //Store the index of each character for modular arithmatic
            int textIndex;
            int keyIndex;
            //Store the cipher result to be added to sendBuffer
            int cipher;

            for (int i = 0; i < strlen(textBuffer); i++) {
                //Character in plaintext
                textChar = textBuffer[i];

                //find in keySet
                char* textPtr = strchr(keySet, textChar);

                //Store index
                textIndex = textPtr - keySet;

                //Character in Key
                keyChar = keyBuffer[i];

                //Find in keySet
                char* keyPtr = strchr(keySet, keyChar);

                //Store Index
                keyIndex = keyPtr - keySet;

                //Check to see if negative and if so, add 27
                int negCheck = (textIndex - keyIndex);
                if (negCheck < 0) {
                    negCheck += 27;
                }

                //modulo 27
                cipher = (negCheck % 27);

                //Find location of cipher in keySet and save into send buffer to return to enc_client
                sendBuffer[i] = keySet[cipher];

            }
            // strcpy(sendBuffer, textBuffer);

             //send encrypted string back to enc_client
            SendReceive(connectionSocket, sendBuffer, strlen(sendBuffer), "send");

            // Close the connection socket for this client
            close(connectionSocket);
            break;
        default:
            // Wait for child's termination
            waitpid(spawnPid, &childStatus, 0);
        }
    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}


/*
Author: Brittany Legget
Date: 3/3/2022
CS 344 Assignment #5 - One-Time-Pads -> enc_clinet

*/

/*
This program connects to enc_server, and asks it to perform a one-time pad style encryption.
By itself, enc_client doesn’t do the encryption - enc_server does.
It processes a file, ensures the key is at least as long as the file, and then sends the data from
the file to enc_server to be encrypted. It will then receive the encrypted data back from enc_server.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()


// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,

    int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname("localhost");
    if (hostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*)&address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}
/*
* Function will handle the sending and receiving of data to and from enc_server
* as well as sending a confirmation of the source to ensure appropraite connection
*/
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
            //printf("Sending Source");
            send(conn, buffer, size, 0);
            remaining = 0;
        }
        //connection source declined
        if (strcmp(type, "dec") == 0) {
            recv(conn, buffer, size, 0);
            remaining = 0;
        }
        //Send Data
        if (strcmp(type, "send") == 0) {

            // First send the length to be expected
            if (len == 0) {
                send(conn, &size, sizeof(size), 0);
                len = 1;
            }
            // Then send message
            chars = send(conn, buffer + received, remaining, 0);
            if (chars < 0) {
                error("CLIENT: ERROR writing to socket");
            }
        }
        //Receive encrypted message
        if (strcmp(type, "recv") == 0) {

            // First receive the length we should expect
            if (len == 0) {
                recv(conn, &exp, sizeof(exp), 0);
                remaining = exp;
                len = 1;
            }
            //Then receive specified length of data
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

/*
* Main function, handles creating and confirming connection, processing data
* from file, sending the data to enc_server, and then receiving the encrypted
* data back in response.
* arg 1 = plaintext
* arg 2 = key
* arg 3 = port
*/
int main(int argc, char* argv[]) {


    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;

    // Delimiter between text and key to be sent to server
    char delimiter = ':';

    //length of text being sent
    int sendLength;

    // Create buffers
    char textBuffer[5000];
    char keyBuffer[5000];
    char sendBuffer[10000];
    char receiveBuffer[5000];
    char source[4] = "dec";
    char returnSource[4];
 

    // Check usage & args
    if (argc < 3) {
        fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
        exit(0);
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]));

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }

    /********************************************************
       PREPARE DATA TO BE SENT TO SERVER
    *********************************************************/

    //Create pointer to open files in read only manner
    FILE* plainText = fopen(argv[1], "r");
    FILE* keyFile = fopen(argv[2], "r");

    // Clear out the buffer arrays
    memset(textBuffer, '\0', sizeof(textBuffer));
    memset(keyBuffer, '\0', sizeof(keyBuffer));
    memset(sendBuffer, '\0', sizeof(sendBuffer));
    memset(receiveBuffer, '\0', sizeof(receiveBuffer));
    memset(returnSource, '\0', sizeof(returnSource));

    // Get contents of files
    fgets(textBuffer, sizeof(textBuffer), plainText);
    fgets(keyBuffer, sizeof(keyBuffer), keyFile);

    // close
    fclose(plainText);
    fclose(keyFile);


    //Remove the trailing \n 
    textBuffer[strcspn(textBuffer, "\n")] = '\0';
    keyBuffer[strcspn(keyBuffer, "\n")] = '\0';

    //Check for invalid characters and exit if present
    for (int i = 0; i < strlen(textBuffer); i++) {
       
            if (textBuffer[i] < 65 || textBuffer[i] > 90) {
                if (textBuffer[i] != 32) {
                    memset(textBuffer, '\0', sizeof(textBuffer));
                    perror("Error: Invalid values in file.");
                    exit(1);
                }
            }
     
    }


    // check that the key is at least as long as the plainText file and error out if not
    if (strlen(keyBuffer) < strlen(textBuffer))
    {
        printf("Error: key %s is too short!\n", argv[2]);
        exit(1);
    }


    //Add the delimiter to the end of the text
    textBuffer[strlen(textBuffer)] = delimiter;

    //Add the plaintext to the send buffer then concatenate the key to the end
    strcpy(sendBuffer, textBuffer);
    strcat(sendBuffer, keyBuffer);

    // Save length of content being sent
    sendLength = strlen(sendBuffer);

    /********************************************************
        SEND DATA TO SERVER
    *********************************************************/

    //Send source of data for confirmation by server
    SendReceive(socketFD, source, strlen(source), "src");

    //Receive results from connection confirmation
    SendReceive(socketFD, returnSource, 3, "dec");

    //If wrong source connection, close connection
    if (strcmp(returnSource, "dec") != 0) {
        printf("Error: dec_client cannot connect to enc_server. Closing connection");

        // Close the connection socket for this client
        close(socketFD);
    }
    else {
        //Then send Data
        SendReceive(socketFD, sendBuffer, sendLength, "send");

        // Finally, receive encrypted data from enc_server      
        SendReceive(socketFD, receiveBuffer, sendLength, "recv");

        //print to standard output
        printf("%s\n", receiveBuffer);
    }

    close(socketFD);
    return 0;
}
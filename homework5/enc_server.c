//This is the enc server file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char* argv[]) {
    int connectionSocket, charsRead, charsWritten;
    char buffer[999999];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
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

    // Accept a connection, blocking if one is not available until one connects
    while (1) {
        connectionSocket = accept(listenSocket,
            (struct sockaddr*)&clientAddress,
            &sizeOfClientInfo);
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }

        //fork and let the child to communicate via socket
        pid_t spawnPid = fork();
        if (spawnPid == 0) {
            // Get the message from the client and display it
            memset(buffer, '\0', sizeof(buffer));
            // Read the client's message from the socket
            charsRead = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
            if (charsRead < 0) {
                error("ERROR reading from socket");
            }
            //verify the client
            if (buffer[0] != '+') {
                memset(buffer, '\0', sizeof(buffer));
                buffer[0] = '+';
                charsWritten = 0;
                while (charsWritten < strlen(buffer)) {
                    charsWritten = send(connectionSocket, buffer, strlen(buffer), 0);
                }
                close(connectionSocket);
                exit(2);
            }

            //send back the verification message to client
            memset(buffer, '\0', sizeof(buffer));
            buffer[0] = '+';
            charsWritten = 0;
            while (charsWritten < strlen(buffer)) {
                charsWritten = send(connectionSocket, buffer, strlen(buffer), 0);
            }

            //read the text
            char text[999999] = { '\0' };
            memset(buffer, '\0', sizeof(buffer));
            charsRead = 0;
            while (buffer[charsRead - 1] != '\n') {
                charsRead = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
                strcat(text, buffer);
            }
            //remove the ending \n
            text[strcspn(text, "\n")] = '\0';

            //read the key
            char key[999999] = { '\0' };
            memset(buffer, '\0', sizeof(buffer));
            charsRead = 0;
            while (buffer[charsRead - 1] != '\n') {
                charsRead = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
                strcat(key, buffer);
            }
            //remove the ending \n
            key[strcspn(key, "\n")] = '\0';


            //perform the encryption
            int i = 0;
            char ctext[999999];
            
            for (i; i < strlen(text); i++) {
                if (key[i] == ' ') {
                    key[i] = '@';
                }
                if (text[i] == ' ') {
                    text[i] = '@';
                }
                int j = (int)text[i];
                int k = (int)key[i];
                int s = j + k - 64;
                while (s > 90) {
                    s = s - 27;
                }
                ctext[i] = (char)s;
                if (ctext[i] == '@') {
                    ctext[i] = ' ';
                }
                if (text[i] == '@') {
                    text[i] = ' ';
                }
            }


            //send the text back with \n
            int t = strlen(ctext);
            ctext[t] = '\n';

            memset(buffer, '\0', sizeof(buffer));
            charsWritten = 0;
            int sum = 0;
            int p = 0;
            int q = 0;
            //make sure string length of buffer is larger than 0 to enter the loop
            buffer[0] = 'H';
            //send the data back, and if it got interrupted, pick up at where it lefted
            while (sum < strlen(buffer)) {
                memset(buffer, '\0', sizeof(buffer));
                p = sum;
                q = 0;
                for (p; p < strlen(ctext); p++) {
                    buffer[q] = ctext[p];
                    q++;
                }
                charsWritten = send(connectionSocket, buffer, strlen(buffer), 0);
                sum = sum + charsWritten;
                memset(buffer, '\0', sizeof(buffer));
            }
            // Close the connection socket for this client
            close(connectionSocket);
            exit(0);
        }
        waitpid(-1, &childStatus, WNOHANG);
    }
    
    // Close the listening socket
    close(listenSocket);
    return 0;
}
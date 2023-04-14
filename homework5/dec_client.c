//This is the dec_client file
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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
    int portNumber,
    char* hostname) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*)&address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char* argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char* hostname = "localhost";
    char buffer[999999];
    // Check usage & args
    if (argc < 4) {
        fprintf(stderr, "You don't have enough parameters\n");
        exit(0);
    }

    //read plain text
    char* filePath = argv[1];
    FILE* inputFile = fopen(filePath, "r");

    if (inputFile == NULL) {
        fprintf(stderr, "The input file didn't open");
        exit(EXIT_FAILURE);
    }

    char currLine[999999] = { '\0' };  //use to store text
    char* r = currLine;   //a pointer to the text that we can use getline() below
    size_t len = 999999;
    ssize_t nread;
    int counter = 0;
    nread = getline(&r, &len, inputFile);

    //check for bad characters
    int e = 0;
    for (e; e < nread - 1; e++) {
        if ((int)currLine[e] < 65 || (int)currLine[e]>90) {
            if ((int)currLine[e] != 32) {
                fprintf(stderr, "enc_client error: input contains bad characerts\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(inputFile);

    //read the key
    char* keyPath = argv[2];
    FILE* inputKey = fopen(keyPath, "r");

    if (inputKey == NULL) {
        fprintf(stderr, "The input file didn't open");
        exit(EXIT_FAILURE);
    }

    char currKey[999999] = { '\0' };
    char* k = currKey;
    size_t klen = 999999;
    ssize_t knread;
    int kcounter = 0;
    knread = getline(&k, &klen, inputKey);

    //check for bad characters
    e = 0;
    for (e; e < nread - 1; e++) {
        if ((int)currKey[e] < 65 && (int)currKey[e]>90) {
            if ((int)currKey[e] != 32) {
                fprintf(stderr, "enc_client error: input key contains bad characerts\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(inputKey);

    //make sure the key is at least as long as the text
    if (strlen(currKey) < strlen(currLine)) {
        fprintf(stderr, "key is too short\n");
        exit(1);
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), hostname);

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Error: could not contact enc_server on port %d\n", atoi(argv[3]));
        exit(2);
    }

    // Clear out the buffer array
    memset(buffer, '\0', sizeof(buffer));

    //set the message we will send to +
    buffer[0] = '-';

    // Send message to server
    charsWritten = 0;
    while (charsWritten < strlen(buffer)) {
        charsWritten = send(socketFD, buffer, strlen(buffer), 0);
    }

    // Get return message from server
    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

    if (charsRead < 0) {
        error("CLIENT: ERROR reading from socket");
    }
    //if the server didn't valify the client, close connection
    if (buffer[0] != '-') {
        fprintf(stderr, "Error: could not contact enc_server on port %d\n", atoi(argv[3]));
        close(socketFD);
        exit(2);
    }
    memset(buffer, '\0', sizeof(buffer));

    //send the text
    charsWritten = 0;
    int sum = 0;
    int p = 0;
    int q = 0;
    //make sure string lenth of buffer is larger than 0 to enter the loop
    buffer[0] = 'H';
    //if send get interrupted, the loop will pick up at where it interrupted and finish sending
    while (sum < strlen(buffer)) {
        memset(buffer, '\0', sizeof(buffer));
        p = sum;
        q = 0;
        for (p; p < strlen(currLine); p++) {
            buffer[q] = currLine[p];
            q++;
        }
        charsWritten = send(socketFD, buffer, strlen(buffer), 0);
        sum = sum + charsWritten;

    }

    //send the key
    memset(buffer, '\0', sizeof(buffer));
    charsWritten = 0;
    sum = 0;
    p = 0;
    q = 0;
    buffer[0] = 'H';
    //if send get interrupted, the loop will pick up at where it interrupted and finish sending
    while (sum < strlen(buffer)) {
        memset(buffer, '\0', sizeof(buffer));
        p = sum;
        q = 0;
        for (p; p < strlen(currKey); p++) {
            buffer[q] = currKey[p];
            q++;
        }
        charsWritten = send(socketFD, buffer, strlen(buffer), 0);
        sum = sum + charsWritten;

    }

    //receive the text
    char final[999999] = { '\0' };
    memset(buffer, '\0', sizeof(buffer));
    charsRead = 0;
    //the text will end with \n, if we didn't get the full message, receive again and append to the old message
    while (buffer[charsRead - 1] != '\n') {
        charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
        strcat(final, buffer);
    }

    //output the text
    printf("%s", &final);

    // Close the socket
    close(socketFD);
    return 0;
}
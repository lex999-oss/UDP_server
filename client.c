#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
/*
	UDP client
*/
#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib, "ws2_32.lib") //Winsock Library

#define SERVER "127.0.0.1"    //ip address of udp server
#define BUFLEN 2048    //Max length of buffer
#define PORT 8888    //The port on which to listen for incoming data

void cleanup(int sock)
{
    closesocket(sock);
    WSACleanup();
}

int main(void) {
    struct sockaddr_in si_other;
    int s, slen = sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    char tree_buf[4*BUFLEN];
    WSADATA wsa;

    //Initialise winsock
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Initialised.\n");

    //create socket
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
        printf("socket() failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    //setup address structure
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

    //start communication
    printf("client> ");
    gets(message);

    char *aux = (char *) malloc(BUFLEN);
    strcpy(aux, message);

    char *arg = strtok(aux, " ");
    arg = strtok(NULL, " ");

    if (0 == strncmp(message, "createfile", strlen("createfile"))) {
        if (NULL != strchr(arg, '.')) {
            //send the message
            if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
                printf("sendto() failed with error code : %d", WSAGetLastError());
                cleanup(s);
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Filename must include extension!\n");
            cleanup(s);
            exit(EXIT_FAILURE);
        }
    } else if (0 == strncmp(message, "deletefile", strlen("deletefile"))) {
        if (NULL != strchr(arg, '.')) {
            //send the message
            if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
                printf("sendto() failed with error code : %d", WSAGetLastError());
                cleanup(s);
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Filename must include extension!\n");
            cleanup(s);
            exit(EXIT_FAILURE);
        }
    } else if (0 == strncmp(message, "append", strlen("append"))) {
        if (NULL != strchr(arg, '.')) {
            //send the message
            if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
                printf("sendto() failed with error code : %d", WSAGetLastError());
                cleanup(s);
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Filename must include extension!\n");
            cleanup(s);
            exit(EXIT_FAILURE);
        }
    } else if (0 == strncmp(message, "close_server", strlen("close_server"))) {
        //send the message
        if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
            printf("sendto() failed with error code : %d", WSAGetLastError());
            cleanup(s);
            exit(EXIT_FAILURE);
        }
        cleanup(s);
        exit(EXIT_SUCCESS);
    } else if (0 == strncmp(message, "createkey", strlen("createkey"))) {
        if ((0 == strcmp(arg, "HKEY_CLASSES_ROOT") ||
             0 == strcmp(arg, "HKEY_CURRENT_CONFIG") ||
             0 == strcmp(arg, "HKEY_CURRENT_USER") ||
             0 == strcmp(arg, "HKEY_LOCAL_MACHINE") ||
             0 == strcmp(arg, "HKEY_USERS"))) {
            arg = strtok(NULL, " ");
            if (0 != strcmp(arg, "")) {
                //send the message
                if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
                    printf("sendto() failed with error code : %d", WSAGetLastError());
                    cleanup(s);
                    exit(EXIT_FAILURE);
                }
            } else {
                printf("SubKey cannot be empty!\n");
                cleanup(s);
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Invalid parameters to create registry key!\n");
            cleanup(s);
            exit(EXIT_FAILURE);
        }
    } else if (0 == strncmp(message, "deletekey", strlen("deletekey"))) {
        if ((0 == strcmp(arg, "HKEY_CLASSES_ROOT") ||
             0 == strcmp(arg, "HKEY_CURRENT_CONFIG") ||
             0 == strcmp(arg, "HKEY_CURRENT_USER") ||
             0 == strcmp(arg, "HKEY_LOCAL_MACHINE") ||
             0 == strcmp(arg, "HKEY_USERS"))) {
            arg = strtok(NULL, " ");
            if (0 != strcmp(arg, "")) {
                //send the message
                if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
                    printf("sendto() failed with error code : %d", WSAGetLastError());
                    cleanup(s);
                    exit(EXIT_FAILURE);
                }
            } else {
                printf("SubKey cannot be empty!\n");
                cleanup(s);
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Invalid parameters to delete registry key!\n");
            cleanup(s);
            exit(EXIT_FAILURE);
        }
    } else if (0 == strncmp(message, "download", strlen("download"))) {
        if (arg == NULL) {
            printf("URL cannot be null!\n");
            cleanup(s);
            exit(EXIT_FAILURE);
        } else {
            arg = strtok(NULL, " ");
            if (arg == NULL) {
                printf("File path cannot be null!\n");
                cleanup(s);
                exit(EXIT_FAILURE);
            } else {
                //send the message
                if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
                    printf("sendto() failed with error code : %d", WSAGetLastError());
                    cleanup(s);
                    exit(EXIT_FAILURE);
                }
            }
        }
    } else if (0 == strncmp(message, "listdir", strlen("listdir"))) {
        if (arg == NULL) {
            printf("Path to dir cannot be null!\n");
        } else {
            //send the message
            if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
                printf("sendto() failed with error code : %d", WSAGetLastError());
                cleanup(s);
                exit(EXIT_FAILURE);
            }
        }
        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR) {
            printf("recvfrom() failed with error code : %d", WSAGetLastError());
            cleanup(s);
            exit(EXIT_FAILURE);
        }
        if (0 == strncmp(buf, "nok", strlen("nok"))) {
            printf("Command failed to execute successfully on remote server!\n");
            cleanup(s);
            exit(EXIT_FAILURE);
        } else {
            printf("Remote server directory for %s", arg);
            printf("%s\n", buf);
        }
    } else if (0 == strncmp(message, "run", strlen("run"))){
        if (arg == NULL){
            printf("Path to executable cannot be null!\n");
            cleanup(s);
            exit(EXIT_FAILURE);
        } else if(NULL == strstr(arg, ".exe"))
        {
            printf("File must be an executable file!\n");
            cleanup(s);
            exit(EXIT_FAILURE);
        }
        //send the message
        if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
            printf("sendto() failed with error code : %d", WSAGetLastError());
            cleanup(s);
            exit(EXIT_FAILURE);
        }
    } else if (NULL != strstr(message, "exit")) {
        cleanup(s);
        exit(EXIT_SUCCESS);
    } else {
        printf("Invalid command!\n");
        cleanup(s);
        exit(EXIT_FAILURE);
    }

    memset(buf, '\0', BUFLEN);
    //receive a reply and print it
    //try to receive some data, this is a blocking call
    if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR) {
        printf("recvfrom() failed with error code : %d", WSAGetLastError());
        cleanup(s);
        exit(EXIT_FAILURE);
    }
    if (0 == strncmp(buf, "ok", strlen("ok")))
        printf("Command executed successfully on remote server!\n");
    else
        printf("Command failed to execute successfully on remote server!\n");

    cleanup(s);

    return 0;
}

#pragma clang diagnostic pop
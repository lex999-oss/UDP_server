#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
/*
	UDP Server with one thread
*/

#include <stdio.h>
#include <wininet.h>
#include <dirent.h>


#pragma comment(lib, "ws2_32.lib") //Winsock Library

#define BUFLEN 512    //Max length of buffer
#define PORT 8888    //The port on which to listen for incoming data

/* Function from Microsoft documentation for recursive registry key deletion(keys in keys/values in keys) */
BOOL RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey) {
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
        return TRUE;

    lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            printf("[debug]Key not found.\n");
            return FALSE; //return false if key is not found (@original-code : return TRUE)
        } else {
            printf("[error]Error opening key.\n");
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd = TEXT('\\');
        lpEnd++;
        *lpEnd = TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) {
        do {

            *lpEnd = TEXT('\0');
            strncat(lpSubKey, szName, MAX_PATH * 2); //concatenate child key name to parent key

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey(hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
        return TRUE;

    return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnode(HKEY hKeyRoot, LPCTSTR lpSubKey) {
    TCHAR szDelKey[MAX_PATH * 2];

    strncpy(szDelKey, lpSubKey, MAX_PATH * 2);
    return RegDelnodeRecurse(hKeyRoot, szDelKey);

}

DWORD WINAPI ThreadFunc(void *param);

BOOL GetDir(LPTSTR path);

char cDirTree[4 * BUFLEN];

int main() {
    SOCKET s;
    struct sockaddr_in server;
    WSADATA wsa;
    HANDLE hThread;

    printf("MAIN THREAD <PID: %lu>", GetCurrentThreadId());
    //Initialise winsock
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Initialised.\n");

    //Create a socket
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }
    printf("Socket created.\n");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    //Bind socket to address
    if (bind(s, (struct sockaddr *) &server, sizeof(server)) == SOCKET_ERROR) {
        printf("[error]Bind failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    puts("Bind done");

    //Start Server Thread

    hThread = CreateThread(NULL, 0, ThreadFunc, &s, 0, NULL);

    if (NULL == hThread) {
        printf("[error]Error creating Server Thread: %lu \n", GetLastError());
    }

    WaitForSingleObject(hThread, INFINITE);

    //cleanup
    closesocket(s);
    CloseHandle(hThread);
    WSACleanup();

    return 0;
}

/* Function for SERVER THREAD instructions */
DWORD WINAPI ThreadFunc(void *param) {
    printf("[server]SERVER THREAD <PID: %lu>\n", GetCurrentThreadId());
    SOCKET s = *(SOCKET *) param;
    struct sockaddr_in si_other;
    int slen;
    slen = sizeof(si_other);
    char *recvbuf = (char *) malloc(BUFLEN);
    char *sendbuf = (char *) malloc(BUFLEN);
    char *aux = (char *) malloc(BUFLEN);
    char *arg;
    while (1) {
        printf("[server]Waiting for data...\n");
        fflush(stdout);

        //clear the buffer by filling null, it might have previously received data
        memset(recvbuf, '\0', BUFLEN);
        memset(sendbuf, '\0', BUFLEN);

        //try to receive some data, this is a blocking call
        if (recvfrom(s, recvbuf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR) {
            printf("[error]recvfrom() failed with error code : %d", WSAGetLastError());
            break;
        }
        strcpy(aux, recvbuf);
        arg = strtok(aux, " ");
        arg = strtok(NULL, " ");
        printf("[server]BUFFER: %s\n", recvbuf);
        if (0 == strncmp(recvbuf, "createfile", strlen("createfile"))) { //create file in server local dir
            HANDLE hFile = CreateFileA(arg, GENERIC_READ | GENERIC_WRITE, 0, NULL, 1, CREATE_ALWAYS, NULL);
            if (hFile != NULL) {
                strcpy(sendbuf, "ok");
                CloseHandle(hFile);
            } else {
                strcpy(sendbuf, "nok");
            }
        } else if (0 == strncmp(recvbuf, "deletefile", strlen("deletefile"))) { //delete file from server's local dir
            if (TRUE == DeleteFileA(arg))
                strcpy(sendbuf, "ok");
            else
                strcpy(sendbuf, "nok");
        } else if (0 == strncmp(recvbuf, "append", strlen("append"))) { //append to file from server's local dir
            HANDLE hAppend = CreateFile(TEXT(arg),                // open file
                                        FILE_APPEND_DATA,         // open for writing
                                        FILE_SHARE_READ,          // allow multiple readers
                                        NULL,                     // no security
                                        OPEN_ALWAYS,              // open or create
                                        FILE_ATTRIBUTE_NORMAL,    // normal file
                                        NULL);                    // no attr. template
            arg = strtok(NULL, " ");
            DWORD dwBytesWritten, dwPos;
            dwPos = SetFilePointer(hAppend, 0, NULL, FILE_END);
            LockFile(hAppend, dwPos, 0, strlen(arg), 0);
            WriteFile(hAppend, arg, strlen(arg), &dwBytesWritten, NULL);
            UnlockFile(hAppend, dwPos, 0, strlen(arg), 0);

            CloseHandle(hAppend);

            strcpy(sendbuf, "ok");
            if (hAppend == INVALID_HANDLE_VALUE) {
                printf("[server]Could not create file! error no. %lu\n", GetLastError());
                strcpy(sendbuf, "nok");
            }
        } else if (0 == strncmp(recvbuf, "createkey", strlen("createkey"))) {
            if (0 == strcmp(arg,
                            "HKEY_CLASSES_ROOT")) { /* check for every known Key -- workaround due to RegCreateKeyEx() */
                HKEY hkResult;                           /*                                                     definition-- */
                arg = strtok(NULL, " ");
                LONG openRes = RegCreateKeyEx(
                        HKEY_CLASSES_ROOT,
                        arg,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkResult,
                        NULL);

                if (openRes == ERROR_SUCCESS) {
                    printf("Success creating key.");
                    strcpy(sendbuf, "ok");
                    RegCloseKey(hkResult);
                } else {
                    printf("Error creating key.");
                    strcpy(sendbuf, "nok");
                }
            } else if (0 == strcmp(arg, "HKEY_CURRENT_CONFIG")) {
                HKEY hkResult;
                arg = strtok(NULL, " ");
                LONG openRes = RegCreateKeyEx(
                        HKEY_CURRENT_CONFIG,
                        arg,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkResult,
                        NULL);

                if (openRes == ERROR_SUCCESS) {
                    printf("[server]Success creating key.");
                    strcpy(sendbuf, "ok");
                    RegCloseKey(hkResult);
                } else {
                    printf("[server]Could not create key! error no. %lu\n", GetLastError());
                    strcpy(sendbuf, "nok");
                }
            } else if (0 == strcmp(arg, "HKEY_CURRENT_USER")) {
                HKEY hkResult;
                arg = strtok(NULL, " ");
                LONG openRes = RegCreateKeyEx(
                        HKEY_CURRENT_USER,
                        arg,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkResult,
                        NULL);

                if (openRes == ERROR_SUCCESS) {
                    printf("Success creating key.");
                    strcpy(sendbuf, "ok");
                    RegCloseKey(hkResult);
                } else {
                    printf("[server]Could not create key! error no. %lu\n", GetLastError());
                    strcpy(sendbuf, "nok");
                }
            } else if (0 == strcmp(arg, "HKEY_LOCAL_MACHINE")) {
                HKEY hkResult;
                arg = strtok(NULL, " ");
                LONG openRes = RegCreateKeyEx(
                        HKEY_LOCAL_MACHINE,
                        arg,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkResult,
                        NULL);

                if (openRes == ERROR_SUCCESS) {
                    printf("[server]Success creating key.");
                    strcpy(sendbuf, "ok");
                    RegCloseKey(hkResult);
                } else {
                    printf("[server]Could not create key! error no. %lu\n", GetLastError());
                    strcpy(sendbuf, "nok");
                }
            } else if (0 == strcmp(arg, "HKEY_USERS")) {
                HKEY hkResult;
                arg = strtok(NULL, " ");
                LONG openRes = RegCreateKeyEx(
                        HKEY_USERS,
                        arg,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkResult,
                        NULL);

                if (openRes == ERROR_SUCCESS) {
                    printf("[server]Success creating key.");
                    strcpy(sendbuf, "ok");
                    RegCloseKey(hkResult);
                } else {
                    printf("[server]Could not create key! error no. %lu\n", GetLastError());
                    strcpy(sendbuf, "nok");
                }
            }
        } else if (0 == strncmp(recvbuf, "deletekey", strlen("deletekey"))) {
            char *big_key = strdup(arg);
            arg = strtok(NULL, " ");
            if (0 == strcmp(big_key, "HKEY_CLASSES_ROOT")) {
                if (TRUE == RegDelnode(HKEY_CLASSES_ROOT, arg)) {
                    printf("[server]Success deleting key.");
                    strcpy(sendbuf, "ok");
                } else {
                    printf("[server]Could not delete key! error no. %lu\n", GetLastError());
                    strcpy(sendbuf, "nok");
                }
            } else if (0 == strcmp(big_key, "HKEY_CURRENT_CONFIG")) {
                if (TRUE == RegDelnode(HKEY_CURRENT_CONFIG, arg)) {
                    printf("[server]Success deleting key.");
                    strcpy(sendbuf, "ok");
                } else {
                    printf("[server]Could not delete key! error no. %lu\n", GetLastError());
                    strcpy(sendbuf, "nok");
                }
            } else if (0 == strcmp(big_key, "HKEY_CURRENT_USER")) {
                if (TRUE == RegDelnode(HKEY_CURRENT_USER, arg)) {
                    printf("[server]Success deleting key.");
                    strcpy(sendbuf, "ok");
                } else {
                    printf("[server]Could not delete key! error no. %lu\n", GetLastError());
                    strcpy(sendbuf, "nok");
                }
            } else if (0 == strcmp(big_key, "HKEY_LOCAL_MACHINE")) {
                if (TRUE == RegDelnode(HKEY_LOCAL_MACHINE, arg)) {
                    printf("[server]Success deleting key.");
                    strcpy(sendbuf, "ok");
                } else {
                    printf("[server]Could not delete key! error no. %lu\n", GetLastError());
                    strcpy(sendbuf, "nok");
                }
            } else if (0 == strcmp(big_key, "HKEY_USERS")) {
                if (TRUE == RegDelnode(HKEY_USERS, arg)) {
                    printf("[server]Success deleting key.");
                    strcpy(sendbuf, "ok");
                } else {
                    printf("[server]Could not delete key! error no. %lu\n", GetLastError());
                    strcpy(sendbuf, "nok");
                }
            }
        } else if (0 == strncmp(recvbuf, "download", strlen("download"))) {
            char *url = strdup(arg);
            arg = strtok(NULL, " ");

            HINTERNET inet_url;
            HINTERNET open;
            open = InternetOpen("DownloadFile",
                                INTERNET_OPEN_TYPE_PRECONFIG,
                                NULL,
                                NULL,
                                0);
            if (open == NULL) {
                printf("[server] InternetOpen failed!\n");
                strcpy(sendbuf, "nok");
            } else {
                printf("[server] InternetOpen success!\n");

            }
            inet_url = InternetOpenUrl(open,
                                       url,
                                       NULL,
                                       0,
                                       0,
                                       0);

            if (inet_url == NULL) {
                printf("[server] InternetOpenUrl failed!\n");
                strcpy(sendbuf, "nok");
            } else {
                printf("[server] InternetOpenUrl success!\n");
            }

            FILE *saved;
            unsigned long buffer;
            char name[128];
            saved = fopen(arg, "wb");

            if (saved == NULL) {
                printf("[server] fopen() failed!\n");
                strcpy(sendbuf, "nok");
            } else {
                printf("[server] fopen() success!\n");

            }

            while (InternetReadFile(inet_url, name, 1, &buffer) &&
                   buffer != 0) { //read byte by byte --very important!!!!
                fwrite(name, sizeof(char), buffer, saved);
                name[buffer] = '\0';
            }
            fclose(saved);
            strcpy(sendbuf, "ok");
        } else if (0 == strncmp(recvbuf, "listdir", strlen("listdir"))) {
            GetDir(arg);
            if (strlen(cDirTree) == 0) {
                printf("[server]Error processing directory!\n");
                strcpy(sendbuf, "nok");
            } else {
                printf("[server]Success processing directory!\n");
                if (sendto(s, cDirTree, strlen(cDirTree), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
                    printf("[error]sendto() failed with error code : %d", WSAGetLastError());
                    break;
                }
                printf("[server]Successfully sent directory tree to client!\n");
                strcpy(sendbuf, "ok");
            }
        } else if (0 == strncmp(recvbuf, "run", strlen("run"))) {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            BOOL bCreatRes = CreateProcess(TEXT(arg),
                                            NULL,
                                            NULL,
                                            NULL,
                                            FALSE,
                                            0,
                                            NULL,
                                            NULL,
                                            &si,
                                            &pi);
            if (bCreatRes == FALSE) {
                printf("[server]Failed to create Process! error no: %lu\n", GetLastError());
                strcpy(sendbuf, "nok");
            } else {
                printf("[server]Created process successfully!\n");
                strcpy(sendbuf, "ok");
            }
        } else if (NULL != strstr(recvbuf, "close_server")) {
            break;
        }
        if (sendto(s, sendbuf, strlen(sendbuf), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
            printf("[error]sendto() failed with error code : %d", WSAGetLastError());
            break;
        }
    }

    free(recvbuf);
    return 0;
}

BOOL GetDir(LPTSTR path) //print content of direcotry recursively
{
    struct dirent *de;  // Pointer for directory entry
    strcpy(cDirTree, "");
    // opendir() returns a pointer of DIR type.
    DIR *dr = opendir(path);

    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("[error]Could not open directory");
        return FALSE;
    }

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    while ((de = readdir(dr)) != NULL) {
        strcat(cDirTree, de->d_name);
        strcat(cDirTree, "\n");
    }
    cDirTree[4 * BUFLEN - 1] = '\0';
    closedir(dr);
    return TRUE;
}

#pragma clang diagnostic pop
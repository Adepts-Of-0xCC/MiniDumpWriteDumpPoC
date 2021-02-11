#define _CRT_SECURE_NO_WARNINGS
#include "covert.h"

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>



SOCKET singleton_socket;

SOCKET getRawSocket(const char* host, u_short port)
{
    WSADATA wsa_data;
    WORD req_version = MAKEWORD(2, 2);
    
    if (!singleton_socket) {
        if (0 != WSAStartup(req_version, &wsa_data)) {
            printf("[!] ERR:: Socket init failed. ERR_CODE %d\n", WSAGetLastError());

            singleton_socket = NULL;
        }
        else {
            singleton_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (singleton_socket == INVALID_SOCKET) {
                printf("[!] ERR:: Socket creation failed. ERR_CODE %d\n", WSAGetLastError());

                singleton_socket = NULL;
            }
            else {
                struct sockaddr_in server_config;
                unsigned long addr = inet_addr(host);
                char* ip;

                if (INADDR_NONE != addr) {
                    server_config.sin_addr.s_addr = addr;
                }
                else {
                    ip = inet_ntoa(*(struct in_addr*)gethostbyname(host)->h_addr_list[0]);
                    server_config.sin_addr.s_addr = inet_addr(ip);
                }
                
                server_config.sin_family = AF_INET;
                server_config.sin_port = htons(port);

                if (connect(singleton_socket, (struct sockaddr*)&server_config, sizeof(server_config)) < 0) {
                    printf("[!] ERR:: Socket connection failed. ERR_CODE %d\n", WSAGetLastError());

                    singleton_socket = NULL;
                }
                else {
                    printf("[i] Connection stablished with %s:%i\n", host, port);
                    printf("Sending...\n");
                }
            }
        }
    }
    
    return singleton_socket;
}

int sendBytesRaw(SOCKET s, const char* buffer, int buffer_size, long pos)
{
    int sent_bytes = 0;
    int tried = 0;
    int real_size;
    char* full_buffer = prepareBuffer(buffer, buffer_size, pos, &real_size);

    sent_bytes = send(s, full_buffer, real_size, 0);

    while (sent_bytes < 0 && tried < MAX_RETRIES) {
        printf("[!] ERR:: Could not send %iB. Retrying... %i\n", buffer_size, tried);
        sent_bytes = send(s, full_buffer, real_size, 0);
        ++tried;
    }

    free(full_buffer);
    return sent_bytes;
}

void closeSocket() {
    closesocket(singleton_socket);
}


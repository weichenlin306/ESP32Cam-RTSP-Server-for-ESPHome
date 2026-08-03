#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef int SOCKET;
typedef int UDPSOCKET;
typedef struct in_addr IPADDRESS;
typedef uint16_t IPPORT;

#define NULLSOCKET (-1)

inline uint32_t getRandom() {
    return esp_random();
}

inline void socketpeeraddr(SOCKET s, IPADDRESS *addr, IPPORT *port) {
    struct sockaddr_in remote_addr;
    socklen_t addr_len = sizeof(remote_addr);
    if (s >= 0 && getpeername(s, (struct sockaddr *)&remote_addr, &addr_len) == 0) {
        *addr = remote_addr.sin_addr;
        *port = ntohs(remote_addr.sin_port);
    } else {
        memset(addr, 0, sizeof(*addr));
        *port = 0;
    }
}

inline void udpsocketclose(UDPSOCKET s) {
    if (s >= 0) {
        close(s);
    }
}

inline UDPSOCKET udpsocketcreate(unsigned short portNum) {
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) return -1;

    int reuse = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portNum);

    if (bind(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(s);
        return -1;
    }

    return s;
}

inline ssize_t socketsend(SOCKET sockfd, const void *buf, size_t len) {
    if (sockfd < 0) return -1;
    const char *ptr = (const char *)buf;
    size_t remaining = len;
    int retries = 0;

    while (remaining > 0) {
        ssize_t sent = send(sockfd, ptr, remaining, 0);
        if (sent > 0) {
            ptr += sent;
            remaining -= sent;
            retries = 0;
        } else if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                vTaskDelay(1);
                retries++;
                if (retries > 100) {
                    return -1;
                }
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }
    return len;
}

inline ssize_t udpsocketsend(UDPSOCKET sockfd, const void *buf, size_t len,
                             IPADDRESS destaddr, IPPORT destport) {
    if (sockfd < 0) return -1;
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr = destaddr;
    dest.sin_port = htons(destport);

    return sendto(sockfd, buf, len, 0, (struct sockaddr *)&dest, sizeof(dest));
}

inline int socketread(SOCKET sock, char *buf, size_t buflen, int timeoutmsec) {
    if (sock < 0) return 0;
    ssize_t numRead = recv(sock, buf, buflen, 0);
    if (numRead > 0) return numRead;
    if (numRead == 0) return 0; // Socket closed
    if (errno == EAGAIN || errno == EWOULDBLOCK) return -1; // Timeout / Would block
    return 0; // Error / closed
}

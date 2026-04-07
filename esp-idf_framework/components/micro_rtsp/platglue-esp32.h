#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define network types
typedef int SOCKET;
typedef int UDPSOCKET;
typedef uint32_t IPADDRESS; // network byte order
typedef uint16_t IPPORT; // host byte order

#define NULLSOCKET -1

// lwip/sockets.h already provides static inline int closesocket(int s);
// We can just rely on standard close() or lwip's closesocket
inline void rtsp_closesocket(SOCKET s) {
    if(s >= 0) {
        close(s);
    }
}
#define closesocket rtsp_closesocket

#define getRandom() esphome::random_uint32()

inline void socketpeeraddr(SOCKET s, IPADDRESS *addr, IPPORT *port) {
    if (s < 0) return;
    struct sockaddr_in peer;
    socklen_t peer_len = sizeof(peer);
    if (getpeername(s, (struct sockaddr*)&peer, &peer_len) == 0) {
        *addr = peer.sin_addr.s_addr;
        *port = ntohs(peer.sin_port);
    }
}

inline void udpsocketclose(UDPSOCKET s) {
    if(s >= 0) {
        close(s);
    }
}

inline UDPSOCKET udpsocketcreate(unsigned short portNum)
{
    UDPSOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if(s < 0) {
        return NULLSOCKET;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(portNum);

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(s);
        return NULLSOCKET;
    }

    int flags = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);

    return s;
}

// TCP sending
inline ssize_t socketsend(SOCKET sockfd, const void *buf, size_t len)
{
    if (sockfd < 0) return -1;
    return send(sockfd, buf, len, 0);
}

inline ssize_t udpsocketsend(UDPSOCKET sockfd, const void *buf, size_t len,
                             IPADDRESS destaddr, IPPORT destport)
{
    if (sockfd < 0) return -1;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = destaddr;
    addr.sin_port = htons(destport);
    return sendto(sockfd, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
}

inline int socketread(SOCKET sock, char *buf, size_t buflen, int timeoutmsec)
{
    if (sock < 0) return 0;

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    uint32_t start = esphome::millis();
    while (true) {
        int numRead = recv(sock, buf, buflen, 0);
        if (numRead > 0) {
            return numRead;
        } else if (numRead == 0) {
            // Client closed connection
            return 0;
        } else {
            // Error or no data
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (timeoutmsec == 0) return -1; // No timeout, exit immediately
                if (esphome::millis() - start >= timeoutmsec) {
                    return -1; // Timeout reached
                }
                vTaskDelay(pdMS_TO_TICKS(10)); // Yield to other FreeRTOS tasks
            } else {
                return 0; // Other error, assume disconnected
            }
        }
    }
}

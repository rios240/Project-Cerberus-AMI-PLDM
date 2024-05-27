/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include "platform_api.h"
#include "cmd_channel_tcp.h"
*/

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>
#include "cmd_channel_tcp.h"
#include "platform_io.h"


void print_packet_data(const uint8_t *data, size_t len) {
    printf("Packet bytes:");
    for (size_t i = 0; i < len; ++i) {
        printf(" %02x", data[i]);
    }
    printf("\n");
}

/*
int set_socket_non_blocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        return CMD_CHANNEL_TCP_ERROR;
    }
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}
*/

/**
* Receive a command packet from a communication channel.  This call will block until a packet
* has been received or the timeout has expired.
*
* @param channel The channel to receive a packet from.
* @param packet Output for the packet data being received.
* @param ms_timeout The amount of time to wait for a received packet, in milliseconds.  A
* negative value will wait forever, and a value of 0 will return immediately.
*
* @return 0 if a packet was successfully received or an error code.
*
int receive_packet(struct cmd_channel *channel, struct cmd_packet *packet, int ms_timeout) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        return CMD_CHANNEL_TCP_ERROR;
    }

    if (set_socket_non_blocking(server_fd) < 0) {
        close(server_fd);
        return CMD_CHANNEL_TCP_ERROR;
    }
    
    // Forcefully attaching socket to the port 5000
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        return CMD_CHANNEL_TCP_ERROR;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);


    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        return CMD_CHANNEL_TCP_ERROR;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        return CMD_CHANNEL_TCP_ERROR;
    }

    struct timeval start, now;
    gettimeofday(&start, NULL);
    long elapsed_ms;

    do {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        gettimeofday(&now, NULL);
        elapsed_ms = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;
    } while (new_socket < 0 && errno == EWOULDBLOCK && elapsed_ms < ms_timeout);

    if (new_socket < 0) {
        close(server_fd);
        return CMD_CHANNEL_TCP_ERROR;
    }
    
    //platform_mutex_lock(&channel->lock);

    size_t valread = read(new_socket, packet->data, MCTP_BASE_PROTOCOL_MAX_PACKET_LEN);

    //platform_mutex_unlock(&channel->lock);

    packet->pkt_size = valread;
    packet->dest_addr = (uint8_t)cmd_channel_get_id(channel);

    close(new_socket);
    close(server_fd);

    return 0;
}*/



/**
* Send a command packet over a communication channel.
*
* Returning from this function does not guarantee the packet has been fully transmitted.
* Depending on the channel implementation, it is possible the packet is still in flight with
* the data buffered in the channel driver.
*
* @param channel The channel to send a packet on.
* @param packet The packet to send.
*
* @return 0 if the the packet was successfully sent or an error code.
*
int send_packet(struct cmd_channel *channel, struct cmd_packet *packet) {
    struct sockaddr_in serv_addr;
    int sock = 0;
    int ms_timeout = 60000;

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        return CMD_CHANNEL_TCP_ERROR;
    }

    struct timeval start, now;
    gettimeofday(&start, NULL);
    long elapsed_ms = 0;
    int connected = 0;

    while (elapsed_ms < ms_timeout && !connected) {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            return CMD_CHANNEL_TCP_ERROR;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
            connected = 1;
        } else {
            close(sock);
        }

        gettimeofday(&now, NULL);
        elapsed_ms = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;
    }

    if (!connected) {
        return CMD_CHANNEL_TCP_ERROR;
    }

    //platform_mutex_lock(&channel->lock);

    send(sock, packet->data, packet->pkt_size, 0);

    //platform_mutex_unlock(&channel->lock);

    close(sock);

    return 0;
}*/

int setup_server_socket() {
    static pthread_mutex_t remove_socket_mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool socket_removed = false;

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        platform_printf("socket"NEWLINE);
        return CMD_CHANNEL_SOCKET_ERROR;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, CMD_CHANNEL_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    pthread_mutex_lock(&remove_socket_mutex);

    if (access(CMD_CHANNEL_SOCKET_PATH, F_OK) == 0) {
        pthread_mutex_lock(&remove_socket_mutex);
        if (!socket_removed) {
            platform_printf("Removing existing socket file" NEWLINE);
            if (unlink(CMD_CHANNEL_SOCKET_PATH) < 0) {
                platform_printf("unlink" NEWLINE);
                close(sockfd);
                pthread_mutex_unlock(&remove_socket_mutex);
                return CMD_CHANNEL_SOCKET_ERROR;
            }
            socket_removed = true;
        }
        pthread_mutex_unlock(&remove_socket_mutex);
    }

    pthread_mutex_unlock(&remove_socket_mutex);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        platform_printf("bind"NEWLINE);
        close(sockfd);
        return CMD_CHANNEL_SOCKET_ERROR;
    }

    if (listen(sockfd, 5) < 0) {
        platform_printf("listen"NEWLINE);
        close(sockfd);
        return CMD_CHANNEL_SOCKET_ERROR;
    }

    return sockfd;
}

int receive_packet(struct cmd_channel *channel, struct cmd_packet *packet, int ms_timeout) {
    int sockfd = setup_server_socket();
    if (sockfd < 0) {
        return sockfd;
    }

    int connfd = accept(sockfd, NULL, NULL);
    if (connfd < 0) {
        platform_printf("accept"NEWLINE);
        close(sockfd);
        return CMD_CHANNEL_SOCKET_ERROR;
    }

    struct timeval timeout;
    timeout.tv_sec = ms_timeout / 1000;
    timeout.tv_usec = (ms_timeout % 1000) * 1000;

    setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    ssize_t received = recv(connfd, packet->data, CMD_MAX_PACKET_SIZE, 0);

    packet->pkt_size = received;
    packet->dest_addr = (uint8_t)cmd_channel_get_id(channel);

    close(connfd);
    close(sockfd);
    return (size_t) received == packet->pkt_size && received != -1 ? 0 : CMD_CHANNEL_SOCKET_ERROR;
}

int send_packet(struct cmd_channel *channel, struct cmd_packet *packet) {
    int sockfd;
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, CMD_CHANNEL_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    while (1) {
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd < 0) {
            platform_printf("socket"NEWLINE);
            return CMD_CHANNEL_SOCKET_ERROR;
        }

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            close(sockfd);
            sleep(1);
            continue;
        }

        break;
    }

    ssize_t sent = send(sockfd, packet->data, packet->pkt_size, 0);

    close(sockfd);

    return (size_t) sent == packet->pkt_size && sent != -1 ? 0 : CMD_CHANNEL_SOCKET_ERROR;
}
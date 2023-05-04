#include <iostream>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int create_userland_socket(const std::string& ip, uint16_t port) {
    // Create a raw socket
    int raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (raw_sock == -1) {
        std::cerr << "Failed to create raw socket" << std::endl;
        return -1;
    }

    // Bind the raw socket to the specified IP and port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (bind(raw_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        std::cerr << "Failed to bind raw socket" << std::endl;
        close(raw_sock);
        return -1;
    }

    // TODO Start a thread to handle arps...

    // Manually handle the TCP handshake and data exchange using the raw socket
    // ...

    return raw_sock;
}

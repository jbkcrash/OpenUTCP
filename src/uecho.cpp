#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "ucommon.hpp"
#include "usockets.hpp"

class EchoClient
{
private:
    int sock_;
    sockaddr_in server_addr_;

public:
    EchoClient() : sock_(-1) {}
    ~EchoClient() { disconnect(); }

    bool connect(const std::string &ip_addr, uint16_t port)
    {
        // Check if already connected
        if (sock_ != -1)
        {
            std::cerr << "EchoClient already connected" << std::endl;
            return false;
        }

        // Create socket
        sock_ = create_userland_socket(ip_addr, port);
        if (sock_ == -1)
        {
            std::cerr << "Failed to create userland socket" << std::endl;
            return false;
        }

        // Must call listen() on the socket before entering our receive loop
        if (listen(sock_, 1) == -1)
        {
            std::cerr << "Failed to listen on userland socket" << std::endl;
            disconnect();
            return false;
        }

        // Set server address
        server_addr_.sin_family = AF_INET;
        server_addr_.sin_port = htons(port);
        if (inet_pton(AF_INET, ip_addr.c_str(), &server_addr_.sin_addr) <= 0)
        {
            std::cerr << "Invalid IP address: " << ip_addr << std::endl;
            disconnect();
            return false;
        }

        // Connect to server
        if (::connect(sock_, (struct sockaddr *)&server_addr_, sizeof(server_addr_)) < 0)
        {
            std::cerr << "Failed to connect to server: " << strerror(errno) << std::endl;
            disconnect();
            return false;
        }

        return true;
    }

    void disconnect()
    {
        if (sock_ != -1)
        {
            close(sock_);
            sock_ = -1;
        }
    }

    bool send_ping()
    {
        if (sock_ == -1)
        {
            std::cerr << "EchoClient not connected" << std::endl;
            return false;
        }

        std::string message = "{\"command\": \"PING\"}";
        int message_len = message.length();

        if (send(sock_, message.c_str(), message_len, 0) != message_len)
        {
            std::cerr << "Failed to send PING message: " << strerror(errno) << std::endl;
            disconnect();
            return false;
        }

        return true;
    }

    std::string receive_message()
    {
        if (sock_ == -1)
        {
            std::cerr << "EchoClient not connected" << std::endl;
            return "";
        }

        // Accept the incoming connection from the server
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(sock_, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock == -1)
        {
            std::cerr << "Failed to accept incoming connection: " << strerror(errno) << std::endl;
            disconnect();
            return "";
        }

        // Disable the Nagle algorithm for better performance over a loopback interface
        int flag = 1;
        int result = setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
        if (result == -1)
        {
            std::cerr << "Failed to disable Nagle algorithm: " << strerror(errno) << std::endl;
            disconnect();
            return "";
        }

        // Receive the message from the server
        char buffer[4096];
        int recv_len = recv(client_sock, buffer, sizeof(buffer), 0);

        if (recv_len < 0)
        {
            std::cerr << "Failed to receive data: " << strerror(errno) << std::endl;
            disconnect();
            return "";
        }
        else if (recv_len == 0)
        {
            std::cerr << "Connection closed by server" << std::endl;
            disconnect();
            return "";
        }

        return std::string(buffer, recv_len);
    }
};
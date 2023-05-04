#ifndef ECHOCLIENT_HPP
#define ECHOCLIENT_HPP

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>

class EchoClient
{
    private:
    int sock_;
    sockaddr_in server_addr_;

public:
    EchoClient();
    ~EchoClient();
    bool connect(const std::string &ip_addr, uint16_t port);
    void disconnect();
    bool send_ping();
    std::string receive_message();
};

#endif // ECHOCLIENT_HPP

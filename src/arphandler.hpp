#pragma once

#include <iostream>
#include <cstring>
#include <iomanip>
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

class ArpHandler
{
private:
    std::string iface_;
    std::string virtual_ip_;
    int m_arp_sock;
    struct sockaddr_ll sll;
    struct ifreq ifr;
    unsigned char *mac = NULL;
    void create_socket();
    void shutdown();
public:
    bool m_Active = false; // Used by FSM to tell the arphandler whether to act or simply prepare to be act in the future.
    ArpHandler(const std::string &iface, const std::string &virtual_ip)
        : iface_(iface), virtual_ip_(virtual_ip){create_socket();};
    ~ArpHandler();
    void decode_arp_reply(const unsigned char* buffer, ssize_t length);
    void decode_arp_request(unsigned char* buffer, ssize_t length);
    void handle_arp();
};


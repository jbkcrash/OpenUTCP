#pragma once

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

class uICMP {
public:
    uICMP(const std::string &iface, const std::string &virtual_ip)
        : iface_(iface), virtual_ip_(virtual_ip){create_socket();};
    ~uICMP();
    void handle_icmp();

    void send_echo_request(const std::string &dest_ip);
    void receive_echo_reply();
private:
    int m_icmp_sock;
    std::string iface_;
    std::string virtual_ip_;
    struct sockaddr_ll sll;
    struct ifreq ifr;
    void shutdown();
    void create_socket();
    void send_echo_reply(const std::string &dest_ip, uint16_t id, uint16_t seq, struct ether_header *eth_header, struct ip *ip_header);
};


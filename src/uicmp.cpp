#include <cstring>
#include <iomanip>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <iostream>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <cstring>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/ether.h>
#include "ucommon.hpp"
#include "uicmp.hpp"

uICMP::~uICMP()
{
    shutdown();
}

void uICMP::shutdown()
{
    if (m_icmp_sock >= 0)
    {
        close(m_icmp_sock);
    }
}

void uICMP::create_socket()
{
    // Create a raw socket for ICMP
    m_icmp_sock = socket(AF_PACKET, SOCK_RAW, IPPROTO_ICMP);
    if (m_icmp_sock < 0)
    {
        std::cerr << "Failed to create raw ICMP socket" << std::endl;
    }

    memset(&sll, 0, sizeof(sll));
    memset(&ifr, 0, sizeof(ifr));

    strncpy(ifr.ifr_name, iface_.c_str(), IFNAMSIZ);

    if (ioctl(m_icmp_sock, SIOCGIFINDEX, &ifr) == -1)
    {
        std::cerr << "Failed to get interface index" << std::endl;
        close(m_icmp_sock);
        return;
    }

    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_IP);

        if (bind(m_icmp_sock, (struct sockaddr *)&sll, sizeof(sll)) == -1)
    {
        std::cerr << "Failed to bind raw socket: " << strerror(errno) << std::endl;
        close(m_icmp_sock);
        return;
    }
}

void uICMP::send_echo_request(const std::string &dest_ip)
{
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    inet_pton(AF_INET, dest_ip.c_str(), &(dest_addr.sin_addr));

    // Create an ICMP header
    struct icmphdr icmp_header;
    icmp_header.type = ICMP_ECHO;
    icmp_header.code = 0;
    icmp_header.un.echo.id = htons(getpid() & 0xFFFF);
    icmp_header.un.echo.sequence = htons(1);
    icmp_header.checksum = 0;
    icmp_header.checksum = csum((unsigned short *)&icmp_header, sizeof(icmp_header) / 2);

    // Send the echo request
    if (sendto(m_icmp_sock, &icmp_header, sizeof(icmp_header), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        std::cerr << "Failed to send ICMP echo request" << std::endl;
    }
}

void uICMP::send_echo_reply(const std::string &dest_ip, uint16_t id, uint16_t seq, struct ether_header *eth_header, struct ip *ip_header) {
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    inet_pton(AF_INET, dest_ip.c_str(), &(dest_addr.sin_addr));

    //Add proper ethernet and IP headers
    // Create an ethernet header
    //ruct ether_header eth_header;
    //memset(eth_header.ether_dhost, 0xff, ETH_ALEN);
    //memset(eth_header.ether_shost, 0xff, ETH_ALEN);



    // Create an ICMP header
    struct icmphdr icmp_header;
    icmp_header.type = ICMP_ECHOREPLY;
    icmp_header.code = 0; // this is always zero for ICMP echo reply and request
    icmp_header.un.echo.id = htons(getpid() & 0xFFFF);
    icmp_header.un.echo.sequence = htons(1);
    icmp_header.checksum = 0;
    icmp_header.checksum = csum((unsigned short *)&icmp_header, sizeof(icmp_header) / 2);

    // Send the echo request
    if (sendto(m_icmp_sock, &icmp_header, sizeof(icmp_header), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        std::cerr << "Failed to send ICMP echo reply" << std::endl;
    }

}

/* void uICMP::receive_echo_reply()
{
    struct sockaddr_in recv_addr;
    socklen_t recv_addr_len = sizeof(recv_addr);
    char recv_buffer[4096];
    ssize_t recv_len = recvfrom(m_icmp_sock, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);

    if (recv_len < 0)
    {
        std::cerr << "Failed to receive ICMP echo reply" << std::endl;
        return;
    }

    struct ip *recv_ip_header = (struct ip *)recv_buffer;
    struct icmphdr *recv_icmp_header = (struct icmphdr *)(recv_buffer + recv_ip_header->ip_hl * 4);

    if (recv_icmp_header->type == ICMP_ECHOREPLY &&
        recv_icmp_header->un.echo.id == htons(getpid() & 0xFFFF))
    {
        std::cout << "Received ICMP echo reply from " << inet_ntoa(recv_addr.sin_addr) << std::endl;
    }
} */

// Used to handle ICMP requests on behalf of the virtual IP
void uICMP::handle_icmp()
{
    // Listen for ICMP packets and reply if necessary
    while (true)
    {

        unsigned char buffer[2048];
        unsigned char payload[2048];
        ssize_t length = recvfrom(m_icmp_sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
        in_addr_t in_addr_vip = inet_addr(virtual_ip_.c_str());

        if (length == -1)
        {
            std::cerr << "Failed to receive ICMP packet" << std::endl;
            continue;
        }

        struct ether_header *eth_header = (struct ether_header *)buffer;
        struct ip *recv_ip_header = (struct ip *)(buffer + sizeof(struct ether_header));

        // Check to see if the packet is an ICMP echo request
        if (recv_ip_header->ip_p != IPPROTO_ICMP)
        {
            continue;
        }

        // Check to see if our virtual IP is the destination
        if (recv_ip_header->ip_dst.s_addr != in_addr_vip)
        {
            continue;
        }

        // Check the ICMP type
        struct icmp *recv_icmp_header = (struct icmp *)(buffer + sizeof(struct ether_header) + recv_ip_header->ip_hl * 4);
        if (recv_icmp_header->icmp_type != ICMP_ECHO)
        {
            continue;
        }
        // Get the ID
        uint16_t id = ntohs(recv_icmp_header->icmp_id);

        // Get the sequence
        uint16_t sequence = ntohs(recv_icmp_header->icmp_seq);

        // Get the source IP Address
        char source_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(recv_ip_header->ip_src), source_ip, INET_ADDRSTRLEN);
                       
        // Get the payload, which is the rest of the packet
        memcpy(payload, buffer + sizeof(struct ether_header) + recv_ip_header->ip_hl * 4 + sizeof(struct icmphdr), length - sizeof(struct ether_header) - recv_ip_header->ip_hl * 4 - sizeof(struct icmphdr));

        // print out the payload
        std::cout << "Payload: " << payload << std::endl;

        // Send the echo reply
        send_echo_reply(source_ip, id, sequence, eth_header, recv_ip_header);

    }

        // Parse Ethernet header
// unsigned char *ptr;
// ptr = eth_header->ether_shost;
// printf("Source MAC: %02x:%02x:%02x:%02x:%02x:%02x
// ", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
// ptr = eth_header->ether_dhost;
// printf("Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x
// ", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);

// // Move past the Ethernet header
// unsigned int header_size = sizeof(struct ether_header);
// ptr = buffer + header_size;

// // Parse IP header
// struct iphdr *ip_header = (struct iphdr *)ptr;
// printf("Source IP: %s
// ", inet_ntoa(*(struct in_addr *)&ip_header->saddr));
// printf("Destination IP: %s
// ", inet_ntoa(*(struct in_addr *)&ip_header->daddr));

// // Move past the IP header
// header_size = ip_header->ihl * 4; // IP header length is in 4-byte words
// ptr += header_size;

// // Parse ICMP header
// struct icmphdr *icmp_header = (struct icmphdr *)ptr;
// printf("ICMP type: %d
// ", icmp_header->type);
// printf("ICMP code: %d
// ", icmp_header->code);

//     }
/*     fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(m_icmp_sock, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int ret = select(m_icmp_sock + 1, &read_fds, NULL, NULL, &timeout);
    if (ret < 0)
    {
        std::cerr << "Failed to select ICMP socket" << std::endl;
        return;
    }
    else if (ret == 0)
    {
        return;
    }

    if (FD_ISSET(m_icmp_sock, &read_fds))
    {
        receive_echo_reply();
    } */
}

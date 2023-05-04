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
#include <netinet/ether.h>
#include "ucommon.hpp"
#include "arphandler.hpp"

ArpHandler::~ArpHandler(){
    shutdown();
}

void ArpHandler::decode_arp_reply(const unsigned char *buffer, ssize_t length)
{
    struct ether_header *eth = (struct ether_header *)buffer;
    struct ether_arp *arp = (struct ether_arp *)(buffer + sizeof(struct ether_header));

    std::cout << "ARP Reply:" << std::endl;
    std::cout << "Ethernet Header:" << std::endl;
    std::cout << "Source MAC: " << ether_ntoa((const struct ether_addr *)eth->ether_shost) << std::endl;
    std::cout << "Destination MAC: " << ether_ntoa((const struct ether_addr *)eth->ether_dhost) << std::endl;

    std::cout << "ARP Header:" << std::endl;
    std::cout << "Hardware Type: " << ntohs(arp->ea_hdr.ar_hrd) << std::endl;
    std::cout << "Protocol Type: " << ntohs(arp->ea_hdr.ar_pro) << std::endl;
    std::cout << "Hardware Address Length: " << (int)arp->ea_hdr.ar_hln << std::endl;
    std::cout << "Protocol Address Length: " << (int)arp->ea_hdr.ar_pln << std::endl;
    std::cout << "Operation Code: " << ntohs(arp->ea_hdr.ar_op) << std::endl;
    std::cout << "Sender MAC: " << ether_ntoa((const struct ether_addr *)arp->arp_sha) << std::endl;
    std::cout << "Sender IP: " << inet_ntoa(*(struct in_addr *)arp->arp_spa) << std::endl;
    std::cout << "Target MAC: " << ether_ntoa((const struct ether_addr *)arp->arp_tha) << std::endl;
    std::cout << "Target IP: " << inet_ntoa(*(struct in_addr *)arp->arp_tpa) << std::endl;
}

void ArpHandler::decode_arp_request(unsigned char *buffer, ssize_t length)
{
    std::cout << "ARP request:" << std::endl;

    struct ether_header *eth = (struct ether_header *)buffer;
    struct ether_arp *arp = (struct ether_arp *)(buffer + sizeof(struct ether_header));

    std::cout << "Ethernet Header:" << std::endl;
    std::cout << "Source MAC: " << ether_ntoa((const struct ether_addr *)eth->ether_shost) << std::endl;
    std::cout << "Destination MAC: " << ether_ntoa((const struct ether_addr *)eth->ether_dhost) << std::endl;

    std::cout << "ARP Header:" << std::endl;
    std::cout << "Hardware Type: " << ntohs(arp->ea_hdr.ar_hrd) << std::endl;
    std::cout << "Protocol Type: " << ntohs(arp->ea_hdr.ar_pro) << std::endl;
    std::cout << "Hardware Address Length: " << (int)arp->ea_hdr.ar_hln << std::endl;
    std::cout << "Protocol Address Length: " << (int)arp->ea_hdr.ar_pln << std::endl;
    std::cout << "Operation Code: " << ntohs(arp->ea_hdr.ar_op) << std::endl;
    std::cout << "Sender MAC: " << ether_ntoa((const struct ether_addr *)arp->arp_sha) << std::endl;
    std::cout << "Sender IP: " << inet_ntoa(*(struct in_addr *)arp->arp_spa) << std::endl;
    std::cout << "Target MAC: " << ether_ntoa((const struct ether_addr *)arp->arp_tha) << std::endl;
    std::cout << "Target IP: " << inet_ntoa(*(struct in_addr *)arp->arp_tpa) << std::endl;
}

void ArpHandler::create_socket()
{
    // Create raw socket for ARP
    m_arp_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));

    if (m_arp_sock == -1)
    {
        std::cerr << "Failed to create raw socket" << strerror(errno) << std::endl;
        return;
    }

    // Bind the socket to the specified interface

    memset(&sll, 0, sizeof(sll));
    memset(&ifr, 0, sizeof(ifr));

    strncpy(ifr.ifr_name, iface_.c_str(), IFNAMSIZ);

    if (ioctl(m_arp_sock, SIOCGIFINDEX, &ifr) == -1)
    {
        std::cerr << "Failed to get interface index" << std::endl;
        close(m_arp_sock);
        return;
    }

    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ARP);

    if (bind(m_arp_sock, (struct sockaddr *)&sll, sizeof(sll)) == -1)
    {
        std::cerr << "Failed to bind raw socket" << std::endl;
        close(m_arp_sock);
        return;
    }

    // Get the MAC Address of the selected IFACE
    // TODO make this a configuration item, e.g. the virtual_mac
    if (ioctl(m_arp_sock, SIOCGIFHWADDR, &ifr) == -1)
    {
        std::cerr << "Error getting MAC address" << std::endl;
        close(m_arp_sock);
        return;
    }

    mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
}
// Add a function to cleanly shutdown the ARP handler
// Add the appropriate code to the main function to call the shutdown function
// Add the appropriate defintion to the .hpp file

void ArpHandler::shutdown()
{
    close(m_arp_sock);
}

// Used to create a loop that listens for ARP requests for our Virtual IP, and responds with the matching virtual MAC.
void ArpHandler::handle_arp()
{

    // Listen for ARP requests and reply if necessary
    while (true)
    {

        unsigned char buffer[2048];
        ssize_t length = recvfrom(m_arp_sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
        in_addr_t in_addr_vip = inet_addr(virtual_ip_.c_str());

        if (length == -1)
        {
            std::cerr << "Failed to receive ARP data" << std::endl;
            continue;
        }

        struct ether_header *eth = (struct ether_header *)buffer;
        struct ether_arp *arp = (struct ether_arp *)(buffer + sizeof(struct ether_header));

        // Check if the packet is an ARP request for our virtual IP
        if (ntohs(eth->ether_type) == ETH_P_ARP &&
                ((ntohs(arp->ea_hdr.ar_op) == ARPOP_REQUEST && !memcmp(&arp->arp_tpa, &in_addr_vip, sizeof(in_addr))) ||
            (ntohs(arp->ea_hdr.ar_op) == ARPOP_REPLY && !memcmp(&arp->arp_spa, &in_addr_vip, sizeof(in_addr)))))
        {
            // q: explain this decision
            // a: we only want to respond to ARP requests, not replies
            if (ntohs(arp->ea_hdr.ar_op) == ARPOP_REQUEST)
            {
                // We have a request for our arp
                decode_arp_request(buffer, sizeof(buffer));

                // Build an ARP reply
                struct ether_header reply_eth = *eth;
                struct ether_arp reply_arp = *arp;

                memcpy(reply_eth.ether_dhost, eth->ether_shost, ETH_ALEN);
                memcpy(reply_eth.ether_shost, eth->ether_dhost, ETH_ALEN);

                reply_arp.ea_hdr.ar_op = htons(ARPOP_REPLY);
                memcpy(reply_arp.arp_tha, arp->arp_sha, ETH_ALEN);
                // memcpy(reply_arp.arp_sha, eth->ether_dhost, ETH_ALEN);
                memcpy(reply_arp.arp_sha, mac, ETH_ALEN);
                memcpy(reply_arp.arp_tpa, arp->arp_spa, sizeof(in_addr));
                memcpy(reply_arp.arp_spa, &arp->arp_tpa, sizeof(in_addr));

                // Combine the Ethernet and ARP headers into a single buffer
                unsigned char reply_buffer[sizeof(reply_eth) + sizeof(reply_arp)];
                memcpy(reply_buffer, &reply_eth, sizeof(reply_eth));
                memcpy(reply_buffer + sizeof(reply_eth), &reply_arp, sizeof(reply_arp));

                // Send the ARP reply
                decode_arp_reply(reply_buffer, sizeof(reply_buffer));
                ssize_t sent_length = sendto(m_arp_sock, reply_buffer, sizeof(reply_buffer), 0, (struct sockaddr *)&sll, sizeof(sll));

                if (sent_length == -1)
                {
                    std::cerr << "Failed to send ARP reply" << std::endl;
                    continue;
                }
            }
        }
    }

    close(m_arp_sock);
}
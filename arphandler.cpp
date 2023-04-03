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

class ArpHandler
{
public:
    ArpHandler(const std::string &iface, const std::string &virtual_ip)
        : iface_(iface), virtual_ip_(virtual_ip)
    {
    }

    void handle_arp()
    {
        // Create raw socket for ARP
        int arp_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
        if (arp_sock == -1)
        {
            std::cerr << "Failed to create raw socket" << std::endl;
            return;
        }

        // Bind the socket to the specified interface
        struct sockaddr_ll sll;
        struct ifreq ifr;

        memset(&sll, 0, sizeof(sll));
        memset(&ifr, 0, sizeof(ifr));

        strncpy(ifr.ifr_name, iface_.c_str(), IFNAMSIZ);

        if (ioctl(arp_sock, SIOCGIFINDEX, &ifr) == -1)
        {
            std::cerr << "Failed to get interface index" << std::endl;
            close(arp_sock);
            return;
        }

        sll.sll_family = AF_PACKET;
        sll.sll_ifindex = ifr.ifr_ifindex;
        sll.sll_protocol = htons(ETH_P_ARP);

        if (bind(arp_sock, (struct sockaddr *)&sll, sizeof(sll)) == -1)
        {
            std::cerr << "Failed to bind raw socket" << std::endl;
            close(arp_sock);
            return;
        }

        // Listen for ARP requests and reply if necessary
        while (true)
        {
            unsigned char buffer[2048];
            ssize_t length = recvfrom(arp_sock, buffer, sizeof(buffer), 0, nullptr, nullptr);

            if (length == -1)
            {
                std::cerr << "Failed to receive data" << std::endl;
                continue;
            }
            struct ether_header *eth = (struct ether_header *)buffer;
            struct ether_arp *arp = (struct ether_arp *)(buffer + sizeof(struct ether_header));

            // Check if the packet is an ARP request for the virtual IP
            if (ntohs(eth->ether_type) == ETH_P_ARP &&
                ntohs(arp->ea_hdr.ar_op) == ARPOP_REQUEST &&
                !memcmp(&arp->arp_tpa, inet_addr(virtual_ip_.c_str()), sizeof(in_addr)))
            {

                // Build an ARP reply
                struct ether_header reply_eth = *eth;
                struct ether_arp reply_arp = *arp;

                memcpy(reply_eth.ether_dhost, eth->ether_shost, ETH_ALEN);
                memcpy(reply_eth.ether_shost, eth->ether_dhost, ETH_ALEN);

                reply_arp.ea_hdr.ar_op = htons(ARPOP_REPLY);
                memcpy(reply_arp.arp_tha, arp->arp_sha, ETH_ALEN);
                memcpy(reply_arp.arp_sha, eth->ether_dhost, ETH_ALEN);
                memcpy(reply_arp.arp_tpa, arp->arp_spa, sizeof(in_addr));
                memcpy(reply_arp.arp_spa, &arp->arp_tpa, sizeof(in_addr));

                // Combine the Ethernet and ARP headers into a single buffer
                unsigned char reply_buffer[sizeof(reply_eth) + sizeof(reply_arp)];
                memcpy(reply_buffer, &reply_eth, sizeof(reply_eth));
                memcpy(reply_buffer + sizeof(reply_eth), &reply_arp, sizeof(reply_arp));

                // Send the ARP reply
                ssize_t sent_length = sendto(arp_sock, reply_buffer, sizeof(reply_buffer), 0, (struct sockaddr *)&sll, sizeof(sll));

                if (sent_length == -1)
                {
                    std::cerr << "Failed to send ARP reply" << std::endl;
                    continue;
                }
            }
        }

        close(arp_sock);
    }

private:
    std::string iface_;
    std::string virtual_ip_;
};

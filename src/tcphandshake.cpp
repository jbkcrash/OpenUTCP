#include <iostream>
#include <cstdlib>
#include <cstring>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

void outgoing_handle_tcp_handshake(int raw_sock, const std::string &dest_ip, uint16_t dest_port)
{
    // Create an IP header
    struct ip ip_header;
    memset(&ip_header, 0, sizeof(ip_header));

    ip_header.ip_hl = 5;
    ip_header.ip_v = 4;
    ip_header.ip_tos = 0;
    ip_header.ip_len = sizeof(struct ip) + sizeof(struct tcphdr);
    ip_header.ip_id = htonl(54321);
    ip_header.ip_off = 0;
    ip_header.ip_ttl = 255;
    ip_header.ip_p = IPPROTO_TCP;
    ip_header.ip_sum = 0;
    ip_header.ip_src.s_addr = inet_addr("0.0.0.0");
    ip_header.ip_dst.s_addr = inet_addr(dest_ip.c_str());

    // Create a TCP header
    struct tcphdr tcp_header;
    memset(&tcp_header, 0, sizeof(tcp_header));

    tcp_header.source = htons(rand() % 65535);
    tcp_header.dest = htons(dest_port);
    tcp_header.seq = htonl(1);
    tcp_header.ack_seq = 0;
    tcp_header.res1 = 0;
    tcp_header.doff = 5;
    tcp_header.fin = 0;
    tcp_header.syn = 1;
    tcp_header.rst = 0;
    tcp_header.psh = 0;
    tcp_header.ack = 0;
    tcp_header.urg = 0;
    tcp_header.res2 = 0;
    tcp_header.window = htons(65535);
    tcp_header.check = 0;
    tcp_header.urg_ptr = 0;

    // Calculate the TCP checksum
    struct pseudo_header
    {
        u_int32_t source_address;
        u_int32_t dest_address;
        u_int8_t placeholder;
        u_int8_t protocol;
        u_int16_t tcp_length;
    } pseudo_header;

    pseudo_header.source_address = inet_addr("0.0.0.0");
    pseudo_header.dest_address = ip_header.ip_dst.s_addr;
    pseudo_header.placeholder = 0;
    pseudo_header.protocol = IPPROTO_TCP;
    pseudo_header.tcp_length = htons(sizeof(struct tcphdr));

    int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
    unsigned char *pseudogram = new unsigned char[psize];

    memcpy(pseudogram, (char *)&pseudo_header, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), &tcp_header, sizeof(struct tcphdr));

    tcp_header.check = csum((unsigned short *)pseudogram, psize / 2);

    // Send the SYN packet
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(dest_port);
    sin.sin_addr.s_addr = ip_header.ip_dst.s_addr;

    int one = 1;
    const int *val = &one;
    setsockopt(raw_sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one));

    unsigned char buffer[sizeof(struct ip) + sizeof(struct tcphdr)];
    memcpy(buffer, &ip_header, sizeof(struct ip));
    memcpy(buffer + sizeof(struct ip), &tcp_header, sizeof(struct tcphdr));

    if (sendto(raw_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        std::cerr << "Failed to send SYN packet" << std::endl;
        delete[] pseudogram;
        return;
    }

    // Wait for the SYN-ACK packet
    while (true)
    {
        struct sockaddr_in recv_addr;
        socklen_t recv_addr_len = sizeof(recv_addr);
        char recv_buffer[4096];
        ssize_t recv_len = recvfrom(raw_sock, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);

        if (recv_len < 0)
        {
            std::cerr << "Failed to receive SYN-ACK packet" << std::endl;
            delete[] pseudogram;
            return;
        }

        struct ip *recv_ip_header = (struct ip *)recv_buffer;
        struct tcphdr *recv_tcp_header = (struct tcphdr *)(recv_buffer + recv_ip_header->ip_hl * 4);

        if (recv_tcp_header->syn == 1 && recv_tcp_header->ack == 1 && ntohs(recv_tcp_header->source) == dest_port)
        {
            // Send the ACK packet
            tcp_header.seq = htonl(ntohl(recv_tcp_header->ack_seq));
            tcp_header.ack_seq = htonl(ntohl(recv_tcp_header->seq) + 1);
            tcp_header.syn = 0;
            tcp_header.ack = 1;

            memcpy(pseudogram + sizeof(struct pseudo_header), &tcp_header, sizeof(struct tcphdr));
            tcp_header.check = 0;
            tcp_header.check = csum((unsigned short *)pseudogram, psize / 2);

            memcpy(buffer + sizeof(struct ip), &tcp_header, sizeof(struct tcphdr));

            if (sendto(raw_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
            {
                std::cerr << "Failed to send ACK packet" << std::endl;
            }
            else
            {
                std::cout << "TCP handshake completed" << std::endl;
            }

            break;
        }
    }

    delete[] pseudogram;
}

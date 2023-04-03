#include <iostream>
#include <cstdlib>
#include <cstring>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include "ucommon.hpp"

void handle_incoming_connection(int raw_sock, const struct ip *recv_ip_header, const struct tcphdr *recv_tcp_header)
{
    // Create an IP header
    struct ip ip_header;
    memset(&ip_header, 0, sizeof(ip_header));
    // ...

    // Create a TCP header
    struct tcphdr tcp_header;
    memset(&tcp_header, 0, sizeof(tcp_header));
    // ...

    // Set the initial sequence and acknowledgment numbers
    tcp_header.seq = htonl(rand());
    tcp_header.ack_seq = htonl(ntohl(recv_tcp_header->seq) + 1);

    // Set the SYN and ACK flags
    tcp_header.syn = 1;
    tcp_header.ack = 1;

    // Calculate the TCP checksum
    // ...

    // Send the SYN-ACK packet
    // ...

    // Wait for the ACK packet
    while (true)
    {
        struct sockaddr_in recv_addr;
        socklen_t recv_addr_len = sizeof(recv_addr);
        char recv_buffer[4096];
        ssize_t recv_len = recvfrom(raw_sock, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);

        if (recv_len < 0)
        {
            std::cerr << "Failed to receive ACK packet" << std::endl;
            return;
        }

        struct ip *ack_ip_header = (struct ip *)recv_buffer;
        struct tcphdr *ack_tcp_header = (struct tcphdr *)(recv_buffer + ack_ip_header->ip_hl * 4);

        if (ack_tcp_header->ack == 1 &&
            ack_ip_header->ip_src.s_addr == recv_ip_header->ip_src.s_addr &&
            ntohs(ack_tcp_header->source) == ntohs(recv_tcp_header->source) &&
            ntohs(ack_tcp_header->dest) == ntohs(recv_tcp_header->dest) &&
            ntohl(ack_tcp_header->ack_seq) == ntohl(tcp_header.seq) + 1)
        {
            std::cout << "TCP handshake completed for incoming connection" << std::endl;
            break;
        }
    }

    // Perform a PING/PONG exchange and then close the connection
    EchoClient client;
    client.connect(inet_ntoa(recv_ip_header->ip_src), ntohs(recv_tcp_header->source));

    // Send a PING message to the client
    client.send_ping();

    while (true)
    {
        std::string response = client.receive_message();

        if (response.empty())
        {
            std::cerr << "Failed to receive PONG message" << std::endl;
            break;
        }

        if (response == "PONG")
        {
            std::cout << \"Received PONG message" << std::endl;
            break;
        }
    }

    // Disconnect the client
    client.disconnect();
}

void accept_inbound_connections(int raw_sock, const std::string &local_ip, uint16_t local_port)
{
    while (true)
    {
        struct sockaddr_in recv_addr;
        socklen_t recv_addr_len = sizeof(recv_addr);
        char recv_buffer[4096];
        ssize_t recv_len = recvfrom(raw_sock, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);

        if (recv_len < 0)
        {
            std::cerr << "Failed to receive packet" << std::endl;
            continue;
        }

        struct ip *recv_ip_header = (struct ip *)recv_buffer;
        struct tcphdr *recv_tcp_header = (struct tcphdr *)(recv_buffer + recv_ip_header->ip_hl * 4);

        // Check if the packet is an incoming SYN packet for the bound socket
        if (recv_tcp_header->syn == 1 && recv_tcp_header->ack == 0 &&
            recv_ip_header->ip_dst.s_addr == inet_addr(local_ip.c_str()) &&
            ntohs(recv_tcp_header->dest) == local_port)
        {
            std::cout << \"Incoming connection from " << inet_ntoa(recv_addr.sin_addr) << ":" << ntohs(recv_tcp_header->source) << std::endl;

            // Handle the incoming connection in a separate thread
            std::thread connection_thread(handle_incoming_connection, raw_sock, recv_ip_header, recv_tcp_header);
            connection_thread.detach();
        }

        // Handle TCP handshake by creating and sending SYN and ACK packets and waiting for SYN-ACK packets.
        if (recv_tcp_header->syn == 1 && recv_tcp_header->ack == 0 &&
            recv_ip_header->ip_dst.s_addr != inet_addr(local_ip.c_str()) &&
            ntohs(recv_tcp_header->dest) != local_port)
        {
            handle_tcp_handshake(raw_sock, inet_ntoa(recv_ip_header->ip_dst), ntohs(recv_tcp_header->dest));
        }
    }
}

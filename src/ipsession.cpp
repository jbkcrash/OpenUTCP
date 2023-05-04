// Write a function to reverse an ip structure for responding to the source
void reverse_ip(struct ip *ip_header)
{
     struct in_addr temp = ip_header->ip_src;
     ip_header->ip_src = ip_header->ip_dst;
     ip_header->ip_dst = temp;
}

// Write a function to hold an IP session for use with userland sockets over RAW
void ip_session(int raw_sock, const std::string &dest_ip, uint16_t dest_port)
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
    ip_header.ip_src.s_addr = inet_addr("//continue
    ip_header.ip_dst.s_addr = inet_addr(dest_ip.c_str());
    //continue
    // Create a TCP header
    struct tcphdr tcp_header;
    memset(&tcp_header, 0, sizeof(tcp_header));
    //continue
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
    //continue
    // Calculate the TCP checksum
    struct pseudo_header
    {
        u_int32_t source_address;
        u_int32_t dest_address;
        u_int8_t placeholder;
        u_int8_t protocol;
        u_int16_t tcp_length;
    } pseudo_header;
    //continue
    pseudo_header.source_address = inet_addr("
    pseudo_header.dest_address = ip_header.ip_dst.s_addr;
    pseudo_header.placeholder = 0;
    pseudo_header.protocol = IPPROTO_TCP;
    pseudo_header.tcp_length = htons(sizeof(struct tcphdr));
    //continue
    int packet_size = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
    char *packet = (char *)malloc(packet_size);
    memcpy(packet, (char *)&pseudo_header, sizeof(struct pseudo_header));
    memcpy(packet + sizeof(struct pseudo_header), &tcp_header, sizeof(struct tcphdr));
    tcp_header.check = checksum((unsigned short *)packet, packet_size);
    //continue
    // Create a sockaddr_in struct for the destination
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip.c_str());
    //continue
    // Send the SYN packet
    if (sendto(raw_sock, &tcp_header, sizeof(tcp_header), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        std::cerr << "Failed to send SYN packet" << std::endl;
        return;
    }
    //continue
    // Receive the SYN/ACK packet
    char buffer[4096];
    socklen_t addr_size = sizeof(dest_addr);
    if (recvfrom(raw_sock, buffer, 4096, 0, (struct sockaddr *)&dest_addr, &addr_size) < 0)
    {
        std::cerr << "Failed to receive SYN/ACK packet" << std::endl;
        return;
    }
    //continue
    // Parse the received packet
    struct ip *ip_header_response = (struct ip *)buffer;
    struct tcphdr *tcp_header_response = (struct tcphdr *)(buffer + sizeof(struct ip));
    //continue
    // Check if the packet is a SYN/ACK
    

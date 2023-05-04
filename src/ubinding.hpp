#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

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

void handle_incoming_connection(int raw_sock, const struct ip *recv_ip_header, const struct tcphdr *recv_tcp_header);
void accept_inbound_connections(int raw_sock, const std::string &local_ip, uint16_t local_port);

#endif /* TCP_SERVER_HPP */ 

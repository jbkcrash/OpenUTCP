#pragma once

/**
Handles the TCP handshake by creating and sending SYN and ACK packets and waiting for SYN-ACK packets.
@param raw_sock - The socket to use for sending and receiving packets.
@param dest_ip - The IP address of the destination host.
@param dest_port - The port number of the destination service.
*/
void outgoing_handle_tcp_handshake(int raw_sock, const std::string &dest_ip, uint16_t dest_port);
#pragma once

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/**
 *  Creates and binds a raw socket to the specified IP address and port number.
 *
 *  @param ip - The IP address to bind the socket to.
 *  @param port - The port number to bind the socket to.
 *
 *  @return The raw socket file descriptor if successful, otherwise -1.
 */
int create_userland_socket(const std::string& ip, uint16_t port);

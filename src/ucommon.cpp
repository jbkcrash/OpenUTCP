#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>

unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;

    for (sum = 0; nwords > 0; nwords--)
    {
        sum += *buf++;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}
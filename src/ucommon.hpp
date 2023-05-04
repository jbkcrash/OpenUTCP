#pragma once


/**


Calculates the checksum for a given buffer of 16-bit words.


@param buf - Pointer to the buffer containing the 16-bit words.
@param nwords - Number of 16-bit words in the buffer.


@return The checksum as an unsigned short.
*/
unsigned short csum(unsigned short *buf, int nwords);


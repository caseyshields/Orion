#include "util/crc.h"

// Obtained from http://www.drdobbs.com/implementing-the-ccitt-cyclical-redundan/199904926
// Written by Bob Felice

int     crc_ok = 0x470F;

unsigned short crc16(data_p, length)
        char *data_p;
        unsigned short length;
{
    unsigned char i;
    unsigned int data;
    unsigned int crc;

    crc = 0xffff;

    if (length == 0)
        return (~crc);

    do {
        for (i = 0, data = (unsigned int)0xff & *data_p++; i < 8; i++, data >>= 1) {
            if ((crc & 0x0001) ^ (data & 0x0001))
                crc = (crc >> 1) ^ POLY;
            else
                crc >>= 1;
        }
    } while (--length);

    crc = ~crc;

    data = crc;
    crc = (crc << 8) | (data >> 8 & 0xFF);

    return (crc);
}
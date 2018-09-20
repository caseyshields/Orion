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

// test ///////////////////////////////////////////////////////////////////////

void test_crc( CuTest * test ) {
    // modified from http://www.drdobbs.com/implementing-the-ccitt-cyclical-redundan/199904926
    // Written by Bob Felice

    static unsigned char string[40];
    string[0] = 'T';
    string[1] = (unsigned char)0xd9;
    string[2] = (unsigned char)0xe4;
    string[3] = '\0';

//    printf ("The crc of \"T\" is 0xD9E4. crc16 returned 0x%X.\r\n\n",
//            crc16(string, (short)1));
    CuAssertIntEquals( test, 0xD9E4, crc16(string, (short)1) );

//    printf ("The crc of \"T 0xD9 0xE4\" is %x. The value of crc_ok is 0x%X.\r\n\n",
//            crc16(string, (short)3), crc_ok);
    CuAssertIntEquals( test, crc_ok, crc16(string, (short)3) );

    strcpy(string, "THE,QUICK,BROWN,FOX,0123456789");
//    printf("The crc of \"%s\" is 0x6E20. crc16 returned 0x%X.\r\n\n",
//           string, crc16 (string, strlen(string)));
    CuAssertIntEquals( test, 0x6E20, crc16(string, strlen(string)) );

    string[0] = (unsigned char)0x03;
    string[1] = (unsigned char)0x3F;
//    puts("CCITT Recommendation X.25 (1984) Appendix I example:");
//    printf("\tThe crc of 0x03 0x3F is 0x5BEC. crc16 returned 0x%X.\r\n\n",
//           crc16(string, (short)2));
    CuAssertIntEquals( test, 0x5BEC, crc16(string, (short)2) );

//    puts("strike RETURN to continue...");
//        getchar();
}
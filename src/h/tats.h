//
// Created by Casey Shields on 6/13/2018.
//

#ifndef STARTRACK_TATS_H
#define STARTRACK_TATS_H

/** Standard TCN data message. Documetation taken from TATS IRS documentation V2.2*/
typedef struct {
    /** Identifies message type. (Message Type 1 RIU tracking data). */
    unsigned char midc;

    /** TIDC.
     *  - 0 Hex. Indicates IFF codes.
     *  - 1 Hex. Indicates RIU Sensor ID.
     *  - 2 Hex. Indicates GPS pod numbers.
     * Sensor ID (TIDC) - 000-FFF HEX code unique for each sensor.*/
    unsigned short riu_sensor_id;

    /** Usigned integer. LSB = 1 millisecond. Maximum count is 59.999 seconds. TCN timeis used to
     * calculate latency. It is also used by some receiving systems for delta calculations. */
    unsigned short tcn_time;

    /** 32 bit signed integers with LSB=0.00390625 (1/256) meters. */
    int E, F, G;

    /**  */
    unsigned char track_status;

    union {
        /**  */
        unsigned short track_id;

        /**  */
        unsigned char symbol_type;
    };

    /** A 16-bit error checksum, Cyclic Redundancy Check. The CRC field consists of the Consultative Committee
     * International for Telegraphy and Telephone (CCITT)/(ITU) V.41 polynomial, x^16 + x^12 + x^5 + 1, with
     * the CRC accumulator preset ti 1's. */
    unsigned short crc;
} MIDC_01;


#endif //STARTRACK_TATS_H

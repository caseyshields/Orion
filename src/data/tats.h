/** @file tats.h
 * @brief TATS structure and constant definitions
 *
 * This header contains a bunch of declarations and methods canabalized from the TPOD project.
 * Removing it from the company network is non-trivial so these limited excerpts will have to
 * suffice until I can finish all the paperwork and process to move the code as a whole.
 *
 * @author Casey Shields
 * @bug This file currently only contains a pertinent subset of the full TATS interface.*/

#ifndef STARTRACK_TATS_H
#define STARTRACK_TATS_H

#include <stdio.h>

/** 2^31/256 meters, or half the maximum distance representable by TATS((2^32)/256 = 16777216 meters).
 * Novas uses Astronomical units for distance and they set their celestial sphere(for the purposes of parallax)
 * at 2*10^14 AU */
#define TATS_CELESTIAL_SPHERE_RADIUS (1<<31)

/** Midc index of TATS message types. */
typedef enum {
    TATS_NULL,
    TATS_TRK_DATA,
    TATS_STATUS_REQ,
    TATS_STATUS_REPLY,
    TATS_REQ_REF,
    TATS_REQ_BS = 10,
    TATS_REPLY_BS,
    TATS_REQ_RNG_TGT,
    TATS_REPLY_RNG_TGT,
    TATS_REQ_LATLON,
    TATS_REPLY_LATLON,
    TATS_REQ_EFG,
    TATS_REPLY_EFG,
    TATS_REQ_MNE,
    TATS_REPLY_MNE,
    TATS_LAST_MSG
} TATS_MSG_ETYPE;

/** Internal international friendly / foe designation. */
typedef enum {
    TATS_FOE,
    TATS_NONPLAYER,
    TATS_FRIEND,
    TATS_IFF_LAST
} TATS_IFF_ETYPE;

/** TIDC indices */
typedef enum {
    TATS_TIDC_IFF = 0x0000,
    TATS_TIDC_RIU = 0x0001,
    TATS_TIDC_GPS = 0x0002,
    TATS_TIDC_LAST
} TATS_TIDC_ETYPE;

/** Sensor role */
typedef enum {
    TATS_SENSOR_NONE,
    TATS_SENSOR_TATS,
    TATS_SENSOR_SLAVE,
    TATS_SENSOR_MASTER,
    TATS_SENSOR_SYMBOL,
    TATS_SENSOR_MASTER_SLAVE,
    TATS_SENSOR_REMOTE,
    TATS_SENSOR_MASTER_REMOTE,
    TATS_SENSOR_LAST
} TATS_SENSOR_ETYPE;

/** Sensor status */
typedef enum {
    TATS_STATUS_RX_AUTO_RNG = 0x01,
    TATS_STATUS_QUALITY = 0x02,
    TATS_STATUS_AUTO_RNG = 0x04,
    TATS_STATUS_AQUIRE = 0x08,
    TATS_STATUS_SIM = 0x10,
    TATS_STATUS_POS_DATA = 0x20,
    TATS_STATUS_UNDEFINED = 0x40,
    TATS_STATUS_AUTO_TRK = 0x80,
    TATS_STATUS_LAST
} TATS_STATUS_ETYPE;

#pragma pack(push)
#pragma pack(1)

typedef struct {
    unsigned char midc;
    unsigned char data[19];
    unsigned short crc;
} TCN_Message;

/** Standard TCN data message. Documentation taken from TATS IRS documentation V2.2*/
typedef struct {
    /** Identifies message type. (Message Type 1 RIU tracking data). */
    unsigned char midc;

    /** Sensor ID (TIDC) - 000-FFF HEX code unique for each sensor.*/
    unsigned short sensor_id : 12;

    /** TIDC.
     *  - 0 Hex. Indicates IFF codes.
     *  - 1 Hex. Indicates RIU Sensor ID.
     *  - 2 Hex. Indicates GPS pod numbers.*/
    unsigned short sensor_type : 4;

    /** Unsigned integer. LSB = 1 millisecond. Maximum count is 59.999 seconds. TCN time is used to
     * calculate latency. It is also used by some receiving systems for delta calculations. */
    unsigned short tcn_time;

    /** 32 bit signed integers with LSB=0.00390625 (1/256) meters. */
    int E, F, G;

    /** A bitmask showing system status. Bit values are obtained by bitwise ORing values from TATS_STATUS_ETYPE  */
    unsigned char track_status;

    /** For single target sensors this field is zero filled. If source is a Multi-Target sensor
     * then bytes 18 and 19 are the track ID. */
    unsigned short int track_id : 14;

    /** If source is an IFF system then byte 19 is zero filled. Byte 18's value is the Symbol type
     * as follows:
     *  - 0 = Foe
     *  - 1 = Non-Player
     *  - 2 = Friend
     * As enumerated in TATS_IFF_ETYPE*/
    unsigned short int symbol_type : 2;

    /** A 16-bit error checksum, Cyclic Redundancy Check. The CRC field consists of the Consultative Committee
     * International for Telegraphy and Telephone (CCITT)/(ITU) V.41 polynomial, x^16 + x^12 + x^5 + 1, with
     * the CRC accumulator preset ti 1's. */
    unsigned short crc;
} MIDC01;
#pragma pack(pop)

//// original tats.h declaration
//#pragma pack(push)
//#pragma pack(1)
//typedef struct {
//    UINT8 midc;
//    UINT16 tidc;
//    UINT16 time;
//    INT32 e;
//    INT32 f;
//    INT32 g;
//    UINT8 status;
//    UINT16 trk_id : 14;
//    UINT16 symbol : 2;
//} /*PACKED*/ TATS_MSG1_STYPE;
//// not sure where the packed macro is defined...
//#pragma pack(pop)

/** Prints a diagnostic message string of the given message to the file. */
void tats_print_midc01( MIDC01 * midc01, FILE * file );

///** General message structure for the TATS Control Network */
typedef struct {
    unsigned char midc;
    char tats_msg[19];
} tcn_gen_msg;

///** Template for TCN receive messages */
//typedef struct {
//    unsigned char midc;
//    unsigned short tidc;
//    unsigned char drs_id;
//    unsigned char spare[16];
//} tcn_rx_msg;


#endif //STARTRACK_TATS_H

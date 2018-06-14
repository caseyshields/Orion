//
// Created by Casey Shields on 6/13/2018.
//

#ifndef STARTRACK_TATS_H
#define STARTRACK_TATS_H

typedef struct {
    unsigned char midc;
    unsigned short riu_sensor_id;
    unsigned short tcn_time;
    int E, F, G;
    unsigned char track_status;
    union {
        unsigned short track_id;
        unsigned char symbol_type;
    };
    unsigned short crc;
} MIDC_01;

#endif //STARTRACK_TATS_H

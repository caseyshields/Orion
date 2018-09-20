#include "data/tats.h"

void tats_print_midc01(MIDC01 * midc01, FILE * file ) {
    fprintf( file,
             "%02d %01d %04d %05d %d %d %d %04d %02X %04X\n",
             midc01->midc,
             midc01->sensor_type,
             midc01->sensor_id,
             midc01->tcn_time,
             midc01->E,
             midc01->F,
             midc01->G,
             midc01->track_id,
             midc01->track_status,
             midc01->crc
    );
    fflush( file );
}
//
//MIDC01 * tats_midc01_idle( MIDC01 * midc01 ) {
//    if( midc01 )
//        memset(midc01, 0, sizeof(MIDC01));
//    else
//        midc01 = calloc(1, sizeof() );
//
//    midc01->midc = TATS_TRK_DATA;
//}

// test ///////////////////////////////////////////////////////////////////////

void test_tats( CuTest * test ) {
    // make sure 1 byte alignment is working...
    CuAssertIntEquals( test, 22, sizeof(MIDC01) );
    CuAssertIntEquals( test, 22, sizeof(TCN_Message) );
}
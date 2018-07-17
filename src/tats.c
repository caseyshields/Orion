#include <h/tats.h>

void tats_print_midc01(MIDC01 * midc01, FILE * file ) {
    fprintf( file,
             "\tmidc : %d\n\ttidc : %x\n\ttime : %d\n\te : %lf\n\tf : %lf\n\tg : %lf\n\tstatus : %1x\n\tcrc : %x",
             midc01->midc, midc01->riu_sensor_id, midc01->tcn_time,
             (midc01->E/3600.0), (midc01->F/3600.0), 0.0,//(int)(midc01->G),
             midc01->track_status, midc01->crc
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
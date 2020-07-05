
#include <list>
#include <string.h>
#include <chrono>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <thread>


#include "h264_2_rtmp.h"
#include "h264_2_rtmp_manager.h"
#include "RingBuffer.h"

#ifdef HIK
#include "hikConnect.h"
#endif


using namespace std;

bool g_main_running_flag = false;
void sig_cb(int signum){
    fprintf(stdout,"%s recv signum:%d\n", __FUNCTION__,signum);
    if (2 == signum){
        g_main_running_flag = false;
    }
}

struct CustomStruct{
    FILE* fp;
    char frames_buf[1*1024*1024];
    int  frames_buf_len = 1*1024*1024;
    int  cur_available_data_pos_begin = 0;
    int  cur_available_data_pos_end = 0;
    int  frame_start_pos = 0;
    int  frame_no = -1;
    bool frame_start_flag = false;
    CycleBuffer*  cycle_buffer = nullptr;
};



static int on_have_data_cb_impl_file( char* buf, int buf_max_len, void* user ){

    CustomStruct *pcs = (CustomStruct*)user;

    int nreadbytes = 0;
    if ( pcs->frame_no == -1){
        nreadbytes = fread( &pcs->frames_buf[ pcs->cur_available_data_pos_end ], 1, pcs->frames_buf_len - pcs->cur_available_data_pos_end, pcs->fp );
        pcs->cur_available_data_pos_end = nreadbytes;
    }

    int data_start = pcs->cur_available_data_pos_begin;

    /* fprintf(stdout,"%s data_start %d, begin %d, end %d, frame_start_pos %d\n", __FUNCTION__, data_start, */ 
    /*         pcs->cur_available_data_pos_begin */
    /*         ,pcs->cur_available_data_pos_end, pcs->frame_start_pos); */

    while(true){
        if (pcs->frames_buf[ data_start ] == 0x00 && pcs->frames_buf[ data_start+1 ] == 0x00 
            && pcs->frames_buf[ data_start + 2] == 0x00  && pcs->frames_buf[ data_start + 3] == 0x01 ){
            
            if (pcs-> frame_start_flag == false){
                /* fprintf(stdout,"%s first frame start %d %02x %02x %02x %02x\n", __FUNCTION__ */
                /*         ,pcs->cur_available_data_pos_begin */
                /*         ,pcs->frames_buf[ data_start ] */
                /*         ,pcs->frames_buf[ data_start+1 ] */  
                /*         ,pcs->frames_buf[ data_start + 2] */   
                /*         ,pcs->frames_buf[ data_start + 3]  ); */
                pcs->frame_start_pos = data_start; 
                ++data_start;
                pcs->frame_start_flag = true;
                continue;
            }else{
                int frame_len = data_start - pcs->frame_start_pos;
                int frame_type = (pcs->frames_buf[ pcs->frame_start_pos  + 4] & 0x1f ); 
                /* fprintf(stdout,"%s frame %d, len %d,  %02x %02x %02x %02x\n", __FUNCTION__, pcs->frame_no, frame_len */
                /*          ,pcs->frames_buf[ data_start ] */
                /*         ,pcs->frames_buf[ data_start+1 ] */  
                /*         ,pcs->frames_buf[ data_start + 2] */   
                /*         ,pcs->frames_buf[ data_start + 3]  ); */
                /* //此处拷贝出去 */
                //memcpy(buf, &pcs->frames_buf[ pcs->frame_start_pos ],  frame_len);
            	IFVFrameHeader_S* pframe = new IFVFrameHeader_S();
		        pframe->dataLen = frame_len;
		        pframe->data = new unsigned char[frame_len];
		        memcpy(pframe->data,&pcs->frames_buf[ pcs->frame_start_pos ],frame_len);
		        pcs->cycle_buffer->pushBuffer(pframe);
                delete pframe->data;
                delete pframe;

                fprintf(stdout," frame_no %d, type %d, len %d \n", pcs->frame_no, frame_type,frame_len);

                pcs->cur_available_data_pos_begin = data_start;
                pcs->frame_no++;

                pcs->frame_start_flag = false;

                std::this_thread::sleep_for(std::chrono::milliseconds(40));

                return frame_len; 
            }
        }
        ++data_start;
        if (data_start == pcs->cur_available_data_pos_end){
            int data_len = pcs->cur_available_data_pos_end - pcs->cur_available_data_pos_begin;

            /* fprintf(stdout,"%s data_start == 1MB %d, begin %d, end %d, len %d\n", __FUNCTION__, data_start */ 
            /*         ,pcs->cur_available_data_pos_begin,pcs->cur_available_data_pos_end,  data_len); */
            memmove( pcs->frames_buf, &pcs->frames_buf[pcs->cur_available_data_pos_begin],  data_len);

            if (pcs->frame_start_flag )
                pcs->frame_start_pos = 0;

            pcs->cur_available_data_pos_begin = 0;
            pcs->cur_available_data_pos_end = data_len;

            nreadbytes = fread( &pcs->frames_buf[ pcs->cur_available_data_pos_end ], 1, pcs->frames_buf_len - pcs->cur_available_data_pos_end, pcs->fp );
            if (nreadbytes <= 0){
                fseek( pcs->fp, 0, SEEK_SET );
                nreadbytes = fread( &pcs->frames_buf[ pcs->cur_available_data_pos_end ], 1, pcs->frames_buf_len - pcs->cur_available_data_pos_end, pcs->fp );
            }
            pcs->cur_available_data_pos_end += nreadbytes;

            return 0;
        }
    }
    return 0;
}

int main(int argc, char** argv){

    signal( 2, sig_cb );

    CycleBuffer *pcycleBuffer = new CycleBuffer;
    CH264_2_RTMP_Manager::start("live", pcycleBuffer);

#ifdef  HIK
    HikConn hik_conn( pcycleBuffer );
    hik_conn.Login();
    hik_conn.StartStream();
#else
    CustomStruct cs;
    cs.fp = fopen("2.h264","r");
    if (!cs.fp){
        fprintf(stdout,"open file failed\n");
        exit(1);
    }
    cs.cycle_buffer = pcycleBuffer;
#endif

    fprintf(stdout,"%s2\n", __FUNCTION__);

    g_main_running_flag = true;
    while( g_main_running_flag ){
#ifndef HIK
        on_have_data_cb_impl_file( nullptr, 0, &cs );
#endif
        std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );
    }

    CH264_2_RTMP_Manager::stop("live");


    return 0;
}


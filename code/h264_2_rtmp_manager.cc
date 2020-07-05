#include <string.h>

#include "h264_2_rtmp_manager.h"
#include "h264_2_rtmp.h"
#include "RingBuffer.h"

std::map<std::string,void*>  CH264_2_RTMP_Manager::m_stream_name_2_rtmp_server_map;
std::mutex                   CH264_2_RTMP_Manager::m_map_lock;


static int on_have_data_cb_impl_hik( char* buf, int buf_max_len, void* user ){
    CycleBuffer * cycle_buffers = (CycleBuffer*)user;
    IFVFrameHeader_S *frame  = nullptr;
    frame = cycle_buffers->getBuffer();
    if ( frame ){
        int frame_len = frame->dataLen;
        //fprintf(stdout,"###%s %d\n", __FUNCTION__, frame_len);*/
        memcpy(buf, frame->data, frame_len);
        cycle_buffers->pop();

        /*
        static int frame_no = 0;
        char file_name[32];
        sprintf(file_name, "%d.264", ++frame_no);
        FILE* fp = fopen( file_name, "wb" );
        if (fp){
            fwrite(buf, 1,frame_len, fp );
            fclose(fp);
        }
        */

        return frame_len;
    }
    return 0;
}

const char* CH264_2_RTMP_Manager::start( const char* stream_name, void* user_cxt ){
    std::lock_guard<std::mutex> lg(m_map_lock);
    auto it = m_stream_name_2_rtmp_server_map.find( stream_name );
    if (it != m_stream_name_2_rtmp_server_map.end()) return ((CH264_2_RTMP*)it->second)->get_rtmp_url();
    else{
        CH264_2_RTMP *rtmp_serv = new CH264_2_RTMP;
        const char* url = rtmp_serv->start( stream_name,on_have_data_cb_impl_hik, user_cxt );
        m_stream_name_2_rtmp_server_map.insert({ stream_name, rtmp_serv} );
        return url;
    }
}

void CH264_2_RTMP_Manager::stop( const char* stream_name ){
    std::lock_guard<std::mutex> lg(m_map_lock);
    auto it = m_stream_name_2_rtmp_server_map.find( stream_name );
    if (it != m_stream_name_2_rtmp_server_map.end()){
        CH264_2_RTMP* p = (CH264_2_RTMP*)it->second;
        delete p;//析构函数里做了stop
    }
}

const char* CH264_2_RTMP_Manager::get_rtmp_url( const char* stream_name ){
    std::lock_guard<std::mutex> lg(m_map_lock);
    auto it = m_stream_name_2_rtmp_server_map.find( stream_name );
    if (it != m_stream_name_2_rtmp_server_map.end()) return ((CH264_2_RTMP*)it->second)->get_rtmp_url();
    else return nullptr;
}

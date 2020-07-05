#pragma once

#include <thread>


typedef int (*have_data_cb)(char* buf, int buf_max_len, void* ctx);

enum  frame_type{
         SPS_PPS_I = 7,
         PPS = 8,
         I = 5,
         P_OR_B = 1
    };


class CH264_2_RTMP{
public:
    CH264_2_RTMP();
    ~CH264_2_RTMP();
    const char* start(const char* stream_name, have_data_cb _cb, void* _user_ctx);
    void stop();
    const char* get_rtmp_url();

private:
    void run();
     
private:
    bool            m_running_flag;
    std::thread     m_thread_impl;

    char            m_rtmp_url[256];
    char            m_stream_name[128];
    unsigned short  m_port; //起始端口1935

    have_data_cb    m_on_have_data_cb;
    void*           m_user_ctx;

    char*           m_sps_buf;
    int             m_sps_buf_len;
    char*           m_pps_buf;
    int             m_pps_buf_len;

};


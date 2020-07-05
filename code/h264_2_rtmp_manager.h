#pragma once

#include <map>
#include <mutex>

class CH264_2_RTMP_Manager{
public:
    static const char* start(const char* stream_name, void*);
    static void        stop( const char* stream_name);
    static const char* get_rtmp_url( const char* stream_name );

private:
    static std::map<std::string,void*>          m_stream_name_2_rtmp_server_map;
    static std::mutex                           m_map_lock;
};


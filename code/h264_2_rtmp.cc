

#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include <assert.h>

#include <vector>

#include "h264_2_rtmp.h"
extern "C" {
    #include "tiny_rtmp_server.h"
}

unsigned short rtmp_begin_port = 1935;


static std::string get_local_ip(){
    
    std::vector<std::string> ips;
    
    struct ifaddrs * ifAddrStruct = NULL;
    void * tmpAddrPtr = NULL;
    
    if (getifaddrs(&ifAddrStruct) != 0)
    {
        //if wrong, go out!
        printf("Somting is Wrong!\n");
        return "127.0.0.1";
    }
    
    struct ifaddrs * iter = ifAddrStruct;
    
    while (iter != NULL) {
        if (iter->ifa_addr->sa_family == AF_INET) { //if ip4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *)iter->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if ( strcmp( addressBuffer, "127.0.0.1" ) != 0 ) 
                ips.push_back( addressBuffer );
        }
        iter = iter->ifa_next;
    }
    //releas the struct
    freeifaddrs(ifAddrStruct);
    
    if (ips.empty()) return "127.0.0.1";
    return ips[0];
}

CH264_2_RTMP::CH264_2_RTMP():m_running_flag(false),m_rtmp_url{0},m_stream_name{0},m_port( rtmp_begin_port++ )
    , m_on_have_data_cb(nullptr), m_user_ctx(nullptr) 
    , m_sps_buf(nullptr), m_sps_buf_len{0}, m_pps_buf(nullptr), m_pps_buf_len{0}{
}

CH264_2_RTMP::~CH264_2_RTMP(){
    stop();
}

const char* CH264_2_RTMP::start(const char* stream_name, have_data_cb _cb, void* _user_ctx){
    m_on_have_data_cb = _cb;
    m_user_ctx = _user_ctx;

    strcpy( m_stream_name, stream_name );

    m_running_flag = true;
    m_thread_impl = std::thread([this](){
                                this->run();
                                });

    std::string local_ip =  get_local_ip();
    sprintf(m_rtmp_url, "rtmp://%s:%d/%s", local_ip.c_str(), m_port, m_stream_name);

    fprintf(stdout," %s %s\n", __FUNCTION__, m_rtmp_url);
    return m_rtmp_url;
}

void CH264_2_RTMP::stop(){
    m_running_flag = false;
    if (m_thread_impl.joinable()){
        m_thread_impl.join();
    }
}

const char* CH264_2_RTMP::get_rtmp_url(){
    return m_rtmp_url;
}

void CH264_2_RTMP::run(){
    int h264_buf_len = 3*1024*1024;
    char* h264_buf = new char[ h264_buf_len ];


	void *rtmp_server = NULL;
	rtmp_server = rs_create(m_port,m_stream_name);
	if(rtmp_server == NULL)
	{
		printf("create rtmp server fail \n");
        goto end;
	}

	if(rs_start(rtmp_server) != RS_ERRCODE_SUCCESS)
	{
		printf("start rtmp server fail \n");
        goto end;
	}

    while( m_running_flag ){
        int nread_bytes = 0;
        if (m_on_have_data_cb && m_user_ctx){
            nread_bytes = m_on_have_data_cb( h264_buf, h264_buf_len, m_user_ctx );
        }

        if (nread_bytes > 4){

            int frame_type = (h264_buf[ 4 ] &0x1f);

            if (frame_type == SPS_PPS_I ){
                int i = 5;
                int j = 0;

                //find next start code
                while(i < nread_bytes ){
                    if (h264_buf[i] == 0x00 && h264_buf[i+1] == 0x00 && h264_buf[i+2] == 0x00 && h264_buf[i+3] == 0x01 ){
                        break;
                    }
                    ++i;
                }

                if (i != m_sps_buf_len){
                    fprintf(stdout,"%s %d %d\n", __FUNCTION__, m_sps_buf_len, i);
                    m_sps_buf_len = i;
                    if (m_sps_buf) delete [] m_sps_buf;
                    m_sps_buf = new char[ m_sps_buf_len ];
                    memcpy( m_sps_buf, h264_buf, m_sps_buf_len );
                 }

                if (m_sps_buf_len > 0){
                    //fprintf(stdout,"%s frame_type %d, len %d\n", __FUNCTION__, m_sps_buf[4]&0x1f, m_sps_buf_len);
                    rs_push_frame(rtmp_server,(unsigned char*) m_sps_buf, m_sps_buf_len);
                }

                j = i;
                ++i;
                while( i < nread_bytes ){
                    if (h264_buf[i] == 0x00 && h264_buf[i+1] == 0x00 && h264_buf[i+2] == 0x00 && h264_buf[i+3] == 0x01 ){
                        break;
                    }
                    ++i;
                }
               
                int pps_l = i - m_sps_buf_len;
                if ( m_pps_buf_len != pps_l ){
                    fprintf(stdout,"%s %d %d\n", __FUNCTION__, m_pps_buf_len, pps_l);
                    m_pps_buf_len = pps_l;
                    if (m_pps_buf) delete [] m_pps_buf;
                    m_pps_buf = new char[ m_pps_buf_len ];
                    memcpy( m_pps_buf, &h264_buf[j], m_pps_buf_len );
                }

                if (m_pps_buf_len > 0){
                    //fprintf(stdout,"%s frame_type %d, len %d\n", __FUNCTION__, m_pps_buf[4]&0x1f, m_pps_buf_len);
		            rs_push_frame(rtmp_server,(unsigned char*) m_pps_buf, m_pps_buf_len);
                }

                //FIRST I FRAME
                int i_l = nread_bytes - m_pps_buf_len - m_sps_buf_len;
                if ( i_l > 0){
                    //fprintf(stdout,"%s frame_type %d, len %d\n", __FUNCTION__, h264_buf[i + 4]&0x1f, nread_bytes - m_pps_buf_len - m_sps_buf_len);
		            rs_push_frame(rtmp_server,(unsigned char*) (&h264_buf[i]), i_l); 
                }
            
            }else if (frame_type == PPS){
                int i = 5;
                //find next start code
                while(i < nread_bytes ){
                    if (h264_buf[i] == 0x00 && h264_buf[i+1] == 0x00 && h264_buf[i+2] == 0x00 && h264_buf[i+3] == 0x01 ){
                        break;
                    }
                    ++i;
                }
                if (i != m_pps_buf_len){
                    fprintf(stdout,"%s %d %d\n", __FUNCTION__, m_pps_buf_len, i);
                    m_pps_buf_len = i;
                    if (m_pps_buf) delete [] m_pps_buf;
                    m_pps_buf = new char[ m_pps_buf_len ];
                    memcpy( m_pps_buf, h264_buf, m_pps_buf_len );
                 }

                if (m_pps_buf_len > 0){
                    //fprintf(stdout,"%s frame_type %d, len %d\n", __FUNCTION__, m_sps_buf[4]&0x1f, m_sps_buf_len);
                    rs_push_frame(rtmp_server,(unsigned char*) m_pps_buf, m_pps_buf_len);
                }
            }else if (frame_type == I){
                /* fprintf(stdout,"%s frame_type %d, len %d\n", __FUNCTION__,h264_buf[4]&0x1f, nread_bytes); */
		        rs_push_frame(rtmp_server,(unsigned char*) m_sps_buf, m_sps_buf_len); 
		        rs_push_frame(rtmp_server,(unsigned char*) m_pps_buf, m_pps_buf_len); 
		        rs_push_frame(rtmp_server,(unsigned char*) h264_buf,nread_bytes); 
            }else{//P B
                //fprintf(stdout,"%s frame_type %d, len %d\n", __FUNCTION__,h264_buf[4]&0x1f, nread_bytes);
		        rs_push_frame(rtmp_server,(unsigned char*) h264_buf,nread_bytes);
            }

        }else{
            std::this_thread::sleep_for( std::chrono::milliseconds(10) );
        }
    }

end:
    rs_stop( rtmp_server);
    rs_destroy( rtmp_server );
    delete []  h264_buf;
    delete [] m_sps_buf;
    delete [] m_pps_buf;
}


#pragma once
#ifndef RINGBUFF__HH__
#define RINGBUFF__HH__


using namespace std;

#define  MAXSIZE  1000
/*
typedef struct
{
    int iSize;//数据的大小
    char* cdata;
}bufData;
typedef struct
{
    bufData mdata[MAXSIZE];
    int iRead;//队头
    int iWrite;//队尾
}bufferPara;

class CycleBuffer
{
public:
    CycleBuffer(void);
    ~CycleBuffer(void);

    BOOL pushBuffer(bufferPara* Q, char* src, int strLen);//缓冲区存入

    char* popBuffer(bufferPara* Q, long& dsLen);//弹出数据

    BOOL InitQueue(bufferPara* Q);//初始化队列
    bufferPara  mbufferPara;
    char* dest;
private:
    //bufferPara* m_RingBuffer;
    HANDLE mutexHandle;
    char* item;

};
*/
typedef struct
{
    unsigned long long timeTick;   //时间(ms)
    unsigned int dataLen;     //数据长度
    unsigned char dataType;     //数据类型(DataType_E)
    unsigned char rsv[3];
    unsigned long long timeStamp;  //编码时间戳(us)
    unsigned char iFrame;    //是否为关键帧
    unsigned char frameRate;   //帧率
    int encodeType;          //编码类型VideoEncodeType_E
    unsigned short width;    //视频宽度
    unsigned short height;    //视频高度
    unsigned char rsv1[8];
    unsigned char* data;
}IFVFrameHeader_S;
        


typedef struct
{
    unsigned long* m_pBuff;
    int    ifront;//队头
    int    iend;//队尾
    long buffSize;
}RINGBUFFER;

class CycleBuffer
{
public:
    CycleBuffer(void);
    ~CycleBuffer(void);

    bool pushBuffer(IFVFrameHeader_S* src);//缓冲区存入

    IFVFrameHeader_S* popBuffer();//弹出数据

    IFVFrameHeader_S* getBuffer();
    void pop();

    bool InitQueue();//初始化队列
    long GetCurrDataSize();
    bool isFull();
    RINGBUFFER* m_pRingBuffer;
   //char* dest;
private:
    //bufferPara* m_RingBuffer;
   // HANDLE mutexHandle;
    //char* item;

};
#endif

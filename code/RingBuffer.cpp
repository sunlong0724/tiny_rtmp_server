
#include "RingBuffer.h"
#include <stdio.h>
#include <string.h>

#define MAXCYCLEBUFFSIZE (1024*1024)
/*
CycleBuffer::CycleBuffer(void)
{
    InitQueue(&mbufferPara);
    dest = new char[MAXCYCLEBUFFSIZE];
    memset(dest, 0, MAXCYCLEBUFFSIZE);
    mutexHandle = CreateMutex(NULL, FALSE, NULL);
}

CycleBuffer::~CycleBuffer(void)
{
    CloseHandle(mutexHandle);
    for (int i = 0; i < MAXSIZE; i++)
    {
        delete[] mbufferPara.mdata[i].cdata;
    }
}

BOOL CycleBuffer::pushBuffer(bufferPara* Q, char* src, int srcLen)
{
    WaitForSingleObject(mutexHandle, INFINITE);
    if ((Q->iWrite + 1) % MAXSIZE != Q->iRead)
    {
        memcpy(Q->mdata[Q->iWrite].cdata, src, srcLen);
        //Q->mdata[Q->iWrite].cdata = src;
        Q->mdata[Q->iWrite].iSize = srcLen;
        Q->iWrite = (Q->iWrite + 1) % MAXSIZE;
        ReleaseMutex(mutexHandle);
        return TRUE;
    }
    else
    {
        ReleaseMutex(mutexHandle);
        return FALSE;
    }

}
char* CycleBuffer::popBuffer(bufferPara* Q, long& dstLen)
{
    WaitForSingleObject(mutexHandle, INFINITE);

    if (Q->iRead != Q->iWrite)
    {
        dest = Q->mdata[Q->iRead].cdata;
        dstLen = Q->mdata[Q->iRead].iSize;
        memcpy(dest, Q->mdata[Q->iRead].cdata, dstLen);
        Q->iRead = (Q->iRead + 1) % MAXSIZE;
        ReleaseMutex(mutexHandle);
        return dest;
    }
    else
    {
        ReleaseMutex(mutexHandle);
        return NULL;
    }

}
BOOL CycleBuffer::InitQueue(bufferPara* Q)
{

    Q->iRead = 0;
    Q->iWrite = 0;
    for (int i = 0; i < MAXSIZE; i++)
    {
        Q->mdata[i].cdata = new char[MAXCYCLEBUFFSIZE];
        memset(Q->mdata[i].cdata, 0, MAXCYCLEBUFFSIZE);
        Q->mdata[i].iSize = 0;
    }
    return TRUE;
}
*/

#define MAXRETURNSIZE (4096)
CycleBuffer::CycleBuffer(void)
{
    InitQueue();
   // dest = new char[MAXRETURNSIZE];
   // mutexHandle = CreateMutex(NULL, FALSE, NULL);
}

CycleBuffer::~CycleBuffer(void)
{
    //CloseHandle(mutexHandle);
    if (m_pRingBuffer)
    {
        delete[]m_pRingBuffer->m_pBuff;
        delete m_pRingBuffer;
        m_pRingBuffer = NULL;
    }
    //delete[]dest;
}
bool CycleBuffer::isFull()
{
    //
    return ((m_pRingBuffer->iend + 1) % m_pRingBuffer->buffSize == m_pRingBuffer->ifront);
}
long CycleBuffer::GetCurrDataSize()
{
    long currentsize =  (m_pRingBuffer->iend  - m_pRingBuffer->ifront + m_pRingBuffer->buffSize) % m_pRingBuffer->buffSize;
    return currentsize;
}

bool CycleBuffer::pushBuffer(IFVFrameHeader_S* src)
{
    //WaitForSingleObject(mutexHandle, INFINITE);

    if (isFull())
    {
        //ReleaseMutex(mutexHandle);
        fprintf(stdout,"%s is Full\n", __FUNCTION__);
        return false;
    }
    if (src != NULL)
    {
        IFVFrameHeader_S *pframe = new IFVFrameHeader_S();
        pframe->width = src->width;
        pframe->dataType = src->dataType;
        pframe->encodeType = src->encodeType;
        pframe->frameRate = src->frameRate;
        pframe->height = src->height;
        pframe->timeStamp = src->timeStamp;
        pframe->timeTick = src->timeTick;
        pframe->width = src->width;
        pframe->iFrame = src->iFrame;
        pframe->dataLen = src->dataLen;
        pframe->data = new unsigned char[pframe->dataLen];
        memcpy(pframe->data, src->data, pframe->dataLen);
        
         m_pRingBuffer->m_pBuff[m_pRingBuffer->iend] = (unsigned long)pframe;
         m_pRingBuffer->iend = (m_pRingBuffer->iend + 1) % m_pRingBuffer->buffSize;
    }
    /*
    long currsize = GetCurrDataSize();
    long leftsize = m_pRingBuffer->buffSize - currsize;
    if (leftsize < srcLen)
    {
        return false;
    }
    for (int i = 0; i < srcLen; i++)
    {
        m_pRingBuffer->m_pBuff[m_pRingBuffer->iend] = (unsigned char)src[i];
        m_pRingBuffer->iend = (m_pRingBuffer->iend + 1) % m_pRingBuffer->buffSize;
    }
    */
    //ReleaseMutex(mutexHandle);
    return true;
    

}
IFVFrameHeader_S* CycleBuffer::getBuffer(){

    if (m_pRingBuffer->ifront == m_pRingBuffer->iend)
    {
        //ReleaseMutex(mutexHandle);
        //fprintf(stdout,"%s is empty()(%d,%d)\n", __FUNCTION__, m_pRingBuffer->ifront, m_pRingBuffer->iend);
        return NULL;
    }

    //fprintf(stdout,"%s cur_size %d, (%d, %d)\n", __FUNCTION__, GetCurrDataSize(), m_pRingBuffer->ifront, m_pRingBuffer->iend);

    IFVFrameHeader_S *dest = (IFVFrameHeader_S *)(m_pRingBuffer->m_pBuff[m_pRingBuffer->ifront]);
    //m_pRingBuffer->ifront = (m_pRingBuffer->ifront + 1) % m_pRingBuffer->buffSize;
    //ReleaseMutex(mutexHandle);
    return dest;  
}

void CycleBuffer::pop(){
    IFVFrameHeader_S *p = (IFVFrameHeader_S *)(m_pRingBuffer->m_pBuff[m_pRingBuffer->ifront]);
    delete [] p->data;
    delete p;

    m_pRingBuffer->ifront = (m_pRingBuffer->ifront + 1) % m_pRingBuffer->buffSize;
}

IFVFrameHeader_S* CycleBuffer::popBuffer()
{
    //WaitForSingleObject(mutexHandle, INFINITE);
   // long currsize = GetCurrDataSize();
    /*if (currsize > MAXRETURNSIZE)
    {
        currsize = MAXRETURNSIZE;
    }
    for (int i = 0; i < currsize; i++)
    {
        dest[i] = m_pRingBuffer->m_pBuff[m_pRingBuffer->ifront];
        m_pRingBuffer->ifront = (m_pRingBuffer->ifront + 1) % m_pRingBuffer->buffSize;
    }
    dstLen = currsize;*/
    if (m_pRingBuffer->ifront == m_pRingBuffer->iend)
    {
        //ReleaseMutex(mutexHandle);
        return NULL;
    }
    IFVFrameHeader_S *dest = (IFVFrameHeader_S *)(m_pRingBuffer->m_pBuff[m_pRingBuffer->ifront]);
    m_pRingBuffer->ifront = (m_pRingBuffer->ifront + 1) % m_pRingBuffer->buffSize;
    //ReleaseMutex(mutexHandle);
    return dest;  

}

bool CycleBuffer::InitQueue()
{
    m_pRingBuffer = new RINGBUFFER();
    m_pRingBuffer->ifront = m_pRingBuffer->iend = 0;
    m_pRingBuffer->buffSize = 15;//MAXCYCLEBUFFSIZE;
    m_pRingBuffer->m_pBuff = new unsigned long[MAXCYCLEBUFFSIZE];
    
    /*
    Q->iRead = 0;
    Q->iWrite = 0;
    for (int i = 0; i < MAXSIZE; i++)
    {
        Q->mdata[i].cdata = new char[MAXCYCLEBUFFSIZE];
        memset(Q->mdata[i].cdata, 0, MAXCYCLEBUFFSIZE);
        Q->mdata[i].iSize = 0;
    }
    */
    return true;
}

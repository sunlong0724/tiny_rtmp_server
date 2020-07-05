#include "hikConnect.h"
#include "stdio.h"
#include <string.h>

HikConn::HikConn(CycleBuffer* pcycleBuffer)
{
	m_lUserId = -1;
	BOOL initState = Init();
	m_pUserDataPtr = this;
	m_pRingBuffer = pcycleBuffer;
	
}
HikConn::~HikConn()
{
	CleanUp();
	
}
void HikConn::CleanUp()
{
	NET_DVR_Cleanup();
}
BOOL HikConn::Init()
{
	//
	return NET_DVR_Init();
}
LONG HikConn::Login()
{ 
	NET_DVR_SetConnectTime(5000, 3);
	NET_DVR_SetReconnect(10000, true);
	//if (!NET_DVR_SetConnectTime(10000, 10))
	{
	//	DWORD dwError = NET_DVR_GetLastError();
	//	DWORD s = dwError;
	}
	memset(&m_loginInfo, 0, sizeof(NET_DVR_USER_LOGIN_INFO));
	m_loginInfo.bUseAsynLogin = false;//不异步登录
	//m_loginInfo.cbLoginResult = NULL;
	strcpy(m_loginInfo.sDeviceAddress,"192.168.1.164");
	//m_loginInfo.byLoginMode = 2;
	strcpy(m_loginInfo.sUserName,"admin");
	strcpy(m_loginInfo.sPassword, "cvdev2018");
	m_loginInfo.wPort = 8000;
	memset(&m_deviceInfo, 0, sizeof(NET_DVR_DEVICEINFO_V40));
	//m_deviceInfo.struDeviceV30.byStartChan = 1;
	LONG userId = NET_DVR_Login_V40(&m_loginInfo, &m_deviceInfo);
	if (userId < 0)
	{
		DWORD dwError = NET_DVR_GetLastError();
		DWORD s = dwError;
	}
	
	/*
	NET_DVR_DEVICEINFO_V30 deviceInfoV30;
	memset(&deviceInfoV30, 0, sizeof(NET_DVR_DEVICEINFO_V30));
	deviceInfoV30.byStartChan = 1;
	m_lUserId = NET_DVR_Login_V30((char *)"192.168.1.167", 8000,(char *) "admin", (char *)"cvdev2018", &deviceInfoV30);
	if (m_lUserId < 0)
	{
		DWORD dwError = NET_DVR_GetLastError();
		DWORD s = dwError;
	}
	*/
	//m_pUserDataPtr->m_lUserId = m_lUserId;
	
	m_lUserId = userId;
	return m_lUserId;
}

void CALLBACK HikConn::RealDataPlayESCallback(
	LONG                      lPreviewHandle,
	NET_DVR_PACKET_INFO_EX* pstruPackInfo, void* pUser)
{
	//
	if (pstruPackInfo == NULL) return;
	if (pstruPackInfo->dwPacketSize == 0) return;
	if (pUser == NULL) return;
    
	HikConn* pUserData = (HikConn*)pUser;
	switch (pstruPackInfo->dwPacketType)
	{
		case 1:
		case 2:
		case 3:
		{
			IFVFrameHeader_S* pframe = new IFVFrameHeader_S();
			if (pstruPackInfo->dwPacketType == 1)
			{
				pframe->iFrame = 1;
			}
			else
			{
				pframe->iFrame = 0;
			}
			pframe->dataLen = pstruPackInfo->dwPacketSize;
			pframe->data = new unsigned char[pstruPackInfo->dwPacketSize];
			memcpy(pframe->data, pstruPackInfo->pPacketBuffer, pstruPackInfo->dwPacketSize);
			pframe->height = pstruPackInfo->wHeight;
			pframe->width = pstruPackInfo->wHeight;
			pUserData->m_pRingBuffer->pushBuffer(pframe);
			//printf("HIK receive %d video buffersize \r\n", pstruPackInfo->dwPacketSize);
            delete pframe->data;
            delete pframe;
			break;
		}
	}
}
void CALLBACK HikConn::RealDataCallback(
	LONG      lRealHandle,
	DWORD     dwDataType,
	BYTE* pBuffer,
	DWORD     dwBufSize,
	void* pUser
)
{
	//
	if (pUser == NULL) return;

	HikConn *pUserData = (HikConn*)pUser;
	switch (dwDataType)
	{
		caseNET_DVR_SYSHEAD:
		{
			break;
		}
	case NET_DVR_AUDIOSTREAMDATA:
	{
		break;
	}
		case NET_DVR_STREAMDATA:
		{
			//流数据
			//printf("HIK receive %d video buffersize \r\n", dwBufSize);
			
			//pUserData->m_pRingBuffer->pushBuffer( (char *)pBuffer);
			break;
		}
		default:
		{
			//
			break;
		}
	}
}

LONG HikConn::StartStream()
{
	//
	NET_DVR_PREVIEWINFO previewInfo;
	memset(&previewInfo, 0, sizeof(NET_DVR_PREVIEWINFO));
	previewInfo.lChannel = 1;
	m_lRealPlayId = NET_DVR_RealPlay_V40(m_lUserId, &previewInfo, NULL, (void *)m_pUserDataPtr);
	if (m_lRealPlayId >= 0)
	{
		//
		NET_DVR_SetESRealPlayCallBack(m_lRealPlayId, HikConn::RealDataPlayESCallback, (void*)m_pUserDataPtr);
	}
	return 1;
}

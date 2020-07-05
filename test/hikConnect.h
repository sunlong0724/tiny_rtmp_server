#pragma once
#include "hikvision/HCNetSDK.h"
#include "RingBuffer.h"

class HikConn
{
private:
	//
	LONG m_lUserId;
	NET_DVR_USER_LOGIN_INFO m_loginInfo;
	NET_DVR_DEVICEINFO_V40 m_deviceInfo;
	LONG m_lRealPlayId;

	class HikConn* m_pUserDataPtr;
	//REALDATACALLBACK
public:
	//
	CycleBuffer* m_pRingBuffer;
	HikConn(CycleBuffer*pcycleBuffer);
	~HikConn();
	BOOL Init();
	void CleanUp();
	LONG Login();
	LONG StartStream();
	static void CALLBACK RealDataPlayESCallback(
		LONG                      lPreviewHandle,
		NET_DVR_PACKET_INFO_EX* pstruPackInfo, void* pUser);
	static void CALLBACK  RealDataCallback(
		LONG      lRealHandle,
		DWORD     dwDataType,
		BYTE* pBuffer,
		DWORD     dwBufSize,
		void* pUser
	);

};

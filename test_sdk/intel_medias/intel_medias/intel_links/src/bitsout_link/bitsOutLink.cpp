
#include "inc/bitsout_link/bitsOutLink.h"
//#include "ReachEncDef.h"
//#include "ReachEncSdk.h"
#include "mfx_samples_config.h"
#include "pipeline_encode.h"
#include "pipeline_user.h"
#include "pipeline_region_encode.h"
#include "pipeline_decode.h"
#include "SDK_codec.h"
//#include "reachApp.h"
//#include "4sdi_socket_port.h"

#include "rh_stream_transport.h"
#include "web_middle_control.h"

extern BITSOUTLINK_INFO *g_pBitoutLinkInfo[8];

#include <sstream>
#include <string>
using namespace std;

std::string typeTostring(int x)
{
	string			sztemp;
	stringstream	sstemp;
	sstemp << x;
	sstemp >> sztemp;
	return sztemp;
}

typedef struct __HDB_FRAME_HEAD {
	unsigned int ID;								//=mmioFOURCC('4','D','S','P');
	unsigned int nTimeTick;    					//鏃堕棿鎴?
	unsigned int nFrameLength; 					//甯ч暱搴?
	unsigned int nDataCodec;   					//缂栫爜绫诲瀷
	//缂栫爜鏂瑰紡
	//瑙嗛 :mmioFOURCC('H','2','6','4');
	//闊抽 :mmioFOURCC('A','D','T','S');
	unsigned int nFrameRate;   					//瑙嗛  :甯х巼
	//闊抽 :閲囨牱鐜?(default:44100)
	//0---44.1KHz
	//1---44.1KHz
	//2---44.1KHz
	//3---48KHz
	unsigned int nWidth;       					//瑙嗛  :瀹?
	//闊抽 :閫氶亾鏁?(default:2)
	unsigned int nHight;       					//瑙嗛  :楂?
	//闊抽 :閲囨牱浣?(default:16)
	unsigned int nColors;      					//瑙嗛  :棰滆壊鏁? (default: 24)
	//闊抽 :闊抽鐮佺巼 (default:64000)
	unsigned int dwSegment;						//鍒嗗寘鏍囧織浣?
	unsigned int dwFlags;							//瑙嗛:  I 甯ф爣蹇?
	//闊抽:  淇濈暀
	unsigned int dwPacketNumber; 					//鍖呭簭鍙?
	unsigned int nOthers;      					//淇濈暀
} HDB_FRAME_HEAD;

/*发送到live-room的端口和index信息*/
// static VIDEO_PORT_INFO s_videoSendLiveRoom[DEC_NUM] = {
// 	{S_HD_LIVE_TF, LIVE_IPC1_INDEX},
// 	{S_HD_LIVE_SF, LIVE_IPC2_INDEX}
// };

/*发送到recoder-room的端口和index信息*/
// static VIDEO_PORT_INFO s_videoSendRecoderRoom[DEC_NUM] = {
// 	{S_HD_RECORD_TF, RECORD_IPC1_INDEX},
// 	{S_HD_RECORD_SF, RECORD_IPC2_INDEX}
// };

static Int32 reach_video_send_init(StreamServer_t *pstreamPrm, char *ip, int port, int sindex)
{
	if((NULL == pstreamPrm) || (NULL == ip)) {
//		nslog(NS_ERROR, "=======reach_video_send_init:param is NULL!\n");
		return -1;
	}

	memset(pstreamPrm, 0, sizeof(StreamServer_t));
#ifndef WIN32
	snprintf(pstreamPrm->ip, IP_LEN, "%s", ip);
#else
	sprintf_s(pstreamPrm->ip, IP_LEN, ip);
#endif
	pstreamPrm->port = port;

	pstreamPrm->sindex = sindex;

//	RH_EncoderMangerServerInit(pstreamPrm);

//	nslog(NS_DEBUG, "reach_video_send_init:ip:%s,port:%d,sindex:%d\n", pstreamPrm->ip, pstreamPrm->port, pstreamPrm->sindex);

	return 0;
}

static Int32 bitsOutLink_createQue(BITSOUTLINK_INFO *info)
{
	Int32 status = LINK_SOK;
	Int32 i = 0;
	
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->BistEmpFullQueHndl.isConnect = LINK_FALSE;

	if(info->QueLen > VIDBITSTREAM_MAX_BITSTREAM_BUFS || 0 == info->QueLen)
	{
		info->QueLen = VIDBITSTREAM_MAX_BITSTREAM_BUFS;
	}


	/*创建空包队列*/
	status = LINK_queCreate(&(info->BistEmpFullQueHndl.pEmpQue),info->QueLen);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create emp que is error!\n");
		return LINK_EFAIL;		
	}

	/*创建满包队列*/
	status = LINK_queCreate(&(info->BistEmpFullQueHndl.pFullQue),info->QueLen);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create full que is error!\n");
		return LINK_EFAIL;		
	}

	for(i = 0; i < info->QueLen; i++)
	{
		/*按默认1080P分配*/
		info->BitstreamBuff[i].Bitstream.Data = NULL;
		status = InitMfxBitstream(&(info->BitstreamBuff[i].Bitstream),VIDEO_WIDTH*VIDEO_HEIGHT);
		if(status != MFX_ERR_NONE)
		{
//			nslog(NS_ERROR,"InitMfxBitstream[%d] is error!\n",i);
			return LINK_EFAIL;
		}
		printf("===============BitstreamBuff[%d]:%p,%p,%d,%d\n",i,&info->BitstreamBuff[i],info->BitstreamBuff[i].Bitstream.Data
			,info->BitstreamBuff[i].Bitstream.DataLength,info->BitstreamBuff[i].Bitstream.DataOffset);
		status = LINK_quePut(&(info->BistEmpFullQueHndl.pEmpQue),(void *)(&(info->BitstreamBuff[i])),LINK_TIMEOUT_NONE);
		if(status != LINK_SOK)
		{
//			nslog(NS_ERROR,"LINK_quePut[%d] is error!\n",i);
			return LINK_EFAIL;
		}
	}

	return status;
	
}

static void bitsOutLink_DataCallBack(void *pvoid,char * pDataBuf,int iBufLen,long iQulality,int iDataType)
{

// 	if(NULL == pvoid)
// 	{
// //		nslog(NS_ERROR,"pvoid is NULL!\n");
// 		return;
// 	}
// 	
// 	if (pDataBuf == NULL || iBufLen <= 0)
// 	{
// //		nslog(NS_ERROR,"param error,[pDataBuf:%p][iBufLen:%d]!",pDataBuf, iBufLen);
// 		return;
// 	}
// 
//  	Int32 status = LINK_SOK;
//  	BITSOUTLINK_INFO *info = NULL;
// 
// 	Bitstream_Buf* pBitStrameBuff;
// 	RH_FRAMEHEAD_t frameHead;
// 	Uint32   frameCount = 0;
// 	Int32 bitsId = 0;
// 	Int32 index = 0;
// // 
// 	info = (BITSOUTLINK_INFO *)pvoid;
// 	if(!info->tskEnable || !info->BistEmpFullQueHndl.isConnect)
// 	{
// 		return;
// 	}
// // 	info->chNo--;
//      if(info->pCurBitstreamBuff == NULL)
//      {
//         /*获取空bitsbuff*/
// 		 for (int i = 0; i < 1; i++){
// 			 status = LINK_queGet(&(g_pBitoutLinkInfo[i]->BistEmpFullQueHndl.pEmpQue), (void **)(&pBitStrameBuff), LINK_TIMEOUT_NONE);
// 		 }
// 
// 		if (LINK_EFAIL == status)
// 		{
// 			 return;
// 		}
//         info->pCurBitstreamBuff = pBitStrameBuff;
//      }
// //  
//     HDB_FRAME_HEAD *pFH = NULL;
//     pFH = (HDB_FRAME_HEAD *)pDataBuf;
//     Int32 iFhLen = sizeof(HDB_FRAME_HEAD);
//     //独立包
// 
//     if(info->RecvFrameOffset >= info->pCurBitstreamBuff->Bitstream.MaxLength)
//     {
//          return;
//     }
// 
//     if(pFH->dwSegment == 3)
//     {
//         memcpy(info->pCurBitstreamBuff->Bitstream.Data, pDataBuf + iFhLen ,iBufLen - iFhLen);
//         info->pCurBitstreamBuff->Bitstream.DataLength = iBufLen - iFhLen;
//     }
//     //结束包
//     else if(pFH->dwSegment == 1)
//     {
//         memcpy(info->pCurBitstreamBuff->Bitstream.Data + info->RecvFrameOffset, pDataBuf + iFhLen, iBufLen - iFhLen);
//         info->RecvFrameOffset += (iBufLen - iFhLen);
//         info->pCurBitstreamBuff->Bitstream.DataLength = info->RecvFrameOffset;
//     }
//     //起始包
//     else if(pFH->dwSegment == 2)
//     {
//         memcpy(info->pCurBitstreamBuff->Bitstream.Data, pDataBuf + iFhLen, iBufLen - iFhLen);
//         info->RecvFrameOffset = (iBufLen - iFhLen);
//         return ;
//     }
//     //中间包
//     else if(pFH->dwSegment == 0)
//     {
//         memcpy(info->pCurBitstreamBuff->Bitstream.Data + info->RecvFrameOffset, pDataBuf + iFhLen, iBufLen - iFhLen);
//         info->RecvFrameOffset += (iBufLen - iFhLen);
//         return ;
//     }
//     info->RecvFrameOffset = 0;
// 	info->pCurBitstreamBuff->Bitstream.DataOffset = 0;
// 	info->frameCount++;
// 
// // 	static int a = 0;
// // 	if (/*info->pCurBitstreamBuff->isKeyFrame == 1 &&*/ a < 50)
// // 	{
// // 		a++;
// // 		string str = string("C:\h264_");
// // 		string sss = typeTostring(a);
// // 		str += sss;
// // 		str += ".h264";
// // 		FILE* fp = fopen(str.c_str(), "w+");
// // 		fwrite(info->pCurBitstreamBuff->Bitstream.Data, info->pCurBitstreamBuff->Bitstream.DataLength, 1, fp);
// // 		fclose(fp);
// // 
// // 	}
// 
// 	for (int i = 0; i < 1; i++)
// 	{
// 		LINK_quePut(&(g_pBitoutLinkInfo[i]->BistEmpFullQueHndl.pFullQue), (void *)info->pCurBitstreamBuff, LINK_TIMEOUT_FOREVER);
// 	}
// 	info->pCurBitstreamBuff = NULL;
// 	printf("put:%d\n", info->frameCount);

	return;
}


static void* bitsOutLink_initSDK(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}
	
	BITSOUTLINK_INFO *info = (BITSOUTLINK_INFO *)arg;
	int m_iHdConnState;
	Int32 status = LINK_SOK;
	char chDirPath[MAX_PATH] = {0};
	char ipcProcType[MAX_PATH] = {0};
	sprintf_s(chDirPath, "%s/%s/%s.log", "e:", info->ip, "EncSdks");


	char chDir[MAX_PATH] = { 0 };
	sprintf_s(chDir, "%s/%s", "e:", info->ip);
	CreateDirectoryA(chDir, NULL);

// 	CLogExport g_cEncServLog;
// 	g_cEncServLog.LogInit(chDirPath, false);
// 	if(info->ipcProcType == IPC_PROC_110)
// 	{
// 		info->ProtocolType = ENC_PROTOCOL_OLD_110;
// 		strcpy(ipcProcType,"Hikvision");
// 	}
// 	else if(info->ipcProcType == IPC_PROC_HC)
// 	{
// 		 info->ProtocolType = ENC_PROTOCOL_IPC;
// 		 strcpy(ipcProcType,"Hikvision");
// 	}
// 	else if(info->ipcProcType == IPC_PROC_DH)
// 	{
// 		 info->ProtocolType = ENC_PROTOCOL_IPC;
// 		 strcpy(ipcProcType,"Dahua");
// 	}
// 	else if(info->ipcProcType ==IPC_PROC_YS)
// 	{
// 		 info->ProtocolType = ENC_PROTOCOL_IPC;
// 		 strcpy(ipcProcType,"Uniview");
// 	}
// 	else
// 	{
// 		info->ProtocolType = ENC_PROTOCOL_IPC;
// 		strcpy(ipcProcType,"Hikvision");
// 	}

//	long m_HdHandleSdk = InitEncSdk(&g_cEncServLog, info->ProtocolType);
//	info->sdk = m_HdHandleSdk;
// 	if(m_HdHandleSdk == 0)
// 	{
//		nslog(NS_ERROR,"InitEncSdk is error!\n");
//		return NULL;
//	}
// 	if (strcmp(info->ip ,"192.168.4.152") != 0)
// 		return NULL;
// 	while( Connect(m_HdHandleSdk,(char *)info->ip,info->port,bitsOutLink_DataCallBack,(void *)info,HD_QUALITY_STREAM,-1,ipcProcType,info->usrName,info->passWd) != 0)
// 	{
//		nslog(NS_WARN,"Connect Encoder HD  failed\n");
// 		sleep(3);
// 	}
// 
// 	 m_iHdConnState = GetCnntState(m_HdHandleSdk);
// 	if (m_iHdConnState == ENC_CNNTING)
// 	{
// 		m_iHdConnState = ENC_NOCNNT;
// 	}
//
//	nslog(NS_INFO," Encoder HD Status is %d",m_iHdConnState);
// 
// 	ReqData(m_HdHandleSdk,HD_QUALITY_STREAM);

	LINK_tskDetach();
	LINK_tskExit(0);
	
	return NULL;
}

Int32 bitsOutLink_disable(void* arg)
{
	if (NULL == arg)
	{
		//		nslog(NS_ERROR,"arg is NULL\n");
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;
	BITSOUTLINK_INFO *info = (BITSOUTLINK_INFO *)arg;

	info->tskEnable = LINK_FALSE;

	return status;
}

Int32 bitsOutLink_enable(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;
	BITSOUTLINK_INFO *info = (BITSOUTLINK_INFO *)arg;

	info->tskEnable = LINK_TRUE;

	return status;
}

Int32 bitsOutLink_uninit(void* arg)
{
// 	BITSOUTLINK_INFO *info = (BITSOUTLINK_INFO *)arg;
// 	UnInitEncSdk(info->sdk, info->ProtocolType);
	return 0;
}


Int32 bitsOutLink_init(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return LINK_EFAIL;
	}

	Int32 status = LINK_SOK;
	BITSOUTLINK_INFO *info = (BITSOUTLINK_INFO *)arg;

	info->tskEnable = LINK_FALSE;

	status = bitsOutLink_createQue(info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"bitsOutLink_createQue is error!\n");
		return status;
	}

	/*初始化live-room发送*/
	Int32 port = 0;
	Int32 bitsId = 0;
	Int32 index = 0;
	char ip[IP_LEN] = "127.0.0.1";
	StreamServer_t streamPrm;
	bitsId = info->chNo;
	/*初始化live room*/
// 	index = s_videoSendLiveRoom[bitsId].index;
// 	port = s_videoSendLiveRoom[bitsId].port;
	if(index != -1) {
		reach_video_send_init(&streamPrm, ip, port, index);
	}
	/*初始化record room*/
//	index = s_videoSendRecoderRoom[bitsId].index;
//	port = s_videoSendRecoderRoom[bitsId].port;
// 	if(index != -1) {
// 		reach_video_send_init(&streamPrm, ip, port, index);
// 	}
    
#if 1
	/*创建编码启动任务*/
	status = LINK_tskCreate(&info->tskHandle,bitsOutLink_initSDK,BITSOUTLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create task is error!\n");
	}
#else
	bitsOutLink_initSDK(info);
#endif
	return status;
}




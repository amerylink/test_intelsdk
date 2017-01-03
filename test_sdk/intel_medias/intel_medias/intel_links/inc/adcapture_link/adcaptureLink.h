#ifndef _AD_CAPTURE_LINK_H_
#define _AD_CAPTURE_LINK_H_
#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"
#include <alsa/asoundlib.h>
//#include "linkVideo.h"
//#include "SDK_codec.h"

#define ADCAPTURE_LINK_MAX_ALLOC_BUFF_NUM    10
#define ADCAPTURE_LINK_DEFAULT_ALLOC_BUFF_NUM 6
#define ADCAPTURE_LINK_MSG_NUM					10


/*nullsrc link 创建参数*/
typedef struct{
	Uint8   DevName[100];
	Uint32  samplerate;
    Uint32  channel;
	Uint32  buffNum;
}ADCAPTURE_LINK_CREATE_INFO;


typedef struct
{
	Uint32 DevesId;
	LINK_TskHndl tskHandle;	
	Uint8 DevName[100];
	Uint32  samplerate;
    Uint32  channel;
	snd_pcm_t				*handle;
}ADCaptureLink_InstObj;

/*nullsrc link的数据信息*/
typedef struct{
	Bool			tskEnable;							//启动任务
	Bool			exitTsk;								//退出任务
	ADCAPTURE_LINK_CREATE_INFO createInfo;                     //创建参数
	LINK_TskHndl tskHandle;							//任务句柄
	LINK_TskHndl tskHandle_msg;	
	audio_buf 			frameBuff[ADCAPTURE_LINK_MAX_ALLOC_BUFF_NUM];
	LINK_EmpFullQueHndl EmpFullQueHndl;				//空满包队列句柄
	Uint32			     Msg[ADCAPTURE_LINK_MSG_NUM];
	LINK_EmpFullQueHndl MsgQueHandle;						//消息队列句柄
	ADCaptureLink_InstObj instObj;
}ADCAPTURE_LINK_INFO;

Int32 ADCaptureLink_createParaInit(ADCAPTURE_LINK_CREATE_INFO *createInfo);
Int32 ADCaptureLink_create(ADCAPTURE_LINK_INFO *info);
Int32 ADCaptureLink_release(ADCAPTURE_LINK_INFO *info);
Int32 ADCaptureLink_enable(ADCAPTURE_LINK_INFO *info);
Int32 ADCaptureLink_disable(ADCAPTURE_LINK_INFO *info);
#endif

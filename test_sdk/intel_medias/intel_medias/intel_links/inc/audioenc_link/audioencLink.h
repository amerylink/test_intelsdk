#ifndef _AUDIO_ENC_LINK_H_
#define _AUDIO_ENC_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"
#include "Media.h"



//#include "linkVideo.h"
//#include "SDK_codec.h"

#define AUDIOENC_LINK_MAX_ALLOC_BUFF_NUM        10
#define AUDIOENC_LINK_DEFAULT_ALLOC_BUFF_NUM    6
#define AUDIOENC_LINK_MSG_NUM					10


/*nullsrc link ��������*/
typedef struct{
	Uint8   DevName[100];
	Uint32  samplerate;
    Uint32  channel;
	Uint32  buffNum;
}AUDIOENC_LINK_CREATE_INFO;


typedef struct
{
	Uint32 DevesId;
	LINK_TskHndl tskHandle;	
	Uint8 DevName[100];
	Uint32  samplerate;
    Uint32  channel;
	TransContext TransCtx; 
}AudioEncLink_InstObj;

/*nullsrc link��������Ϣ*/
typedef struct{
	Bool			tskEnable;							//��������
	Bool			exitTsk;								//�˳�����
	AUDIOENC_LINK_CREATE_INFO createInfo;                     //��������
	LINK_TskHndl tskHandle;							//������
	LINK_TskHndl tskHandle_msg;	
	audio_buf 			frameBuff[AUDIOENC_LINK_MAX_ALLOC_BUFF_NUM];
	LINK_EmpFullQueHndl EmpFullQueHndl;				//���������о��
	LINK_EmpFullQueHndl *PreEmpFullQueHndl;	//֡���������о��
	LINK_EmpFullQueHndl MsgQueHandle;						//��Ϣ���о��
	AudioEncLink_InstObj instObj;
}AUDIOENC_LINK_INFO;

Int32 AudioEncLink_createParaInit(AUDIOENC_LINK_CREATE_INFO *createInfo);
Int32 AudioEncLink_create(AUDIOENC_LINK_INFO *info);
Int32 AudioEncLink_release(AUDIOENC_LINK_INFO *info);
Int32 AudioEncLink_enable(AUDIOENC_LINK_INFO *info);
Int32 AudioEncLink_disable(AUDIOENC_LINK_INFO *info);
Int32 AudioEncLink_connect(AUDIOENC_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);


#endif

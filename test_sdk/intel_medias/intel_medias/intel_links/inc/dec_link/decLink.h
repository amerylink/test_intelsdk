#ifndef _DEC_LINK_H_
#define _DEC_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"
#include "SDK_codec.h"


#define DEC_LINK_MAX_ALLOC_BUFF_NUM 10
#define DEC_LINK_DEFAULT_ALLOC_BUFF_NUM 6

/*dec link ��������*/
typedef struct{
	Uint32 queLen;
	Uint32 chNo;
}DEC_CREATE_INFO;

/*dec link��������Ϣ*/
typedef struct{
	Bool			tskEnable;							//��������
	Bool			exitTsk;								//�˳�����
	Bool			isDecHeadFinish;						//����ͷ��Ϣ�Ƿ����
	DEC_CREATE_INFO createInfo;                     //��������
	sAPI_DecoderInputParams decInputParam;           //�����������
	sAPI_EncoderInputParams AllocFramePara;		//����frame����
	LINK_TskHndl tskHandle;							//������
	LINK_EmpFullQueHndl *PreEmpFullQueHndl;	//���������о��
	FRAME_Buf 			frameBuff[DEC_LINK_MAX_ALLOC_BUFF_NUM];
	LINK_EmpFullQueHndl EmpFullQueHndl;				//���������о��
	int				id;
}DEC_LINK_INFO;

Int32 decLink_create(DEC_LINK_INFO *info);
Int32 decLink_release(DEC_LINK_INFO *info);
Int32 decLink_enable(DEC_LINK_INFO *info);
Int32 decLink_disable(DEC_LINK_INFO *info);
Int32 decLink_connect(DEC_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
void decLink_printf(DEC_LINK_INFO *info);
Int32 decLink_inputParaInit(sAPI_DecoderInputParams *pPrama);
Int32 decLink_createParaInit(DEC_CREATE_INFO *createInfo);
Int32 decLink_kill(DEC_LINK_INFO *info);

#endif

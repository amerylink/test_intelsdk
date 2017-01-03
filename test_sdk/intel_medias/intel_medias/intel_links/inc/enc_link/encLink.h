#ifndef _ENC_LINK_H_
#define _ENC_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"
#include "SDK_codec.h"
#include "Media.h"

/**
**	���붯̬����
*/
typedef struct __enc_dyn_para
{
	Uint32 nKeyframeIntelval;    		//Encode Key Frame Intelval	
	Uint32 nEncodeStreambit;           	 //Encode Stream bit kbs
	Uint32 nWidth;            				//Source YUV Width
	Uint32 nHeight;           				 //Source YUV Height
	Uint32 nEncodeFrameRate;            //Encode frame rate 1-30
}enc_Dyn_Para_t;

typedef struct __enc_skip_frame_info
{
	Uint32 start_time;
	Uint32 prev_fps;
	Uint32 prev_time;
	Uint32 frame_cnt;
	Uint32 frame_mis;
	Uint32 skip_flag;
}ENC_SKIP_FRAME_INFO;



typedef struct
{
	enc_Dyn_Para_t encDynPara;
	sAPI_EncoderInputParams encPara;					//�������
	//sAPI_EncoderInputParams encAllocFramePara;		//�������frame����
	Uint32		encQueNum;							//������г���
	LINK_TskHndl tskHandle;							//������
	Bool			tskEnable;							//��������
	Bool			exitTsk;								//�˳�����
	Bool			needSetPara;						//��Ҫ���ñ������
	Bool			IsJpegEnc;
	Bitstream_Buf BitstreamBuff[VIDBITSTREAM_MAX_BITSTREAM_BUFS];
	LINK_EmpFullQueHndl *FrameEmpFullQueHndl;			//֡���������о��
	LINK_EmpFullQueHndl BitsEmpFullQueHndl;				//�������������о��	
	ENC_SKIP_FRAME_INFO encSkipFrameInfo;
}ENC_LINK_INFO;

/*�ⲿ�ӿں�������*/
/*����link*/
Int32 encLink_create(ENC_LINK_INFO *info);
/*�ͷ�link*/
Int32 encLink_release(ENC_LINK_INFO *info);
/*����link*/
Int32 encLink_connect(ENC_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
/*ʹ��link*/
Int32 encLink_enable(ENC_LINK_INFO *info);
/*��ʹ��link*/
Int32 encLink_disable(ENC_LINK_INFO *info);
/*���ñ��붯̬����*/
Int32 encLink_setEncPara(ENC_LINK_INFO *info,enc_Dyn_Para_t *para);
/*��ʼ���������������*/
void encLink_EncInputParaInit(sAPI_EncoderInputParams *lp);

void encLink_printf(ENC_LINK_INFO *info);

#endif

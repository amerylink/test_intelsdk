#ifndef _ENC_LINK_H_
#define _ENC_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"
#include "SDK_codec.h"
#include "Media.h"

/**
**	编码动态参数
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
	sAPI_EncoderInputParams encPara;					//编码参数
	//sAPI_EncoderInputParams encAllocFramePara;		//编码分配frame参数
	Uint32		encQueNum;							//编码队列长度
	LINK_TskHndl tskHandle;							//任务句柄
	Bool			tskEnable;							//启动任务
	Bool			exitTsk;								//退出任务
	Bool			needSetPara;						//需要设置编码参数
	Bool			IsJpegEnc;
	Bitstream_Buf BitstreamBuff[VIDBITSTREAM_MAX_BITSTREAM_BUFS];
	LINK_EmpFullQueHndl *FrameEmpFullQueHndl;			//帧空满包队列句柄
	LINK_EmpFullQueHndl BitsEmpFullQueHndl;				//码流空满包队列句柄	
	ENC_SKIP_FRAME_INFO encSkipFrameInfo;
}ENC_LINK_INFO;

/*外部接口函数声明*/
/*创建link*/
Int32 encLink_create(ENC_LINK_INFO *info);
/*释放link*/
Int32 encLink_release(ENC_LINK_INFO *info);
/*连接link*/
Int32 encLink_connect(ENC_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
/*使能link*/
Int32 encLink_enable(ENC_LINK_INFO *info);
/*不使能link*/
Int32 encLink_disable(ENC_LINK_INFO *info);
/*设置编码动态参数*/
Int32 encLink_setEncPara(ENC_LINK_INFO *info,enc_Dyn_Para_t *para);
/*初始化编码器输入参数*/
void encLink_EncInputParaInit(sAPI_EncoderInputParams *lp);

void encLink_printf(ENC_LINK_INFO *info);

#endif

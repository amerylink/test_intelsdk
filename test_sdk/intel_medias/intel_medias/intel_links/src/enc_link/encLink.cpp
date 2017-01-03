
#include "inc/enc_link/encLink.h"
#include "mfx_samples_config.h"
#include "pipeline_encode.h"
#include "pipeline_user.h"
#include "pipeline_region_encode.h"
#include "pipeline_decode.h"
#include "SDK_codec.h"

#include <libavcodec/avcodec.h>  
#include <libavformat/avformat.h>  

#include "../DDrawDisplay.h"
extern CDDrawDisplay* g_pDisplay[8 + 2];

Uint8* g_pyv1080;
Uint8* g_pyv360;
extern FILE *fp1080;
extern FILE *fp360;
extern FILE *fpdec1;


void NV12toYV12(uint8_t *inyuv, uint8_t *outyuv, int numfmt, int width, int heihgt)
{
	int i, j, ysize;
	uint8_t *up_out = NULL, *vp_out = NULL, *yp_out = NULL, *yp_in = NULL, *up_in = NULL, *vp_in = NULL;


	ysize = width * heihgt;

	up_out = outyuv + ysize;
	vp_out = outyuv + ysize * 5 / 4;
	yp_out = outyuv;

	up_in = inyuv + ysize;
	vp_in = inyuv + ysize + 1;
	yp_in = inyuv;

	memcpy(yp_out, yp_in, ysize);

	for (i = 0; i < width / 2; i++){
		for (j = 0; j < heihgt / 2; j++){
			//			printf("i,j,numfmt: %d %d %d\n",i, j, numfmt);
			*up_out = *up_in;
			*vp_out = *vp_in;
			up_out += 1;
			vp_out += 1;
			up_in += 2;
			vp_in += 2;
		}
	}


}
#pragma comment(lib,"../Lib/ffmpeg/avcodec.lib")
#pragma comment(lib,"../Lib/ffmpeg/avformat.lib")
//#pragma comment(lib,"../Lib/ffmpeg/avfilter.lib")
#pragma comment(lib,"../Lib/ffmpeg/avutil.lib")

static Int32 encLink_processJpeg(ENC_LINK_INFO *info,video_info_t *pEnc_info);

void encLink_EncInputParaInit(sAPI_EncoderInputParams *lp)
{
	memset(lp,0,sizeof(sAPI_EncoderInputParams));
	lp->nEncoderType=1;
	lp->nMemoryType=SYSTEM_MEMORY;
	lp->nSrcFormat=1;
	lp->nEncodeFormat=0;
	lp->nEncodeProfile=ENCODER_High_Profile;
	lp->nEncodeLevels=ENCODER_Profile_Levels4;
	lp->nEncodeStreamType=0;
	lp->nEncodeQP=10;
	lp->nKeyframeIntelval=30;
	lp->nEncodeStreambit=4000;
	lp->nEncodeFrameRate=30.0;
	lp->nBuffNum		= LINK_DEFAULT_ALLOC_BUFF_NUM;
	lp->nSrcWidth=VIDEO_WIDTH;	
	lp->nSrcHeight=VIDEO_HEIGHT;
	lp->nDstWidth=VIDEO_WIDTH;	
	lp->nDstHeight=VIDEO_HEIGHT;
	lp->nDstX=0;	
	lp->nDstY=0;
	lp->pHandle=NULL;

}

static int encLink_CreatInit(ENC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}
	
	info->tskEnable = LINK_FALSE;
	info->exitTsk = LINK_FALSE;
	info->needSetPara = LINK_FALSE;
	info->FrameEmpFullQueHndl = NULL;

	info->encSkipFrameInfo.frame_cnt=0;
	info->encSkipFrameInfo.start_time=0;
	info->encSkipFrameInfo.skip_flag=0;
	info->encSkipFrameInfo.prev_time=0;
	info->encSkipFrameInfo.prev_fps=0;
	info->encSkipFrameInfo.frame_mis=0;

	g_pyv1080 = (Uint8*)malloc(1920*1080 * 3 / 2);
	g_pyv360 = (Uint8*)malloc(1920 * 1080 * 3 / 2);

	return status;
}
static Int32 encLink_createQue(ENC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	Int32 i = 0;
	
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->BitsEmpFullQueHndl.isConnect = LINK_FALSE;

	if(info->encQueNum > VIDBITSTREAM_MAX_BITSTREAM_BUFS || 0 == info->encQueNum)
	{
		info->encQueNum = VIDBITSTREAM_MAX_BITSTREAM_BUFS;
	}


	/*创建空包队列*/
	status = LINK_queCreate(&(info->BitsEmpFullQueHndl.pEmpQue),info->encQueNum);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create emp que is error!\n");
		return LINK_EFAIL;		
	}

	/*创建满包队列*/
	status = LINK_queCreate(&(info->BitsEmpFullQueHndl.pFullQue),info->encQueNum);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create full que is error!\n");
		return LINK_EFAIL;		
	}

	for(i = 0; i < info->encQueNum; i++)
	{
		/*按默认1080P分配*/
		info->BitstreamBuff[i].Bitstream.Data = NULL;
		status = InitMfxBitstream(&(info->BitstreamBuff[i].Bitstream),VIDEO_WIDTH*VIDEO_HEIGHT);
		if(status != MFX_ERR_NONE)
		{
//			nslog(NS_ERROR,"InitMfxBitstream[%d] is error!\n",i);
			return LINK_EFAIL;
		}

		info->BitstreamBuff[i].channelNum = info->encPara.nEncNo;
		printf("===============BitstreamBuff[%d]:%p,%p,%d,%d\n",i,&info->BitstreamBuff[i],info->BitstreamBuff[i].Bitstream.Data
			,info->BitstreamBuff[i].Bitstream.DataLength,info->BitstreamBuff[i].Bitstream.DataOffset);
		status = LINK_quePut(&(info->BitsEmpFullQueHndl.pEmpQue),(void *)(&(info->BitstreamBuff[i])),LINK_TIMEOUT_NONE);
		if(status != LINK_SOK)
		{
//			nslog(NS_ERROR,"LINK_quePut[%d] is error!\n",i);
			return LINK_EFAIL;
		}
	}

	return status;
	
}

#define 	INTERVAL_TIME      1000
#define MAX_INTERVAL_TIME	660000
#define	MAX_MP_TIME		720000
#define 	MP_INTERVAL_TIME      3000
#define  STD_FPS_30			2997
#define  STD_FPS_25			2500
#define  STD_FPS_1			100
#define FRAME_MAX_CNT		40000

static Bool  EncLink_doSkipFrame(ENC_LINK_INFO *info)
{
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_TRUE;
	}
	Uint32 fps=0;
	Uint32 timeout=0;
	Uint32 std_fps=0;
	Uint32 temp_frame_cnt=0;
	Uint32 currTime =  link_get_run_time();
	if( 0 ==  info->encSkipFrameInfo.start_time  ||  info->encSkipFrameInfo.prev_fps != (Uint32)info->encPara.nEncodeFrameRate){ 
		info->encSkipFrameInfo.prev_fps= (Uint32)info->encPara.nEncodeFrameRate;
		info->encSkipFrameInfo.start_time = currTime;
		info->encSkipFrameInfo.frame_cnt=0;
		info->encSkipFrameInfo.frame_mis=0;
		info->encSkipFrameInfo.skip_flag = currTime;
	}

	if( 30 == (Uint32)info->encPara.nEncodeFrameRate){
		std_fps = STD_FPS_30;
	}
	else if(1 == (Uint32)info->encPara.nEncodeFrameRate)
	{
		std_fps = STD_FPS_1;
	}
	else{
		std_fps = STD_FPS_25;
	}
	info->encSkipFrameInfo.frame_cnt++;

	timeout = (currTime - info->encSkipFrameInfo.start_time);
	if (timeout == 0)
		timeout = 1;

	if((info->encSkipFrameInfo.frame_cnt > FRAME_MAX_CNT) || (timeout > (FRAME_MAX_CNT*100*1000)/std_fps)){
		
		Uint32 temp_fps2 = ((info->encSkipFrameInfo.frame_mis+info->encSkipFrameInfo.frame_cnt)*1000*100)/(timeout);	
		
		printf("------11chId=%d,Total Frame=%d, Time=%d, frame_mis=%d-,AveFps=%0.3f-----Infps=%d------\n",
 			info->encPara.nEncNo,info->encSkipFrameInfo.frame_cnt,timeout,(info->encSkipFrameInfo.frame_mis),
 			(float)(info->encSkipFrameInfo.frame_cnt*1000.0)/(link_get_run_time()-info->encSkipFrameInfo.skip_flag),temp_fps2);
		info->encSkipFrameInfo.frame_cnt = 0;
		info->encSkipFrameInfo.start_time = currTime;
		info->encSkipFrameInfo.frame_mis = 0;
		info->encSkipFrameInfo.skip_flag=currTime;
		timeout = 1;
	}
	temp_frame_cnt=(std_fps*timeout)/(100*1000);

	if(  info->encSkipFrameInfo.frame_cnt  > (temp_frame_cnt+1 )){
		info->encSkipFrameInfo.frame_mis++;
		info->encSkipFrameInfo.frame_cnt--;		 
		return LINK_TRUE;
	}


	if(  (currTime -  info->encSkipFrameInfo.prev_time ) > 6000){
		int temp_fps= (info->encSkipFrameInfo.frame_cnt*1000*100)/(timeout);
		if (info->encPara.nEncNo == 0)
				printf("------nID =%d, Time=%d,------>AveFPS=%d.......------\n",
					info->encPara.nEncNo,timeout,temp_fps);
		info->encSkipFrameInfo.prev_time = currTime;
	}

	return LINK_FALSE;
}

static void* EncLink_Process(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}

	Int32 status = LINK_SOK;
	Bool skipStatus = LINK_FALSE;
	ENC_LINK_INFO *info = (ENC_LINK_INFO *)arg;
	video_info_t enc_info;
	int i = 0;

//	prctl(PR_SET_NAME, "EncLink_Process", NULL, NULL, NULL);
	
	while(!(info->exitTsk))
	{
		if (info->encPara.nEncNo == 0)
		{
			printf("111111aeettttttteadfasdfeeeaa1111111\n");
		}
		/*编码没有启动，不执行编码*/
		if(LINK_FALSE == info->tskEnable)
		{
//			usleep(30);
			Sleep(3);
			continue;
		}

		if((NULL == info->FrameEmpFullQueHndl) || (LINK_FALSE == info->FrameEmpFullQueHndl->isConnect))
		{
//			usleep(30);
			Sleep(3);
			continue;
		}


		if(LINK_FALSE == info->BitsEmpFullQueHndl.isConnect)
		{
//			usleep(30);
			Sleep(3);
			continue;
		}
		/*获取输入buff*/
		status = LINK_queGet(&(info->FrameEmpFullQueHndl->pFullQue),(void **)(&(enc_info.pEncSurfaces)),LINK_TIMEOUT_FOREVER);
		if(LINK_EFAIL == status)
		{
//			usleep(10);

			Sleep(1);
			continue;
		}
        
#if 0
		if(enc_info.pEncSurfaces->channelNum != info->encPara.nEncNo)
		{
			LINK_quePut(&(info->FrameEmpFullQueHndl->pEmpQue),(void *)(enc_info.pEncSurfaces),LINK_TIMEOUT_NONE);
			continue;			
		}
#endif

		skipStatus  = EncLink_doSkipFrame(info);
		if(skipStatus)
		{
			LINK_quePut(&(info->FrameEmpFullQueHndl->pEmpQue),(void *)(enc_info.pEncSurfaces),LINK_TIMEOUT_FOREVER);
			continue;			
		}
   
		/*获取输出buff*/
		status = LINK_queGet(&(info->BitsEmpFullQueHndl.pEmpQue),(void **)(&(enc_info.pEncBitStrameBuff)),LINK_TIMEOUT_FOREVER);
		if(LINK_EFAIL == status)
		{
//			usleep(10);
			Sleep(1);
			continue;
		}

//		printf("=========%p,%p\n",enc_info.pEncSurfaces,enc_info.pEncSurfaces->pSurfaces);

		if(LINK_FALSE == info->IsJpegEnc)
		{
			/*编码参数设置*/
			if((LINK_TRUE == info->needSetPara) ||
				(enc_info.pEncSurfaces->pSurfaces->Info.CropW != info->encPara.nDstWidth) ||
				(enc_info.pEncSurfaces->pSurfaces->Info.CropH != info->encPara.nDstHeight) ||
				(enc_info.pEncSurfaces->pSurfaces->Info.CropX != info->encPara.nDstX) ||
				(enc_info.pEncSurfaces->pSurfaces->Info.CropY != info->encPara.nDstY))
			{
				/*测试*/
				#if 0
				enc_info.pEncSurfaces->pSurfaces->Info.CropW = 1280;
				enc_info.pEncSurfaces->pSurfaces->Info.CropH = 720;
				#endif
				if(LINK_TRUE == info->needSetPara)
				{
					info->encPara.nDstWidth = info->encDynPara.nWidth;
					info->encPara.nDstHeight = info->encDynPara.nHeight;
					info->encPara.nEncodeStreambit = info->encDynPara.nEncodeStreambit;
					info->encPara.nEncodeFrameRate = info->encDynPara.nEncodeFrameRate;
					info->encPara.nKeyframeIntelval = info->encDynPara.nKeyframeIntelval;
					info->needSetPara = LINK_FALSE;
				}

//				printf("============%d,%d,%d,%d\n",enc_info.pEncSurfaces->pSurfaces->Info.CropX,enc_info.pEncSurfaces->pSurfaces->Info.CropY,
//					enc_info.pEncSurfaces->pSurfaces->Info.CropW,enc_info.pEncSurfaces->pSurfaces->Info.CropH);

				if(enc_info.pEncSurfaces->pSurfaces->Info.CropW != info->encPara.nDstWidth)
				{
					info->encPara.nDstWidth = enc_info.pEncSurfaces->pSurfaces->Info.CropW;
				}

				if(enc_info.pEncSurfaces->pSurfaces->Info.CropH != info->encPara.nDstHeight)
				{
					info->encPara.nDstHeight = enc_info.pEncSurfaces->pSurfaces->Info.CropH;
				}

				if(enc_info.pEncSurfaces->pSurfaces->Info.CropX != info->encPara.nDstX)
				{
					info->encPara.nDstX = enc_info.pEncSurfaces->pSurfaces->Info.CropX;
				}

				if(enc_info.pEncSurfaces->pSurfaces->Info.CropY != info->encPara.nDstY)
				{
					info->encPara.nDstY = enc_info.pEncSurfaces->pSurfaces->Info.CropY;
				}

				enc_info.EncBitStrameBuffList.numBufs = 0;
				enc_info.EncFrameBuffList.numBufs = 0;		
				status = API_IntelMedia_SDK_HW_EncResetPara((void *)&info->encPara,(void *)&enc_info);
//				if(0 != status)
//				{
//					nslog(NS_ERROR,"reset Enc Para is error!\n");
//				}

				/*放空包到bit满包队列*/
				if(enc_info.EncBitStrameBuffList.numBufs > 0)
				{
					for(i = 0; i < enc_info.EncBitStrameBuffList.numBufs; i++)
					{
						status = LINK_quePut(&(info->BitsEmpFullQueHndl.pEmpQue),(void *)(enc_info.EncBitStrameBuffList.bufs[i]),LINK_TIMEOUT_NONE);
						if(LINK_EFAIL == status)
						{
//							nslog(NS_ERROR,"Put bits pEmpQue is error!\n");
						}
					}
				}
				/*放空包到frame空包队列*/
				if(enc_info.EncFrameBuffList.numBufs > 0)
				{
					for(i = 0; i < enc_info.EncFrameBuffList.numBufs; i++)
					{
						printf("=============enc_info.EncFrameBuffList.bufs[%d]:%p\n",i,enc_info.EncFrameBuffList.bufs[i]);
						status = LINK_quePut(&(info->FrameEmpFullQueHndl->pEmpQue),(void *)(enc_info.EncFrameBuffList.bufs[i]),LINK_TIMEOUT_NONE);
						if(LINK_EFAIL == status)
						{
//							nslog(NS_ERROR,"Put bits pEmpQue is error!\n");
						}
					}
				}
			}
			
		}

		enc_info.EncBitStrameBuffList.numBufs = 0;
		enc_info.EncFrameBuffList.numBufs = 0;
		if(LINK_FALSE == info->IsJpegEnc)
		{
			if (enc_info.pEncSurfaces->pSurfaces->Info.CropW == 1920){
				memset(g_pyv1080, 0, 1920 * 1080 * 3 / 2);
				NV12toYV12(enc_info.pEncSurfaces->pSurfaces->Data.Y, g_pyv1080, 1, enc_info.pEncSurfaces->pSurfaces->Info.CropW, enc_info.pEncSurfaces->pSurfaces->Info.CropH);
 				g_pDisplay[0]->DisplayVideo((char*)g_pyv1080/*enc_info.pEncSurfaces->pSurfaces->Data.Y*/, enc_info.pEncSurfaces->pSurfaces->Info.CropW*enc_info.pEncSurfaces->pSurfaces->Info.CropH * 3 / 2);
// 				fwrite((char*)g_pyv1080/*enc_info.pEncSurfaces->pSurfaces->Data.Y*/, enc_info.pEncSurfaces->pSurfaces->Info.CropW*enc_info.pEncSurfaces->pSurfaces->Info.CropH * 3 / 2, 1, fp1080);

			}
			else if (enc_info.pEncSurfaces->pSurfaces->Info.CropW == 640)
			{
				memset(g_pyv360, 0, 1920 * 1080 * 3 / 2);
				NV12toYV12(enc_info.pEncSurfaces->pSurfaces->Data.Y, g_pyv360, 1, enc_info.pEncSurfaces->pSurfaces->Info.CropW, enc_info.pEncSurfaces->pSurfaces->Info.CropH);
				g_pDisplay[1]->DisplayVideo((char*)g_pyv360/*enc_info.pEncSurfaces->pSurfaces->Data.Y*/, enc_info.pEncSurfaces->pSurfaces->Info.CropW*enc_info.pEncSurfaces->pSurfaces->Info.CropH * 3 / 2);

			}

			if (info->encPara.nEncNo == 0){
				int t = link_get_run_time();
				status = API_IntelMedia_SDK_HW_EncFrame((void *)&info->encPara, (void *)&enc_info);
				int t2 = link_get_run_time();
				printf("%d----------------\n", t2 - t);
			}
			else
			{
				status = API_IntelMedia_SDK_HW_EncFrame((void *)&info->encPara, (void *)&enc_info);
			}
//			if(0 != status)
//			{
//				nslog(NS_ERROR,"enc Frame is error!\n");
//			}
		}
		else
		{
			status = encLink_processJpeg(info,&enc_info);
			if(0 != status)
			{
//				nslog(NS_ERROR,"processJpeg is error!\n");
			}
		}

		
		/*放瞒包到bit满包队列*/
		if(enc_info.EncBitStrameBuffList.numBufs > 0)
		{
			for(i = 0; i < enc_info.EncBitStrameBuffList.numBufs; i++)
			{
//				printf("===============enc_info.EncBitStrameBuffList.bufs[%d]:%p,%p,%d,%d\n",i,enc_info.EncBitStrameBuffList.bufs[i],enc_info.EncBitStrameBuffList.bufs[i]->Bitstream.Data
//					,enc_info.EncBitStrameBuffList.bufs[i]->Bitstream.DataLength,enc_info.EncBitStrameBuffList.bufs[i]->Bitstream.DataOffset);
				status = LINK_quePut(&(info->BitsEmpFullQueHndl.pFullQue),(void *)(enc_info.EncBitStrameBuffList.bufs[i]),LINK_TIMEOUT_NONE);
				if(LINK_EFAIL == status)
				{
//					nslog(NS_ERROR,"Put bits FullQue is error!\n");
				}
			}
		}
		/*放空包到frame空包队列*/
		if(enc_info.EncFrameBuffList.numBufs > 0)
		{
			for(i = 0; i < enc_info.EncFrameBuffList.numBufs; i++)
			{
//				printf("=============enc_info.EncFrameBuffList.bufs[%d]:%p\n",i,enc_info.EncFrameBuffList.bufs[i]);
				status = LINK_quePut(&(info->FrameEmpFullQueHndl->pEmpQue),(void *)(enc_info.EncFrameBuffList.bufs[i]),LINK_TIMEOUT_NONE);
				if(LINK_EFAIL == status)
				{
//					nslog(NS_ERROR,"Put bits pEmpQue is error!\n");
				}
			}
		}

	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}

static void encLink_420spTo420(FRAME_Buf* frame,Uint8 *pdata)
{
	mfxFrameInfo& pSrcInfo = frame->pSurfaces->Info;
	mfxFrameData& pSrcData = frame->pSurfaces->Data;	

	Uint32 xStart= pSrcInfo.CropX;
	Uint32 yStart = pSrcInfo.CropY;
	Uint32 w = pSrcInfo.CropW;   
	Uint32 h = pSrcInfo.CropH;    
	Uint32 pitch = pSrcData.Pitch;
	Uint32 x,y,u = 0,v = 0,yPos;
	Uint8 *uvSrcPtr = (Uint8 *)pSrcData.UV;
	Uint8 *ySrcPtr = (Uint8 *)pSrcData.Y;
	Uint8 *yPtr = pdata;
	Uint8 *uPtr = (Uint8 *)(yPtr + (w * h));
	Uint8 *vPtr = (Uint8 *)(uPtr + ((w*h) >> 2));

	u = 0;
	v = 0;
	for(y = (yStart >> 1); y < ((yStart + h) >> 1); y++)
	{
		ySrcPtr = (Uint8 *)(pSrcData.Y + (y << 1)*pitch + xStart);	
		yPtr = (Uint8 *)(pdata + w*(y << 1));
		memcpy(yPtr,ySrcPtr,w);
		ySrcPtr = (Uint8 *)(pSrcData.Y + ((y << 1) + 1)*pitch + xStart);
		yPtr = (Uint8 *)(pdata + w*((y << 1) + 1));
		memcpy(yPtr,ySrcPtr,w);
		uvSrcPtr = (Uint8 *)(pSrcData.UV + y*pitch);
		for(x = xStart; x < (xStart + w);)
		{
			if((x % 2) == 0)
				uPtr[u++] =  uvSrcPtr[x++];
			else
				vPtr[v++] =  uvSrcPtr[x++];
			
		}
	}
	
}

static Int32 encLink_processJpeg(ENC_LINK_INFO *info,video_info_t *pEnc_info)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info || NULL == pEnc_info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	AVFormatContext* pFormatCtx;
	AVOutputFormat* fmt;  
	AVStream* video_st;  
	AVCodecContext* pCodecCtx;  
	AVCodec* pCodec; 

	Uint8* picture_buf;  
	AVFrame* picture;  
	AVPacket pkt;  
	int y_size;  
	int got_picture=0;  
	int size; 
	Uint8 * pJpegBuff = (Uint8 * )pEnc_info->pEncBitStrameBuff->Bitstream.Data;
	mfxFrameInfo& pSrcInfo = pEnc_info->pEncSurfaces->pSurfaces->Info;

	int ret=0;
	static int first = 0;
	/*
	av_register_all();

	pFormatCtx = avformat_alloc_context();

	//Guess format  
	fmt = av_guess_format("mjpeg", NULL, NULL); 

	pFormatCtx->oformat = fmt; 

	video_st = avformat_new_stream(pFormatCtx, 0);  
	if (video_st==NULL){ 
//		nslog(NS_ERROR,"avformat_new_stream is error\n");  
	       return LINK_EFAIL;  
	}  
	pCodecCtx = video_st->codec;  
	pCodecCtx->codec_id = fmt->video_codec;  
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;  
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;  

	pCodecCtx->width = pSrcInfo.CropW;    
	pCodecCtx->height = pSrcInfo.CropH; 

	pCodecCtx->time_base.num = 1;

	pCodecCtx->time_base.den = 25;

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);  
	if (!pCodec){  
//		nslog(NS_ERROR,"Codec not found.\n");  
		return LINK_EFAIL;  
	}  
	if (avcodec_open2(pCodecCtx, pCodec,NULL) < 0){  
//		nslog(NS_ERROR,"Could not open codec.\n");  
		return LINK_EFAIL;  
	} 

	picture = av_frame_alloc();  
	size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);  
	picture_buf = (uint8_t *)av_malloc(size);  
	if (!picture_buf)  
	{ 
//		nslog(NS_ERROR,"Could not av_malloc picture_buf.\n"); 
		return LINK_EFAIL;  
	}  
	avpicture_fill((AVPicture *)picture, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

	y_size = pCodecCtx->width * pCodecCtx->height;  
	av_new_packet(&pkt,y_size*3);  

	//Write Header  
	//avformat_write_header(pFormatCtx,NULL);  

	encLink_420spTo420(pEnc_info->pEncSurfaces,picture_buf);
	  
	picture->data[0] = picture_buf;              // Y  
	picture->data[1] = picture_buf+ y_size;      // U   
	picture->data[2] = picture_buf+ y_size*5/4;  // V  

	//Encode  
	ret = avcodec_encode_video2(pCodecCtx, &pkt,picture, &got_picture);  
	if(ret < 0){  
//		nslog(NS_ERROR,"Encode Error.\n");  
		return LINK_EFAIL;  
	}  
	if (got_picture==1){  
		//pkt.stream_index = video_st->index;  
		 //ret = av_write_frame(pFormatCtx, &pkt);  
		 memcpy(pJpegBuff,pkt.data,pkt.size);
		pEnc_info->pEncBitStrameBuff->Bitstream.DataOffset = 0;
		pEnc_info->pEncBitStrameBuff->Bitstream.DataLength = pkt.size;
		pEnc_info->pEncBitStrameBuff->frameWidth = pCodecCtx->width;
		pEnc_info->pEncBitStrameBuff->frameHeight = pCodecCtx->height;
		pEnc_info->pEncBitStrameBuff->isKeyFrame = 1;
	}  

	av_free_packet(&pkt);  
	//Write Trailer  
	//av_write_trailer(pFormatCtx);  

	if (video_st){  
		avcodec_close(video_st->codec);  
		av_free(picture);  
		av_free(picture_buf);  
	}  
	avio_close(pFormatCtx->pb);  
	avformat_free_context(pFormatCtx); 
	*/
	pEnc_info->EncBitStrameBuffList.bufs[pEnc_info->EncBitStrameBuffList.numBufs] = pEnc_info->pEncBitStrameBuff;
	pEnc_info->EncBitStrameBuffList.numBufs++;

	pEnc_info->EncFrameBuffList.bufs[pEnc_info->EncFrameBuffList.numBufs] = pEnc_info->pEncSurfaces;
	pEnc_info->EncFrameBuffList.numBufs++;

	return status;

}

Int32 encLink_create(ENC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}
	/*初始化*/
	status = encLink_CreatInit(info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"encLink_CreatInit is error!\n");
		return LINK_EFAIL;		
	}
	
	/*创建编码输出队列*/
	status = encLink_createQue(info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create que is error!\n");
		return LINK_EFAIL;		
	}
	
	if(LINK_FALSE == info->IsJpegEnc)
	{
		/*创建编码器*/
		status = API_IntelMedia_SDK_HW_EncInitial((void *)&info->encPara);
		if(0 != status)
		{
//			nslog(NS_ERROR,"enc init is error!\n");
		}
	}

	/*创建编码启动任务*/
	status = LINK_tskCreate(&info->tskHandle,EncLink_Process,ENCLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create task is error!\n");
	}

	return status;
}

Int32 encLink_enable(ENC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->tskEnable = LINK_TRUE;
	
	return status;
}

Int32 encLink_disable(ENC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->tskEnable = LINK_FALSE;
	
	return status;
}

Int32 encLink_connect(ENC_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info || NULL == QueHndl)
	{
//		nslog(NS_ERROR,"info or QueHndl is NULL\n");
		return LINK_EFAIL;
	}

	info->FrameEmpFullQueHndl = QueHndl;

	info->FrameEmpFullQueHndl->isConnect = LINK_TRUE;

	return status;
}

Int32 encLink_release(ENC_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	Int32 i = 0;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->exitTsk = LINK_TRUE;

	LINK_tskJoin(&info->tskHandle);

	API_IntelMedia_SDK_HW_EncRelease(&info->encPara);
	//API_IntelMedia_SDK_HW_EncRelease(info->encAllocFramePara);

	for(i = 0; i < info->encQueNum; i++)
		WipeMfxBitstream(&(info->BitstreamBuff[i].Bitstream));

	LINK_queDelete(&(info->BitsEmpFullQueHndl.pEmpQue));
	LINK_queDelete(&(info->BitsEmpFullQueHndl.pFullQue));
	
	info->FrameEmpFullQueHndl = NULL;

	return status;
}

Int32 encLink_setEncPara(ENC_LINK_INFO *info,enc_Dyn_Para_t *para)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info || NULL == para)
	{
//		nslog(NS_ERROR,"info or para is NULL\n");
		return LINK_EFAIL;
	}

	info->encDynPara.nWidth = para->nWidth;
	info->encDynPara.nHeight = para->nHeight;
	info->encDynPara.nEncodeStreambit = para->nEncodeStreambit;
	info->encDynPara.nEncodeFrameRate = para->nEncodeFrameRate;
	info->encDynPara.nKeyframeIntelval = para->nKeyframeIntelval;	

	info->needSetPara = LINK_TRUE;

	return status;
}

void encLink_printf(ENC_LINK_INFO *info)
{
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return;
	}
	
//	nslog(NS_INFO,"==============encLink param %d:==============\n",info->encPara.nEncNo);

//	nslog(NS_INFO,
// 		"encQueNum:%d\n"
// 		"tskEnable:%d\n"
// 		"exitTsk:%d\n"
// 		,info->encQueNum,
// 		info->tskEnable,
// 		info->exitTsk
// 		);
	
// 	nslog(NS_INFO,"=============encLink input param:===============\n");
// 	nslog(NS_INFO,
// 		"nEncoderType:%d\n"
// 		"nMemoryType:%d\n"
// 		"nSrcFormat:%d\n"
// 		"nEncodeFormat:%d\n"
// 		"nEncodeProfile:%d\n"
// 		"nEncodeLevels:%d\n"
// 		"nEncodeQP:%d\n"
// 		"nKeyframeIntelval:%d\n"
// 		"nEncodeStreambit:%d\n"
// 		"nWidth:%d\n"
// 		"nHeight:%d\n"
// 		"x:%d\n"
// 		"y:%d\n"
// 		"nBuffNum:%d\n"
// 		"nEncodeFrameRate:%f\n",
// 		info->encPara.nEncoderType,
// 		info->encPara.nMemoryType,
// 		info->encPara.nSrcFormat,
// 		info->encPara.nEncodeFormat,
// 		info->encPara.nEncodeProfile,
// 		info->encPara.nEncodeLevels,
// 		info->encPara.nEncodeQP,
// 		info->encPara.nKeyframeIntelval,
// 		info->encPara.nEncodeStreambit,
// 		info->encPara.nDstWidth,
// 		info->encPara.nDstHeight,
// 		info->encPara.nDstX,
// 		info->encPara.nDstY,
// 		info->encPara.nBuffNum,
// 		info->encPara.nEncodeFrameRate
// 		);
// 
// 	nslog(NS_INFO,"\n=============encLink BitsQue:isConnect:%d============\n",info->BitsEmpFullQueHndl.isConnect);
// 	
// 	nslog(NS_INFO,"pEmpQue:\n");
// 	nslog(NS_INFO,
// 		"len:%d\n"
// 		"count:%d\n",
// 		info->BitsEmpFullQueHndl.pEmpQue.len,
// 		info->BitsEmpFullQueHndl.pEmpQue.count
// 		);
// 
// 	nslog(NS_INFO,"pFullQue:\n");
// 	nslog(NS_INFO,
// 		"len:%d\n"
// 		"count:%d\n",
// 		info->BitsEmpFullQueHndl.pFullQue.len,
// 		info->BitsEmpFullQueHndl.pFullQue.count
// 		);

	return;
	
}



/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2005-2014 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#include "mfx_samples_config.h"

#include <memory>
#include "pipeline_encode.h"
#include "pipeline_user.h"
#include "pipeline_region_encode.h"
#include "pipeline_decode.h"
#include "SDK_codec.h"
#include <stdarg.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#if 0
/**
**	与发送线程交互结构类型
*/
typedef struct __audio_send_info
{
	char *frame_buf[AUDIO_FRAME_SENDBUF_NUM];		//缓冲buffer
	int	 data_len[AUDIO_FRAME_SENDBUF_NUM];	 //buf长度 需要发送的数据长度
	int	 write_no;						//写buf序号
	int  read_no;						//读buf序号
}audio_send_info_t;
#define MV_MODE      1

#define VAL_CHECK(val, argIdx, argName) \
{ \
    if (val) \
    { \
        PrintHelp(NULL, MSDK_STRING("Input argument number %d \"%s\" require more parameters"), argIdx, argName); \
        return MFX_ERR_UNSUPPORTED;\
    } \
}
extern video_info_t g_video1_info;
extern video_info_t g_video2_info;
extern video_info_t g_video3_info;
extern video_info_t g_video4_info;
extern video_info_t g_video5_info;
extern video_info_t g_video6_info;
extern _audio_info_t g_audio1_info;
audio_send_info_t g_audio1_send_info = {{0}};

video_info_t g_decvideo_info[(DEC_MAX_NUM)];
video_info_t g_movievideo_info[2];//0:high,1:low

int g_EncStartFlag[(ENC_NUM)]={0};
int g_DecStartFlag[(DEC_MAX_NUM)]={0};
sAPI_EncoderInputParams g_tEncoderPar[ENC_NUM];
sAPI_DecoderInputParams g_tDecoderPar[DEC_MAX_NUM];

pthread_mutex_t mutex;
void mutexLock()
{
	pthread_mutex_lock(&mutex);
}

void mutexunLock()
{
	pthread_mutex_unlock(&mutex);
}


int mutexLockInit()
{
	return pthread_mutex_init(&mutex, NULL);
}

int mutexLockdeInit()
{
	return pthread_mutex_destroy(&mutex);
}


int print_params_dec(sInputParams_dec* pParams,int decno)
{    
	printf("dddddddddddddddddddd dec%d param\n",decno);
	printf("videoType[%d]=%d\n",decno,pParams->videoType);
	printf("mode[%d]=%d\n",decno,pParams->mode);
	printf("memType[%d]=%d\n",decno,pParams->memType);
	printf("bUseHWLib[%d]=%d\n",decno,pParams->bUseHWLib);
	printf("bIsMVC[%d]=%d\n",decno,pParams->bIsMVC);
	printf("bLowLat[%d]=%d\n",decno,pParams->bLowLat);
	printf("bCalLat[%d]=%d\n",decno,pParams->bCalLat);
	printf("nMaxFPS[%d]=%d\n",decno,pParams->nMaxFPS);
	printf("nWallCell[%d]=%d\n",decno,pParams->nWallCell);
	printf("nWallW[%d]=%d\n",decno,pParams->nWallW);
	printf("nWallH[%d]=%d\n",decno,pParams->nWallH);
	printf("nWallMonitor[%d]=%d\n",decno,pParams->nWallMonitor);
	printf("bWallNoTitle[%d]=%d\n",decno,pParams->bWallNoTitle);
	printf("nWallTimeout[%d]=%d\n",decno,pParams->nWallTimeout);
	printf("numViews[%d]=%d\n",decno,pParams->numViews);
	printf("nRotation[%d]=%d\n",decno,pParams->nRotation);
	printf("nAsyncDepth[%d]=%d\n",decno,pParams->nAsyncDepth);
	printf("gpuCopy[%d]=%d\n",decno,pParams->gpuCopy);
	printf("nThreadsNum[%d]=%d\n",decno,pParams->nThreadsNum);
	printf("SchedulingType[%d]=%d\n",decno,pParams->SchedulingType);
	printf("Priority[%d]=%d\n",decno,pParams->Priority);
	printf("width[%d]=%d\n",decno,pParams->width);
	printf("height[%d]=%d\n",decno,pParams->height);
	printf("fourcc[%d]=%d\n",decno,pParams->fourcc);
	printf("nFrames[%d]=%d\n",decno,pParams->nFrames);
	printf("strSrcFile[%d]=%s\n",decno,pParams->strSrcFile);
	printf("strDstFile[%d]=%s\n",decno,pParams->strDstFile);
}
int print_params(sInputParams* pParams,int encno)
{
	printf("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee enc%d params\n",encno);
	printf("nTargetUsage[%d]=%d\n",encno,pParams->nTargetUsage);
	printf("CodecId[%d]=%d\n",encno,pParams->CodecId);
	printf("ColorFormat[%d]=%d\n",encno,pParams->ColorFormat);
	printf("nPicStruct[%d]=%d\n",encno,pParams->nPicStruct);
	printf("nWidth[%d]=%d\n",encno,pParams->nWidth);
	printf("nHeight[%d]=%d\n",encno,pParams->nHeight);
	printf("dFrameRate[%d]=%d\n",encno,pParams->dFrameRate);
	printf("nBitRate[%d]=%d\n",encno,pParams->nBitRate);
	printf("MVC_flags[%d]=%d\n",encno,pParams->MVC_flags);
	printf("nQuality[%d]=%d\n",encno,pParams->nQuality);

	printf("numViews[%d]=%d\n",encno,pParams->numViews);

	printf("nDstWidth[%d]=%d\n",encno,pParams->nDstWidth);
	printf("nDstHeight[%d]=%d\n",encno,pParams->nDstHeight);
	printf("memType[%d]=%d\n",encno,pParams->memType);
	printf("bUseHWLib[%d]=%d\n",encno,pParams->bUseHWLib);
	printf("strSrcFile[%d]=%s\n",encno,pParams->strSrcFile);

    //sPluginParams pluginParams;
	//printf("=%d\n",pParams->);

	printf("HEVCPluginVersion[%d]=%d\n",encno,pParams->HEVCPluginVersion);
	printf("nRotationAngle[%d]=%d\n",encno,pParams->nRotationAngle);
	printf("strPluginDLLPath[%d]=%s\n",encno,pParams->strPluginDLLPath);

	printf("nAsyncDepth[%d]=%d\n",encno,pParams->nAsyncDepth);
	printf("nRateControlMethod[%d]=%d\n",encno,pParams->nRateControlMethod);
	printf("nLADepth[%d]=%d\n",encno,pParams->nLADepth);
	printf("nMaxSliceSize[%d]=%d\n",encno,pParams->nMaxSliceSize);
	printf("nQPI[%d]=%d\n",encno,pParams->nQPI);
	printf("nQPP[%d]=%d\n",encno,pParams->nQPP);
	printf("nQPB[%d]=%d\n",encno,pParams->nQPB);
	printf("nNumSlice[%d]=%d\n",encno,pParams->nNumSlice);
	printf("UseRegionEncode[%d]=%d\n",encno,pParams->UseRegionEncode);
}
static void *videoenc_thread(void *param)
{
	mfxU16 encno=*((mfxU16*)param);
	pthread_detach(pthread_self());
	printf("API_IntelMedia_SDK_HW_EncInitial start encno=%d\n",encno);
	API_IntelMedia_SDK_HW_EncInitial((void *)(&g_tEncoderPar[encno]));
	printf("API_IntelMedia_SDK_HW_EncInitial net enc end encno=%d,piplehand=%x\n",encno,g_tEncoderPar[encno].pHandle);	
    if((encno <= 11)&&(encno >= 6))
    {//sdi 5路
		printf("sdi create_caputure_thread %d fb=%d\n",encno,g_tEncoderPar[encno].nEncodeFrameRate);	
        create_caputure_thread(encno,(int)(g_tEncoderPar[encno].nEncodeFrameRate));
		printf("sdi create_caputure_thread %d \n",encno);	
    }
	
	printf("API_IntelMedia_SDK_HW_EncFrame start encno=%d\n",encno);	 
	API_IntelMedia_SDK_HW_EncFrame((void *)(&g_tEncoderPar[encno]));
	printf("API_IntelMedia_SDK_HW_EncFrame over encno=%d\n",encno);	 
	 
	printf("API_IntelMedia_SDK_HW_EncRelease start encno=%d\n",encno); 
	API_IntelMedia_SDK_HW_EncRelease((void *)(&g_tEncoderPar[encno]));
	printf("API_IntelMedia_SDK_HW_EncRelease over encno=%d\n",encno);  
	
	return NULL;
}
static void *audio_send_thread(void *param)
{
	audio_send_info_t *p_audio_send_info=(audio_send_info_t*)param;
	
	pthread_detach(pthread_self());

#if 0	
	FILE *pfOut;
	pfOut=fopen("/home/quxin410/sdk/video/captrue.aac","wb");
	if(pfOut==NULL)
	{
		printf("fopen out error\n");	 
		return NULL;
	}
#endif

	while(1)
	{
		do
		{
			 if(p_audio_send_info->read_no!=p_audio_send_info->write_no)
			 {
				 break;
			 }
			 usleep(1000*10);
		}while(1);
		
		//usleep(42665);
		//printf("audio_send_thread p_audio_send_info->read_no=%d,p_audio_send_info->write_no=%d\n",p_audio_send_info->read_no,p_audio_send_info->write_no);
		reach_audio_send_curdata(0, (unsigned char *)(g_audio1_send_info.frame_buf[g_audio1_send_info.read_no]), g_audio1_send_info.data_len[g_audio1_send_info.read_no]);
		//fwrite(p_audio_send_info->frame_buf[p_audio_send_info->read_no],1,p_audio_send_info->data_len[p_audio_send_info->read_no],pfOut);//可以写文件 可以发送
		//printf("send data addr=%p,fwritebuf[%d] audio len %d\n",p_audio_send_info->frame_buf[p_audio_send_info->read_no],p_audio_send_info->read_no,p_audio_send_info->data_len[p_audio_send_info->read_no]);

		p_audio_send_info->read_no++;
		
		if(p_audio_send_info->read_no>=AUDIO_FRAME_SENDBUF_NUM)
		{
			p_audio_send_info->read_no=0;
		}
	}
}
static void *audioenc_thread(void *param)
{
	pthread_t           p_audio_caputure1_thread;
	pthread_t           p_audio_send1_thread;
	mfxU16 encno=*((mfxU16*)param);
	TransContext tTransCtx;
	TransContext * pTransCtx;
	MediaPacket outpkt;
	int i=0,ret=0;
	int num=0;
	RH_FRAMEHEAD_t frameHead;
	unsigned int PacketNo = 0;

	pTransCtx=&tTransCtx;
	pthread_detach(pthread_self());
	for(i = 0; i < AUDIO_FRAME_SENDBUF_NUM;i++)
	{
		g_audio1_send_info.frame_buf[i] = (char *)malloc(MAX_AUDIO_ES_LEN+FF_INPUT_BUFFER_PADDING_SIZE);
		if(NULL == g_audio1_send_info.frame_buf[i])
		{
			printf ("g_audio1_send_info.malloc i = %d fail.\n", i);
			return NULL;
		}
		memset(g_audio1_send_info.frame_buf[i],0,MAX_AUDIO_ES_LEN+FF_INPUT_BUFFER_PADDING_SIZE);
	}
	
	MediaSysInit();
	MediaTranscodeInit(pTransCtx,NULL,NULL);
	AudioPcmInfoInit(pTransCtx,44100,2,AV_SAMPLE_FMT_S16);
	ret=AudioEncInfoInit(pTransCtx,AV_CODEC_ID_AAC,44100,64000,2,AV_SAMPLE_FMT_S16);
	AudioFilterInit(pTransCtx,"aformat=s16:44100:stereo");
	ret=pthread_create(&p_audio_caputure1_thread, NULL, audio_caputure1_thread, (void *)&g_audio1_info);
	if(ret<0) 
	{
		printf("!!!!!Failed to create audio_caputure1_thread thread\n");
	}
	else
	{
		printf("create audio_caputure1_thread thread\n");
	}
	ret=pthread_create(&p_audio_send1_thread, NULL, audio_send_thread, (void *)&g_audio1_send_info);
	if(ret<0) 
	{
		printf("!!!!!Failed to create audio_send_thread thread\n");
	}
	else
	{
		printf("create audio_send_thread thread\n");
	}

	printf("encode audio start!\n");

#if 0	
	FILE *pfOut;
	pfOut=fopen("/home/quxin410/sdk/video/captrue.pcm","wb");
	if(pfOut==NULL)
	{
		printf("fopen out error\n");	 
		return NULL;
	}
#endif

	while(1)
	{//循环编码
		while(1)
		{
			 if(g_audio1_info.read_no!=g_audio1_info.write_no)
			 {
				 break;
			 }
			 usleep(1000);
		}
		//printf("wwwwwwwwwwg_audio1_info.read_no=%d,g_audio1_info.write_no=%d\n",g_audio1_info.read_no,g_audio1_info.write_no);

		AudioAddPcmData(pTransCtx,g_audio1_info.frame_buf[g_audio1_info.read_no],AUDIO_FRAME_LEN);
		
		//fwrite(g_audio1_info.frame_buf[g_audio1_info.read_no],1,AUDIO_FRAME_LEN,pfOut);//可以写文件 可以发送
		AudioFilterInputFrame(pTransCtx);
		AudioFilterOutputFrame(pTransCtx);
		while (pTransCtx->bGetAudioDecFrame)
		{
			if(g_audio1_send_info.read_no == g_audio1_send_info.write_no)
			{
				outpkt.pData=(uint8_t*)g_audio1_send_info.frame_buf[g_audio1_send_info.write_no];
				AudioEncode(pTransCtx,&outpkt);
				if (outpkt.iLen > 0)
				{
					g_audio1_send_info.data_len[g_audio1_send_info.write_no]=outpkt.iLen;

					//MediaPacketFree(&outpkt);
					g_audio1_send_info.write_no ++;
					if(g_audio1_send_info.write_no >= AUDIO_FRAME_SENDBUF_NUM)
					{
						g_audio1_send_info.write_no = 0;
					}
				}
				AudioFilterOutputFrame(pTransCtx);
			}
			else if(g_audio1_send_info.read_no != (g_audio1_send_info.write_no + 1))
			{
				num = g_audio1_send_info.write_no + 1;
				if(num >= AUDIO_FRAME_SENDBUF_NUM)
				{
					num = 0;
				}
				if(g_audio1_send_info.read_no != num)
				{
					outpkt.pData=(uint8_t*)g_audio1_send_info.frame_buf[g_audio1_send_info.write_no];
					AudioEncode(pTransCtx,&outpkt);
					if (outpkt.iLen > 0)
					{
						g_audio1_send_info.data_len[g_audio1_send_info.write_no]=outpkt.iLen;
						//MediaPacketFree(&outpkt);
						g_audio1_send_info.write_no = num;
					}
					AudioFilterOutputFrame(pTransCtx);
				}
				else
				{
					usleep(1000);
				}
			}
			else
			{
				usleep(1000);
			}
		}
		g_audio1_info.read_no++;
		
		if(g_audio1_info.read_no>=FRAME_BUF_NUM)
		{
			g_audio1_info.read_no=0;
		}
	}
	printf("audio process over!233333333333333333\n");
	return NULL;
}

static void *videodec_thread(void *param)
{
	mfxU16 decno=*((mfxU16*)param);
	pthread_detach(pthread_self());
	
	printf("videodec_thread decno=%d\n",decno);	
	API_IntelMedia_SDK_HW_DecInitial((void *)(&g_tDecoderPar[decno]));
	printf("API_IntelMedia_SDK_HW_DecInitial net dec end Decno=%d,piplehand=%x\n",decno,g_tDecoderPar[decno].pHandle);	

	printf("API_IntelMedia_SDK_HW_DecFrame start decno=%d\n",decno);	 
	API_IntelMedia_SDK_HW_DecFrame((void *)(&g_tDecoderPar[decno]));
	printf("API_IntelMedia_SDK_HW_DecFrame over decno=%d\n",decno);	 
	 
	printf("API_IntelMedia_SDK_HW_DecRelease start decno=%d\n",decno); 
	API_IntelMedia_SDK_HW_DecRelease((void *)(&g_tDecoderPar[decno]));
	printf("API_IntelMedia_SDK_HW_DecRelease over decno=%d\n",decno);  
	
	return NULL;
}
void Encoder_Config1(sAPI_EncoderInputParams *lp)
{
	lp->nEncoderType=1;
	lp->nMemoryType=SYSTEM_MEMORY;
	lp->nSrcFormat=1;
	lp->nEncodeFormat=0;
	lp->nEncodeProfile=ENCODER_High_Profile;
	lp->nEncodeLevels=ENCODER_Profile_Levels4;
	lp->nEncodeStreamType=0;
	lp->nEncodeQP=10;
	lp->nKeyframeIntelval=30;
	lp->nEncodeStreambit=2000;
	lp->nEncodeFrameRate=30.0;
	lp->nSrcWidth=VIDEO_WIDTH;	
	lp->nSrcHeight=VIDEO_HEIGHT;
	lp->pHandle=NULL;
	

}
int sysint()
{
	int i=0,j=0;
	int ipc_video_len=1280*720*3/2;
	//初始化发送码流
	printf("reach_video_send_process start\n");  
	reach_video_send_process();
	reach_video_recv_processThread(NULL);
	printf("reach_video_send_process end\n"); 
	TrackTskInit();
	init_tracer("127.0.0.1");
	printf("TrackTskInit end\n"); 	

	mutexLockInit();

}
int main(int argc, char *argv[])
{
	pthread_t           p_enc_thread[ENC_NUM];
	pthread_t           p_audioenc_thread[ENC_NUM];
	pthread_t           p_dec_thread[DEC_MAX_NUM];
	int ipc_video_len=1280*720*3/2;
	int encno[ENC_NUM];
	int decno[DEC_MAX_NUM];
	int i=0,j=0;
	int ret;

	Signal(SIGPIPE, SIG_IGN);
	Signal(SIGINT, SIG_IGN);

	
	sInputParams_dec* pParams_dec[DEC_MAX_NUM];

    mfxStatus sts = MFX_ERR_NONE; // return value check
    ret=sysint();
	if(ret==-1)
	{
		printf("eeeeeeeeee error main Failed to sysint\n");
		return 0;
	}
	printf("main finish sysint!\n");
	//ipc解码
	g_DecStartFlag[0]=1;	//ipc teacher
	g_DecStartFlag[1]=1;	//ipc blackboard
	g_DecStartFlag[2]=1;	//ipc stu sd0
	g_DecStartFlag[3]=1;	//ipc stu sd1
	for(i=0; i<6; i++)
	{
		decno[i]=i;
		if(g_DecStartFlag[i]!=0)
		{
			for(j = 0; j < FRAME_BUF_NUM;j++)
			{
				g_decvideo_info[i].frame_buf[j] = (char *)malloc(ipc_video_len);
				if(NULL == g_decvideo_info[i].frame_buf[j])
				{
					printf ("g_decvideo_info[%d] malloc j = %d fail.\n", i,j);
					return -1;
				}
				memset(g_decvideo_info[i].frame_buf[j],0,ipc_video_len);
			}
			g_tDecoderPar[i].poutvideo_info=&g_decvideo_info[i];
			g_tDecoderPar[i].nDecNo=i;
			
			ret=pthread_create(&p_dec_thread[i], NULL, videodec_thread, (void *)&decno[i]);
			if(ret<0) 
			{
				printf("main Failed to create dec_thread thread%d\n",decno[i]);
				return 0;
			}
			else
			{
				printf("main pthread_create dec_thread %d\n",decno[i]);
			}
			
			g_EncStartFlag[i]=1;
			Encoder_Config1(&g_tEncoderPar[i]);  
			g_tEncoderPar[i].pvideo_info=&g_decvideo_info[i];
			g_tEncoderPar[i].nEncNo=i;
			g_tEncoderPar[i].nSrcWidth=IPC_VIDEO_WIDTH;	
			g_tEncoderPar[i].nSrcHeight=IPC_VIDEO_HEIGHT;
			
			ret=pthread_create(&p_enc_thread[i], NULL, videoenc_thread, (void *)&decno[i]);
			if(ret<0) 
			{
				printf("main Failed to create videoenc_thread thread%d\n",decno[i]);
				return 0;
			}
			else
			{
				printf("main pthread_create videoenc_thread %d\n",decno[i]);
			}
		}
	}
//编码 sdi
	g_tEncoderPar[6].pvideo_info=&g_video1_info;
	g_tEncoderPar[7].pvideo_info=&g_video2_info;
	g_tEncoderPar[8].pvideo_info=&g_video3_info;
	g_tEncoderPar[9].pvideo_info=&g_video4_info;
	g_tEncoderPar[10].pvideo_info=&g_video5_info;
	g_tEncoderPar[11].pvideo_info=&g_video6_info;
	g_EncStartFlag[6]=1;//sdi
	g_EncStartFlag[7]=1;//sdi
	g_EncStartFlag[8]=1;//sdi
	g_EncStartFlag[9]=1;//sdi
	g_EncStartFlag[10]=1;//sdi
	g_EncStartFlag[11]=1;//vga
	//编码 movie
	for(j = 0; j < FRAME_BUF_NUM;j++)
	{
		g_movievideo_info[0].frame_buf[j] = (char *)malloc(VIDEO_FRAME_LEN);
		if(NULL == g_movievideo_info[0].frame_buf[j])
		{
			printf ("g_movievideo_info[0] malloc j = %d fail.\n", j);
			return -1;
		}
		memset(g_movievideo_info[0].frame_buf[j],0,VIDEO_FRAME_LEN);
		
		g_movievideo_info[1].frame_buf[j] = (char *)malloc(VIDEO_FRAME_LEN);
		if(NULL == g_movievideo_info[1].frame_buf[j])
		{
			printf ("g_movievideo_info[1] malloc j = %d fail.\n", j);
			return -1;
		}
		memset(g_movievideo_info[1].frame_buf[j],0,VIDEO_FRAME_LEN);
	}
	g_tEncoderPar[ENC_MOVIE_HEIGH].pvideo_info=&g_movievideo_info[0];
	g_tEncoderPar[ENC_MOVIE_LOW].pvideo_info=&g_movievideo_info[1];
	g_EncStartFlag[ENC_MOVIE_HEIGH]=1;//电影模式
	//g_EncStartFlag[ENC_MOVIE_LOW]=0;//电影模式
    for(i=6; i<ENC_NUM; i++)
	{
		encno[i]=i;
		if(g_EncStartFlag[i]!=0)
		{
			Encoder_Config1(&g_tEncoderPar[i]);  
			g_tEncoderPar[i].nEncNo=i;
			if(i==ENC_MOVIE_HEIGH)
			{
				g_tEncoderPar[i].nEncodeFrameRate=30.0;
			}
			ret=pthread_create(&p_enc_thread[i], NULL, videoenc_thread, (void *)&encno[i]);
			if(ret<0) 
			{
				printf("main Failed to create videoenc_thread thread%d\n",encno[i]);
				return 0;
			}
			else
			{
				printf("main pthread_create videoenc_thread %d\n",encno[i]);
			}
		}
	}	
	//audio 
	i=0;
	ret=pthread_create(&p_audioenc_thread[i], NULL, audioenc_thread, (void *)&encno[i]);
	if(ret<0) 
	{
		printf("main Failed to create videoenc_thread thread%d\n",encno[i]);
		return 0;
	}
	else
	{
		printf("main pthread_create audioenc_thread %d\n",encno[i]);
	}

        int n = 0;
        double ave = 0; 

	while(1)
	{
		sleep(2);
	}
	return 0;

}

#endif



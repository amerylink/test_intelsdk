/*********************************************************************************



********************************************************************************/
#ifndef _SDK_CODEC_H_
#define _SDK_CODEC_H_
	//最大编码路数
#define DEC_MAX_NUM (6)//6ipc-dec
#define IPC_VIDEO_WIDTH 		(704)					//视频宽度
#define IPC_VIDEO_HEIGHT		(576)					//视频宽度

//DEBUG
#define SDK_Codec_Debug() { \
 printf("Find File [%s] Error Line [%d]\n",__FILE__,__LINE__); \
 }

enum API_EncoderType {
	ENCODER_TYPE_SOFTWARE = 0x00,
	ENCODER_TYPE_HARDWARE = 0x01,
};

enum API_MemType {
	ENCODER_SYSTEM_MEMORY = 0x00,
	ENCODER_D3D9_MEMORY = 0x01,
	ENCODER_D3D11_MEMORY = 0x02,
};

enum API_DataFormat {
	ENCODER_DATA_YUV420 = 0x00,
	ENCODER_DATA_YV12 = 0x01,
};

enum API_EncodeFormat {
	ENCODER_FORMAT_H264 = 0x00,
};

enum API_EncodeProfile {
	ENCODER_Baseline_Profile = 0x00,
	ENCODER_Main_Profile = 0x01,
	ENCODER_High_Profile = 0x02,
};

enum API_EncodeLevels {
	ENCODER_Profile_Levels41 = 0x00,
	ENCODER_Profile_Levels1 = 0x01,
	ENCODER_Profile_Levels2 = 0x02,
	ENCODER_Profile_Levels3 = 0x03,
	ENCODER_Profile_Levels4 = 0x04,
};

enum API_EncodeStreamContorl {
	API_ENCODER_Stream_CBR = 0x00,
	API_ENCODER_Stream_VBR = 0x02,

};

enum API_EncodeFrameFormat {
	ENCODER_I_Frame = 0x00,
	EENCODER_P_Frame = 0x01,
	ENCODER_B_Frame = 0x02,
};


struct sAPI_EncoderFrame
{
	unsigned int nSrcFrameFormat;    //Input YUV Format 0 is YUV420 , 1 is YV12
	unsigned int nSrcWidth;             //Encode Width
	unsigned int nSrcHeight;            //Encode Height
	unsigned char *y;                //Input Y
	unsigned char *u;                //Input U
	unsigned char *v;                //Input V
	unsigned char *nv_y;                //Input NV12
	unsigned char *nv_uv;                //Input UV
	unsigned int nDstFrameFormat;      //Encode 0 is I frame 1 is P frame 2 is B frame
	unsigned int nDstFrameLen;         //Encode frame length
	unsigned char *dstbuf;
	unsigned long IputTimeStamp;
	unsigned long frameTimeStamp;
	unsigned long DTSStamp;
	void *sharehandlein;
	void *m_pshareD3DFrameAllocator;
	//EncoderFrameDate lppframeCallback;                       //Frame Callback

};

struct sAPI_EncoderInputParams
{
	unsigned int nEncoderType;     //Encoder Type 0 is Software , 1 is Hardware
	unsigned int nMemoryType;      //Memory Type 0 is System , 1 is D3D9 , 2 is D3D11
	unsigned int nSrcFormat;       //Input YUV Format 0 is YUV420 , 1 is YV12
	unsigned int nEncodeFormat;    //Encode Format 0 is H264
	unsigned int nEncodeProfile;   //Encode Profile 0 is baseline , 1 is main , 2 is high
	unsigned int nEncodeLevels;    //Encode Profile Levels 1-4
	unsigned int nEncodeStreamType;    //Encode Stream Control 0 is CBR 1 is VBR
	unsigned int nEncodeQP;         //Encode Video quality 10-50 defualt 10
	unsigned int nKeyframeIntelval;    //Encode Key Frame Intelval
	unsigned int nEncodeStreambit;            //Encode Stream bit kbs
	unsigned int nSrcWidth;            //Source YUV Width
	unsigned int nSrcHeight;            //Source YUV Height
	unsigned int nDstX;            //Encode x
	unsigned int nDstY;            //Encode y
	unsigned int nDstWidth;            //Encode Width
	unsigned int nDstHeight;            //Encode Height
	unsigned int nBuffNum;            //分配的buff数
	unsigned int nEncNo;
	double nEncodeFrameRate;            //Encode frame rate 1-30
	//sAPI_EncoderFrame *pframe;      //Encode Frame Buffer
	void *pHandle;                       //Encode Handle

};

struct sAPI_FrameSurfaceAllocParams
{
	unsigned int nWidth;            //Width
	unsigned int nHeight;            //Height
};

struct sAPI_DecoderInputParams
{
	void *pHandle;                       //decode Handle
	unsigned int nBuffNum;            //分配的buff数
	unsigned int nDecNo;
};

//Hardware Alloc buff
int API_IntelMedia_SDK_HW_AllocBuff(void *lp);
//Hardware Encode reset Param
int API_IntelMedia_SDK_HW_EncResetPara(void *lp,void *encInfo);
//Hardware Encode Initial
int API_IntelMedia_SDK_HW_EncInitial(void *lp);

//Hardware Encode Release
int API_IntelMedia_SDK_HW_EncRelease(void *lp);

//Hardware Encode Frame
int API_IntelMedia_SDK_HW_EncFrame(void *lp,void *encInfo);

int API_IntelMedia_SDK_HW_DecInitial_CreateSession(void *lp);
int API_IntelMedia_SDK_HW_DecInitial_DecHeader(void *lp,Bitstream_Buf* pBitStrameBuff);
int API_IntelMedia_SDK_HW_DecInitial_CreateDec(void *lp);
int API_IntelMedia_SDK_HW_DecInitial_reach(void *lp);
//Hardware Decode Initial
int API_IntelMedia_SDK_HW_DecInitial(void *lp);

//Hardware Decode Release
int API_IntelMedia_SDK_HW_DecRelease(void *lp);

//Hardware Decode Frame
int API_IntelMedia_SDK_HW_DecFrame(void *lp,void *videoInfo);
//buff分配参数初始化
void API_IntelMedia_SDK_HWAllocParaInit(void *lp);
//buff分配
mfxFrameSurface1* API_IntelMedia_SDK_HW_GetBuff(void *lp);
#endif


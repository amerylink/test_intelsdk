#if 1

/*****************************************************************************************
*  ��������ͨ�ŵĽӿڣ�����WEB,TCP_SDK������Ԥ�ƵĴ����пأ�����ͨ���Ľӿڷ��ء�
*
*
*
*
*******************************************************************************************/
#ifndef _MID_CONTROL_H__
#define _MID_CONTROL_H__

#if 0
enum
{
	VP0 = 0,
	VP1 = 1,
	ALL_VP,
	MAX_VP,
};
#endif

//�ж������ֶ�
#define 	WEB_IDENTIFIER 			0xaaaaaaaa
#define 	TCP_SDK_IDENTIFIER  0xaaaaaaab

#define INPUT_1     2
#define INPUT_2     1
#define XML_TEXT_MAX_LENGTH 4096

#define ENCODESERVER_1260_PORT   30113
#define HD_SERVER_PORT	   30114
#define SD_UPGRADE_SERVER_PORT	   30115
#define SDK_LISTEN_PORT		4000
#define	ENCODESERVER_PORT		41110
#define MSG_SET_TEST_CTRL	0xf123
#define MSG_MKFSEXT3_SDA		0xf234

enum{
	IPC_PROC_110 = 0,
	IPC_PROC_HC,
	IPC_PROC_DH,
	IPC_PROC_YS,
	IPC_PROC_MAX
};

typedef enum __webparsetype__ {
    INT_TYPE = 0,
    STRING_TYPE,
    STRUCT_TYPE,
} webparsetype;

typedef struct __webMsgInfo_1260 {
	int 			identifier;  //0xaaaaaaaa ��ʶһ����Ч����
	int				len;      //�ܳ�
	webparsetype	type;
} webMsgInfo_1260;


//##############################################�ṹ��##end####################################################

//##############################################�ṹ��##end####################################################


//##############################################�ṹ��##end####################################################
//#############################�忨��Ϣ####################################

//CARD_1���� cmd = old_cmd | INPUT_CARD_1
//CARD_1���� cmd = old_cmd | INPUT_CARD_2

//channel num (cmd>>4)&0xf
//cmd = cmd | (((channel+1)&0xf)<<16) ;


//############�ź�״̬########################
#define	MSG_GETINPUTSIGNALINFO 			0x1112		//��ȡ�źŷֱ�����Ϣ
#define	MSG_SIGNALDETAILINFO			0x1114	//��ȡ�ź���ϸ��Ϣ
#define	MSG_REVISE_PICTURE				0x1115  //HVֵ����
#define MSG_GETCOLORSPACE				0x1136   //�õ���ɫ�ռ�
#define MSG_SETCOLORSPACE				0x1137   //������ɫ�ռ�
#define MSG_GETHDCPVALUE				0x113F  //��ȡ�ź�HDCP״̬
#define	MSG_GETSIGNALINFO				0x222F  //�ź�Դ��Ϣ
#define	MSG_GET_ENC_INFO				0x223F  //��������Ϣ
#define	MSG_SETCLASSTYPE				0x1321  //��������
#define	MSG_GETCLASSTYPE				0x1322  //��������
//###########��Ƶ������Ϣ######################
#define	MSG_GETOUTPUTVIDEOINFO			0x1117	//��ȡ�����������
#define	MSG_SETOUTPUTVIDEOINFO			0x1118	//���ñ����������
#define	MSG_SETH264ENCODEPARAM			0x1119  //����264�ĸ߼������������
#define	MSG_GETVIDEOPARAM				0x111B	 //��ȡ��Ƶ����
#define	MSG_SETVIDEOPARAM				0x111F   //������Ƶ����
#define MSG_GETH264ENCODEPARAM 			0x113D  //��ȡH264�߼�����

#define MSG_SETSCALEINFO  				0X2025				//zl
#define MSG_GETSCALEINFO  				0X2026				//zl

#define CONTROL_ROOM_SET_PREVIEW_PASS     	(0X9000)	  //ZL
#define ROOM_CONTROL_GET_PREVIEW_PASS		(0X9001)	  //ZL

#define MSG_GET_REC_INFO					0X2027				//ZL


#define MSG_GETVIDEOENCODEINFO		0X1602
#define MSG_SETVIDEOENCODEINFO		0X1603
#define MSG_GETENABLETEXTINFO		0X1604
#define MSG_SETENABLETEXTINFO		0X1605


//###########��Ƶ��Ϣ##########################
#define	MSG_GETAUDIOPARAM 				0x111A	 //��ȡ��Ƶ����
#define	MSG_SETAUDIOPARAM				0x111E   //������Ƶ����
#define MSG_SETMUTE						0x1150  //������Ƶ����
#define MSG_GETMUTE						0x1153 //��ȡ��Ƶ����

//###########logo ,��Ļ########################
#define MSG_UPLOADIMG      				0x1139	//�ϴ�logoͼƬ
#define MSG_GET_LOGOINFO  				0x113A	//��ȡlogoͼƬ��Ϣ
#define MSG_SET_LOGOINFO  				0x113B	//����logoͼƬ��Ϣ
#define MSG_GET_TEXTINFO  				0x1154 //��ȡ��Ļ״̬
#define MSG_SET_TEXTINFO  				0x1155 //������Ļ״̬
#define MSG_GET_MAXPOS                 		 0x1156 //��ȡ������Ļ�����xy����


//############ϵͳ��Ϣ#########################
#define MSG_GETSOFTCONFIGTIME 			0x1110  //��ȡ��������ʱ��
#define	MSG_GETCPULOAD  				0x1111	//��ȡCPUռ����

#define 	MSG_GET_NETWORK					0X111B	//��ȡ�������
#define 	MSG_SET_NETWORK					0x111a//�����������
#define	MSG_GETSYSPARAM					0x111C	 //��ȡϵͳ����
#define	MSG_SETSYSPARAM					0x1120   //����ϵͳ����
#define	MSG_UPDATESYS					0x112B  //����ϵͳ����
#define	MSG_REBOOTSYS					0x112C  //����ϵͳ����

#define	MSG_REBOOTSYS_F					0xFF2C  //����ϵͳ����

#define	MSG_GETSERIALNO					0x112A  //��ȡ����
#define	MSG_GETTOOLS_V				0xFF1C

//############ԶңЭ��############################
#define	MSG_FAR_CTRL						0x1126   //Զң����
#define MSG_GET_CTRL_PROTO				0x1127 //��ȡԶңЭ��
#define	MSG_SET_CTRL_PROTO				0x1128 //����ԶңЭ��








//###############input begin############################










//###############input end############################


#define	MSG_GETSTREAMPARAM				0x1122
#define	MSG_SETSTREAMPARAM				0x1123
#define	MSG_GETRTSPPARAM				0x1124
#define	MSG_SETRTSPPARAM				0x1125

#define	MSG_SYNCTIME					0x112D
#define	MSG_SAVEPARAM					0x112E
#define	MSG_START_MULT					0x112F
#define	MSG_STOP_MULT					0x1130
//#define	MSG_GET_PROTOCOL				0x1131
#define	MSG_SETDEVICETYPE				0x1132
#define	MSG_GETDEVICETYPE				0x1133
#define MSG_BLUE_REVISE					0x1134
#define MSG_RESTORESET					0x1135







#define MSG_GETENCODETIME				0x1156
#define MSG_SDK_LOGIN 						0x1157
#define MSG_GETSOFTVERSION                     0x1158


// zyb
//#define MSG_SET_SIGNAL_DETECTION			0x2023


/*stream output */
#define MSG_STREAM_GET_PROTOCOL 0x1200
/*rtsp*/
#define MSG_RTSP_GET_USED      0X1201  		/*rtsp is used*/
#define MSG_RTSP_GET_GINFO    0x1202  		/*rtsp get server global info*/
#define MSG_RTSP_SET_GINFO     0X1203 		/*rtsp set server global info*/
#define MSG_RTSP_GET_CINFO    0x1204 		 /*rtsp get common info*/
#define MSG_RTSP_SET_CINFO    0x1205  		/*rtsp set common info*/
#define MSG_RTSP_SET_STATUS   0x1206           /*rtsp set start,stop,pause status*/
#define MSG_RTSP_ADD_SERVER   0x1208          /*add rtsp server*/
#define MSG_RTSP_DEL_SERVER    0x1209        /*del rtsp server*/
#define MSG_GET_SDP_INFO       0x120a
#define MSG_GET_RTSP_USER_NUM  0x120b

//#define MSG_RTSP_SET_STATUS  0x1310

/*multcast */
#define MSG_MULT_GET_NUM      0X1301  		/*get mult server total num*/
#define MSG_MULT_GET_CINFO    0x1304 		 /*mult get common info*/
#define MSG_MULT_SET_CINFO    0x1305  		/*mult set common info*/
#define MSG_MULT_SET_STATUS   0x1306           /*mult set start,stop,pause status*/
#define MSG_MULT_ADD_SERVER   0x1308          /*add mult server*/
#define MSG_MULT_DEL_SERVER    0x1309        /*del mult server*/
#define MSG_MULT_GET_TS_RATE   0x1310           /*get mult  ts rate*/
#define MSG_MULT_GET_RTP_RATE 0x1311 		/*get mult rtp rate*/
#define MSG_MULT_GET_RTPTS_RATE 0x1312 		/*get mult rtpts rate*/
//#define MSG_MULT_SET_STATUS  0x1310


/*******����ʱ����****************/

#define MSG_GET_STREAM_STA					(0x2076)
#define MSG_GET_RTMP_STREAM_STA				(0x2077)


//#define MSG_SETSHOWTEXTLOGO   0x9901
#define MSG_GET_BUILD_TIME 0x9901

#define MSG_CHANNEL_NUM 0x9902
#define	HD_MSG_GET2SDI				0x1330  //��������

#define MSG_CONTROL_SET_ROOM_TYPE 0x9903

#define MSG_CONTROL_SET_PLATFORM 0x9904
#define MSG_CONTROL_GET_PLATFORM 0x9905
#define MSG_CONTROL_GET_SOLAINFO 0x9906
#define MSG_CONTROL_SET_SOLAINFO 0x9907

#define MSG_CONTROL_SET_LOCKINFO 	0xee01
#define MSG_CONTROL_SET_ENC_PARAM	0xee02
#define MSG_CONTROL_SET_COMMON_PARAM	0xee03
#define MSG_CONTROL_GET_RTMPINFO 0x9908
#define MSG_CONTROL_SET_RTMPINFO 0x9909

#define MSG_CONTROL_GET_SOBEYINFO 0x990a
#define MSG_CONTROL_SET_SOBEYINFO 0x990b

#define MSG_CONTROL_GET_MCINFO 0x990c
#define MSG_CONTROL_SET_MCINFO 0x990d

#define MSG_CONTROL_SET_UPLOAD_TYPE 0x990e
#define MSG_CONTROL_GET_UPLOAD_TYPE 0x990f

//add by lichl
#define MSG_SET_SWMS_INFO				0x1f00
#define MSG_GET_SWMS_INFO				0x1f01

#define MSG_SET_HDMI1_RES				0X1f04
#define MSG_SET_HDMI2_RES				0X1f14
#define MSG_GET_HDMI1_RES				0x1f05
#define MSG_GET_HDMI2_RES				0x1f15

#define WEB_SET_TRACER_INFO				0x1f06
#define WEB_GET_TRACER_INFO				0x1f07

#define WEB_GET_RAMOTE_CTRL			0x1f08
#define WEB_SET_RAMOTE_CTRL			0x1f09

#define MSG_GET_TRACER_INDEX			0x1f0a

#define MSG_TO_ROOM_VODPLAY_CTRL		(0X8024)


#define	MSG_DEL_RECOURSE					(0X9027)//zl
#define	MSG_ADD_FILENOTES				(0X9028)//zl
#define	MSG_SHOW_FILELIST					(0X9128)//zl
#define	MSG_SELECT_FILELIST				(0X9129)//zl
#define 	MSG_SETMANUALSTATUS				(0X912a)//wuzc


/*CGI->HD*/
#define	HD_GET_INPUTTYPE				0x111D	 //��ȡDVI/SDI
#define	HD_SET_INPUTTYPE				0x1121   //����DVI/SDI
#define 	HD_GET_NETWORK				0x1141	//HD->Type:SYSPARAMS
#define 	HD_GET_NETWORK_1				0x1162	//HD->Type:SYSPARAMS
#define 	HD_SET_NETWORK				0x1142//HD->Type:SYSPARAMS
#define 	HD_SET_NETWORK_1				0x1163//HD->Type:SYSPARAMS
#define 	HD_GET_SYSINFO					0x1143//HD->Type:System_Info_t
#define 	HD_GET_SIGNALINFO				0x1144//HD->Type:SDI_Signal_Info  4SDI signal Info
#define  	HD_SET_IPCADDR					0X1145//HD->Type:Char  [16]
#define  	HD_GET_IPCADDR					0X1146//HD->Type:Char  [16]
#define 	HD_SET_HDMIRES					0X1147//HD->Type:OutputResolution_1
#define 	HD_GET_HDMIRES					0X1148//HD->Type:OutputResolution_1
#define 	HD_SET_SYNCTIME				0X1149//HD->Type:DATE_TIME_INFO
#define 	HD_SET_ROOMTYPE				0X1150//HD->Type:DATE_TIME_INFO
#define 	HD_GET_DEVICETYPE				0X1151//HD->Type:DATE_TIME_INFO
#define 	HD_REBOOT_MSG				 	0x1152	//HD->type : reboot

#define	SD_SYSUPGRADE						(0X2008)
#define	MSG_POWER_OFF					0X1153	//power off

#define  	HD_GET_TEACHER_IPCADDR					0X1154//HD->Type:Char  [16]
#define  	HD_SET_TEACHER_IPCADDR						0X1156//HD->Type:Char  [16]

#define  	HD_GET_STUDENT_IPCADDR					0X1155//HD->Type:Char  [16]
#define  	HD_SET_STUDENT_IPCADDR						0X1157//HD->Type:Char  [16]

#define 	MSG_SET_SOBYADDR				0X1160
#define 	MSG_GET_SOBYADDR				0X1161

#define  	HD_GET_INTERACTADDR					0X1300 //HD->Type:Char  [16]
#define  	HD_SET_INTERACTADDR					0X1302 //HD->Type:Char  [16]

#define  	HD_GET_IPCINPUT1ADDR					0X1303//HD->Type:Char  [16]
#define  	HD_SET_IPCINPUT1ADDR					0X1304//HD->Type:Char  [16]

#define  	HD_GET_IPCINPUT2ADDR					0X1305//HD->Type:Char  [16]
#define  	HD_SET_IPCINPUT2ADDR					0X1306//HD->Type:Char  [16]
//##############################################�����ֶ�##end####################################################

//##############################################������##begin####################################################

#define 				CLIENT_RETSUCCESSVALUE 				0
#define 				CLIENT_ERR_UNKNOWCMD					0xf1
#define					CLIENT_ERR_PARAMLEN						0xf2
#define 				CLIENT_ERR_TCPCONNECT					0xf3
#define   			CLIENT_ERR_TCPSEND						0xf4
#define 				CLIENT_ERR_TCPRECV						0xf5
#define 				CLIENT_ERR_PARAMSVALUE				0xf6
#define  				CLIENT_ERR_USERNAME						0xf7
#define 				CLIENT_ERR_PASSWORD						0xf8


#define    			SERVER_RET_OK 								0X00
#define        		SERVER_RET_UNKOWN_CMD 						0xf01
#define        		SERVER_RET_INVAID_PARM_LEN  				0xf02               //������ͨ�Žṹ�峤�Ȳ�һ�£���Ҫ���Ӧͨ�ŵ�ͷ�ļ�
#define        		SERVER_RET_INVAID_PARM_VALUE  				0xf03
#define        		SERVER_RET_SOCK_MAX_NUM  					0xf04
#define        		SERVER_RET_RECV_FAILED  					0xf05
#define        		SERVER_RET_SEND_FAILED  					0xf06
#define        		SERVER_RET_ID_ERROR  						0xf07
#define       		SERVER_RET_USER_INVALIED  					0xf08
#define 			SERVER_INTERNAL_ERROR						0xf09                //�������ڲ����󣬳��ָô��󣬱������ϲ�ԭ��
#define             SERVER_VERIFYFILE_FAILED				    0XF0A
#define             SERVER_SYSUPGRADE_FAILED				    0XF0B
#define             SERVER_SYSROLLBACK_FAILED					0xF0C
#define             SERVER_GETDEVINFO_FAILED       				0xF0D
#define             SERVER_HAVERECORD_FAILED					0xF0E


//##############################################������##end####################################################

#define  webMsgInfo webMsgInfo_1260
#define MSGINFOHEAD			sizeof(webMsgInfo_1260)

typedef struct SDI_Signal_Info{
	char tea[32];
	int tea_status;
	char stu[32];
	int stu_status;
	char ptea[32];
	int ptea_status;
	char pstu[32];
	int pstu_status;
}SDI_Signal_Info;

typedef struct VGA_Signal_Info{
	char vga[32];
	int vga_status;
}VGA_Signal_Info;

typedef struct _File_notes_{
	int mode;//�޸���
	char file_name[128];
	char modify[128];
}File_Notes;


typedef struct REMOTE_CTRL_INFO{
	int tea_procotol;
	int tea_addr;
	int stu_procotol;
	int stu_addr;
}Remote_Ctrl_t;

typedef enum __OutputResolution_1 {
    LOCK_AUTO = 0,
	LOCK_CUSTOM = 1,
    LOCK_1920x1080P = 2,
    LOCK_1280x720P = 3,
    LOCK_1400x1050_VGA = 4,
    LOCK_1366x768_VGA = 5,
    LOCK_1280x1024_VGA = 6,
    LOCK_1280x768_VGA = 7,
    LOCK_1024x768_VGA = 8,
    LOCK_720x480_D1 = 9,
    LOCK_352x288_D1 = 10,
    LOCK_MAX
} OutputResolution_1;

typedef struct _SYSTEM_INFO_{
	char serial_no[64];
	char type_no[64];
	char web_version[16];
	char ctrl_version[16];
	char ctrl_built_time[64];
	char hd_version[32];
	char hd_ker_version[64];
	char hd_FPGA_version[64];
	char hd_built_time[64];
	char hd_uboot_version[32];
	int total_space;
	int available_space;
	char ip[20];
}System_Info_t;

typedef struct move_mode_Info_{
	int model;
	int res;
}Moive_Info_t;

typedef struct _SCALE_INFO_{
	int resolution;  //OutputResolution
	int scalemode;   //o: ratio 1:full
	int outwidth;    //custom width
	int outheight;    //custom heigh
	int	sign_flag_1;
	int	sign_flag_2;
}WebScaleInfo;

typedef struct WEB_AUDIOEncodePARAM_{
	unsigned int 	Codec;  							//Encode Type
	unsigned int 	SampleRateIndex; 						//SampleRate 44.1 48
	unsigned int 	BitRate;  						//BitRate  128000
	unsigned int 	Channel;  						//channel numbers
	unsigned int 	SampleBit;  						//SampleBit  16
	unsigned char  	LVolume;					//left volume       0 --------31
	unsigned char  	RVolume;					//right volume      0---------31
	unsigned short  InputMode;             // ��Ҫ��������֧�֣�����ƽ�����ƽ��MAC
	unsigned int	mp_input;               // �ϳ������1  or 2
	unsigned int 	Mute;                    //mute 0 not  1 mute
	unsigned int    type;                    //line in  or Mic
}WEB_AudioEncodeParam;



typedef struct _PANEL_REC_INFO_{

	int rec_total_time;
	char rec_file_path[256];

}PanelRecInfo;


typedef struct _VIDEO_ENCODE_INFO_{
//	int 					enable;        //maybe not need
//	SceneConfiguration   	preset ;			//��������ѡ�
	unsigned int 		 	nFrameRate;   	//��Ƶ֡��
	unsigned int			IFrameinterval; 	//I ֡���
	unsigned int 			sBitrate;			//��Ƶ����
	unsigned int 			sCbr;
	unsigned int 			nQuality;
}WebVideoEncodeInfo;

typedef struct _ENABLE_TEXT_INFO_
{
	int text_show;
	int logo_show;
}WebEnableTextInfo;


typedef enum {
    LOCK_SCALE = 1,
    UNLOCK_SCALE = 2
} LOCK_SCALE_STATUS;

#define VIDEO_QUALITY 0
#define VIDEO_SBITRATE 1



#if 0
typedef struct _RTP_SDP_INFO_T{
	int channel;
	char ip[16];
	int vport;
	int aport;
	char buff[1024];
}RTP_SDP_INFO;
typedef enum __ENUM_PROTOCOL__ {
    OUTPUT_TS_STREAM = 0,
    OUTPUT_RTP_STREAM,
    OUTPUT_RTSP_STREAM,
    OUTPUT_RTMP_STREAM,
    OUTPUT_MAX_PROTOCOL
} Enum_Protocol;



/*protocol struct*/
typedef struct __PROTOCOL__ {
	/*protocol count*/
	unsigned char count ;
	/*protocol open status  1 -- open */
	/*0--- close all */
	unsigned int status: 1 ;
	/*TS stream*/
	unsigned int TS: 1;
	/*RTP*/	
	unsigned int RTP: 1;
	/*RTSP*/
	unsigned int RTSP: 1;
	/*RTMP*/
	unsigned int RTMP: 1;
	/*reserve*/
	unsigned char temp[7];
} Protocol;

/*mult addr*/
typedef struct __MULT_ADDR_PORT {
	char chIP[16];
	unsigned short nPort;
	short nStatus;
} MultAddr;





//ý����Э���������
typedef struct StreamParam {
	Enum_Protocol   	protocol;			//Э��ѡ�
	char				ip[16];			//ip
	unsigned int			port;				//�˿ںš�
	unsigned int			MTUParam;		//MTUֵ��
	unsigned int			TTLParam;		//TTLֵ��
	unsigned int			TOSParam;		//TOSֵ��
	unsigned int			shapFlag;			//�������ͱ�־��
	unsigned int			actualBandwidth;	//ʵ�ʴ�����
	unsigned int			reservation;		//������
} StreamParam;

typedef enum __StreamProtocol {
    TSoverUDP,
    TSoverRTP,
    DirectRTP,
    QuickTime,
} StreamProtocol;
//rtsp��������
typedef struct RtspParam {
	unsigned int 		keepAtctive;
	unsigned int			MTUParam;				//MTUֵ��
	StreamProtocol	protocol;					//rtsp������Э�顣
	char				boardCastip[16];			//�鲥ip;
	unsigned int			boardCastPort;				//�鲥�˿ں�;
	unsigned int			reservation;				//������
} RtspParam;
#endif

#if 0
typedef struct __SYSPARAMS {
	unsigned char szMacAddr[8];				//MAC address
	unsigned int dwAddr;					//IP address
	unsigned int dwGateWay;					//getway
	unsigned int dwNetMark;					//sub network mask
	char strName[16];						//encode box name
	char strVer[10];						//Encode box version
	unsigned short unChannel;							//channel numbers
	char bType; 							//flag  0 -------VGABOX     3-------200 4-------110 5-------120 6--------1200
	char bTemp[3]; 							//bTemp[0] ------1260
	int nTemp[7];							//reserve  nTemp[0] ��ʾDHCP FLAG
} SYSPARAMS;
#endif

typedef struct _DATE_TIME_INFO {
	int year;
	int month;
	int mday;
	int hours;
	int min;
	int sec;
} DATE_TIME_INFO;

typedef enum __inputtype {
    DVI=0,
    SDI,
    VGA,
    AUTO
} inputtype;




//ɫ�ʿռ�ת��
typedef struct pic_revise {
	short hporch;    //col
	short vporch;    //row
	int temp[4];
} pic_revise ;

typedef enum _POS_TYPE_T{
    ABSOLUTE2= 0,
    TOP_LEFT ,
    TOP_RIGHT ,
    BOTTOM_LEFT ,
    BOTTOM_RIGHT ,
    CENTERED
} POS_TYPE;

#if 1
typedef struct __textinfo {
	int xpos;
	int ypos;
	int width;
	int height;
	int showtime;
	int enable;
	int alpha;
	int show;
	int postype;
	int color[3];  //������ɫRGB
	char msgtext[128];
} TextInfo;
#endif
typedef enum __colorspace {
    RGB = 0,
    YUV,
} ColorSpace;
//	�����ź�Դ����Ϣ��
typedef struct InputSignalInfo {
	int  mode;			//�ź�Դ�ֱ�����Ϣ��
	ColorSpace  spaceFlag;			//��ɫ�ռ��־��
	unsigned int 	hdcpFlag;			//����HDCP��Ȩ������־��
	unsigned int		reservation;		//������
} InputSignalInfo;

typedef enum __SceneConfiguration {
    movies = 0,
    motionless,
    motion,
    custom,
} SceneConfiguration;

#define JEPG_FILE_NEMA  ("/opt/dvr_rdk/ti816x_2.8/logo_temp.jpg")

typedef struct LogoInfo {
	int x;	//��ʾλ������,���Ͻ�Ϊ0,0
	int y;
	int width;	//logoͼƬ����
	int height;
	int enable;	//�Ƿ���ʾlogo	,��ʾ:1,����ʾ:0
	char filename[32];//ͼƬ����15���ַ�,Ĭ��Ϊlogo.png
	int alpha;//͸����[0,100]
	int isThrough;//�Ƿ�͸��ĳһ����ֵ,ʵ���οյ�Ч��,͸��1,��͸2
	int filter;//���isThroughΪ1����������һ��Ҫ͸��ֵ��һ��Ϊ0x00��0xFF
	int errcode;//������Ϊ0��ʾû�д���1��ʾ��Ч��logo�ļ�
	int format;	//���� -> ͨ��
	int postype; // pos type
} LogoInfo;

typedef struct _ip_config
{
	int  iptype;
	char ipaddr[20];
	char netmask[20];
	char gateway[20];
	char dns[20];
}IpConfig;

typedef struct _IPCInfo{
	int ipcProcType;
	int stauts;
	int conStauts;
	char ip[16];
}Ipc_Info_t;

/*��ǰ����״̬*/
typedef struct __RESIZE_PARAM__ {
	unsigned int dbstatus;//�Ƿ��ڵ���״̬
	unsigned int flag ;//�Ƿ��������ֱ���״̬
	unsigned int scalemode ; // ģʽ
	int black;//��ڱ�
	int cap_w;			//source width
	int cap_h;			//source height
	int dst_w;			//dest  width
	int dst_h;			//dest height
	int x;
	int y;
	int nFrame;		//֡��
	int nBitrate;   //����
	int nTemp;		//����
} ResizeParam;

typedef struct SignalInfo_ {
	char Signal[32];
	int HPV;
	int TMDS;
	int VsyncF;
	int HDCP;
	int RGB_YPRPR;
} SignalInfo_t;

typedef struct EncInfo_ {
	char fpga_version[8];
	char kernel_version[64];
	char uboot_version[32];
} EncInfo_t;

typedef struct SolaInfo_{
	int open;
	char sola_url[128];
}SolaInfo_t;

typedef struct SobeyInfo_ {
	int open;
	char SobeyIP[16];
} SobeyInfo_t;

typedef struct MediaCenter2Info_ {
	int open;
	char MediaCenter2IP[16];
	int Port;
} MediaCenter2Info_t;


typedef struct LockInfo_ {
	int lock;
	char key[128];
} LockInfo_t;

typedef struct _THIRD_INFO_T{
	int hd;
	char hd_url[128];
	int sd;
	char sd_url[128];
}Thirdplatforminfo_s;

typedef struct EncParam_ {
	char chs[64];
	int height;
	int width;
	int framerate;
	int bitrate;
} EncParam_t;


//H264�����������
typedef struct H264EncodeParam {
	unsigned int		param1;
	unsigned int		param2;
	unsigned int		param3;
	unsigned int		param4;
	unsigned int		param5;
	unsigned int		reservation;		//������

} H264EncodeParam;


//�����Ƶ�źŵĲ�������
typedef struct OutputVideoInfo {
	int resolution;		//��������ֱ��ʡ�
	unsigned int 		 resizeMode;		//����ģʽѡ��0���������� 1��ȫ������
	unsigned int			 encodelevel;		//���뼶��0��high profile   1��baseline profile
	SceneConfiguration   preset ;			//��������ѡ�
	unsigned int 		 nFrameRate;   	//��Ƶ֡��
	unsigned int			 IFrameinterval; 	//I ֡���
	unsigned int 			 sBitrate;			//��Ƶ���ʡ�
	int					logo_show;
	int 					text_show;
	//unsigned int			logotext;		//�Ƿ���ʾlogo/text��
} OutputVideoInfo;


typedef struct _CamerInfo{
	int ZoomSpeed;
}CamerInfo_t;


#endif
#endif
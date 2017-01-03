#ifndef _CAPTURE_LINK_H_
#define _CAPTURE_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"
#include "SDK_codec.h"

#define FRAME_BUF_NUM			(20)						//����֡��
#define VIDEO_FRAME_LEN		(1920*1088 *3/2)			//һ֡������󳤶�
#define DEV_FILE_NAME_LEN		(64)						//�豸���Ƴ���
#define CAPTURE_QUE_LEN		(10)						//�ɼ����еĳ���

/*�ɼ�link��������Ϣ*/
typedef struct{
	Int8 			dev_name[DEV_FILE_NAME_LEN];		//�豸����
	Int32		video_type;							//��Ƶ���ͣ���1080P30��1080P60��720p60
	Int32		captureHandle;						//�豸fd
	Uint32		frame_len;							//�ɼ����ݳ���
	Uint32 		captureWidth;						//��Ƶ���
	Uint32 		captureHeight;						//��Ƶ�߶�
	Uint32		ColorFormat;						//��Ƶ����ɫ�ռ�
	Int32 		captureCh;							//�ڼ�·��Ƶ����0��ʼ�����Ϊ15
	Bool			captureStatus;						//�ɼ�link������״̬
	Uint32		captureFrameNum;						//�ɼ�frame��
	FRAME_Buf	captureFrame[CAPTURE_QUE_LEN];		//�ɼ����֡
	LINK_TskHndl tskHandle;							//������
	LINK_EmpFullQueHndl EmpFullQueHndl;				//���������о��
}CAP_LINK_INFO;

/*�ⲿ�ӿں�������*/
/*����link*/
Int32 captureLink_create(CAP_LINK_INFO *info);
/*�ͷ�link*/
Int32 captureLink_release(CAP_LINK_INFO *info);
/*����link*/
Int32 captureLink_connect(CAP_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
/*ʹ��link*/
Int32 captureLink_enable(CAP_LINK_INFO *info);
/*��ʹ��link*/
Int32 captureLink_disable(CAP_LINK_INFO *info);


#endif

#ifndef _CAPTURE_LINK_H_
#define _CAPTURE_LINK_H_

#include "linkCommon.h"
#include "linkQueue.h"
#include "linkTask.h"

#include "linkVideo.h"
#include "SDK_codec.h"

#define FRAME_BUF_NUM			(20)						//缓冲帧数
#define VIDEO_FRAME_LEN		(1920*1088 *3/2)			//一帧数据最大长度
#define DEV_FILE_NAME_LEN		(64)						//设备名称长度
#define CAPTURE_QUE_LEN		(10)						//采集队列的长度

/*采集link的数据信息*/
typedef struct{
	Int8 			dev_name[DEV_FILE_NAME_LEN];		//设备名称
	Int32		video_type;							//视频类型，如1080P30，1080P60，720p60
	Int32		captureHandle;						//设备fd
	Uint32		frame_len;							//采集数据长度
	Uint32 		captureWidth;						//视频宽度
	Uint32 		captureHeight;						//视频高度
	Uint32		ColorFormat;						//视频的颜色空间
	Int32 		captureCh;							//第几路视频，从0开始，最大为15
	Bool			captureStatus;						//采集link的启动状态
	Uint32		captureFrameNum;						//采集frame数
	FRAME_Buf	captureFrame[CAPTURE_QUE_LEN];		//采集输出帧
	LINK_TskHndl tskHandle;							//任务句柄
	LINK_EmpFullQueHndl EmpFullQueHndl;				//空满包队列句柄
}CAP_LINK_INFO;

/*外部接口函数声明*/
/*创建link*/
Int32 captureLink_create(CAP_LINK_INFO *info);
/*释放link*/
Int32 captureLink_release(CAP_LINK_INFO *info);
/*连接link*/
Int32 captureLink_connect(CAP_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl);
/*使能link*/
Int32 captureLink_enable(CAP_LINK_INFO *info);
/*不使能link*/
Int32 captureLink_disable(CAP_LINK_INFO *info);


#endif

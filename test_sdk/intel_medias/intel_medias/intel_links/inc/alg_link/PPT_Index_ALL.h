#ifndef _ALLPARAMETER_H
#define _ALLPARAMETER_H

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef BYTE
typedef unsigned char BYTE;
#endif


typedef struct
{
	int width;                     //图象宽度
	int height;                    //图象高度
	int kd_y;                      //图像跨度_y
	int kd_uv;                     //图像跨度_uv
	int yuv_mode;                  //图像的YUV格式，2==yuv420, 1==yuv422
	unsigned int frame_sum;        //帧总数
	unsigned int static_frm_num;   //静止阈值，当某帧图像停留大于该值后才可能插入新的索引，默认为3帧
	unsigned char *p_org_Ypels;    //原始图像序列Y部分首地址
	unsigned char *p_org_Upels;	   //原始图像序列U部分首地址
	unsigned char *p_org_Vpels;	   //原始图像序列V部分首地址
	int *prog_out_memory;          //供索引检测程序使用的内存分配
	int isIndex;                   //返回是否是新的索引
	int indexNum;                  //当前所在索引号

	int un_use[19];                //保留部分，请勿使用
} CUSTOM_SET_ENC_PARAM;

typedef struct
{
	int width;     // 图像的宽    000
	int height;    // 图像的高    004
	int step_y;    // 图像跨度(Y) 008
	int step_uv;   // 图像跨度(UV)012

	int frameno;   //             016
	int pic_size;  // 有效图像大小020
	int IndexBeginNo;   // 索引开始帧 024
	int IndexZoomNo;    // 索引缩放帧(结束帧) 028
	int static_mark;    // 标记帧是否为静止帧 032
	int StillFrmNum;    // 静止帧的数目       036
	// 临时变量
	int IsIndex;        //                    040
	int IndexNum;       //                    044
	int diff_sum;    //当前帧相对于上一帧变化的像素个数(作为返回用)   048
	int static_frm_num; // 静止阈值，当某帧图像停留大于该值后才可能插入新的索引  052

	BYTE *alloc_buf; //记录分配的内存地址起始   056
	BYTE *buf[3];    //Y_data,Y_pre,Y_idx交替使用内存 060

	BYTE *Y_data_org;  // 存放原始图像可能带跨度的YUV数据  072
	BYTE *U_data_org;  //                                  076
	BYTE *V_data_org;  //                                  080
	int  Y_cur;   // 存放去掉跨度后的当前帧Y数据的索引     084
	int  Y_pre;   // 存放前一帧的Y数据的索引               088
	int  Y_idx;   // 存放索引帧的Y数据的索引               092

	BYTE *diff0;// 标记当前帧与前一帧的异同                096
	BYTE *diff1;// 标记前一帧与索引帧的异同                100
}lImage;
void ImagePageIndex(lImage *img);

// ===================================================================
void ImagePreTreatment(lImage *img);        // 图像预处理
// ===================================================================
void ImageGaussFilter(lImage *img); // 图像高斯滤波

//获得索引检测程序所需要的内存大小
int PPT_Index_mem_size(CUSTOM_SET_ENC_PARAM *p_user);

//索引检测程序初始化：内存分配，变量初始化
//int* PPT_Index_init(CUSTOM_SET_ENC_PARAM *p_user);
int PPT_Index_init(CUSTOM_SET_ENC_PARAM *p_user,lImage *img);

//索引程序内存释放（如果有另外申请内存）
void PPT_Index_free(lImage *img);

//索引检测主程序
void PPT_Index_run(lImage *img, CUSTOM_SET_ENC_PARAM *p_user);


#endif

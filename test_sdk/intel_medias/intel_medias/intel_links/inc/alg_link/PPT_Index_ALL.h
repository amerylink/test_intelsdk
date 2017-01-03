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
	int width;                     //ͼ����
	int height;                    //ͼ��߶�
	int kd_y;                      //ͼ����_y
	int kd_uv;                     //ͼ����_uv
	int yuv_mode;                  //ͼ���YUV��ʽ��2==yuv420, 1==yuv422
	unsigned int frame_sum;        //֡����
	unsigned int static_frm_num;   //��ֹ��ֵ����ĳ֡ͼ��ͣ�����ڸ�ֵ��ſ��ܲ����µ�������Ĭ��Ϊ3֡
	unsigned char *p_org_Ypels;    //ԭʼͼ������Y�����׵�ַ
	unsigned char *p_org_Upels;	   //ԭʼͼ������U�����׵�ַ
	unsigned char *p_org_Vpels;	   //ԭʼͼ������V�����׵�ַ
	int *prog_out_memory;          //������������ʹ�õ��ڴ����
	int isIndex;                   //�����Ƿ����µ�����
	int indexNum;                  //��ǰ����������

	int un_use[19];                //�������֣�����ʹ��
} CUSTOM_SET_ENC_PARAM;

typedef struct
{
	int width;     // ͼ��Ŀ�    000
	int height;    // ͼ��ĸ�    004
	int step_y;    // ͼ����(Y) 008
	int step_uv;   // ͼ����(UV)012

	int frameno;   //             016
	int pic_size;  // ��Чͼ���С020
	int IndexBeginNo;   // ������ʼ֡ 024
	int IndexZoomNo;    // ��������֡(����֡) 028
	int static_mark;    // ���֡�Ƿ�Ϊ��ֹ֡ 032
	int StillFrmNum;    // ��ֹ֡����Ŀ       036
	// ��ʱ����
	int IsIndex;        //                    040
	int IndexNum;       //                    044
	int diff_sum;    //��ǰ֡�������һ֡�仯�����ظ���(��Ϊ������)   048
	int static_frm_num; // ��ֹ��ֵ����ĳ֡ͼ��ͣ�����ڸ�ֵ��ſ��ܲ����µ�����  052

	BYTE *alloc_buf; //��¼������ڴ��ַ��ʼ   056
	BYTE *buf[3];    //Y_data,Y_pre,Y_idx����ʹ���ڴ� 060

	BYTE *Y_data_org;  // ���ԭʼͼ����ܴ���ȵ�YUV����  072
	BYTE *U_data_org;  //                                  076
	BYTE *V_data_org;  //                                  080
	int  Y_cur;   // ���ȥ����Ⱥ�ĵ�ǰ֡Y���ݵ�����     084
	int  Y_pre;   // ���ǰһ֡��Y���ݵ�����               088
	int  Y_idx;   // �������֡��Y���ݵ�����               092

	BYTE *diff0;// ��ǵ�ǰ֡��ǰһ֡����ͬ                096
	BYTE *diff1;// ���ǰһ֡������֡����ͬ                100
}lImage;
void ImagePageIndex(lImage *img);

// ===================================================================
void ImagePreTreatment(lImage *img);        // ͼ��Ԥ����
// ===================================================================
void ImageGaussFilter(lImage *img); // ͼ���˹�˲�

//�����������������Ҫ���ڴ��С
int PPT_Index_mem_size(CUSTOM_SET_ENC_PARAM *p_user);

//�����������ʼ�����ڴ���䣬������ʼ��
//int* PPT_Index_init(CUSTOM_SET_ENC_PARAM *p_user);
int PPT_Index_init(CUSTOM_SET_ENC_PARAM *p_user,lImage *img);

//���������ڴ��ͷţ���������������ڴ棩
void PPT_Index_free(lImage *img);

//�������������
void PPT_Index_run(lImage *img, CUSTOM_SET_ENC_PARAM *p_user);


#endif

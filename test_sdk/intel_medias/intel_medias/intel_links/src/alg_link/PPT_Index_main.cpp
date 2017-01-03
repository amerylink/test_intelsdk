
#include "../inc/alg_link/PPT_Index_ALL.h"


int PPT_Index_mem_size(CUSTOM_SET_ENC_PARAM *p_user)
{
	int ret_value;
/*	int uv_ratio;
	int pic_w,pic_h;

	pic_w = p_user->width;
	pic_h = p_user->height;
	if(p_user->yuv_mode == 0)//YUV420
	{
		uv_ratio = 4;
	}
	else //YUV422
	{
		uv_ratio = 2;
	}

	//1024*2为了预留一些空间
	ret_value = (pic_h*pic_w*4+pic_h*pic_w/uv_ratio*2)*sizeof(BYTE)+sizeof(lImage)+1024*2;
*/
//	ret_value = (p_user->kd_y*p_user->height*3)*sizeof(BYTE)+sizeof(lImage)+1024*2;
	ret_value = (p_user->width*p_user->height*5)*sizeof(BYTE)+sizeof(lImage)+1024*2;
	ret_value = (ret_value + 63)&(~63); //64字节对齐
	return ret_value;
}

int PPT_Index_init(CUSTOM_SET_ENC_PARAM *p_user,lImage *img)
{
	int i,k;
	int *buf_ooo;
	int picsize=0;
	int difsize=0;
//  k = (p_user->kd_y*p_user->height*3 )*sizeof(BYTE)+sizeof(lImage)+1024*2;
	k = (p_user->width*p_user->height*5 )*sizeof(BYTE)+sizeof(lImage)+1024*2;
	k = (k+63) & (~63);
	buf_ooo = p_user->prog_out_memory;
	if(buf_ooo==NULL)
	{
		buf_ooo = (int *)malloc(64*4+1024*2+k);
		if(buf_ooo == 0)
		{
			return -1;
		}
	}

	img->alloc_buf = (BYTE *)buf_ooo; //如果有另外分配内存，记录下首地址
	img->height = p_user->height;
	img->width  = p_user->width;
	img->step_y = p_user->kd_y;
	img->step_uv = p_user->kd_uv;
	img->frameno = 0;
	img->pic_size = img->height*img->width;
	img->IndexBeginNo = 0;
	img->IndexZoomNo  = 0;
	img->static_mark = 0;
	img->StillFrmNum = 0;
	img->IsIndex = 0;                                  // 标记该帧是否为索引帧
	img->IndexNum = 0;
	img->diff_sum = 0;
	img->static_frm_num = p_user->static_frm_num;

	//difsize=((unsigned int)buf_ooo+63)&(~63);
	//img->buf[0] = (BYTE*)buf_ooo+difsize;
	img->buf[0] = (BYTE*)buf_ooo;
	picsize = (img->width*img->height+63)&(~63);
	img->buf[1] = img->buf[0]+picsize;
	img->buf[2] = img->buf[1]+picsize;
	img->Y_data_org = NULL;
	img->U_data_org = NULL;
	img->V_data_org = NULL;
	img->Y_cur  = 0;
	img->Y_pre  = 0;
	img->Y_idx  = 0;
	img->diff0 = img->buf[2]+picsize;
	img->diff1 = img->diff0+picsize;
  	return 0;
}

void PPT_Index_free(lImage *img)
{
	if(img!=NULL)
	{
		if(img->alloc_buf)
			free(img->alloc_buf);
	}
}

void PPT_Index_run(lImage *img, CUSTOM_SET_ENC_PARAM *p_user)
{
	img->Y_data_org = p_user->p_org_Ypels;
	img->U_data_org = p_user->p_org_Upels;
	img->V_data_org = p_user->p_org_Vpels;

  ImagePreTreatment(img);
  ImagePageIndex(img);

//==========================================================================================
	if (img->IsIndex)
	{
		img->IndexNum++;
		p_user->isIndex = 1;
		p_user->indexNum = img->IndexNum;

		img->IsIndex = 0;
	}                                                               
	img->frameno++;
}



#include "../inc/alg_link/PPT_Index_ALL.h"

//int StatDifference(BYTE *org, BYTE *pre, BYTE *dst, BYTE *cur, int iPicW, int iPicH, int step, int *out_diff_sum);
int StatDifference(BYTE *cur, BYTE *pre, BYTE *dst, int iPicW, int iPicH, int *out_diff_sum);
int StatDistinctDifference(BYTE *data0, BYTE *data1, BYTE *dst, int pic_size);
int CheckNewIndex(BYTE *src, BYTE *mark, int pic_size);
int diff_projection(unsigned char *diff, int iPicW, int iPicH);

/*
 * \brief
 *     图像预处理
 *
 * \remark
 *     减少图像噪声
 *
 * \author
 *     Li
 *
 * \date
 *     2010.08.23
 */
//对原始图像Y_data_org做高斯滤波
//结果保存到img->buf[img->Y_cur]中
void ImagePreTreatment(lImage *img)// 高斯滤波3x3
{
  int i,j,ii,jj;
  int mask[3][3] =  {{1,2,1},{2,4,2},{1,2,1}};
  int fValue;
  int step;
  int width, height;
  BYTE *p_pixel;
  BYTE *src;
  BYTE *dst;

  step = img->step_y;
  width = img->width;
  height = img->height;
  src = img->Y_data_org;
  dst = img->buf[img->Y_cur];

  //目标内存先清0(实际只需要清0第一行和最后一行，第一列和最后一列)
  memset(dst, 0, img->pic_size);

  //除边缘块以外的区域滤波
  for(j=1; j<height-1; j++)
  {
    for(i=1; i<width-1; i++)
    {
      fValue = 0;
      // 指向第i行，第j列个元素
      p_pixel = src + j*step + i;

      for(jj=-1; jj<2; jj++)
      {
        for(ii=-1; ii<2; ii++)
        {
          fValue += p_pixel[jj*step+ii]*mask[jj+1][ii+1];
        }
      }
      dst[j*width+i] = fValue/16;
    }
  }

	return;
}

//图像翻页索引处理
void ImagePageIndex(lImage *img)
{
  int change;

  //计算和上一帧的差别，确定相对于上一帧是否是有变化
//  change = StatDifference(img->Y_data_org, img->buf[img->Y_pre], img->diff0, img->buf[img->Y_cur], img->width, img->height, img->step_y, &img->diff_sum);
  change = StatDifference(img->buf[img->Y_cur], img->buf[img->Y_pre], img->diff0, img->width, img->height, &img->diff_sum);

  if(img->frameno) //非第一帧
  {
    if(change) //变化帧
    {
      if(img->static_mark) //如果是在一个静止序列后面，则可能是索引
      {

        //看上一帧和索引的显著变化
        change = StatDistinctDifference(img->buf[img->Y_pre], img->buf[img->Y_idx], img->diff1, img->pic_size);

        if(change) //上一帧和索引有显著差异
        {
          //看上一帧和索引差异部分在这帧和上一帧之间的匹配程度
          //匹配较高则不是新的索引
          img->IsIndex = CheckNewIndex(img->diff0, img->diff1, img->pic_size);
        }
        else //上一帧和索引无显著差异
        {
          //看差异分布确定是否可能是索引
          img->IsIndex = diff_projection(img->diff0, img->width, img->height);
        }

        img->static_mark = 0;
      }

      //重置停留时间
      img->StillFrmNum = 0;
    }
    else //没有变化
    {
      //记录该帧不变化停留时间
      img->StillFrmNum++;
    }
  }
  else //第一帧
  {
    //第一帧看做一个新的索引
		img->IsIndex = 1;
    img->static_mark = 0;
    img->StillFrmNum = 0;
  }

  if(img->StillFrmNum>img->static_frm_num) //不变时间超过static_frm_num帧认为是一个停留
  {
    img->static_mark = 1;
  }

  //指针交换
  img->Y_pre = img->Y_cur;

  img->Y_cur = img->Y_cur+1;
  if(img->Y_cur==3)
  {
    img->Y_cur = 0;
  }
  if(img->Y_cur == img->Y_idx)
  {
    img->Y_cur = img->Y_cur+1;
    if(img->Y_cur==3)
    {
      img->Y_cur = 0;
    }
  }

  if(img->IsIndex)
  {
    img->Y_idx = img->Y_pre;
  }

  return ;
}


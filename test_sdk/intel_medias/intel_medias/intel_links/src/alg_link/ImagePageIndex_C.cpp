
#include "../inc/alg_link/PPT_Index_ALL.h"

//int StatDifference(BYTE *org, BYTE *pre, BYTE *dst, BYTE *cur, int iPicW, int iPicH, int step, int *out_diff_sum);
int StatDifference(BYTE *cur, BYTE *pre, BYTE *dst, int iPicW, int iPicH, int *out_diff_sum);
int StatDistinctDifference(BYTE *data0, BYTE *data1, BYTE *dst, int pic_size);
int CheckNewIndex(BYTE *src, BYTE *mark, int pic_size);
int diff_projection(unsigned char *diff, int iPicW, int iPicH);

/*
 * \brief
 *     ͼ��Ԥ����
 *
 * \remark
 *     ����ͼ������
 *
 * \author
 *     Li
 *
 * \date
 *     2010.08.23
 */
//��ԭʼͼ��Y_data_org����˹�˲�
//������浽img->buf[img->Y_cur]��
void ImagePreTreatment(lImage *img)// ��˹�˲�3x3
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

  //Ŀ���ڴ�����0(ʵ��ֻ��Ҫ��0��һ�к����һ�У���һ�к����һ��)
  memset(dst, 0, img->pic_size);

  //����Ե������������˲�
  for(j=1; j<height-1; j++)
  {
    for(i=1; i<width-1; i++)
    {
      fValue = 0;
      // ָ���i�У���j�и�Ԫ��
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

//ͼ��ҳ��������
void ImagePageIndex(lImage *img)
{
  int change;

  //�������һ֡�Ĳ��ȷ���������һ֡�Ƿ����б仯
//  change = StatDifference(img->Y_data_org, img->buf[img->Y_pre], img->diff0, img->buf[img->Y_cur], img->width, img->height, img->step_y, &img->diff_sum);
  change = StatDifference(img->buf[img->Y_cur], img->buf[img->Y_pre], img->diff0, img->width, img->height, &img->diff_sum);

  if(img->frameno) //�ǵ�һ֡
  {
    if(change) //�仯֡
    {
      if(img->static_mark) //�������һ����ֹ���к��棬�����������
      {

        //����һ֡�������������仯
        change = StatDistinctDifference(img->buf[img->Y_pre], img->buf[img->Y_idx], img->diff1, img->pic_size);

        if(change) //��һ֡����������������
        {
          //����һ֡���������첿������֡����һ֮֡���ƥ��̶�
          //ƥ��ϸ������µ�����
          img->IsIndex = CheckNewIndex(img->diff0, img->diff1, img->pic_size);
        }
        else //��һ֡����������������
        {
          //������ֲ�ȷ���Ƿ����������
          img->IsIndex = diff_projection(img->diff0, img->width, img->height);
        }

        img->static_mark = 0;
      }

      //����ͣ��ʱ��
      img->StillFrmNum = 0;
    }
    else //û�б仯
    {
      //��¼��֡���仯ͣ��ʱ��
      img->StillFrmNum++;
    }
  }
  else //��һ֡
  {
    //��һ֡����һ���µ�����
		img->IsIndex = 1;
    img->static_mark = 0;
    img->StillFrmNum = 0;
  }

  if(img->StillFrmNum>img->static_frm_num) //����ʱ�䳬��static_frm_num֡��Ϊ��һ��ͣ��
  {
    img->static_mark = 1;
  }

  //ָ�뽻��
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


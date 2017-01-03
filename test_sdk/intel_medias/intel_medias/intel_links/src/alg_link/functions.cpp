
#include "../inc/alg_link/PPT_Index_ALL.h"

//ͳ�Ƶ�ǰ֡����һ֮֡��Ĳ���
//�����Ƿ����б仯
//org���ݴ����step
//˳����ɽ�ԭʼ����ȥ����ȴ���cur�ڴ�Ĳ���
//dst: ��������֡
/*
int StatDifference(BYTE *org, BYTE *pre, BYTE *dst, BYTE *cur, int iPicW, int iPicH, int step, int *out_diff_sum)
{
  int i,j;
  int diff_sum;
  int diff_count;

  diff_sum = 0;
  diff_count = 0;

  for(j=0; j<iPicH; j++)
  {
    for(i=0; i<iPicW; i++)
    {
      dst[j*iPicW+i] = abs(org[j*step+i] - pre[j*iPicW+i]);
      cur[j*iPicW+i] = org[j*step+i];

      if(dst[j*iPicW+i]>10)
      {
        diff_sum += dst[j*iPicW+i];
        diff_count++;
        dst[j*iPicW+i] = 255;
      }
      else
      {
        dst[j*iPicW+i] = 0;
      }
    }
  }

  *out_diff_sum = diff_sum;//�����в�������ظ���

  //�����ܺ�С��1000�����ǲ������С��ͼ���С��1/1000����Ϊ���ޱ仯
//  if(diff_sum < 1000 || diff_count < iPicW*iPicH/1000)
  if(diff_sum < 1000 || diff_count*1000 < iPicW*iPicH)
  {
    return 0; //�ж�Ϊû�в���
  }
  
  return 1;
}
*/
int StatDifference(BYTE *cur, BYTE *pre, BYTE *dst, int iPicW, int iPicH, int *out_diff_sum)
{
  int i,j;
  int diff_sum;
  int diff_count;

  diff_sum = 0;
  diff_count = 0;

  for(i=0; i<iPicW*iPicH; i++)
  {
    dst[i] = abs(cur[i] - pre[i]);

    if(dst[i]>10)
    {
      diff_sum += dst[i];
      diff_count++;
      dst[i] = 255;
    }
    else
    {
      dst[i] = 0;
    }
  }

  *out_diff_sum = diff_sum;//�����в�������ظ���

  //�����ܺ�С��1000�����ǲ������С��ͼ���С��1/1000����Ϊ���ޱ仯
//  if(diff_sum < 1000 || diff_count < iPicW*iPicH/1000)
  if(diff_sum < 1000 || diff_count*1000 < iPicW*iPicH)
  {
    return 0; //�ж�Ϊû�в���
  }
  
  return 1;
}

//ͳ����һ֡������֮֡�����������
//�����Ƿ��������Ա仯
int StatDistinctDifference(BYTE *data0, BYTE *data1, BYTE *dst, int pic_size)
{
  int i;
  int diff_sum;
  int diff_count;

  diff_sum = 0;
  diff_count = 0;

  for(i=0; i<pic_size; i++)
  {
    dst[i] = abs(data0[i] - data1[i]);

    if(dst[i]>10)
    {
//      diff_sum += dst[i];
      diff_count++;
      dst[i] = 255;
    }
    else
    {
      dst[i] = 0;
    }
  }

  //�������С��ͼ���С��1/100����Ϊ���������仯
  if(diff_count*100 < pic_size)
  {
    return 0; //�ж�Ϊû�в���
  }
  
  return 1; //�����������ͼ���1/100��Ϊ���������仯
}

//���ݵ�ǰ֡����һ֡��mark�б�ǲ��첿�ֵ�ƥ��̶���ȷ���Ƿ����µ�����
int CheckNewIndex(BYTE *src, BYTE *mark, int pic_size)
{
  int i;
  size_t count,sum;

  count = sum = 0;

  for(i=0; i<pic_size; i++)
  {
    if(mark[i]) 
    {
      sum++; //ͳ��ԭ���ı���������ܺ�
      if(src[i] == 0) //�ڲ��������У�ͳ���޲���ĵ�ĸ���
      {
        count++; //ͳ��ƥ���
      }
    }
  }

  //ƥ���
  if(count*2 >= sum)
  {
    return 0; //ƥ��Ƚϸ������µ�����
  }
  else
  {
    return 1;
  }
}

// ����ͼͶӰ��ȷ���Ƿ����������
int diff_projection(unsigned char *diff, int iPicW, int iPicH)
{
  int fr_x,fr_y;
  int num_points_h,num_points_v;
  int top,bottom,left,right;
  int diff_width;
  int diff_height;
  int result;

  num_points_h = num_points_v = -1;
  top = bottom = left = right = -1;
  diff_width = diff_height = 0;

  //���ϵ����ҵ���һ�в���ϴ��
  for(fr_y=0; fr_y<iPicH; fr_y++) //����ÿһ��
  {
    num_points_h = 0;
    for(fr_x=0; fr_x<iPicW; fr_x++) //����ÿһ��
    {
      if(diff[fr_y*iPicW+fr_x]) //������в���ĵ�
      {
        num_points_h++;//ͳ��ˮƽ�в�������
      }
    }
    if(num_points_h>20) //�ҵ���һ������20���ز������
    {
      top = fr_y;
      break;
    }
  }

  //���µ����ҵ���һ�в���ϴ��
  for(fr_y=iPicH-1; fr_y>=0; fr_y--) //����ÿһ��
  {
    num_points_h = 0;
    for(fr_x=0; fr_x<iPicW; fr_x++) //����ÿһ��
    {
      if(diff[fr_y*iPicW+fr_x]) //������в���ĵ�
      {
        num_points_h++;//ͳ��ˮƽ�в�������
      }
    }
    if(num_points_h>20) //�ҵ���һ������20���ز������
    {
      bottom = fr_y;
      break;
    }
  }

  //�������ҵ���һ�в���ϴ��
  for(fr_x=0; fr_x<iPicW; fr_x++) //����ÿһ��
  {
    num_points_v = 0;
    for(fr_y=0; fr_y<iPicH; fr_y++) //����ÿһ��
    {
      if(diff[fr_y*iPicW+fr_x]) //������в���ĵ�
      {
        num_points_v++;//ͳ�ƴ�ֱÿ�в�������
      }
    }
    if(num_points_v>20) //�ҵ���һ������20���ز������
    {
      left = fr_x;
      break;
    }
  }

  //���ҵ����ҵ���һ�в���ϴ��
  for(fr_x=iPicW-1; fr_x>=0; fr_x--) //����ÿһ��
  {
    num_points_v = 0;
    for(fr_y=0; fr_y<iPicH; fr_y++) //����ÿһ��
    {
      if(diff[fr_y*iPicW+fr_x]) //������в���ĵ�
      {
        num_points_v++;//ͳ�ƴ�ֱÿ�в�������
      }
    }
    if(num_points_v>20) //�ҵ���һ������20���ز������
    {
      right = fr_x;
      break;
    }
  }

  diff_width = right - left + 1;
  diff_height = bottom - top + 1;

  result = 0;

  if(diff_width*2 > iPicW && diff_height*10 > iPicH*4) //����ռ��������ʾ�������
  {
    result = 1;
  }
  else if(diff_height*10 > iPicH*4 && left > 0 && left*4 < iPicW) //ֻ�������е����Ҳ��
  {
    result = 1;
  }
  else if(diff_width*2 > iPicW && top > 0 && top*2 < iPicH) //ֻ���ϰ���е�Ҳ��
  {
    result = 1;
  }
  else if(left*4 < iPicW && left > 0 && top > 0 && top*2 < iPicH) //���Ͻ�
  {
    result = 1;
  }

  return result;
}

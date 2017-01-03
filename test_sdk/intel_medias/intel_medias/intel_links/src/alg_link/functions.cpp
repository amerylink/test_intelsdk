
#include "../inc/alg_link/PPT_Index_ALL.h"

//统计当前帧和上一帧之间的差异
//返回是否是有变化
//org数据带跨度step
//顺便完成将原始数据去掉跨度存入cur内存的操作
//dst: 保存差异的帧
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

  *out_diff_sum = diff_sum;//保存有差异的像素个数

  //差异总和小于1000或者是差异个数小于图像大小的1/1000则认为是无变化
//  if(diff_sum < 1000 || diff_count < iPicW*iPicH/1000)
  if(diff_sum < 1000 || diff_count*1000 < iPicW*iPicH)
  {
    return 0; //判断为没有差异
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

  *out_diff_sum = diff_sum;//保存有差异的像素个数

  //差异总和小于1000或者是差异个数小于图像大小的1/1000则认为是无变化
//  if(diff_sum < 1000 || diff_count < iPicW*iPicH/1000)
  if(diff_sum < 1000 || diff_count*1000 < iPicW*iPicH)
  {
    return 0; //判断为没有差异
  }
  
  return 1;
}

//统计上一帧和索引帧之间的显著差异
//返回是否是有明显变化
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

  //差异个数小于图像大小的1/100则认为是无显著变化
  if(diff_count*100 < pic_size)
  {
    return 0; //判断为没有差异
  }
  
  return 1; //差异个数大于图像的1/100认为是有显著变化
}

//根据当前帧和上一帧在mark中标记差异部分的匹配程度来确定是否是新的索引
int CheckNewIndex(BYTE *src, BYTE *mark, int pic_size)
{
  int i;
  size_t count,sum;

  count = sum = 0;

  for(i=0; i<pic_size; i++)
  {
    if(mark[i]) 
    {
      sum++; //统计原来改变过的像素总和
      if(src[i] == 0) //在差异区域中，统计无差异的点的个数
      {
        count++; //统计匹配点
      }
    }
  }

  //匹配度
  if(count*2 >= sum)
  {
    return 0; //匹配度较高则不是新的索引
  }
  else
  {
    return 1;
  }
}

// 差异图投影法确定是否可能是索引
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

  //从上到下找到第一行差异较大点
  for(fr_y=0; fr_y<iPicH; fr_y++) //遍历每一行
  {
    num_points_h = 0;
    for(fr_x=0; fr_x<iPicW; fr_x++) //遍历每一列
    {
      if(diff[fr_y*iPicW+fr_x]) //如果是有差异的点
      {
        num_points_h++;//统计水平行差异点个数
      }
    }
    if(num_points_h>20) //找到第一个大于20像素差异的行
    {
      top = fr_y;
      break;
    }
  }

  //从下到上找到第一行差异较大点
  for(fr_y=iPicH-1; fr_y>=0; fr_y--) //遍历每一行
  {
    num_points_h = 0;
    for(fr_x=0; fr_x<iPicW; fr_x++) //遍历每一列
    {
      if(diff[fr_y*iPicW+fr_x]) //如果是有差异的点
      {
        num_points_h++;//统计水平行差异点个数
      }
    }
    if(num_points_h>20) //找到第一个大于20像素差异的行
    {
      bottom = fr_y;
      break;
    }
  }

  //从左到右找到第一列差异较大点
  for(fr_x=0; fr_x<iPicW; fr_x++) //遍历每一列
  {
    num_points_v = 0;
    for(fr_y=0; fr_y<iPicH; fr_y++) //遍历每一行
    {
      if(diff[fr_y*iPicW+fr_x]) //如果是有差异的点
      {
        num_points_v++;//统计垂直每列差异点个数
      }
    }
    if(num_points_v>20) //找到第一个大于20像素差异的列
    {
      left = fr_x;
      break;
    }
  }

  //从右到左找到第一列差异较大点
  for(fr_x=iPicW-1; fr_x>=0; fr_x--) //遍历每一列
  {
    num_points_v = 0;
    for(fr_y=0; fr_y<iPicH; fr_y++) //遍历每一行
    {
      if(diff[fr_y*iPicW+fr_x]) //如果是有差异的点
      {
        num_points_v++;//统计垂直每列差异点个数
      }
    }
    if(num_points_v>20) //找到第一个大于20像素差异的列
    {
      right = fr_x;
      break;
    }
  }

  diff_width = right - left + 1;
  diff_height = bottom - top + 1;

  result = 0;

  if(diff_width*2 > iPicW && diff_height*10 > iPicH*4) //基本占满整个显示区域的算
  {
    result = 1;
  }
  else if(diff_height*10 > iPicH*4 && left > 0 && left*4 < iPicW) //只有左半边有的情况也算
  {
    result = 1;
  }
  else if(diff_width*2 > iPicW && top > 0 && top*2 < iPicH) //只有上半边有的也算
  {
    result = 1;
  }
  else if(left*4 < iPicW && left > 0 && top > 0 && top*2 < iPicH) //左上角
  {
    result = 1;
  }

  return result;
}

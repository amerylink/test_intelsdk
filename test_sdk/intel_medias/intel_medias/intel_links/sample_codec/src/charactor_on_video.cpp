/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2011-2014 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#include <ft2build.h>
#include <freetype/freetype.h>
#include <math.h>

#include <iconv.h>
#include <stdio.h>
#include "charactor_on_video.h"

#pragma comment(lib,"freetype262d.lib")
#pragma comment(lib,"libiconv.lib")

int CodeConvert(char* pSrcCode, char* pDstCode, char* pSrcBuff, int iSrcLen, char* pDstBuff, int iDstLen)
{
    iconv_t cd;
    int ret;
    char* pIn = pSrcBuff;
    char* pOut = pDstBuff;
    cd = iconv_open(pDstCode, pSrcCode);
    if(cd == 0)
    {
        return 0;
    }
    memset(pDstBuff, 0, iDstLen);
    /*if(iconv(cd, pIn, &iSrcLen, pDstBuff, &iDstLen) == -1)
    {
        return 0;
    }*/

    iconv_close(cd);
    return 1; 
}


int FontOnPic(unsigned char* pVideo, char* pFont, int iPosX, int iPosY, int iPicWidth, int iPicHeight)
{
	FT_Library library;
	FT_Face face;
	FT_Error error;
	FT_UInt charIdx;
	//string  sFont;//="aaa";
	char*  buffer;             //video date
	int  startX,startY;       //font posision

     //printf("step loop start\n");
	//init
	error = FT_Init_FreeType(&library);
	//printf("step1 err = %d\n", error);

	//creat
	error = FT_New_Face(library, "/home/quxin410/sdk/STSONG.TTF", 0, &face);
     //error = FT_New_Face(library, "/home/quxin410/sdk/test.ttf", 0, &face);
	//printf("step2 err = %d , unknown macro =%d\n", error, FT_Err_Unknown_File_Format);
   
	//set
	//error = FT_Set_Char_Size(face, 16*64, 16*64, 96, 96);
     error = FT_Set_Char_Size(face, 0, 16*32, 300, 300);
     //error = FT_Set_Pixel_Sizes(face, 0, 16);
	//printf("step3 err = %d\n", error);

	//index
     int iIndex;
     //CodeConvert(GB2312, UTF8, pFont, strlen(pFont), );
	charIdx = FT_Get_Char_Index(face, (0x5317));
    //g_m++;
     printf("char index = %d\n", charIdx);

	//load pic
	error = FT_Load_Glyph(face, charIdx, FT_LOAD_DEFAULT);
	//printf("step4 err = %d\n", error);

     if(face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
     {
         error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
         //error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
         //printf("step5 err = %d\n", error);        
     }
	//


	//cp to buff
    //printf("%d - %d\n", face->glyph->bitmap.rows, face->glyph->bitmap.width);
     #ifdef TEST_FONT
     iPosX = 400;
     iPosY = 400;
     iPicWidth = 1920;

     #endif
    int n = 0;
    for(int i = iPosY; i < iPosY + face->glyph->bitmap.rows; i++)
    {
        for(int j = iPosX; j < iPosX + face->glyph->bitmap.width; j++)
        {
            if(face->glyph->bitmap.buffer[n] != 0)
            {
             *(pVideo + i * iPicWidth + j) = (face->glyph->bitmap.buffer[n]);
            
            //printf("%d",face->glyph->bitmap.buffer[n]);
            //n++;               
            }
            n++;
        }
        //printf("\n");
    }

    //index
     //static int g_m = 0;
    charIdx = FT_Get_Char_Index(face, 0x5318);//(face, (0x5318));
    //g_m++;
     printf("char index = %d\n", charIdx);

    //load pic
    error = FT_Load_Glyph(face, charIdx, FT_LOAD_DEFAULT);
    //printf("step4 err = %d\n", error);

     if(face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
     {
         error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
         //error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
         //printf("step5 err = %d\n", error);        
     }
    //


    //cp to buff
    //printf("%d - %d\n", face->glyph->bitmap.rows, face->glyph->bitmap.width);
     n = 0;
    for(int i = 400; i < 400 + face->glyph->bitmap.rows; i++)
    {
        for(int j = 480; j < 480 + face->glyph->bitmap.width; j++)
        {
            if(face->glyph->bitmap.buffer[n] != 0)
            {
             *(pVideo + i * 1280 + j) = (face->glyph->bitmap.buffer[n]);
            
            //printf("%d",face->glyph->bitmap.buffer[n]);
            //n++;               
            }
            n++;
        }
        //printf("\n");
    }



    //index
     //static int g_m = 0;
    charIdx = FT_Get_Char_Index(face, (0x5319));
    //g_m++;
     //printf("char index = %d\n", charIdx);

    //load pic
    error = FT_Load_Glyph(face, charIdx, FT_LOAD_DEFAULT);
    //printf("step4 err = %d\n", error);

     if(face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
     {
         error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
         //error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
         //printf("step5 err = %d\n", error);        
     }
    //


    //cp to buff
    //printf("%d - %d\n", face->glyph->bitmap.rows, face->glyph->bitmap.width);
     n = 0;
    for(int i = 400; i < 400 + face->glyph->bitmap.rows; i++)
    {
        for(int j = 560; j < 560 + face->glyph->bitmap.width; j++)
        {
            if(face->glyph->bitmap.buffer[n] != 0)
            {
             *(pVideo + i * 1280 + j) = (face->glyph->bitmap.buffer[n]);
            
            //printf("%d",face->glyph->bitmap.buffer[n]);
            //n++;               
            }
            n++;
        }
        //printf("\n");
    }


    //index
     //static int g_m = 0;
    charIdx = FT_Get_Char_Index(face, (0x5419));
    //g_m++;
     //printf("char index = %d\n", charIdx);

    //load pic
    error = FT_Load_Glyph(face, charIdx, FT_LOAD_DEFAULT);
    //printf("step4 err = %d\n", error);

     if(face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
     {
         error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
         //error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
         //printf("step5 err = %d\n", error);        
     }
    //


    //cp to buff
    //printf("%d - %d\n", face->glyph->bitmap.rows, face->glyph->bitmap.width);
     n = 0;
    for(int i = 400; i < 400 + face->glyph->bitmap.rows; i++)
    {
        for(int j = 640; j < 640 + face->glyph->bitmap.width; j++)
        {
            if(face->glyph->bitmap.buffer[n] != 0)
            {
             *(pVideo + i * 1280 + j) = (face->glyph->bitmap.buffer[n]);
            
            //printf("%d",face->glyph->bitmap.buffer[n]);
            //n++;               
            }
            n++;
        }
        //printf("\n");
    }
    //==========================
    FT_Done_FreeType(library);
	return 0;
}


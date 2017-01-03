#include "encLink.h"
#include "../inc/alg_link/algLink.h"
#include "mfx_samples_config.h"
#include "pipeline_encode.h"
#include "pipeline_user.h"
#include "pipeline_region_encode.h"
#include "pipeline_decode.h"
#include "SDK_codec.h"

#ifdef WIN32
#include "windows.h"
#define usleep	Sleep
#endif

Int32 AlgSetScdAction(AlgScdObj *scdObj, void (*Action)(Int32))
{

    if(scdObj != NULL)
    {
        scdObj->action = Action;
    }

    return 0;
}

Int32 algScdCreateInstObj(AlgScdObj *scdObj, AlgScdParams *prm)
{
    scdObj->m_user_param.width  =   prm->width;
	scdObj->m_user_param.height =  prm->height;
	scdObj->m_user_param.kd_y   =  prm->width;
	scdObj->m_user_param.kd_uv  =  prm->width/2;
	scdObj->m_user_param.yuv_mode = 2;
	scdObj->m_user_param.static_frm_num = -1;
	scdObj->m_user_param.isIndex  = 0;
	scdObj->m_user_param.indexNum = 0;
	int size = PPT_Index_mem_size(&scdObj->m_user_param);
	scdObj->m_user_param.prog_out_memory = (int *) malloc(size);
	memset(scdObj->m_user_param.prog_out_memory,0,size);
	PPT_Index_init(&scdObj->m_user_param,&scdObj->m_Image);
    scdObj->ptr[0] =  (Int8*)malloc(1920*1080);
    scdObj->ptr[1] =  (Int8*)malloc(1920*1080);
    scdObj->ptr[2] =  (Int8*)malloc(1920*1080);

    return 0;
}


static Int32 algLink_createQue(ALG_LINK_INFO *info)
{
	Int32 status = LINK_SOK;
	Int32 i = 0;
	//FILE *fSrcWrite = NULL;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}


	info->EmpFullQueHndl.isConnect = LINK_FALSE;

    if(info->createInfo.queLen > ALG_LINK_MAX_ALLOC_BUFF_NUM || 0 == info->createInfo.queLen)
	{
		info->createInfo.queLen = ALG_LINK_MAX_ALLOC_BUFF_NUM;
	}

	
	/*创建空包队列*/
	status = LINK_queCreate(&(info->EmpFullQueHndl.pEmpQue),info->createInfo.queLen);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create emp que is error!\n");
		return LINK_EFAIL;		
	}
    
	/*创建满包队列*/
	status = LINK_queCreate(&(info->EmpFullQueHndl.pFullQue),info->createInfo.queLen);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create full que is error!\n");
		return LINK_EFAIL;		
	}

	return status;
}


Int32 algLink_createParaInit(ALG_CREATE_INFO *createInfo)
{
	Int32 status = LINK_SOK;

	if(NULL == createInfo)
	{
//		nslog(NS_ERROR,"createInfo is NULL\n");
		return LINK_EFAIL;
	}
  
    createInfo->algscdPrm.width = 352;
    createInfo->algscdPrm.height= 288;
	
	return status;
}


static Int32 AlgScd_Process(AlgScdObj *Obj,FRAME_Buf *pFrame)
{

    if(NULL == Obj || NULL == pFrame)
	{
//		nslog(NS_ERROR,"para is NULL\n");
		return LINK_EFAIL;
	}

    mfxFrameData& pData = pFrame->pSurfaces->Data;
    mfxFrameInfo& pInfo = pFrame->pSurfaces->Info;


    Uint32 i,j;
    for(i = 0; i < pInfo.CropH; i++)
    {
        memcpy(Obj->ptr[0] + Obj->m_user_param.kd_y*i , pData.Y + pData.Pitch*i, pInfo.CropW);
    }


    for(i = 0; i < pInfo.CropH/2; i++)
    {
         for(j = 0; j < pInfo.CropW; j++)
         {
            if(j%2 == 0)
            {
                *(Obj->ptr[1] + Obj->m_user_param.kd_uv*i + j/2) = *(pData.UV + pData.Pitch*i + j);
            }
            else
            {
                *(Obj->ptr[2] + Obj->m_user_param.kd_uv*i + j/2) = *(pData.UV + pData.Pitch*i + j);
            }
         }
    }  

   
   
    Obj->m_user_param.p_org_Ypels = (unsigned char *)Obj->ptr[0];
    Obj->m_user_param.p_org_Upels = (unsigned char *)Obj->ptr[1];
    Obj->m_user_param.p_org_Vpels = (unsigned char *)Obj->ptr[2];
    Obj->m_user_param.isIndex     = 0;
    PPT_Index_run(&Obj->m_Image,&Obj->m_user_param);

    if(Obj->m_user_param.isIndex) 
        return true;

    return  false;
}

static void* AlgLink_tskRun(void* arg)
{
	if(NULL == arg)
	{
//		nslog(NS_ERROR,"arg is NULL\n");
		return NULL;
	}
//	prctl(PR_SET_NAME, "AlgLink_tskRun", NULL, NULL, NULL);
	Int32 status = LINK_SOK;
	FRAME_Buf *InFrame = NULL;
	FRAME_Buf *OutFrame = NULL;
	ALG_LINK_INFO *info = (ALG_LINK_INFO *)arg;
     Uint32 *msg = NULL;
    
	Uint32 framNum = 0;
	Uint32 startTime = 0;
	Uint32 CurrenTime = 0;

	while(!(info->exitTsk))
	{
		if(LINK_FALSE == info->tskEnable)
		{
			usleep(30);
			continue;
		}


		if((NULL == info->PreEmpFullQueHndl) || (LINK_FALSE == info->PreEmpFullQueHndl->isConnect))
		{
			usleep(30);
			continue;
		}
	

		/*获取输出buff*/
		status = LINK_queGet(&(info->PreEmpFullQueHndl->pFullQue),(void **)(&OutFrame),LINK_TIMEOUT_FOREVER);
		if(LINK_EFAIL == status)
		{
			continue;
		}

//		CurrenTime = link_get_run_time();

        static unsigned int i = 0;

        if(i %10 == 0)
        {
    
        		status = AlgScd_Process(&info->algscdObj, OutFrame);
        		if(true == status)
        		{
//        			nslog(NS_INFO,"=========AlgScd_Process is True!\n");
                 if(info->algscdObj.action)
                 {
                     info->algscdObj.action(1);
                 }
        		}
        }
        i++;
	//	printf("===========alg process time:%d\n",link_get_run_time() - CurrenTime);

		/*还输出满包*/
		status = LINK_quePut(&(info->PreEmpFullQueHndl->pEmpQue),(void *)OutFrame,LINK_TIMEOUT_FOREVER);
         framNum++;
		CurrenTime = link_get_run_time();
		if((CurrenTime - startTime) >= 5000)
		{
//			nslog(NS_INFO,"=========akgLink_Process:fps:%d\n",framNum / 5);
			framNum = 0;
			startTime = link_get_run_time();
			
		}
	}

	LINK_tskDetach();
	LINK_tskExit(0);

	return NULL;
}


Int32 algLink_create(ALG_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}
	
	info->exitTsk = LINK_FALSE;
	info->tskEnable = LINK_FALSE;
	info->PreEmpFullQueHndl = NULL;

	status = pthread_mutex_init(&info->lock, NULL);
//	if(status!=LINK_SOK)
//		nslog(NS_ERROR,"pthread_mutex_init is error\n");


   
    //创建队列
	status = algLink_createQue(info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create que is error!\n");
		return LINK_EFAIL;
	}
    

    algScdCreateInstObj(&info->algscdObj, &info->createInfo.algscdPrm);

   	status = LINK_tskCreate(&info->tskHandle,AlgLink_tskRun,ALGLINK_TSK_PRI,0,info);
	if(LINK_EFAIL == status)
	{
//		nslog(NS_ERROR,"create task is error!\n");
	}
   
	return status;
}

Int32 algLink_release(ALG_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->exitTsk = LINK_TRUE;

	LINK_tskJoin(&info->tskHandle);

	return status;
}

Int32 algLink_enable(ALG_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->tskEnable = LINK_TRUE;

	return status;
}


Int32 algLink_connect(ALG_LINK_INFO *info,LINK_EmpFullQueHndl *QueHndl)
{
	Int32 status = LINK_SOK;
	
	if(NULL == info || NULL == QueHndl)
	{
//		nslog(NS_ERROR,"info or QueHndl is NULL\n");
		return LINK_EFAIL;
	}

	info->PreEmpFullQueHndl = QueHndl;

	info->PreEmpFullQueHndl->isConnect = LINK_TRUE;

	return status;
}

Int32 algLink_disable(ALG_LINK_INFO *info)
{
	Int32 status = LINK_SOK;

	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return LINK_EFAIL;
	}

	info->tskEnable = LINK_FALSE;

	return status;
}

void algLink_printf(ALG_LINK_INFO *info)
{
	if(NULL == info)
	{
//		nslog(NS_ERROR,"info is NULL\n");
		return;
	}

	//nslog(NS_INFO,"==============swmsLink param %d:==============\n",info->createInfo.chNo);

	return;
}



#include "linkTask.h"


int LINK_tskCreate(LINK_TskHndl *hndl, LINK_TskEntryFunc entryFunc, Uint32 pri, Uint32 stackSize, void *prm)
{
	int status=LINK_SOK;
	pthread_attr_t thread_attr;
	struct sched_param schedprm;

	if(hndl == NULL)
	{
		nslog(NS_ERROR,"LINK_thrCreate\n");
		return LINK_EFAIL;
	}

	// initialize thread attributes structure
	status = pthread_attr_init(&thread_attr);

	if(status!=LINK_SOK) {
		nslog(NS_ERROR,"LINK_thrCreate() - Could not initialize thread attributes\n");
		return status;
	}

	if(stackSize!=LINK_THR_STACK_SIZE_DEFAULT)
		pthread_attr_setstacksize(&thread_attr, stackSize);

	status |= pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
	status |= pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);

	if(pri>LINK_THR_PRI_MAX)   
		pri=LINK_THR_PRI_MAX;
	else if(pri<LINK_THR_PRI_MIN)   
		pri=LINK_THR_PRI_MIN;

	schedprm.sched_priority = pri;
	status |= pthread_attr_setschedparam(&thread_attr, &schedprm);

	if(status !=LINK_SOK) {
		nslog(NS_ERROR,"LINK_thrCreate() - Could not initialize thread attributes\n");
		goto error_exit;
	}

	status = pthread_create(&hndl->hndl, &thread_attr, entryFunc, prm);

	if(status!=LINK_SOK) {
		nslog(NS_ERROR,"LINK_thrCreate() - Could not create thread [%d]\n", status);
	}

error_exit:  
	pthread_attr_destroy(&thread_attr);

  return status;
}

void LINK_tskDetach(void)
{
	pthread_detach(pthread_self());
}

int LINK_tskJoin(LINK_TskHndl *hndl)
{
	int status=LINK_SOK;
	void *returnVal;

	if(hndl == NULL || hndl->hndl == 0)
	{
		nslog(NS_ERROR,"LINK_thrJoin\n");
		return LINK_EFAIL;
	}

	status = pthread_join(hndl->hndl, &returnVal); 

	return status;    
}

int LINK_tskDelete(LINK_TskHndl *hndl)
{

	int status=LINK_SOK;
	if(hndl == NULL || hndl->hndl == 0)
	{
		nslog(NS_ERROR,"LINK_thrDelete\n");
		return LINK_EFAIL;
	}

	status |= pthread_cancel(hndl->hndl); 
	status |= LINK_tskJoin(hndl);
	return status;    
}

int LINK_tskChangePri(LINK_TskHndl *hndl, Uint32 pri)
{
	int status = LINK_SOK;
	struct sched_param schedprm;  

	if(hndl == NULL || hndl->hndl == 0)
	{
		nslog(NS_ERROR,"LINK_thrChangePri\n");
		return LINK_EFAIL;
	}

	if(pri>LINK_THR_PRI_MAX)   
		pri=LINK_THR_PRI_MAX;
	else if(pri<LINK_THR_PRI_MIN)   
		pri=LINK_THR_PRI_MIN;

	schedprm.sched_priority = pri;  
	status |= pthread_setschedparam(hndl->hndl, SCHED_FIFO, &schedprm);

	return status;
}

int LINK_tskExit(void *returnVal)
{
	pthread_exit(returnVal);
	return LINK_SOK;
}

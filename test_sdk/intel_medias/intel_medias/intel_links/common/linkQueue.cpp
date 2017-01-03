#include "linkQueue.h"

Int32 LINK_queCreate(LINK_QueHndl *hndl, Uint32 maxLen)
{
	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t cond_attr;
	Int32 status=LINK_SOK;

	hndl->curRd = hndl->curWr = 0;
	hndl->count = 0;
	hndl->len   = maxLen;
	hndl->queue = (Uint8 **)malloc(sizeof(Uint8 *)*hndl->len);

	if(hndl->queue==NULL) {
//		nslog(NS_ERROR,"LINK_queCreate() = %d \r\n", status);
		return LINK_EFAIL;
	}

	status |= pthread_mutexattr_init(&mutex_attr);
	status |= pthread_condattr_init(&cond_attr);  

	status |= pthread_mutex_init(&hndl->lock, &mutex_attr);
	status |= pthread_cond_init(&hndl->condRd, &cond_attr);    
	status |= pthread_cond_init(&hndl->condWr, &cond_attr);  

	if (status != LINK_SOK)
		//		nslog(NS_ERROR,"LINK_queCreate() = %d \r\n", status);
		return LINK_EFAIL;

	pthread_condattr_destroy(&cond_attr);
	pthread_mutexattr_destroy(&mutex_attr);

	return status;
}

Int32 LINK_queDelete(LINK_QueHndl *hndl)
{
	if(hndl->queue!=NULL)
		free(hndl->queue);

	pthread_cond_destroy(&hndl->condRd);
	pthread_cond_destroy(&hndl->condWr);
	pthread_mutex_destroy(&hndl->lock);  

	return LINK_SOK;
}



Int32 LINK_quePut(LINK_QueHndl *hndl, void *value, Uint32 timeout)
{
	Int32 status = LINK_EFAIL;

	pthread_mutex_lock(&hndl->lock);
	while(1) {
		if( hndl->count < hndl->len ) {
			hndl->queue[hndl->curWr] = (Uint8 *)value;
			hndl->curWr = (hndl->curWr+1)%hndl->len;
			hndl->count++;
			status = LINK_SOK;
			pthread_cond_signal(&hndl->condRd);
			break;
		} else {
			if(timeout == LINK_TIMEOUT_NONE)
				break;

			status = pthread_cond_wait(&hndl->condWr, &hndl->lock);
		}
	}

	pthread_mutex_unlock(&hndl->lock);

	return status;
}


Int32 LINK_queGet(LINK_QueHndl *hndl, void **value, Uint32 timeout)
{
	Int32 status = LINK_EFAIL;

	pthread_mutex_lock(&hndl->lock);

	while(1) {
		if(hndl->count > 0 ) {

			if(value!=NULL) {
				*value = (void *)hndl->queue[hndl->curRd];
			}

			hndl->curRd = (hndl->curRd+1)%hndl->len;
			hndl->count--;
			status = LINK_SOK;
			pthread_cond_signal(&hndl->condWr);

			break;
		}
		else {
			if(timeout == LINK_TIMEOUT_NONE)
				break;

			status = pthread_cond_wait(&hndl->condRd, &hndl->lock);
			if (status == LINK_EFAIL)
			{
				int b = 0;
			}
		}
	}
	pthread_mutex_unlock(&hndl->lock);
	if (status == LINK_EFAIL)
	{
		int b = 0;
	}
	return status;
}


Uint32 LINK_queGetQueuedCount(LINK_QueHndl *hndl)
{
	Uint32 queuedCount = 0;

	pthread_mutex_lock(&hndl->lock);
	queuedCount = hndl->count;
	pthread_mutex_unlock(&hndl->lock);
	return queuedCount;
}

Int32 LINK_quePeek(LINK_QueHndl *hndl, void **value)
{
	int status = LINK_EFAIL;
	pthread_mutex_lock(&hndl->lock);
	if(hndl->count > 0 ) {
		if(value!=NULL) {
			*value = (void *)hndl->queue[hndl->curRd];
		}
	}
	pthread_mutex_unlock(&hndl->lock);

	return status;
}

Bool LINK_queIsEmpty(LINK_QueHndl *hndl)
{
	Bool isEmpty;

	pthread_mutex_lock(&hndl->lock);
	if (hndl->count == 0)
	{
		isEmpty = LINK_TRUE;
	}
	else
	{
		isEmpty = LINK_FALSE;
	}
	pthread_mutex_unlock(&hndl->lock);

	return isEmpty;
}

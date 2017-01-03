#ifndef _LINK_QUEUE_H_
#define _LINK_QUEUE_H_
#include "linkCommon.h"

typedef struct {

	Uint32 curRd;
	Uint32 curWr;
	Uint32 len;
	Uint32 count;

	Uint8 **queue;

	pthread_mutex_t lock;
	pthread_cond_t  condRd;
	pthread_cond_t  condWr;
  
}LINK_QueHndl;

typedef struct {
	LINK_QueHndl pEmpQue;
	LINK_QueHndl pFullQue;
	Bool			 isConnect;
}LINK_EmpFullQueHndl;

Int32 LINK_queCreate(LINK_QueHndl *hndl, Uint32 maxLen);
Int32 LINK_queDelete(LINK_QueHndl *hndl);
Int32 LINK_quePut(LINK_QueHndl *hndl, void  *value, Uint32 timeout);
Int32 LINK_queGet(LINK_QueHndl *hndl, void **value, Uint32 timeout);
Int32 LINK_quePeek(LINK_QueHndl *hndl, void **value);
Uint32 LINK_queGetQueuedCount(LINK_QueHndl *hndl);
Bool LINK_queIsEmpty(LINK_QueHndl *hndl);

#endif

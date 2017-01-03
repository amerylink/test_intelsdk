#ifndef _LINK_TASK_H_
#define _LINK_TASK_H_

#include "linkCommon.h"

#define LINK_THR_PRI_MAX                 sched_get_priority_max(SCHED_RR)
#define LINK_THR_PRI_MIN                 sched_get_priority_min(SCHED_RR)

#define LINK_THR_PRI_DEFAULT             ( LINK_THR_PRI_MIN + (LINK_THR_PRI_MAX-LINK_THR_PRI_MIN)/2 )

#define LINK_THR_STACK_SIZE_DEFAULT      0

typedef void * (*LINK_TskEntryFunc)(void *);

typedef struct {

  pthread_t      hndl;
  
} LINK_TskHndl;

int LINK_tskCreate(LINK_TskHndl *hndl, LINK_TskEntryFunc entryFunc, Uint32 pri, Uint32 stackSize, void *prm);
void LINK_tskDetach(void);
int LINK_tskJoin(LINK_TskHndl *hndl);
int LINK_tskDelete(LINK_TskHndl *hndl);
int LINK_tskChangePri(LINK_TskHndl *hndl, Uint32 pri);
int LINK_tskExit(void *returnVal);


#endif

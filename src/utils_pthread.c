/****************************************************
*
* 线程创建部分公共接口
*
****************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <errno.h>

#include "utils_pthread.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct  tagTHREAD_
{
	char		strName[32];
	int			bDestory;
	int			bTrigger;
	int			bRuning;
	void *		pUserCtx;
	int			bUsedCtx;
	pthread_t 	thr;
    pthread_mutex_t lock;
} stTHREAD,*pstTHREAD;

#define   API_THREAD_STACK_SIZE  8*1024

void * Utils_Pthread_Create(int nThrPriority, int nThrStackSize,Utils_Pthread_Func pFunc,void * arg)
{
	int nThreadStackSize = API_THREAD_STACK_SIZE;
	int nTimes = 10;
	pstTHREAD pThread = (pstTHREAD)malloc(sizeof(stTHREAD));
	if(NULL == pThread)
	{
		return NULL;
	}
	//memset(&pThread , 0 , sizeof(stTHREAD));
    pthread_mutex_init(&pThread->lock, NULL);

	nThreadStackSize = (1024 > nThrStackSize) ? API_THREAD_STACK_SIZE : nThrStackSize;
    pthread_mutex_trylock(&pThread->lock);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, nThreadStackSize);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	pThread->bDestory = false;
	pThread->bRuning = true;
	pThread->bTrigger = true;
	pThread->bUsedCtx = false;
	pThread->pUserCtx = (NULL == arg) ? NULL : arg;
	if(0 != pthread_create(&pThread->thr, &attr, pFunc, (void *)pThread))
	{
		pthread_attr_destroy(&attr);
		pthread_mutex_unlock(&pThread->lock);
		goto ERROR;
	}
	pthread_attr_destroy(&attr);
    pthread_mutex_unlock(&pThread->lock);

	while(nTimes--)
	{
		if(true == pThread->bUsedCtx)
		{
			break;
		}
		usleep(1000);
	}

	return (void *)pThread;

ERROR:
	if(NULL != pThread)
	{
		pthread_mutex_destroy(&pThread->lock);
		free(pThread);
	}
	return NULL;

}

int Utils_Pthread_Destory(void * hThread)
{
	int nTimes = 1000;
	pstTHREAD pThread = (pstTHREAD)hThread;
	if(NULL == pThread)
	{
		return false;
	}
	if(true == pThread->bDestory)
	{
		return false;
	}

	pthread_mutex_lock(&pThread->lock);
	pThread->bTrigger = false;
	pThread->bDestory = true;
	pthread_mutex_unlock(&pThread->lock);
	while(nTimes--)
	{
		if(false == pThread->bRuning)
		{
			break;
		}
		pthread_mutex_lock(&pThread->lock);
		pThread->bTrigger = false;
		pThread->bDestory = true;
		pthread_mutex_unlock(&pThread->lock);
		usleep(20*1000);
	}

	//pthread_cancel(pThread->thr);
	//pthread_join(pThread->thr, NULL);
	pthread_mutex_destroy(&pThread->lock);
	free(pThread);
	pThread = NULL;
	return true;
}

int Utils_Pthread_Triger(void * hThread)
{
	pstTHREAD pThread = (pstTHREAD)hThread;
	if(NULL == pThread)
	{
		return false;
	}
	return pThread->bTrigger;
}

void * Utils_Pthread_Cancel(void * hThread)
{
	pstTHREAD pThread = (pstTHREAD)hThread;
	if(NULL == pThread)
	{
		return NULL;
	}
    pthread_mutex_lock(&pThread->lock);
	pThread->bRuning = false;
	pThread->bTrigger = false;
    pthread_mutex_unlock(&pThread->lock);
	return NULL;
}

void * Utils_Pthread_SetName(char * pThrName)
{
	if(NULL == pThrName)
	{
		return NULL;
	}
	prctl(PR_SET_NAME, pThrName);
	return NULL;
}

void * Utils_Pthread_GetUserCtx(void * hThread)
{
	pstTHREAD pThread = (pstTHREAD)hThread;
	if(NULL == pThread)
	{
		return NULL;
	}
	pThread->bUsedCtx = true;
	if(NULL == pThread->pUserCtx)
	{
		return NULL;
	}
	return pThread->pUserCtx;
}


int Utils_Pthread_Lock(void * hThread)
{
	pstTHREAD pThread = (pstTHREAD)hThread;
	if(NULL == pThread)
	{
		return false;
	}
	pthread_mutex_lock(&pThread->lock);
	return true;
}
int Utils_Pthread_UnLock(void * hThread)
{
	pstTHREAD pThread = (pstTHREAD)hThread;
	if(NULL == pThread)
	{
		return false;
	}
    pthread_mutex_unlock(&pThread->lock);
	return true;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


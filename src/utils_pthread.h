/*
 * @Author: Danny Yu
 * @Copyright: 2022 Shenzhen Qualmesh Technology Co., Ltd.
 * @Date: 2022-09-21 19:58:58
 * @Description: 
 */
#ifndef _UTILS_PTHREAD_H_
#define _UTILS_PTHREAD_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


typedef void *(*Utils_Pthread_Func)(void *);

void *	Utils_Pthread_Create(int nThrPriority, int nThrStackSize,Utils_Pthread_Func pFunc,void * arg);
int 	Utils_Pthread_Destory(void * hThread);
int 	Utils_Pthread_Triger(void * hThread);
void * 	Utils_Pthread_Cancel(void * hThread);
void * 	Utils_Pthread_SetName(char * pThrName);
void * 	Utils_Pthread_GetUserCtx(void * hThread);
int 	Utils_Pthread_Lock(void * hThread);
int		Utils_Pthread_UnLock(void * hThread);

#define Utils_Pthread_CreateEx(pFunc,arg) Utils_Pthread_Create(0, 0, pFunc, arg)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif

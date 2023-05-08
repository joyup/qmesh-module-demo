/****************************************************
*
* QMESH PLC Module
*
****************************************************/

#include "plc_module_sdk.h"
#include "plc_module_protocol.h"
#include "common.h"
#include "qmesh_semaphore.h"
#include "semaphore.h"
#include "time.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */
#define CMD_WAITE_TIME_OUT              30      // 3S

sem_t gQiMSemHandle[QIM_SEM_BUTT];
uint8_t gSyncFlag[QIM_SEM_BUTT] = {0};	//为解决两次发送STA应答现象

/**
 * @brief 信号初始化
 *
 * @param none
 *
 * @return 0：成功 -1：失败
 */
int QiM_SemInit()
{
	int  i = 0;
	int s32Ret = 0;
	
	for(i = 0; i < QIM_SEM_BUTT; i++)
	{
		s32Ret = sem_init(gQiMSemHandle + i, 0, 0);

		if(s32Ret == -1)
		{
			LOGE("sem init failed,sem:%d\n", i);
			return s32Ret;
		}
	}

	return s32Ret;
}

/**
 * @brief 信号销毁
 *
 * @param none
 *
 * @return 0：成功 
 */
int QiM_SemDestroy()
{
	int  i = 0;
	
	for(i = 0; i < QIM_SEM_BUTT; i++)
	{
		sem_destroy(gQiMSemHandle+i);
	}

	return 0;
}

/**
 * @brief 信号post
 *
 * @param u32Cmd  命令枚举便
 *
 * @return 0：成功 其它：失败
 */
int QiM_SemPost(unsigned int u32Cmd)
{
	if ((u32Cmd <= QIM_SEM_NONE) || (u32Cmd >= QIM_SEM_BUTT))
		return -1;

	if(gSyncFlag[u32Cmd] == 0)
		return 0;
	
	gSyncFlag[u32Cmd] = 0;
	sem_post(gQiMSemHandle + u32Cmd);
	return 0;
}

/**
 * @brief 信号wait
 *
 * @param u32Cmd  	 命令枚举便
 * @param u32TimeOut 超时时间 单位 s
 *
 * @return 0：成功 其它：失败
 */
int QiM_SemWait(unsigned int u32Cmd, unsigned int u32TimeOut)
{
	if ((u32Cmd < QIM_SEM_NONE) || (u32Cmd >= QIM_SEM_BUTT))
		return -1;

	gSyncFlag[u32Cmd] = 1;

#if 1
	struct timespec ts;

	ts.tv_sec = time(NULL) + u32TimeOut;
    ts.tv_nsec = 0;

	return sem_timedwait(gQiMSemHandle + u32Cmd, &ts);

#else
	unsigned int u32Cnt = u32TimeOut * 10;

	do
	{
		if(sem_trywait(gQiMSemHandle + u32Cmd) == 0)
		{
			return 0;
		}

		if( errno != EAGAIN )
		{
			return -1;
		}

		usleep(100000); //100ms

		if (--u32Cnt < 1)
			return -1;

	} while(1);
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

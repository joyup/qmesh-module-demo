/*
 * @Author: Danny Yu
 * @Copyright: 2022 Shenzhen Qualmesh Technology Co., Ltd.
 * @Date: 2022-06-12 21:26:44
 * @Description: 
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

typedef enum _QIM_SEM_
{
	QIM_SEM_NONE = 0,
	QIM_SEM_PLC_DATA_PARSED,
	QIM_SEM_PLC_DATA_REPORTED,
	QIM_SEM_PLC_CCO_DATA_READY,

	QIM_SEM_BUTT
}QIM_SEM_EN;

int QiM_SemInit(void);
int QiM_SemDestroy(void);
int QiM_SemPost(unsigned int u32Cmd);
int QiM_SemWait(unsigned int u32Cmd, unsigned int u32TimeOut);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

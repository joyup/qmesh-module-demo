/*
 * @Author: Danny Yu
 * @Copyright 2022 Shenzhen Qualmesh Technology Co., Ltd.
 * @Date: 2022-06-10
 * @Description: Qualmesh SDK头文件
 */
#ifndef _QMESH_SDK_H_
#define _QMESH_SDK_H_

#ifdef ANDROID_NDK
#include "jni.h"
#endif

#ifdef __cplusplus
extern "C"{
#endif

typedef	enum _PLC_DATA_TYPE_
{
	PLC_DATA_TYPE_INT = 1,
	PLC_DATA_TYPE_BOOL,
	PLC_DATA_TYPE_STRING,
	PLC_DATA_TYPE_ENUM,
	PLC_DATA_TYPE_ARRAY
}PLC_DATA_TYPE;

/**
 * @description: 子设备数据上报回调函数
 * @param {unsignd char} *data    收到的子设备发过来的数据
 * @return {none}
 */
typedef void (*qmesh_dev_state_cb)(unsigned char *data);

typedef void (*qmesh_dev_remove_cb)(const char *dev_id);
typedef void (*qmesh_dev_online_cb)(const char *dev_id, const char *dev_type, const char *data);

/**
 * @description: 开始运行主程序
 * @return {int}  0：成功，-1：失败
 */
int qmesh_start();

/**
 * @description: 获取在线子设备列表
 * @param {none}
 * @return none
 */
void qmesh_get_all_dev_list();

/**
 * @description: 添加设备到白名单
 * @param {char} **dev_id   待添加的字符串形式mac地址数组指针
 * @param {int} count   添加的设备数量，一次性最大添加50个，可多次添加
 * @return {0} 成功，其他失败
 */
int qmesh_add_device_to_whitelist(const char **dev_id, int count);

/**
 * @description: 从白名单中删除设备
 * @param {char} **dev_id   待删除的字符串形式mac地址数组指针
 * @param {int} count   删除的设备数量，一次性最大删除50个
 * @return {0} 成功，其他失败
 */
int qmesh_delete_device(const char **dev_id, int count);

/**
 * @description: 允许设备入网
 * @return {0} 成功，其他失败
 */
int qmesh_enable_join_gateway(void);

/**
 * @description: 关闭设备入网
 * @return {0} 成功，其他失败
 */
int qmesh_disable_join_gateway(void);

/**
 * @description: 发送自定义协议数据到子设备
 * @param {uint8_t} *data
 * @param {int} len
 * @return {*}
 */
int qmesh_trans_data(uint8_t *data, int len);

int qmesh_uart_init(const char *uart_name, int s32BaudRate);

#ifdef __cplusplus
}
#endif


#endif


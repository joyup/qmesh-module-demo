/*
 * @Author: Danny Yu
 * @Copyright 2022 Shenzhen Qualmesh Technology Co., Ltd.
 * @Date: 2022-06-10
 * @Description: Qualmesh SDK
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "qmesh_sdk.h"
#include "common.h"
#include "plc_module_sdk.h"
#include "plc_module_protocol.h"
#include "qmesh_semaphore.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "semaphore.h"
#include <pthread.h>
#include "utils_pthread.h"

#ifdef ANDROID_NDK
#include "jni.h"
#endif

#define UART "/dev/ttyS1"

extern QIM_CCO_INFO_DATA gQimCcoInfo;
extern unsigned char gu8CcoRespErrCode;	//CCO Response State

uint8_t m_needRebootCCO = 0;
int uart_fd = -1;

static pthread_mutex_t uart_send_data_lock;

void qmesh_dev_remove(const char *dev_id){};
void qmesh_dev_online(const char *dev_id, const char *dev_type, const char *data){};
void qmesh_dev_state(unsigned char *data);
qmesh_dev_remove_cb dev_remove = qmesh_dev_remove;
qmesh_dev_online_cb dev_online = qmesh_dev_online;
qmesh_dev_state_cb dev_state = qmesh_dev_state;

void qmesh_dev_state(unsigned char *data)
{
	QIM_PLC_RX_DATA_HEAD *pRxDataHead = (QIM_PLC_RX_DATA_HEAD *)data;
	LOGD("Received %d bytes: from dev %02x%02x%02x%02x%02x%02x\n", pRxDataHead->u16UserDataLen, 
		pRxDataHead->u8SrcMac[0], pRxDataHead->u8SrcMac[1], pRxDataHead->u8SrcMac[2],
		pRxDataHead->u8SrcMac[3], pRxDataHead->u8SrcMac[4], pRxDataHead->u8SrcMac[5]);
	
	LOGD("Received data :\n");
	debug_printf_array(pRxDataHead->u8UserData, pRxDataHead->u16UserDataLen);

	//to do sta data process
	//...

}

int _qmesh_plc_send(unsigned char *u8CmdData, int s32Len)
{
    pthread_mutex_lock(&uart_send_data_lock);
	LOGD("Uart Sended %d bytes: %x %x %x %x ...\n", s32Len, u8CmdData[0], u8CmdData[1], u8CmdData[2], u8CmdData[3]);
	//uart_write(uartsocket, u8CmdData, s32Len);
	write(uart_fd, u8CmdData, s32Len);
    pthread_mutex_unlock(&uart_send_data_lock);
	return 0;
}

int qmesh_uart_init(const char *uart_name, int s32BaudRate)
{
	uart_fd = open(uart_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (uart_fd <= 0) {
		LOGE("open uart:%s error,ret:%d", uart_name, uart_fd);
		return -1;
	}
	else
	{
		struct termios oldtio;
		struct termios newtio;
		memset((void *)&newtio, 0, sizeof(struct termios));
		tcgetattr(uart_fd, &oldtio);

		newtio.c_cflag = CS8 | CLOCAL | CREAD;
		newtio.c_iflag = 0;	// IGNPAR | ICRNL
		newtio.c_oflag = 0;
		newtio.c_lflag = 0;	// ICANON

		cfsetispeed(&newtio, s32BaudRate);
		cfsetospeed(&newtio, s32BaudRate);

		newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
		newtio.c_cc[VMIN] = 1; /* blocking read until 1 character arrives */
		tcflush(uart_fd, TCIOFLUSH);
		tcsetattr(uart_fd, TCSANOW, &newtio);

		// 设置为非阻塞
		//fcntl(uart_file_handle, F_SETFL, O_NONBLOCK);

		LOGD("\nOpen uart hdndle: %d\n", uart_fd);

		return uart_fd;
	}
	return -1;
}

/**
 * @brief 监控串口及NetSocket的数据
 *
 * @param arg  参数
 *
 * @return null
 */
void* _qmesh_main_thread(void *arg)
{
	void* phThread = (void*)arg;

	int s32Ret = 0;
	unsigned char u8Buf[4096] __attribute__ ((aligned (32))) = {0};
	int s32BufLen = 0;
	QIM_PLC_FRAME_HEAD *pPlcFrameHead = (QIM_PLC_FRAME_HEAD *)u8Buf;

	LOGD("\nuart thread start\n");
	if(NULL == phThread)
	{
		goto ERROR;
	}
	
	//qmesh_uart_init(uart_dev_name, B460800);

	while (Utils_Pthread_Triger(phThread))
	{
		usleep(50000);

		s32Ret = read(uart_fd, u8Buf + s32BufLen, sizeof(u8Buf) - s32BufLen);
		if(s32Ret > 0)
		{
			#if 0
				printf("\e[1;46m STA Rx Data:");
				for(int i= 0; i<s32Ret;i++)
				{
					if(i%16 == 0)
						printf("\n");
					printf("%02x ", (u8Buf + s32BufLen)[i]);
				}
				printf("\e[0m\n");
			#endif
			s32BufLen += s32Ret;
			LOGD("Uart Received Len = %d\n", s32BufLen);
		}

		if (s32BufLen < 8)
			continue;

		if (pPlcFrameHead->u8Head != 0x48)
		{
			// reset cco
			//......
			s32BufLen = 0;
			memset(u8Buf, 0, sizeof(u8Buf));
			continue;
		}

		int payload_data_len = sizeof(QIM_PLC_FRAME_HEAD) + pPlcFrameHead->u16Len + 2; // Frame Head Len + User Data Len + CRC(2)
		if (s32BufLen < payload_data_len)
			continue;

		QiM_Sdk_ParseRespData(u8Buf, payload_data_len);

		s32BufLen -= payload_data_len;

		if (s32BufLen > 0)
		{
			// memcpy(u8Buf, u8Buf+payload_data_len, s32BufLen);	//memcpy复制数据会出错，具体原因未知
			for (int i = 0; i < s32BufLen; i++)
				u8Buf[i] = (u8Buf + payload_data_len)[i];
		}
		else
		{
			s32BufLen = 0;
			memset(u8Buf, 0, sizeof(u8Buf));
		}
	}

	ERROR:
	if (uart_fd > 0)
	{
		close(uart_fd);
		uart_fd = -1;
	}

	Utils_Pthread_Cancel(phThread);
	return NULL;
}

//开始运行主程序
int qmesh_start()
{
	int s32Ret;

	s32Ret = QiM_SemInit();
	if (s32Ret < 0)
	{
		LOGE("QiM_SemInit error,ret:%d\n", s32Ret);
		return -1;
	}

	//创建CCO通信监控线程
	Utils_Pthread_CreateEx(_qmesh_main_thread, NULL);

	return 0;
}

//添加设备到白名单中
int qmesh_add_device_to_whitelist(const char **dev_id, int count)
{
	uint8_t *data_buf = malloc(count * 6 + 2);
	if(data_buf == NULL)
	{
		LOGE("malloc error\n");
		return -1;
	}
	if(count>50)
	{
		LOGE("count out of range\n");
		return -1;
	}
	*((uint16_t *)data_buf) = count;
	for (int i = 0; i < count; i++)
	{
		HexStringToBytes(dev_id[i], data_buf + 2 + i * 6);
	}

	QiM_Sdk_Send_CMD(CCO_CMD_ADD_WL_NODE, data_buf, count * 6 + 2);
	int s32Ret = QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
	if (s32Ret < 0)
		LOGE("CCO_CMD_ADD_WL_NODE error,ret: %d\n", s32Ret);
	free(data_buf);
	return s32Ret;
}

void qmesh_get_all_dev_list()
{
	int s32Ret;
	char cco_mac[13] = {0};

	BytesToHexString(gQimCcoInfo.u8CcoMac, 6, cco_mac);
	//获得online device list
	QiM_Sdk_Send_CMD(CCO_CMD_GET_TOPOLOGY_NODE_SUM, NULL, 0);
	s32Ret = QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
	if (s32Ret < 0)
	{
		LOGD("CCO_CMD_GET_TOPOLOGY_NODE_SUM error,ret:%d\n", s32Ret);
		return;
	}
	LOGD("qmesh_get_all_dev_list:: tp sum = %d\n", gQimCcoInfo.u16TopologyNum);
	int online_device_num = gQimCcoInfo.u16TopologyNum;

	//清空上次的设备列表
	memset(gQimCcoInfo.tp_info_mac_list, 0, sizeof(gQimCcoInfo.tp_info_mac_list));
	gQimCcoInfo.u16SizeOfMacList = 0;

	for (int i = 1; i < online_device_num;) // PLC Start Index from 1
	{
		QIM_PLC_GET_TP_INFO stGetTpInfo;
		stGetTpInfo.u16Num = online_device_num + 1 - i;
		stGetTpInfo.u16StartIndex = i;

		QiM_Sdk_Send_CMD(CCO_CMD_GET_TOPOLOGY_NODE_INFO, (U8 *)&stGetTpInfo, sizeof(stGetTpInfo));
		s32Ret = QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
		if (s32Ret < 0)
		{
			LOGD("CCO_CMD_GET_TOPOLOGY_NODE_INFO error,ret: %d\n", s32Ret);
			continue;
		}

		if (gQimCcoInfo.u16SizeOfMacList >= online_device_num)
			break;
		else
			i = gQimCcoInfo.u16SizeOfMacList + 1;
	}
	return;
}

//删除设备
int qmesh_delete_device(const char **dev_id, int count)
{
	uint8_t *data_buf = malloc(count * 6 + 2);
	*((uint16_t *)data_buf) = count;
	for (int i = 0; i < count; i++)
	{
		HexStringToBytes(dev_id[i], data_buf + 2 + i * 6);
		dev_remove(dev_id[i]);
	}

	QiM_Sdk_Send_CMD(CCO_CMD_DEL_WL_NODE, data_buf, count * 6 + 2);
	int s32Ret = QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
	if (s32Ret < 0)
	{
		LOGE("QiM_SemWait error,ret:%d\n", s32Ret);
	}
	free(data_buf);
	m_needRebootCCO = 1;
	return 0;
}

//允许设备入网
int qmesh_enable_join_gateway(void)
{
	uint32_t state = 0;
	QiM_Sdk_Send_CMD(CCO_CMD_SET_WL_STATE, (uint8_t *)&state, 4);
	int s32Ret = QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
	if (s32Ret < 0)
	{
		LOGD("QIM_SEM_PLC_DATA_PARSED error,ret:%d\n", s32Ret);
		return -1;
	}
	return 0;
}

//关闭设备入网
int qmesh_disable_join_gateway(void)
{
	uint32_t state = 1;
	QiM_Sdk_Send_CMD(CCO_CMD_SET_WL_STATE, (uint8_t *)&state, 4);
	int s32Ret = QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
	if (s32Ret < 0)
		LOGD("QIM_SEM_PLC_DATA_PARSED error,ret:%d\n", s32Ret);
	return s32Ret;
}

// cco复位
int qmesh_reboot_cco(void)
{
	int s32Ret;
	int delay_time = 0;
	//获得online device list
	QiM_Sdk_Send_CMD(CCO_CMD_RESTART, (uint8_t *)&delay_time, 1);
	s32Ret = QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
	if (s32Ret < 0)
	{
		LOGE("CCO_CMD_RESTART error,ret: %d\n", s32Ret);
		return -1;
	}
	return 0;
}

int qmesh_trans_data(uint8_t *data, int len)
{
	int s32Ret;
	QiM_Sdk_Send_CMD(CCO_CMD_TRANS_DATA, data, len);
	s32Ret = QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
	if (s32Ret < 0)
	{
		LOGE("qmesh_trans_data timeout,ret: %d\n", s32Ret);
		return -1;
	}
	if(gu8CcoRespErrCode != 0)
	{
		LOGD("qmesh_trans_data had error code:%d\n", gu8CcoRespErrCode);
		return -1;
	}
	return 0;
}

//广播方式发送数据，所有子设备都可以收到
void qmesh_broadcast_setting_attr(unsigned char *data)
{
	const char *mac = "FFFFFFFFFFFF";
	//to do something
	//......

	return;
}

//注册子设备数据上报回调函数
void qmesh_register_callback(qmesh_dev_remove_cb remove, qmesh_dev_online_cb online, qmesh_dev_state_cb state)
{
	if (remove != NULL)
		dev_remove = remove;
	if (online != NULL)
		dev_online = online;
	if (state != NULL)
		dev_state = state;
	return;
}

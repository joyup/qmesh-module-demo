/*
 * @Author: Danny Yu
 * @Copyright: 2022 Shenzhen Qualmesh Technology Co., Ltd.
 * @Date: 2024-06-28 09:46:40
 * @Description: qmesh sdk demo
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <termios.h>
#include "plc_module_sdk.h"
#include "plc_module_protocol.h"
#include "qmesh_semaphore.h"
#include "qmesh_sdk.h"
#include "common.h"

#define UART "/dev/ttyUSB0"

extern QIM_CCO_INFO_DATA gQimCcoInfo;
/**
 * @description: 如需打印日志，可定制实现此函数
 * @param {char} *fmt
 * @return {*}
 */
void dbg_printf(const char *fmt, ...)
{
	char wzLog[1024] = {0};
	va_list args;
	va_start(args, fmt);
	vsprintf( wzLog, fmt, args);
	va_end(args);

    //to do ...
    //your own log function
    printf("%s", wzLog);
}

void qmesh_get_cco_info(void)
{
    int s32Ret;
	QiM_Sdk_Send_CMD(CCO_CMD_GET_VER_INFO, NULL, 0);
	s32Ret = QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
	if (s32Ret < 0)
	{
		LOGE("CCO_CMD_GET_VER_INFO Timeout,ret: %d\n", s32Ret);
        return;
	}
    dbg_printf("cco company info = 0x%x\n", gQimCcoInfo.u16CompanyType);
    dbg_printf("cco chip info = 0x%x\n", gQimCcoInfo.u16ChipType);
    dbg_printf("cco version info = 0x%x\n", gQimCcoInfo.u16SoftVer);

	//获得CCO Mac地址
	QiM_Sdk_Send_CMD(CCO_CMD_GET_MAC_ADDR, NULL, 0);
	QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
    dbg_printf("cco mac addr = %02X %02X %02X %02X %02X %02X\n", 
        gQimCcoInfo.u8CcoMac[0], gQimCcoInfo.u8CcoMac[1], gQimCcoInfo.u8CcoMac[2], 
        gQimCcoInfo.u8CcoMac[3], gQimCcoInfo.u8CcoMac[4], gQimCcoInfo.u8CcoMac[5]);
    return;
}

void qmesh_test(int cmd)
{
    int s32Ret;
    
    switch (cmd)
    {
    case 0:
        qmesh_start(UART);
        break;
    case 1:
        qmesh_get_cco_info();
        break;
    case 2: // 获得在线PLC设备数量
    {
        QiM_Sdk_Send_CMD(CCO_CMD_GET_TOPOLOGY_NODE_SUM, NULL, 0);
        s32Ret = QiM_SemWait(QIM_SEM_PLC_CCO_DATA_READY, CMD_WAIT_TIME_OUT);
        if (s32Ret < 0)
        {
            LOGD("CCO_CMD_GET_TOPOLOGY_NODE_SUM error,ret:%d\n", s32Ret);
            return;
        }
        LOGD("_qmesh_query_online_dev:: tp sum = %d\n", gQimCcoInfo.u16TopologyNum);
        break;
    }
    case 3:
    {
        // 获得在线PLC设备，如白名单打开，则白名单外设备不会出现在列表中
        qmesh_get_all_dev_list(0);
    	for (int j = 0; j < gQimCcoInfo.u16SizeOfMacList; j++)
		{
			char *mac = gQimCcoInfo.tp_info_mac_list[j];

			LOGD("qmesh_get_all_dev_list:: dev[%d] mac:%s\n", j, mac);
		}

        break;
    }
    case 4:
    {
        //搜索子设备（关闭白名单，允许所有设备入网）
        int ret = qmesh_enable_join_gateway();
        if(ret == -1)
            printf("enabled join gateway failed\n");
        break;
    }
    case 5:
    {
        //停止搜索子设备（打开白名单，白名单列表外设备禁止入网）
        int ret = qmesh_disable_join_gateway();
        if(ret == -1)
            printf("disabled join gateway failed\n");
        break;
    }
    case 6:
    {
        //添加n个设备到白名单
        const char *devList[] = {"E0B72E0800FA", "E0B72E0800FB", "E0B72E0800FC", "E0B72E0800FD", "E0B72E0800FE", "E0B72E0800FF"};
        int ret = qmesh_add_device_to_whitelist(devList, sizeof(devList)/sizeof(devList[0]));
        if(ret == -1)
            printf("add device to whitelist failed\n");
        break;
    }
    case 7:
    {
        //从白名单中删除n个设备
        const char *devList[] = {"E0B72E0800FA", "E0B72E0800FB", "E0B72E0800FC", "E0B72E0800FD", "E0B72E0800FE", "E0B72E0800FF"};
        int ret = qmesh_delete_device(devList, sizeof(devList)/sizeof(devList[0]));
        if(ret == -1)
            printf("del device to whitelist failed\n");
        break;
    }
    case 8:
    {
        //发送自定义协议数据
        const char *dest_dev_mac = "E0B72E0602AE";
        uint8_t test_data[8] = {0x00, 0x55, 0xaa, 0x55, 0xaa, 0xff, 0xcc, 0x33};

        QIM_PLC_TX_DATA_HEAD *tx_head = malloc(sizeof(QIM_PLC_TX_DATA_HEAD)+ sizeof(test_data));
        
        tx_head->u16UserDataLen = sizeof(test_data);
        HexStringToBytes(dest_dev_mac, tx_head->u8DestAddr);
        memcpy(tx_head->u8UserData, test_data, sizeof(test_data));

        int ret = qmesh_trans_data((unsigned char *)tx_head, sizeof(QIM_PLC_TX_DATA_HEAD)+ sizeof(test_data));
        if(ret == -1)
            printf("send data failed\n");
        break;
    }
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    int baudrate = 0;
    char *uartname = NULL;
    if(argc > 1)
    {
        for(int i = 1; i < argc; i++)
        {
            if(argv[i][0] == '-')
            {
                switch(argv[i][1])
                {
                    case 'b':
                        baudrate = atoi(argv[++i]);
                        break;
                    case 'n':
                        uartname = argv[++i];
                        break;
                    case 'h':
                        printf("usage: %s [-b baudrate] [-n uartname]\n", argv[0]);
                        return -1;
                    default:
                        break;
                }
            }
        }
    }
    
    if(baudrate == 0)
        baudrate = B460800;
    else
    {
        switch(baudrate)
        {
            case 460800:
                baudrate = B460800;
                break;
            case 9600:
                baudrate = B9600;
                break;
            case 115200:
                baudrate = B115200;
                break;
            default:
                printf("unsupported baudrate\n");
                return -1;
        }
    }
    if(uartname == NULL)
        uartname = UART;

    if(qmesh_uart_init(uartname, baudrate) < 0)
    {
        printf("qmesh_uart_init %s failed\n", uartname);
        return -1;
    }
    else
        printf("qmesh_uart_init %s success\n", uartname);

	qmesh_test(0);

    //qmesh_register_callback(NULL, NULL, dev_state_cb);
    while(1)
    {
        int c = getchar();
        if(c>'0' && c<='9') {
            qmesh_test(c - '0');
        } else if(c=='a' || c=='b' || c=='c' || c=='d' || c=='e' || c=='f') {
            qmesh_test(c - 'a' + 10);
        }
        sleep(1);
    }
}

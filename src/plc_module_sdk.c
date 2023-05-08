/****************************************************
*
* 奇脉PLC模块 对外功能接口
*
****************************************************/
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "plc_module_sdk.h"
#include "plc_module_protocol.h"
#include "qmesh_semaphore.h"
#include "common.h"
#include "qmesh_sdk.h"

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#define MODEL_COMM_ADDR 12
#define STA_CMD_MAX_RETRY 5

extern int _qmesh_plc_send(unsigned char *u8CmdData, int s32Len);
extern void qmesh_dev_state(unsigned char *data);
extern unsigned short Crc16_Ccitt(unsigned char *s8Buf, int s32Len);

QIM_CCO_INFO_DATA gQimCcoInfo  = {0};
QIM_PLC_FILE_TRANS_RESP	gQimFileTransResp = { 0 };
unsigned char gu8CcoRespErrCode;	//CCO Response State
unsigned char gu8StaRespStatus;		//STA Response Status

/**
* @brief 获取序列号
*
* @param null
*
* @return 序列号
*/
int QiM_GetSeq(void)
{
	static unsigned int u32Seq = 0;

	if (++u32Seq > 65535)
		u32Seq = 0;

	return u32Seq;
}

void QiM_Sdk_Send_Resp()
{
	QIM_PLC_TRANS_DATA_RESP resp = {0};
	QiM_Sdk_Send_CMD(CCO_CMD_RECV_DATA, (U8 *)&resp, sizeof(QIM_PLC_TRANS_DATA_RESP));
}

int QiM_Sdk_Send_CMD(U16 u16Cmd, U8 *pData, int dataLen)
{
	unsigned char u8CmdData[1024];
	int s32PacketLen = 0;
	
	memset(u8CmdData, 0, sizeof(u8CmdData));

	switch(u16Cmd)
	{	
		case CCO_CMD_GET_VER_INFO:
		case CCO_CMD_GET_MAC_ADDR:
		case CCO_CMD_GET_COMM_ADDR:
		case CCO_CMD_GET_WL_SUM:
		case CCO_CMD_CLR_WL:
		case CCO_CMD_SET_AUTO_REG:
		case CCO_CMD_GET_WL_STATE:
		case CCO_CMD_GET_TOPOLOGY_NODE_SUM:
			s32PacketLen = QiM_Protocol_Build_CCO_Tx_Data(QiM_GetSeq(), u16Cmd, NULL, 0, u8CmdData, sizeof(u8CmdData));
			break;
		case CCO_CMD_SET_COMM_ADDR:
		{
			//设置模组通信地址
			QIM_MODEL_ADDR_INFO_ST *pstAddrInfo = (QIM_MODEL_ADDR_INFO_ST *)pData;
			U8 u8CommAddr[8]={0};
			if (MODEL_COMM_ADDR != strlen((char *)pstAddrInfo->u8Addr))
				return -1;
			QiM_Protocol_MacCharToInt(6, pstAddrInfo->u8Addr, u8CommAddr);
			s32PacketLen = QiM_Protocol_Build_CCO_Tx_Data(QiM_GetSeq(), u16Cmd, u8CommAddr, 8, u8CmdData, sizeof(u8CmdData));
			break;
		}
		case CCO_CMD_SET_WL_STATE:
		{	//设置白名单状态 0：关闭 1：开启
			U32 u32State = *pData;
			if (u32State > 1)
			{
				LOGE("state:%d not correct, shoud 0 or 1\n", u32State);
				return -1;
			}
			s32PacketLen = QiM_Protocol_Build_CCO_Tx_Data(QiM_GetSeq(), u16Cmd, (U8 *)&u32State, 4, u8CmdData, sizeof(u8CmdData));
			break;
		}
		case CCO_CMD_GET_TOPOLOGY_NODE_INFO:
		{	//获取拓扑节点信息
			//StartIndex  查询起始序号  从 1 开始，其中 1 为主节点，后续为从节点。每次查询必须从序号 1 起始查询。
			//Num  查询数量  一次操作不要超过 QIM_MODEL_TP_MAX_NUM

			QIM_PLC_GET_TP_INFO *pGetTpInfo = (QIM_PLC_GET_TP_INFO *)pData;
			s32PacketLen = QiM_Protocol_Build_CCO_Tx_Data(QiM_GetSeq(), u16Cmd, (U8 *)pGetTpInfo, 4, u8CmdData, sizeof(u8CmdData));
			break;
		}
		case CCO_CMD_GET_WL_INFO:
 		{	//获取白名单节点信息
 			//StartIndex  查询起始序号 
			//Num  查询数量	一次操作不要超过 QIM_MODEL_WL_MAX_NUM
			QIM_PLC_GET_WL_INFO *pGetWlInfo = (QIM_PLC_GET_WL_INFO *)pData;
			s32PacketLen = QiM_Protocol_Build_CCO_Tx_Data(QiM_GetSeq(), u16Cmd, (U8 *)pGetWlInfo, 4, u8CmdData, sizeof(u8CmdData));
			break;
		}
		case CCO_CMD_RESTART:
		{	//设置模组重启
			int s32DelayTime = *pData;
			s32PacketLen = QiM_Protocol_Build_CCO_Tx_Data(QiM_GetSeq(), u16Cmd, (U8 *)&s32DelayTime, 4, u8CmdData, sizeof(u8CmdData));
			break;
		}
		case CCO_CMD_ADD_WL_NODE:
		case CCO_CMD_DEL_WL_NODE:
		{	
			uint16_t node_num = *(uint16_t *)pData;
			if (node_num > QIM_MODEL_WL_MAX_NUM)
			{
				LOGE("wl num:%d ,should less than:%d\n", node_num, QIM_MODEL_WL_MAX_NUM);
				return -1;
			}
			s32PacketLen = QiM_Protocol_Build_CCO_Tx_Data(QiM_GetSeq(), u16Cmd, pData, 2+node_num*6, u8CmdData, sizeof(u8CmdData));
			break;
		}
		case CCO_CMD_TRANS_DATA:
		case CCO_CMD_RECV_DATA:
		case CCO_CMD_BUS_COMMAND:
		case CCO_CMD_TRANS_FILE:
			if (dataLen == 0)
			{
				LOGE("cmd data len error\n");
				break;
			}
			s32PacketLen = QiM_Protocol_Build_CCO_Tx_Data(QiM_GetSeq(), u16Cmd, pData, dataLen, u8CmdData, sizeof(u8CmdData));
			break;
		case CCO_CMD_GET_RUN_TIME:
		case CCO_CMD_TRANS_COMMAND:
		case CCO_CMD_RECV_COMMAND:
		default:
			LOGE("unsupport cmd:0x%04x\n", u16Cmd);
			s32PacketLen = -1;
			break;
	}	//buid data

	//通过串口发送数据
	if (s32PacketLen < 0)
	{
		LOGE("QiM_Protocol_Build_ModelUsualCtrlData error,cmd = 0x%x, ret:%d\n", u16Cmd, s32PacketLen);
		return -1;
	}
	_qmesh_plc_send(u8CmdData, s32PacketLen);

	return 0;
}

/**
 * @brief 解析CCO发过来的数据入口
 *
 * @param u8RespBuf  数据
 * @param s32BufLen  数据长度
 *
 * @return 0：成功 -1：失败
 */
int QiM_Sdk_ParseRespData(unsigned char* u8RespBuf, int s32RespBufLen)
{
	int s32Ret = 0;
	QIM_PLC_FRAME_HEAD *pFrameDataHead;
	U8 *pFrameData;

	U16 u16Crc = Crc16_Ccitt(u8RespBuf, s32RespBufLen-2);	
	U8 u8CrcHigh = (u16Crc >> 8) & 0x00FF;
	U8 u8CrcLow = (u16Crc) & 0x00FF;

	if((u8CrcHigh != u8RespBuf[s32RespBufLen-2]) || (u8CrcLow != u8RespBuf[s32RespBufLen-1]))
		return -1;

	pFrameDataHead = (QIM_PLC_FRAME_HEAD *)u8RespBuf;
	pFrameData = u8RespBuf + sizeof(QIM_PLC_FRAME_HEAD);

	LOGD("head:0x%02x,ctrl:0x%02x,cmd:0x%04x,seq:0x%04x,len:0x%04x\n",
			pFrameDataHead->u8Head, pFrameDataHead->u8Ctrl,pFrameDataHead->u16Cmd,pFrameDataHead->u16Seq,pFrameDataHead->u16Len);

	switch(pFrameDataHead->u16Cmd)
	{	
		case CCO_CMD_GET_VER_INFO:
		{
			memcpy((void *)&gQimCcoInfo, pFrameData,  sizeof(QIM_MODEL_VER_INFO_ST));
			break;
		}
		case CCO_CMD_GET_MAC_ADDR:
		{
			memcpy((void*)&gQimCcoInfo.u8CcoMac , pFrameData,  6);
			break;
		}
		case CCO_CMD_GET_COMM_ADDR:
		{
			memcpy(gQimCcoInfo.u8CcoCommAddr , pFrameData,  6);
			break;
		}
		case CCO_CMD_SET_COMM_ADDR:
		case CCO_CMD_ADD_WL_NODE:
		case CCO_CMD_DEL_WL_NODE:
		case CCO_CMD_CLR_WL:
		case CCO_CMD_SET_AUTO_REG:
		{		
			s32Ret = pFrameData[0];
			if(s32Ret)
			{
				U8 u8ErrCode = pFrameData[1];
				LOGE("cco err code:0x%d\n", u8ErrCode);
			}
			break;
		}
		case CCO_CMD_RESTART:
		{
			s32Ret = pFrameData[0];
			break;
		}
		case CCO_CMD_GET_WL_SUM:
		{
			gQimCcoInfo.u16WhiteListNum = *(U16*)pFrameData;
			break;
		}
		case CCO_CMD_GET_WL_INFO:
		{
			QIM_WL_INFO_HEAD* pWlInfo = (QIM_WL_INFO_HEAD*)pFrameData;
			uint8_t* pWlInfoData = (uint8_t*)(pFrameData + sizeof(QIM_WL_INFO_HEAD));

			for (int i = 0; i < pWlInfo->u16GetNum; i++)
			{
				//LOGD("index:%d, id:%s, u32Tei:%d, u32ProxyTel:%d, u32NodeInfo:%d\n", i, stAddrArry[i].u8Addr,
				//	stAddrArry[i].u32Tei, stAddrArry[i].u32ProxyTel, stAddrArry[i].u32NodeInfo);
				BytesToHexString(pWlInfoData+i*6, 6, gQimCcoInfo.wl_info_mac_list[i]);
			}
			break;
		}
		case CCO_CMD_SET_WL_STATE:
		case CCO_CMD_GET_WL_STATE:
		{
			gQimCcoInfo.u8WhiteListState = pFrameData[0];
			break;
		}
		case CCO_CMD_GET_TOPOLOGY_NODE_SUM:
		{
			gQimCcoInfo.u16TopologyNum = *((unsigned short *)pFrameData);
			break;
		}
		case CCO_CMD_GET_TOPOLOGY_NODE_INFO:
		{
			QIM_TP_INFO_HEAD *pTpInfo = (QIM_TP_INFO_HEAD *)pFrameData;
			QIM_TP_NODE_INFO* pTpInfoData = (QIM_TP_NODE_INFO *)(pFrameData + sizeof(QIM_TP_INFO_HEAD));
			int current_mac_list_index = gQimCcoInfo.u16SizeOfMacList;
			LOGD("u16GetNum:%d\n", pTpInfo->u16GetNum);
			LOGD("u16SizeOfMacList:%d\n", gQimCcoInfo.u16SizeOfMacList);
			for (int i = 0; i < pTpInfo->u16GetNum; i++)
			{
				//LOGD("index:%d, u16Tei:%x, u16ProxyTel:%x, u16NodeInfo:%x\n", i,
				//	pTpInfoData[i].u16Tei, pTpInfoData[i].u16ProxyTel, pTpInfoData[i].u8NodeInfo);
				//if ((pTpInfoData[i].u8NodeInfo & 0xf0) == 0x40)	//CCO Node
				//	continue;
				BytesToHexString(pTpInfoData[i].u8MacAddr, 6, gQimCcoInfo.tp_info_mac_list[current_mac_list_index+i]);
				//LOGD("mac: %s\n", gQimCcoInfo.tp_info_mac_list[current_mac_list_index+i]);
			}
			gQimCcoInfo.u16SizeOfMacList += pTpInfo->u16GetNum;
			break;
		}		
		case CCO_CMD_TRANS_FILE:
			memcpy((U8 *)&gQimFileTransResp, pFrameData, sizeof(QIM_PLC_FILE_TRANS_RESP));
			LOGD("u8FunCode:0x%02x,u8RespState:0x%02x,u8RespReason:0x%02x,u16RespStaNum:0x%04x\n",
					gQimFileTransResp.u8FunCode, gQimFileTransResp.u8RespState, gQimFileTransResp.u8RespReason, gQimFileTransResp.u16RespStaNum);
			break;
		case CCO_CMD_TRANS_DATA:
		{
			QIM_PLC_TRANS_DATA_RESP *pResp = (QIM_PLC_TRANS_DATA_RESP *)pFrameData;
			if(pResp->state != 0)
			{
				LOGE("received data error, reason = %d\n", pResp->reason);
				gu8CcoRespErrCode = pResp->reason;
			}
			else
				gu8CcoRespErrCode = 0;
		}
			break;
		case CCO_CMD_BUS_COMMAND:
			qmesh_dev_state(pFrameData);
			return 0;
		case CCO_CMD_RECV_DATA:
			qmesh_dev_state(pFrameData);
			QiM_Sdk_Send_Resp();
			return 0;
		default:
			LOGE("unknown cmd:0x%04x\n", pFrameDataHead->u16Cmd);
			s32Ret = -1;
			break;
	}
	QiM_SemPost(QIM_SEM_PLC_CCO_DATA_READY);

	return s32Ret;
}

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */


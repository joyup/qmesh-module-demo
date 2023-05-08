#pragma once
#ifndef   _PLC_MODULE_SDK_H_
#define   _PLC_MODULE_SDK_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#ifndef QM_DATA_TYPE_DEF
#define QM_DATA_TYPE_DEF
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
#endif

#define QIM_PLC_MAX_DEV_NUM 	500	//最大子设备数量
#define QIM_MODEL_MAC_LEN 		12	 //设备MAC地址长度
#define QIM_MODEL_WL_MAX_NUM 	80  //操作白名单一次最大数量
#define QIM_MODEL_TP_MAX_NUM 	40  //操作拓扑一次最大数量
#define QIM_STA_NET_ADDR_LEN 	4  	//设备应用地址长度

#define CMD_WAIT_TIME_OUT	3		//CCO命令响应等待时间3S
#define BUS_CMD_WAIT_TIME_OUT	1000	//BUS命令响应等待时间1S，单位ms
#define BUS_CMD_RETRY_CNT		3		//BUS命令重试次数

/*
* CCO状态码
*/
typedef enum _QIM_STATE_CODE_
{
	QIM_STATE_SUCCESS	 	= 0x00,	//正常
	QIM_STATE_OVERTIME 		= 0x01, //通信超时
	QIM_STATE_INVALID_FORMAT= 0x02, //格式错误
	QIM_STATE_BUSY  	    = 0x03, //节点忙
	QIM_STATE_UNKNOWN  	    = 0xff  //未知错误 
}QIM_STATE_CODE_EN;
	
/*
* STA状态码
*/
typedef enum _QIM_STA_STATE_CODE_
{
	QIM_STA_STATE_SUCCESS	 	= 0x00,	//正常
	QIM_STA_STATE_INVALID_REQ 	= 0x01, //无法解析
	QIM_STA_STATE_LIMIT			= 0x02, //设备控制受限
	QIM_STA_STATE_UNREADABLE 	= 0x03, //属性不可读
	QIM_STA_STATE_UNWRITABLE 	= 0x04,//属性不可写
	QIM_STA_STATE_PARM_ERROR 	= 0x05, //参数值错误
	QIM_STA_STATE_UNKNOWN  	    = 0x06  //未知错误 
}QIM_STA_STATE_CODE_EN;	

/*
* PLC模组信息
*/
typedef struct _QIM_MODEL_VER_INFO_
{
	unsigned short u16CompanyType;	//厂商代码
	unsigned short u16ChipType;	//芯片类型
	unsigned short u16SoftVer;		//软件版本：BCD格式
	unsigned short u16Resv;
}QIM_MODEL_VER_INFO_ST;

/*
* 地址信息
*/
typedef struct _QIM_MODEL_ADDR_INFO_
{
	//unsigned char u8AddrLen;					//MAC地址长度
	unsigned char u8Addr[QIM_MODEL_MAC_LEN+1];	//MAC地址
	unsigned char u8NetAddr[QIM_STA_NET_ADDR_LEN+1]; //设备应用地址
}QIM_MODEL_ADDR_INFO_ST;

/*
* 白名单信息
*/
typedef struct _QIM_MODEL_WL_INFO_
{
	unsigned int u32WlNum;						//白名单节点数量
	unsigned int u32StartSeq;					//起始查询序号
	unsigned int u32GetNum;						//本次查询获取到的白名单数量
	QIM_MODEL_ADDR_INFO_ST* pstAddrInfo; 		//查询到的白名单地址信息集合 指针由创建者负责释放
}QIM_MODEL_WL_INFO_ST;

/*
* 添加/删除白名单
*/
typedef struct _QIM_MODEL_WL_CTRL_
{
	unsigned int u32Num;						//数量本次操作不要超过 QIM_MODEL_WL_MAX_NUM
	QIM_MODEL_ADDR_INFO_ST* pstAddrInfo; 		//信息指针由创建者负责释放
}QIM_MODEL_WL_CTRL_ST;

/*
* 拓扑节点信息
*/
typedef struct _QIM_MODEL_TP_NODE_INFO_
{
	unsigned char u8AddrLen;					//地址长度
	unsigned char u8Addr[QIM_MODEL_MAC_LEN+1];	//地址
	unsigned int u32Tei;						//节点标识最大不超过512
	unsigned int u32ProxyTel;					//本节点的代理站点节点标识
	unsigned int u32NodeInfo;					//节点信息 [3:0]代表本站点的网络层级0：代表0层级，依次类推；[7:4]表示本站点的
												//网络角色0：无效1：表示末梢节点(STA)2：表示代理节点3：保留4：表示主节点
}QIM_MODEL_TP_NODE_INFO_ST;

/*
* 拓扑信息
*/
typedef struct _QIM_MODEL_TP_INFO_
{
	unsigned int u32TotalNum;					//节点总数量
	unsigned int u32StartSeq;					//起始序号，从 1 开始，其中 1 为主节点，后续为从节点。每次查询必须从序号 1 起始查询
	unsigned int u32GetNum;						//本次查询获取到的数量
	QIM_MODEL_TP_NODE_INFO_ST* pstInfo; 		//查询到的白名单地址信息集合 指针由创建者负责释放
}QIM_MODEL_TP_INFO_ST;

typedef struct _QIM_TP_INFO_HEAD_
{
	unsigned short u16TotalNum;			//节点总数量
	unsigned short u16StartSeq;			//起始序号，从 1 开始，其中 1 为主节点，后续为从节点。每次查询必须从序号 1 起始查询
	unsigned short u16GetNum;			//本次查询获取到的数量
	unsigned short u16Reserved;
}QIM_TP_INFO_HEAD, QIM_WL_INFO_HEAD;

typedef struct _QIM_TP_NODE_INFO_
{
	unsigned char u8MacAddr[6];		//地址
	unsigned short u16Tei;			//节点标识最大不超过512
	unsigned short u16ProxyTel;		//本节点的代理站点节点标识
	unsigned char u8NodeInfo;		//节点信息 [3:0]代表本站点的网络层级0：代表0层级，依次类推；[7:4]表示本站点的
									//网络角色0：无效1：表示末梢节点(STA)2：表示代理节点3：保留4：表示主节点
	unsigned char u8Reserved;
}QIM_TP_NODE_INFO;

/*
 *	文件传输结构体
 */
typedef struct _QIM_PLC_FILE_TRANS_INIT_
{
	U8 u8FunCode;
	U8 u8FileAttr;
	U16 u16SegmentTotalCount;
	U32 u32FileLength;
	U32 u32FileCrc;
	U32 u32TransTimeout;
}QIM_PLC_FILE_TRANS_INIT;

typedef struct _QIM_PLC_FILE_TRANS_DATA_HEAD_
{
	U8 u8FunCode;
	U8 u8Reserve;
	U16 u16SegmentNum;
	U16 u16SegmentSize;
	U16 u16SegmentCrc;
}QIM_PLC_FILE_TRANS_DATA_HEAD;

typedef struct _QIM_PLC_FILE_QUERY_PROCESS_STATE_
{
	U8 u8FunCode;
	U8 u8Reserve[3];
}QIM_PLC_FILE_QUERY_PROCESS_STATE;

typedef struct _QIM_PLC_FILE_SET_TRANS_LIST_
{
	U8 u8FunCode;
	U8 u8StaMacCnt;
	U8 u8StaMacList[0];
}QIM_PLC_FILE_SET_TRANS_LIST;

typedef struct _QIM_PLC_FILE_TRANS_RESP_
{
	U8 u8FunCode;
	U8 u8RespState;
	union {
		U16 u16RespStaNum;
		struct {
			U8 u8RespReason;
			U8 u8Reserve;
		};
	};
}QIM_PLC_FILE_TRANS_RESP;

/*
 *	PLC发送数据包结构体
 */
typedef struct _QIM_PLC_FRAME_HEAD_
{
	U8 u8Head;
	U8 u8Ctrl;
	U16 u16Cmd;
	U16 u16Seq;
	U16 u16Len;
}QIM_PLC_TX_FRAME_HEAD, QIM_PLC_FRAME_HEAD;

typedef struct _QIM_PLC_GET_INFO_
{
	U16 u16StartIndex;
	U16 u16Num;
}QIM_PLC_GET_WL_INFO,QIM_PLC_GET_TP_INFO;

typedef struct _QIM_PLC_TX_DATA_HEAD_
{
	unsigned char u8DestAddr[6];
	unsigned short u16UserDataLen;
	unsigned char u8UserData[0];
}QIM_PLC_TX_DATA_HEAD;

typedef struct _QIM_PLC_RX_DATA_HEAD_
{
	unsigned char u8SrcMac[6];
	unsigned short u16UserDataLen;
	unsigned char u8UserData[0];
}QIM_PLC_RX_DATA_HEAD;

typedef struct _QIM_PLC_TRANS_DATA_RESP_
{
	unsigned char state;
	unsigned char reason;
	unsigned short reserve;
}QIM_PLC_TRANS_DATA_RESP;

typedef struct _QIM_PLC_STA_FRAME_HEAD_
{
	unsigned char u8StaMac[6];
	unsigned short u16StaDataLen;
	unsigned char u8DataMajorVer;
	unsigned char u8DataMinorVer;
	unsigned short u16StaDataSeq;
	unsigned char u8DataFuncCode;
	unsigned char u8DataStatusCode;
	unsigned short u16DataDevAddr;
}QIM_PLC_STA_TX_FRAME_HEAD, QIM_PLC_STA_RX_FRAME_HEAD;

typedef struct _QIM_CCO_INFO_DATA_
{
	unsigned short u16CompanyType;	//厂商代码
	unsigned short u16ChipType;		//芯片类型
	unsigned short u16SoftVer;		//软件版本：BCD格式
	unsigned short u16Resv;

	unsigned char u8CcoMac[6];		//Mac Addr
	unsigned char u8CcoCommAddr[6];	//Comm Addr

	unsigned short u16TopologyNum;	//Topology数量
	char tp_info_mac_list[QIM_PLC_MAX_DEV_NUM][12+1];	//Mac list
	unsigned short u16SizeOfMacList;//当前Mac list大小

	unsigned short u16WhiteListNum;	//白名单数量
	unsigned char u8WhiteListState;	//白名单状态
	char wl_info_mac_list[QIM_PLC_MAX_DEV_NUM][12+1];	//Mac list;
}QIM_CCO_INFO_DATA;

int QiM_Sdk_ParseRespData(unsigned char* u8RespBuf, int s32BufLen);
int QiM_Sdk_Send_CMD(U16 u16Cmd, U8 *pData, int dataLen);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif

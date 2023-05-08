/****************************************************
*
* 奇脉PLC模块 通信协议的组装与解析
*
****************************************************/
#include <stdio.h>
#include <string.h>
#include "plc_module_sdk.h"
#include "plc_module_protocol.h"
#include "common.h"

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

//#define min(a,b) ( ((a)>(b)) ? (b):(a) )

/* 帧相关定义 */
#define QIM_FRAME_HEAD 					(0x48)	//帧头
#define QIM_FRAME_MAX_LEN				(512)	//帧的最大数据长度
#define QIM_FRAME_MIN_LEN				(10)	//帧的最小数据长度
#define QIM_FRAME_DATA_MAX_LEN 			(QIM_FRAME_MAX_LEN - QIM_FRAME_MIN_LEN)		//帧中DATA域的最大长度
#define QIM_FRAME_DATA_MIN_LEN 			(0)		//帧中DATA域的最小长度

#define QIM_FRAME_CTRL_BOARD_SLAVE 		0x00   //此帧报文是由主控设备发出的下行报文 此帧报文来自从动站
#define QIM_FRAME_CTRL_BOARD_MASTER 	0x40   //此帧报文是由主控设备发出的下行报文 此帧报文来自启动站
#define QIM_FRAME_CTRL_BLE_SLAVE 		0x80   //此帧报文是由通信模组发出的上行报文 此帧报文来自从动站
#define QIM_FRAME_CTRL_BLE_MASTER 		0xc0   //此帧报文是由通信模组发出的上行报文 此帧报文来自启动站

/* 帧中DATA域 STA帧相关定义 */
#define QIM_STA_FRAME_MAX_LEN			(QIM_FRAME_DATA_MAX_LEN)		//STA帧的最大数据长度
#define QIM_STA_FRAME_MIN_LEN			(16)	//STA帧的最小数据长度
#define QIM_STA_FRAME_DATA_MAX_LEN 		(QIM_STA_FRAME_MAX_LEN - QIM_STA_FRAME_MIN_LEN)		//STA帧中DATA域的最大长度
#define QIM_STA_FRAME_DATA_MIN_LEN 		(0)		//STA帧中DATA域的最小长度


#define PLC_MAJOR_VER 					(1) 	//PLC主版本
#define PLC_MINOR_VER 					(0)		//PLC次版本


/*
*	STA数据类型
*/
typedef enum _QIM_STA_DATA_TYPE_ 
{
	QIM_STA_DATA_TYPE_NONE		= 0x00, 	
	QIM_STA_DATA_TYPE_INT		= 0x01,   
	QIM_STA_DATA_TYPE_BOOL		= 0x02,   
	QIM_STA_DATA_TYPE_STRING	= 0x03,  
	QIM_STA_DATA_TYPE_EMUN		= 0x04,  
	QIM_STA_DATA_TYPE_ARRAY 	= 0x05 	 
}QIM_STA_DATA_TYPE_EN;


/**
 * @brief 字符格式的MAC转化为int格式
 *          "112233445566"  ===> 0x11 0x22 0x33 0x44 0x55 0x66
 *
 * @param u8In   输入
 * @param u8Out   输出
 *
 * @return > 0：成功 -1：失败
 */
int QiM_Protocol_MacCharToInt(unsigned char u8Num, unsigned char* u8In, unsigned char* u8Out)
{	
	HexStringToBytes((const char *)u8In, u8Out);
	return 0;
}

/**
 * @brief int格式的MAC转化为字符格式
 *          0x11 0x22 0x33 0x44 0x55 0x66 ===>  "112233445566"
 *
 * @param u8In   输入
 * @param u8Out   输出B
 *
 * @return > 0：成功 -1：失败
 */
int QiM_Protocol_MacIntToChar(unsigned char u8Num, unsigned char* u8In, unsigned char* u8Out)
{	
	if (6 == u8Num)
	{
		snprintf((char *)u8Out, 13, "%02X%02X%02X%02X%02X%02X", 
					u8In[0], u8In[1], u8In[2], 
					u8In[3], u8In[4], u8In[5]);
	}
	else if (2 == u8Num)
	{
		snprintf((char *)u8Out, 5, "%02X%02X", 
					u8In[1], u8In[0]);
	}
	
	return 0;
}

/**
* @brief 构造CCO通信Frame
*
* @param u32Seq 	序列号
* @param u32Cmd 	命令字节
* @param u8InData 	待发送的数据段
* @param u32InDataLen	发送数据长度
* @param u8OutData  创建后的数据buf，返回构造好的Frame
* @param u32OutDataMaxLen 数据buf最大长度
*
* @return -1:失败 >0:数据长度
* @comment: 1、未检测输出buf的溢出情况。2、未检测输入buf的非法指针情况
*/
int QiM_Protocol_Build_CCO_Tx_Data(unsigned int  u32Seq, unsigned int u32Cmd, U8 *u8InData, U32 u32InDataLen, U8* u8OutData, U32 u32OutDataMaxLen)
{
	unsigned short u16Crc = 0;
	int s32DataLen = sizeof(QIM_PLC_TX_FRAME_HEAD) + u32InDataLen;

	QIM_PLC_TX_FRAME_HEAD *pTxFrameHead = (QIM_PLC_TX_FRAME_HEAD *)u8OutData;
	
	pTxFrameHead->u8Head = QIM_FRAME_HEAD;
	pTxFrameHead->u8Ctrl = QIM_FRAME_CTRL_BOARD_MASTER;	
	pTxFrameHead->u16Cmd = u32Cmd;
	pTxFrameHead->u16Seq = u32Seq;
	pTxFrameHead->u16Len = u32InDataLen;
	
	if (u32InDataLen > 0)
		memcpy(u8OutData + sizeof(QIM_PLC_TX_FRAME_HEAD), u8InData, u32InDataLen);

	u16Crc = Crc16_Ccitt(u8OutData, s32DataLen);	
	u8OutData[s32DataLen] = (u16Crc >> 8) & 0x00FF;
	u8OutData[s32DataLen + 1] = (u16Crc) & 0x00FF;

	s32DataLen += 2;
	
	return s32DataLen;
}

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */


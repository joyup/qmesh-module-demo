/*
 * @Author: Danny Yu
 * @Copyright: 2022 Shenzhen Qualmesh Technology Co., Ltd.
 * @Date: 2023-05-04 21:05:43
 * @Description: 
 */
#ifndef   _PLC_MODULE_QIM_PROTOCOL_H_
#define   _PLC_MODULE_QIM_PROTOCOL_H_

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#define CHECK_IS_NULL(Poniter) { \
	if (NULL == Poniter) \
	{ \
		SX_LOGE("data is null, please check\n");\
		return -1; \
	} \
}

typedef enum _CCO_CMD_ 
{
    CCO_CMD_GET_VER_INFO     		= 0x0001,      /* get version information  */
    CCO_CMD_GET_MAC_ADDR     		= 0x0002,      /* get mac address  */
    CCO_CMD_GET_COMM_ADDR    		= 0x0003,      /* get communication address  */
    CCO_CMD_SET_COMM_ADDR    		= 0x0004,      /* set communication address  */
    CCO_CMD_RESTART          		= 0x0005,      /* restart  */
    CCO_CMD_TRANS_FILE       		= 0x0006,      /* transfer file  */
    CCO_CMD_GET_RUN_TIME			= 0x0007,      /* get model run time */
	CCO_CMD_GET_WL_SUM	 	 		= 0x0010,	   /* get white list sum  */
	CCO_CMD_GET_WL_INFO	 			= 0x0011,	   /* get white list  */
	CCO_CMD_ADD_WL_NODE	 			= 0x0012,	   /* add node to whitelist  */
	CCO_CMD_DEL_WL_NODE	 			= 0x0013,	   /* delete whitelist node  */
	CCO_CMD_CLR_WL   	 			= 0x0014,	   /* clear whitelist */
	CCO_CMD_SET_AUTO_REG			= 0x0015,	   /* enable automatic networking */
	CCO_CMD_SET_WL_STATE	 		= 0x0016,	   /* set whitelist state */
	CCO_CMD_GET_WL_STATE	 		= 0x0017,	   /* get whitelist state  */
	CCO_CMD_GET_TOPOLOGY_NODE_SUM	= 0x0020,	   /* get network topology node sum   */
	CCO_CMD_GET_TOPOLOGY_NODE_INFO	= 0x0021,	   /* get network topology node info  */
	CCO_CMD_TRANS_DATA	 			= 0x0100,	   /* transfer data  */
	CCO_CMD_RECV_DATA	 			= 0x0101,	   /* receive data */
	CCO_CMD_TRANS_COMMAND	 		= 0x0110,	   /* transfer command  */
	CCO_CMD_RECV_COMMAND	 		= 0x0111,	   /* receive command  */
	CCO_CMD_BUS_COMMAND	 			= 0x0120	   /* bus data communication command  */	
} CCO_CMD_EN;

typedef enum _STA_CMD_ 
{
    STA_CMD_GET_VER_INFO     		= 0x0001,      /* get version information  */
    STA_CMD_GET_MAC_ADDR     		= 0x0002,      /* get mac address  */
    STA_CMD_GET_COMM_ADDR    		= 0x0003,      /* get communication address  */
    STA_CMD_RESTART          		= 0x0005,      /* restart  */
} STA_CMD_EN;


int QiM_Protocol_MacCharToInt(unsigned char u8Num, unsigned char* u8In, unsigned char* u8Out);
int QiM_Protocol_MacIntToChar(unsigned char u8Num, unsigned char* u8In, unsigned char* u8Out);

int QiM_Protocol_Build_CCO_Tx_Data(unsigned int  u32Seq, unsigned int u32Cmd, U8 *u8InData, U32 u32InDataLen, U8* u8OutData, U32 u32OutDataMaxLen);
int QiM_Protocol_Build_STA_Tx_Data(unsigned int  u32Seq, unsigned int u32Func, QIM_MODEL_ADDR_INFO_ST* pstDevAddr, U8 *u8InData, U32 u32InDataLen, U8* u8OutData, U32 u32OutDataMaxLen, U8 responseFlag);


#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif


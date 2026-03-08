/*-------------------------------------------------*/
/*           超子说物联网STM32系列开发板           */
/*-------------------------------------------------*/
/*                                                 */
/*            操作LoRa模块功能的源文件             */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f4xx_hal.h"  //包含需要的头文件
#include "usart.h"            //包含需要的头文件
#include "lora.h"	          //包含需要的头文件


LoRaParameter LoRaSetData = {            //模块工作参数
	0x00,                                //模块地址高字节
	0x01,                                //模块地址低字节
	0x01,                                //模块网络地址
	LoRa_9600,                           //模块串口 波特率 9600
	LoRa_8N1,                            //串口工作模式 8数据位 无校验 1停止位	
	LoRa_19_2,                           //模块空中速率 19.2K
	LoRa_Data240,                        //数据分包大小 240字节
	LoRa_RssiDIS,                        //关闭RSSI功能
	LoRa_FEC_22DBM,                      //发射功率22dbm
	LoRa_CH23,                           //模块信道23 对应频率433MHz	
	LoRa_RssiByteDIS,                    //禁用RSSI字节功能
	LoRa_ModePOINT,                      //定点模式
	LoRa_RelayDIS,                       //禁用中继模式
	LoRa_LBTDIS,                         //禁用LBT
	LoRa_WorTX,                          //Wor模式发送        只在模式1才有效
	LoRa_Wor2000ms,                      //Wor周期2000毫秒    只在模式1才有效
	0x22,                                //模块加密秘钥高字节
	0x33,                                //模块加密秘钥低字节	
};

/*-------------------------------------------------*/
/*函数名：初始化模块的IO                           */
/*参  数：timeout： 超时时间（10ms的倍数）         */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LoRa_GPIO_Init(void)
{
    // 内容可以清空，因为CubeMX生成的MX_GPIO_Init已经配置好了PA0, PA1, PA4, PA5
    // 仅仅保留模式切换和延时即可
    LoRa_MODE2;      
    HAL_Delay(200);   
}
/*-------------------------------------------------*/
/*函数名：LoRa上电复位                             */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
uint8_t LoRa_Reset(int16_t timeout)
{
    LoRa_PowerOFF;                  //先OFF
	HAL_Delay(200);                  //延时
	LoRa_PowerON;                   //再ON
	
	while(timeout--)                //等待复位成功
	{                           
		HAL_Delay(100);              //延时100ms
		if(LoRa_AUX==1)             //需要等到高电平1，才能推出while，表示复位完毕
			break;       			//主动跳出while循环
		u1_printf("%d ",timeout);   //串口输出现在的超时时间		
	} 
	u1_printf("\r\n");              //串口输出信息
	if(timeout<=0)return 1;         //如果timeout<=0，说明超时时间到了，也没能等到LoRa_AUX高电平，返回1
	return 0;                       //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：初始化模块                               */
/*参  数：无                                       */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
uint8_t LoRa_Init(void)
{
	LoRa_GPIO_Init();                                 //引脚初始化
	
	u1_printf("准备复位模块\r\n");   	              //串口输出信息
	if(LoRa_Reset(100)){                              //复位模块，返回值如果非零，都表示错误
		u1_printf("复位模块失败\r\n");   	          //串口输出信息
		return 1;                                     //返回1
	}else u1_printf("模块复位成功\r\n");   	          //串口输出信息	
	
	u1_printf("准备设置模块\r\n");   	              //串口输出信息
	if(LoRa_Set(100)){                                //设置模块，返回值如果非零，都表示错误
		u1_printf("设置模块失败\r\n");   	          //串口输出信息
		return 2;                                     //返回2
	}else u1_printf("设置模块成功\r\n");   	          //串口输出信息	
	
	u1_printf("准备查询模块\r\n");   	              //串口输出信息
	if(LoRa_Get(100)){                                //设置模块，返回值如果非零，都表示错误
		u1_printf("查询模块失败\r\n");   	          //串口输出信息
		return 3;                                     //返回3
	}else u1_printf("查询模块成功\r\n");   	          //串口输出信息	
	
	LoRa_MODE0;                                       //切换到模式0
	HAL_Delay(500);                                   //适当延时
    return 0;	                                      //正确返回0
}
/*-------------------------------------------------*/
/*函数名：LoRa设置工作参数                         */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
uint8_t LoRa_Set(int16_t timeout)
{
	UART2_ControlCB.UsartRxDataOutPtr = UART2_ControlCB.UsartRxDataInPtr;
  UART2_ControlCB.Usart_RxCounter = 0;

	uint8_t cmd[12];                                         //保存设置参数的数组缓冲区，命令共计12字节
	 
	cmd[0] = 0xC0;                                           //设置参数指令的起始字节，固定值0xC0
	cmd[1] = 0x00;                                           //设置参数的起始寄存器地址，从寄存器0开始设置，一共9个
	cmd[2] = 0x09;                                           //一共设置9个寄存器
	
	cmd[3] = LoRaSetData.LoRa_AddrH;                         //设置模块地址高字节
	cmd[4] = LoRaSetData.LoRa_AddrL;                         //设置模块地址低字节
	cmd[5] = LoRaSetData.LoRa_NetID;                         //设置模块网络地址	
	cmd[6] = LoRaSetData.LoRa_Baudrate | LoRaSetData.LoRa_UartMode | LoRaSetData.LoRa_airvelocity;    //设置模块 波特率 串口模式 空中速率
	cmd[7] = LoRaSetData.LoRa_DataLen | LoRaSetData.LoRa_Rssi | LoRaSetData.LoRa_TxPower;             //设置模块 数据分包长度 RSSI噪声是否使能 发射功率
	cmd[8] = LoRaSetData.LoRa_CH;                            //设置模块信道	
	cmd[9] = LoRaSetData.LoRa_RssiByte | LoRaSetData.LoRa_DateMode | LoRaSetData.LoRa_Relay | LoRaSetData.LoRa_LBT | LoRaSetData.LoRa_WORmode | LoRaSetData.LoRa_WORcycle; //设置模块 是否使用RSSI字节功能 传输方式 是否中继 是否使用LBT WOR模式收发控制 WOR周期
	cmd[10] = LoRaSetData.LoRa_KeyH;                         //设置模块加密秘钥高字节
	cmd[11] = LoRaSetData.LoRa_KeyL;                         //设置模块加密秘钥低字节
	
	if(LoRa_AUX==1){                                         //AUX是高电平时，表示空闲，才可发送命令		
		while(timeout--){                                    //等待超时时间到0
		    LoRa_TxData(cmd,12);                             //发送数据	
			HAL_Delay(200);                                  //延时	
			if((UART2_ControlCB.UsartRxDataOutPtr->StartPtr[0]==0xC1)&&(UART2_ControlCB.UsartRxDataOutPtr->EndPtr - UART2_ControlCB.UsartRxDataOutPtr->StartPtr==12)){                         //如果是的第一个字节是0xC1，表示收到了，模块的回复				
				break;       						         //主动跳出while循环
			}
			u1_printf("%d ",timeout);                        //串口输出现在的超时时间
		}
		u1_printf("\r\n");                                   //串口输出信息
		if(timeout<=0)return 1;                              //如果timeout<=0，说明超时时间到了，也没能收到0xC1，返回1
		UART2_ControlCB.UsartRxDataOutPtr ++;                                         //串口1的 UsartRxDataOutPtr指针 下移
		if(UART2_ControlCB.UsartRxDataOutPtr==UART2_ControlCB.UsartRxDataEndPtr)      //如果到达处理接收数据的结构体数组尾部了
			UART2_ControlCB.UsartRxDataOutPtr = &UART2_ControlCB.Usart_RxLocation[0]; //串口1的 UsartRxDataOutPtr指针 回到处理接收数据的结构体数组开头  
	}else return 2;                                          //LoRa_AUX不是高电平，表示忙，现在不能设置参数
	return 0;                                                //正确返回0
}
/*-------------------------------------------------*/
/*函数名：LoRa查询工作参数                         */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
uint8_t LoRa_Get(int16_t timeout)
{
	UART2_ControlCB.UsartRxDataOutPtr = UART2_ControlCB.UsartRxDataInPtr;
  UART2_ControlCB.Usart_RxCounter = 0;

	uint8_t cmd[3]={0xC1,0x00,0x07};                         //C1是查询模块的起始字节 00表示从寄存器0开始读取 07表示读取7个寄存器 
	uint8_t ch;                                              //保存信道的变量
	 
	if(LoRa_AUX==1){                                         //AUX是高电平时，表示空闲，才可发送命令
		LoRa_TxData(cmd,3);                                  //发送数据				
		while(timeout--){                                    //等待超时时间到0
			HAL_Delay(200);                                  //延时
			if((UART2_ControlCB.UsartRxDataOutPtr->StartPtr[0]==0xC1)&&(UART2_ControlCB.UsartRxDataOutPtr->EndPtr - UART2_ControlCB.UsartRxDataOutPtr->StartPtr==10)){  //如果是的第一个字节是0xC1，表示收到了，模块的回复				
				break;       						                                          //主动跳出while循环
			}
			u1_printf("%d ",timeout);                        //串口输出现在的超时时间
		}
		u1_printf("\r\n");                                                       //串口输出信息
		if(timeout<=0)return 2;                                                  //如果timeout<=0，说明超时时间到了，也没能收到0xC1，返回2
		else{ 		         			           	                             //反之，表示正确，说明收到0xC0，通过break主动跳出while
			u1_printf("模块地址:0x%02X%02X\r\n",UART2_ControlCB.UsartRxDataOutPtr->StartPtr[3],UART2_ControlCB.UsartRxDataOutPtr->StartPtr[4]);  //串口输出信息
			u1_printf("模块网络ID:0x%02X\r\n",UART2_ControlCB.UsartRxDataOutPtr->StartPtr[5]);                   //串口输出信息
			
			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[6]&0xE0){         //判断串口波特率
				case LoRa_1200 : u1_printf("波特率 1200\r\n");                   //串口输出信息
								 break;                                          //跳出
				case LoRa_2400 : u1_printf("波特率 2400\r\n");                   //串口输出信息
								 break;                                          //跳出
				case LoRa_4800 : u1_printf("波特率 4800\r\n");                   //串口输出信息
								 break;                                          //跳出
				case LoRa_9600 : u1_printf("波特率 9600\r\n");                   //串口输出信息
								 break;                                          //跳出
				case LoRa_19200 : u1_printf("波特率 19200\r\n");                 //串口输出信息
								 break;                                          //跳出
				case LoRa_38400 : u1_printf("波特率 38400\r\n");                 //串口输出信息
								 break;                                          //跳出
				case LoRa_57600 : u1_printf("波特率 57600\r\n");                 //串口输出信息
								 break;                                          //跳出
				case LoRa_115200: u1_printf("波特率 115200\r\n");                //串口输出信息
								 break;                                          //跳出
			}	
			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[6]&0x18){         //判断串口参数
				case LoRa_8N1 :	 u1_printf("8数据位 无校验 1停止位\r\n");        //串口输出信息
								 break;                                          //跳出
				case LoRa_8O1 :	 u1_printf("8数据位 奇校验 1停止位\r\n");        //串口输出信息
								 break;                                          //跳出
				case LoRa_8E1 :	 u1_printf("8数据位 偶校验 1停止位\r\n");        //串口输出信息
								 break;                                          //跳出
			}		
			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[6]&0x07){         //判断空中速率
				case LoRa_0_3 :  u1_printf("空中速率 0.3K\r\n");                 //串口输出信息
								 break;                                          //跳出
				case LoRa_1_2 :  u1_printf("空中速率 1.2K\r\n");                 //串口输出信息
								 break;                                          //跳出
				case LoRa_2_4 :  u1_printf("空中速率 2.4K\r\n");                 //串口输出信息
								 break;                                          //跳出
				case LoRa_4_8 :  u1_printf("空中速率 4.8K\r\n");                 //串口输出信息
								 break;                                          //跳出
				case LoRa_9_6 :  u1_printf("空中速率 9.6K\r\n");                 //串口输出信息
								 break;                                          //跳出
				case LoRa_19_2 : u1_printf("空中速率 19.2K\r\n");                //串口输出信息
								 break;                                          //跳出
			}
			
			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[7]&0xC0){         //判断数据分包大小
				case LoRa_Data240 : u1_printf("数据分包大小：240字节\r\n");      //串口输出信息
								    break;                                       //跳出
				case LoRa_Data128 : u1_printf("数据分包大小：128字节\r\n");      //串口输出信息
								    break;                                       //跳出
				case LoRa_Data64  : u1_printf("数据分包大小：64字节\r\n");       //串口输出信息
								    break;                                       //跳出
				case LoRa_Data32  : u1_printf("数据分包大小：32字节\r\n");       //串口输出信息
								    break;                                       //跳出
			}	
			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[7]&0x20){         //判断是否启用RSSI功能
				case LoRa_RssiEN  :	u1_printf("启用RSSI功能\r\n");               //串口输出信息
								    break;                                       //跳出
				case LoRa_RssiDIS :	u1_printf("禁用RSSI功能\r\n");               //串口输出信息
								    break;                                       //跳出
			}		
			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[7]&0x03){         //判断发射功率
				case LoRa_FEC_22DBM :  u1_printf("发射功率 22dbm\r\n");          //串口输出信息
								       break;                                    //跳出
				case LoRa_FEC_17DBM :  u1_printf("发射功率 17dbm\r\n");          //串口输出信息
								       break;                                    //跳出
				case LoRa_FEC_13DBM :  u1_printf("发射功率 13dbm\r\n");          //串口输出信息
								       break;                                    //跳出
				case LoRa_FEC_10DBM :  u1_printf("发射功率 10dbm\r\n");          //串口输出信息
								       break;                                    //跳出
			}
			
			ch = UART2_ControlCB.UsartRxDataOutPtr->StartPtr[8] & 0x7F;          //保存信道
			u1_printf("信道:0x%02X  对应频率%dMHz\r\n",ch,410+ch);               //串口输出信息
			
			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[9]&0x80){         //判断RSSI字节功能
				case LoRa_RssiByteEN  : u1_printf("启用RSSI字节功能\r\n");       //串口输出信息
								        break;                                   //跳出
				case LoRa_RssiByteDIS : u1_printf("禁用RSSI字节功能\r\n");       //串口输出信息
								        break;                                   //跳出
			}
			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[9]&0x40){         //判断传输模式
				case LoRa_ModeTRANS :  u1_printf("透明传输\r\n");                //串口输出信息
								       break;                                    //跳出
				case LoRa_ModePOINT :  u1_printf("定点传输\r\n");                //串口输出信息
								       break;                                    //跳出
			}
			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[9]&0x20){        //判断中继功能
				case LoRa_RelayEN  :  u1_printf("启用中继\r\n");                //串口输出信息
								      break;                                    //跳出
				case LoRa_RelayDIS :  u1_printf("禁用中继\r\n");                //串口输出信息
								      break;                                    //跳出
			}
			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[9]&0x10){        //判断LBT功能
				case LoRa_LBTEN  :  u1_printf("启用LBT\r\n");                   //串口输出信息
								      break;                                    //跳出
				case LoRa_LBTDIS :  u1_printf("禁用LBT\r\n");                   //串口输出信息
								      break;                                    //跳出
			}
//			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[9]&0x08){        //判断WOR模式
//				case LoRa_WorTX  :  u1_printf("Wor模式发送\r\n");               //串口输出信息
//								      break;                                    //跳出
//				case LoRa_WorRX :  u1_printf("Wor模式接收\r\n");                //串口输出信息
//								      break;                                    //跳出
//			}
//			switch(UART2_ControlCB.UsartRxDataOutPtr->StartPtr[9]&0x07){        //判断WOR周期
//				case LoRa_Wor500ms  : u1_printf("WOR周期时间 500毫秒\r\n");     //串口输出信息
//								      break;                                    //跳出
//				case LoRa_Wor1000ms : u1_printf("WOR周期时间 1000毫秒\r\n");    //串口输出信息
//								      break;                                    //跳出
//				case LoRa_Wor1500ms : u1_printf("WOR周期时间 1500毫秒\r\n");    //串口输出信息
//								      break;                                    //跳出
//				case LoRa_Wor2000ms : u1_printf("WOR周期时间 2000毫秒\r\n");    //串口输出信息
//								      break;                                    //跳出
//				case LoRa_Wor2500ms : u1_printf("WOR周期时间 2500毫秒\r\n");    //串口输出信息
//								      break;                                    //跳出
//				case LoRa_Wor3000ms : u1_printf("WOR周期时间 3000毫秒\r\n");    //串口输出信息
//								      break;                                    //跳出
//				case LoRa_Wor3500ms : u1_printf("WOR周期时间 3500毫秒\r\n");    //串口输出信息
//								      break;                                    //跳出
//				case LoRa_Wor4000ms : u1_printf("WOR周期时间 4000毫秒\r\n");    //串口输出信息
//								      break;                                    //跳出
//			}
			UART2_ControlCB.UsartRxDataOutPtr ++;                                         //串口1的 UsartRxDataOutPtr指针 下移
			if(UART2_ControlCB.UsartRxDataOutPtr==UART2_ControlCB.UsartRxDataEndPtr)      //如果到达处理接收数据的结构体数组尾部了
				UART2_ControlCB.UsartRxDataOutPtr = &UART2_ControlCB.Usart_RxLocation[0]; //串口1的 UsartRxDataOutPtr指针 回到处理接收数据的结构体数组开头  
		}
	}else return 1;                                                              //AUX不是高电平时，表示忙，返回1
	return 0;                                                                    //正确 返回0
}

void LoRa_PointSend(uint8_t destH, uint8_t destL, uint8_t destCH,
                    const uint8_t *payload, uint16_t len)
{
    uint8_t buf[3 + 256];              // 够用就行，别超
    if (len > 256) len = 256;

    buf[0] = destH;
    buf[1] = destL;
    buf[2] = destCH;
    memcpy(&buf[3], payload, len);

    u2_TxData(buf, len + 3);           // 你工程里 LoRa_TxData 就是 u2_TxData
}


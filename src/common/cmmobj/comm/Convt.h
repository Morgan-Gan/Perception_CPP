/**********************************************************************
功能描述:转换码
输入参数:pu8InBuf 待转码字节串 
		 u16InBufLen待转码字节长度
输出参数:pu8OutBuf 转码后字节串
返 回 值: >0:转换码数据长度  ==0:出错
其它说明:发送数据时，如果一帧数据中（桢头帧尾除外的其他字节），出现帧头（0x7d）、帧尾（0x0B）或者换码符（0x34）， 
    	 则在该数据前插入换码符（0x34），并将该字节用其反码替代。
		 接收数据时，将接收数据中的换码符（x34）抛弃，并将其后一字节数据取反。
* 修改记录1:
*    修改日期:
*    版 本 号:
*    修 改 人:
*    修改内容:
***********************************************************************/
static int ConvtCode(unsigned char *pu8InBuf, int s32InBufLen, unsigned char *pu8OutBuf)
{
	if (NULL == pu8InBuf || NULL == pu8OutBuf)
	{
		return 0;
	}

	int k = 0;
	unsigned char u8Temp = 0x00;

	for (int i = 0; i < s32InBufLen; i++, k++)
	{
		u8Temp = pu8InBuf[i];
		if(( 0x7D== u8Temp) || (0x0B == u8Temp) || (0x34 == u8Temp))
		{
			pu8OutBuf[k] = 0x34;
			k++;
			pu8OutBuf[k] = ~u8Temp;
		}
		else
		{
			pu8OutBuf[k] = u8Temp;
		}
	}

	return k;
}

/**********************************************************************
功能描述:反转换码
输入参数:pu8InBuf 待反转码字节串 
		 u16InBufLen待反转码字节长度
输出参数:pu8OutBuf 反转码后字节串
返 回 值: >0:反转换码数据长度  ==0:出错
其它说明:发送数据时，如果一帧数据中（桢头帧尾除外的其他字节），出现帧头（0x7d）、帧尾（0x0B）或者换码符（0x34）， 
    	 则在该数据前插入换码符（0x34），并将该字节用其反码替代。
		 接收数据时，将接收数据中的换码符（x34）抛弃，并将其后一字节数据取反。
* 修改记录1:
*    修改日期:
*    版 本 号:
*    修 改 人:
*    修改内容:
***********************************************************************/
static int UnconvtCode(unsigned char *pu8InBuf, int s32InBufLen, unsigned char *pu8OutBuf)
{
	if (NULL == pu8InBuf || NULL == pu8OutBuf)
	{
		return 0;
	}

	int	k = 0;
	unsigned char u8Temp = 0x00;

	for(int i = 0; i < s32InBufLen;)
	{
		u8Temp = pu8InBuf[i++];
		if(u8Temp == 0x34)
		{
			u8Temp = ~pu8InBuf[i++];
		}

		pu8OutBuf[k++] = u8Temp;
	}

	return k;
}
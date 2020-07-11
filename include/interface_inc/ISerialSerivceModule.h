#ifndef __SERIAL_H__
#define __SERIAL_H__

enum COMPORT
{
    COM0 = 0,
    COM1,
    COM2,
    COM3,
    ttyUSB0,
    ttyUSB1,
    ttyUSB2
};

#ifdef __cplusplus
    extern "C"{
#endif

    //功能: 打开串口
    //index 参考COMPORT
    int serial_OpenPort(int index);

    //功能：设置串口通信参数
    //fd:打开串口后返回的文件句柄
    //speed是波特率,参数如下：
			//0:B2400;
			//1:B4800;
			//2:B9600;
			//3:B19200;
			//4:B38400;
			//5:B57600;
			//6:B115200;
    //databits数据位:
			//0:5位
			//1:6位
			//2:7位
			//3:8位
    //stopbits停止位:
                        //1:1位
                        //2:2位
    //stopbits校验位:
                        //0:无校验
                        //1:奇校验
                        //2:偶校验
    int serial_SetPara(int fd, int speed, int databits, int stopbits, int parity);

    //功能:向串口发送数据
    //fd:打开串口后返回的文件句柄
    //data:已经分配好的缓冲数组,用于存放即将发送的串口数据
    //datalength:缓冲区的数据长度
    int serial_WriteData(int fd, const char *data, int datalength);

    //功能: 读取串口数据
    //fd:打开串口后返回的文件句柄
    //data:已经分配好的缓冲数组,用于接收串口数据
    //datalength:每次最多读取多少个字节
    int serial_ReadData(int fd, unsigned char *data, int datalength);

    //功能:关闭串口
    //fd:打开串口后返回的文件句柄
    void serial_ClosePort(int fd);


#ifdef __cplusplus
    }
#endif
#endif

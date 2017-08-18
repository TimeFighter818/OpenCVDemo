//编译说明：
//在解决方案资源管理器中，把cser.h加入到头文件中，把cser.c加入到源文件中
//在cser.c文件上鼠标右键，选择“属性”
//在属性中的C / C++常规：公共语言运行时支持，设成公共语言运行时支持 / clr
//在属性中的C / C++代码生成：启用最小重新生成，设成否
//在属性中的C / C++代码生成：启用C++异常，设成否
//在属性中的C / C++代码生成：基本运行时检查，设成默认值

//数据格式说明
//通讯双方遵守每一帧数据固定字节长度（CMD_LENGTH），并且以字节CMD_END_BYTE结束
//注意，这样的话帧数据中其他字节不能等于CMD_END_BYTE，否则接收方会以为帧结束，重新开始接收下一帧

#define CMD_LENGTH 6   //一帧数据的字节数
#define MT_ADDR 0x82
#define MCU_ADDR 0x81
#define CMD_END_BYTE 0xFF  //帧最后一个字节标志，表示帧结束。
//可供调用的串口函数
short SendFrame(unsigned char* buff);
short GetFrame(unsigned char* buff);
void InitPort(short ser_mode);
void ClosePort(void);

//自定义的发命令的函数
short EnableMT(unsigned char bOn);
short SetSpeed(short nLeft, short nRight);
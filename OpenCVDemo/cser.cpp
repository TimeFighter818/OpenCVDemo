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

#using <System.dll>

using namespace System;
using namespace System::IO::Ports;
using namespace System::Threading;

#include "cser.h"
public ref class PortChat
{
private:
	static bool _continue;
	static bool _frame_rec_ok;  //是否接收到一个完整帧
	static SerialPort^ _serialPort;  //串口指针
	static Thread^ readThread;  //读串口线程
	static array<unsigned char,1>^ _buff; //发送缓冲区
	static array<unsigned char, 1>^ _buff_rec; //接收缓冲区
	static short _ser_mode;  //串口工作模式。=0 ：每次通讯由本程序发起，图像处理完发送一个数据帧后等待stm32返回应答帧，将应答帧的第2、3字节作为函数返回值
							//				=1：每次通讯由STM32发起，本程序一直处于等待接收数据状态，当接收到一个数据帧后，根据要求返回一帧数据。

public:
	/*自定义的函数*/
	static void InitPort(short ser_mode)  //初始化串口函数
	{
		_ser_mode = ser_mode;
		Console::WriteLine("Init Serial Port...");
		// Create a new SerialPort object with default settings.
		_serialPort = gcnew SerialPort();
		_buff = gcnew array<unsigned char, 1>(CMD_LENGTH);
		_buff_rec = gcnew array<unsigned char, 1>(CMD_LENGTH);

		// 用户在串口中输入串口参数 Allow the user to set the appropriate properties.
		_serialPort->PortName = "COM3";
		_serialPort->BaudRate = 19200;
		_serialPort->PortName = SetPortName(_serialPort->PortName);
		_serialPort->BaudRate = SetPortBaudRate(_serialPort->BaudRate);
		_serialPort->Parity = SetPortParity(_serialPort->Parity);
		_serialPort->DataBits = SetPortDataBits(_serialPort->DataBits);
		_serialPort->StopBits = SetPortStopBits(_serialPort->StopBits);
		_serialPort->Handshake = SetPortHandshake(_serialPort->Handshake);

		//设置串口读写超时时间 Set the read/write timeouts
		_serialPort->ReadTimeout = 500;
		_serialPort->WriteTimeout = 500;

		_frame_rec_ok = false;
		_continue = false;
		//启用读串口线程，如果你是在发完命令后等待应答信息的话，则不要启用读串口线程。
		if (_ser_mode == 1)
		{
			readThread = gcnew Thread(gcnew ThreadStart(PortChat::Read));
			//启动读串口线程
			readThread->Start();
			_continue = true;  //为真，则串口读线程开始接收数据帧
		}


		//打开串口
		_serialPort->Open();
		
		Console::WriteLine("Serial Port is Opened.");
	}
	static Int16 SendBuff(unsigned char * buff)
	{
		Int16 res;
		res = -1;
		Console::Write("CMD:");
		for (int i = 0; i < CMD_LENGTH; i++)
		{
			_buff[i] = buff[i];
			Console::Write("0X{0:X} ", _buff[i]);
		}
		Console::WriteLine(". ");
		_serialPort->RtsEnable = false;  //如果是使用485通讯，把读/写引脚设成写

		if (_ser_mode == 0)	_serialPort->ReadExisting();  //模式0中，在发送之前，把串口读缓冲器中数据先读完。
		try
		{
			//_serialPort->Write(_buff, 0, CMD_LENGTH);
			for (int i = 0; i < CMD_LENGTH / 2; i++)
			{
				_serialPort->Write(_buff, i * 2, 2);
			}
		}
		catch (TimeoutException ^)  //如果try区块中的代码执行失败，会跳到这里执行
		{
			Console::WriteLine("Serial Port Write Time Out.");
		}
		_serialPort->RtsEnable = true; //如果是使用485通讯，把读/写引脚设成读

		//下面这段代码表示发送完数据以后，就等着接收应答数据。
		if (_ser_mode == 0)  //模式0，发送完数据等待接收应答数据，超时没收到，则返回默认的-1了。
		{
			try
			{
				_serialPort->Read(_buff, 0, CMD_LENGTH); //等待接收CMD_LENGTH字节的数据帧
				res = (_buff[2] << 8) + _buff[3];  //返回数据帧的第2、3字节组成的整数
			}
			catch (TimeoutException ^)
			{
				Console::WriteLine("Module has no response.");
			}
		}
		else
		{
			res = 0;   //如果是模式1，则始终是返回0
		}
		
		return res;
	}
	static void ClosePort(void)
	{
		if (_ser_mode == 1)
			readThread->Join();  //模式1，在InitPort中启用了读串口线程，则要在这里关闭读线程

		_serialPort->Close();  //关闭串口
		Console::WriteLine("Serial Port is Closed .");
	}


	static void Read()
	{
		short i;
		
		array<unsigned char, 1>^ buff_rec;
		buff_rec = gcnew array<unsigned char, 1>(1);
		i = 0;
		while (_continue)
		{
			try
			{
				//int nByte;
				//nByte = _serialPort->ReadByte(); //读一个字节。跟下面这个Read类似，看哪一个能成功读到1个字节。
				//buff_rec[0] = nByte;
				//if (nByte != -1)
				if (_serialPort->Read(buff_rec, 0, 1) > 0)   //从串口读到一个字节
				{
					//如果上一次的数据帧没有处理完毕，则丢弃接收到的字节？
					if (_frame_rec_ok == true)
					{
						;
					}
					else
					{
						_buff_rec[i] = buff_rec[0];  //把一个字节保存到类私有变量_buff_rec中去
						i++;
						if (buff_rec[0] == CMD_END_BYTE && i == CMD_LENGTH) //如果接收到了帧长度字节，而且最后一个字节是结束标志，则表明接收成功
						{
							_frame_rec_ok = true;
						}
						else
						{
							_frame_rec_ok = false;
						}
					}
					if (buff_rec[0] == CMD_END_BYTE || i >= CMD_LENGTH) //如果接收到结束字节或者接收到帧长度时，下次从0开始
					{
						i = 0;
					}
				}
			}
			catch (TimeoutException ^) {}
		}
	}

	static String^ SetPortName(String^ defaultPortName)
	{
		String^ portName;

		Console::WriteLine("Available Ports:");
		for each (String^ s in SerialPort::GetPortNames())
		{
			Console::WriteLine("   {0}", s);
		}

		Console::Write("COM port({0}): ", defaultPortName);
		portName = Console::ReadLine();

		if (portName == "")
		{
			portName = defaultPortName;
		}
		return portName;
	}

	static Int32 SetPortBaudRate(Int32 defaultPortBaudRate)
	{
		String^ baudRate;

		Console::Write("Baud Rate({0}): ", defaultPortBaudRate);
		baudRate = Console::ReadLine();

		if (baudRate == "")
		{
			baudRate = defaultPortBaudRate.ToString();
		}

		return Int32::Parse(baudRate);
	}

	static Parity SetPortParity(Parity defaultPortParity)
	{
		String^ parity;

		Console::WriteLine("Available Parity options:");
		for each (String^ s in Enum::GetNames(Parity::typeid))
		{
			Console::WriteLine("   {0}", s);
		}

		Console::Write("Parity({0}):", defaultPortParity.ToString());
		parity = Console::ReadLine();

		if (parity == "")
		{
			parity = defaultPortParity.ToString();
		}

		return (Parity)Enum::Parse(Parity::typeid, parity);
	}

	static Int32 SetPortDataBits(Int32 defaultPortDataBits)
	{
		String^ dataBits;

		Console::Write("Data Bits({0}): ", defaultPortDataBits);
		dataBits = Console::ReadLine();

		if (dataBits == "")
		{
			dataBits = defaultPortDataBits.ToString();
		}

		return Int32::Parse(dataBits);
	}

	static StopBits SetPortStopBits(StopBits defaultPortStopBits)
	{
		String^ stopBits;

		Console::WriteLine("Available Stop Bits options:");
		for each (String^ s in Enum::GetNames(StopBits::typeid))
		{
			Console::WriteLine("   {0}", s);
		}

		Console::Write("Stop Bits({0}):", defaultPortStopBits.ToString());
		stopBits = Console::ReadLine();

		if (stopBits == "")
		{
			stopBits = defaultPortStopBits.ToString();
		}

		return (StopBits)Enum::Parse(StopBits::typeid, stopBits);
	}

	static Handshake SetPortHandshake(Handshake defaultPortHandshake)
	{
		String^ handshake;

		Console::WriteLine("Available Handshake options:");
		for each (String^ s in Enum::GetNames(Handshake::typeid))
		{
			Console::WriteLine("   {0}", s);
		}

		Console::Write("Handshake({0}):", defaultPortHandshake.ToString());
		handshake = Console::ReadLine();

		if (handshake == "")
		{
			handshake = defaultPortHandshake.ToString();
		}

		return (Handshake)Enum::Parse(Handshake::typeid, handshake);
	}
	static bool GetFrameFlag(void)
	{
		return _frame_rec_ok;
	}
	static void ClearFrameFlag(void)
	{
		_frame_rec_ok = false;
	}
	static unsigned char GetRecBuff(short i)
	{
		if (i < CMD_LENGTH)
			return _buff_rec[i];
		else
			return 0;
	}
};

/*自定义函数结束*/

//下面是可供调用的串口函数
short SendFrame(unsigned char buff[])
{
	return PortChat::SendBuff(buff);
}

//如果使用串口读线程，可以调用这个函数获得接收到的数据帧，如果数据帧不完整，返回-1，完整的话，返回第2个字节，并把所有自己保存在buff参数中
short GetFrame(unsigned char buff[])
{
	short i;
	if (PortChat::GetFrameFlag() == false) return -1;  //如果没有接收到一个完整帧，则返回-1；
	for (i = 0; i < CMD_LENGTH; i++)
	{
		buff[i] = PortChat::GetRecBuff(i);
	}
	PortChat::ClearFrameFlag(); //接收完后，清除接收标志
	return buff[2];
}

//ser_mode 串口工作模式。=0 ：每次通讯由本程序发起，图像处理完发送一个数据帧后等待stm32返回应答帧，将应答帧的第2、3字节作为函数返回值
//						=1：每次通讯由STM32发起，本程序一直处于等待接收数据状态，当接收到一个数据帧后，根据要求返回一帧数据。
void InitPort(short ser_mode)
{
	PortChat::InitPort(ser_mode);
}
void ClosePort(void)
{
	PortChat::ClosePort();
}

//下面是自定义的发命令的函数，可根据自己的需要增加函数
short EnableMT(unsigned char bOn)  //使能电机,一般用于_ser_mode=0
{
	unsigned char buff[CMD_LENGTH];
	buff[0] = MT_ADDR;    //目标器件地址
	buff[1] = MCU_ADDR;		//发送器件地址
	buff[2] = 0x32;			//命令类型
	buff[3] = 0;
	buff[4] = bOn;			//命令参数
	buff[5] = CMD_END_BYTE;	//帧结束字节
	return SendFrame(buff);
}
short SetSpeed(short nLeft, short nRight)  //发送左右轮速度，一般用于_ser_mode=0
{
	unsigned char buff[CMD_LENGTH];
	if (nLeft < -63) nLeft = -63;
	if (nLeft > 63) nLeft = 63;
	if (nRight < -63) nRight = -63;
	if (nRight > 63) nRight = 63;


	buff[0] = MT_ADDR;
	buff[1] = MCU_ADDR;
	buff[2] = 0x24; //命令类型
	buff[3] = nLeft+63; //命令参数
	buff[4] = nRight+63;	//命令参数
	buff[5] = CMD_END_BYTE; //帧结束字节
	return SendFrame(buff);
}

//array<Byte>^ MakeManagedArray(unsigned char* input, int len)
//{
//	array<Byte>^ result = gcnew array<Byte>(len);
//	for (int i = 0; i < len; i++)
//	{
//		result[i] = input[i];
//	}
//	return result;
//}
//����˵����
//�ڽ��������Դ�������У���cser.h���뵽ͷ�ļ��У���cser.c���뵽Դ�ļ���
//��cser.c�ļ�������Ҽ���ѡ�����ԡ�
//�������е�C / C++���棺������������ʱ֧�֣���ɹ�����������ʱ֧�� / clr
//�������е�C / C++�������ɣ�������С�������ɣ���ɷ�
//�������е�C / C++�������ɣ�����C++�쳣����ɷ�
//�������е�C / C++�������ɣ���������ʱ��飬���Ĭ��ֵ

#using <System.dll>

using namespace System;
using namespace System::IO::Ports;
using namespace System::Threading;

#include "cser.h"
public ref class PortChat
{
private:
	static bool _continue;
	static bool _frame_rec_ok;  //�Ƿ���յ�һ������֡
	static SerialPort^ _serialPort;  //����ָ��
	static Thread^ readThread;  //�������߳�
	static array<unsigned char,1>^ _buff; //���ͻ�����
	static array<unsigned char, 1>^ _buff_rec; //���ջ�����

public:
	/*�Զ���ĺ���*/
	static void InitPort()  //��ʼ�����ں���
	{
		Console::WriteLine("Init Serial Port...");
		// Create a new SerialPort object with default settings.
		_serialPort = gcnew SerialPort();
		_buff = gcnew array<unsigned char, 1>(CMD_LENGTH);
		_buff_rec = gcnew array<unsigned char, 1>(CMD_LENGTH);

		// �û��ڴ��������봮�ڲ��� Allow the user to set the appropriate properties.
		_serialPort->PortName = "COM3";
		_serialPort->BaudRate = 19200;
		_serialPort->PortName = SetPortName(_serialPort->PortName);
		_serialPort->BaudRate = SetPortBaudRate(_serialPort->BaudRate);
		_serialPort->Parity = SetPortParity(_serialPort->Parity);
		_serialPort->DataBits = SetPortDataBits(_serialPort->DataBits);
		_serialPort->StopBits = SetPortStopBits(_serialPort->StopBits);
		_serialPort->Handshake = SetPortHandshake(_serialPort->Handshake);

		//���ô��ڶ�д��ʱʱ�� Set the read/write timeouts
		_serialPort->ReadTimeout = 500;
		_serialPort->WriteTimeout = 500;


		//���ö������̣߳���������ڷ��������ȴ�Ӧ����Ϣ�Ļ�����Ҫ���ö������̡߳�
		//readThread = gcnew Thread(gcnew ThreadStart(PortChat::Read));
		//�����������߳�
		//readThread->Start();
		//_continue = true;
		//_frame_rec_ok = false;  //�����������ڶ������߳���Ҫ�õ���������ö������̣߳�����ҲҪȡ��ע��		


		//�򿪴���
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
		_serialPort->RtsEnable = false;  //�����ʹ��485ͨѶ���Ѷ�/д�������д
		_serialPort->ReadExisting();  //�ڷ���֮ǰ���Ѵ��ڶ��������������ȶ��ꡣ
		try
		{
			//_serialPort->Write(_buff, 0, CMD_LENGTH);
			for (int i = 0; i < CMD_LENGTH / 2; i++)
            {
                _serialPort->Write(_buff, i * 2, 2);
            }
		}
		catch (TimeoutException ^)  //���try�����еĴ���ִ��ʧ�ܣ�����������ִ��
		{
			Console::WriteLine("Serial Port Write Time Out.");
		}
		_serialPort->RtsEnable = true; //�����ʹ��485ͨѶ���Ѷ�/д������ɶ�

		//������δ����ʾ�����������Ժ󣬾͵��Ž���Ӧ�����ݡ�
		//��������˶������̣߳�������Ͳ��ܶ��ˡ����Ҫ����������ڣ�Ҫ�رն������̡߳�
		try
		{
			_serialPort->Read(_buff, 0, CMD_LENGTH); //�ȴ�����CMD_LENGTH�ֽڵ�����֡
			res = (_buff[3] << 8) + _buff[4];  //��������֡�ĵ�3��4�ֽ���ɵ�����
			
		}
		catch (TimeoutException ^)
		{
			Console::WriteLine("Module has no response.");
		}
		
		return res;
	}
	static void ClosePort(void)
	{
		//readThread->Join();  //�����InitPort�������˶������̣߳���Ҫ������رն��߳�
		_serialPort->Close();  //�رմ���
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
				if (_serialPort->Read(buff_rec, 0, 1) > 0)   //�Ӵ��ڶ���һ���ֽ�
				{
					_buff_rec[i] = buff_rec[0];  //��һ���ֽڱ��浽��˽�б���_buff_rec��ȥ
					i++;
					if (buff_rec[0] == CMD_END_BYTE && i == CMD_LENGTH) //������յ���֡�����ֽڣ��������һ���ֽ��ǽ�����־����������ճɹ�
					{
						_frame_rec_ok = true;
					}
					else
					{
						_frame_rec_ok = false;
					}
					if (buff_rec[0] == CMD_END_BYTE || i >= CMD_LENGTH) //������յ������ֽڻ��߽��յ�֡����ʱ���´δ�0��ʼ
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
	static unsigned char GetRecBuff(short i)
	{
		if (i < CMD_LENGTH)
			return _buff_rec[i];
		else
			return 0;
	}
};

/*�Զ��庯������*/

//�����ǿɹ����õĴ��ں���
short SendCmd(unsigned char buff[])
{
	return PortChat::SendBuff(buff);
}

//���ʹ�ô��ڶ��̣߳����Ե������������ý��յ�������֡���������֡������������-1�������Ļ������ص�2���ֽڣ����������Լ�������buff������
short GetCmd(unsigned char buff[])
{
	short i;
	if (PortChat::GetFrameFlag() == false) return -1;
	for (i = 0; i < CMD_LENGTH; i++)
	{
		buff[i] = PortChat::GetRecBuff(i);
	}
	return buff[2];
}

void InitPort(void)
{
	PortChat::InitPort();
}
void ClosePort(void)
{
	PortChat::ClosePort();
}

//�������Զ���ķ�����ĺ���
short EnableMT(unsigned char bOn)
{
	unsigned char buff[CMD_LENGTH];
	buff[0] = MT_ADDR;
	buff[1] = MCU_ADDR;
	buff[2] = 0x32;
	buff[3] = 0;
	buff[4] = bOn;
	buff[5] = CMD_END_BYTE;
	return SendCmd(buff);
}
short SetSpeed(short nLeft, short nRight)
{
	unsigned char buff[CMD_LENGTH];
	if (nLeft < -63) nLeft = -63;
	if (nLeft > 63) nLeft = 63;
	if (nRight < -63) nRight = -63;
	if (nRight > 63) nRight = 63;


	buff[0] = MT_ADDR;
	buff[1] = MCU_ADDR;
	buff[2] = 0x24;
	buff[3] = nLeft+63;
	buff[4] = nRight+63;
	buff[5] = CMD_END_BYTE;
	return SendCmd(buff);
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
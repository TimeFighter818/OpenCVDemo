//����˵����
//�ڽ��������Դ�������У���cser.h���뵽ͷ�ļ��У���cser.c���뵽Դ�ļ���
//��cser.c�ļ�������Ҽ���ѡ�����ԡ�
//�������е�C / C++���棺������������ʱ֧�֣���ɹ�����������ʱ֧�� / clr
//�������е�C / C++�������ɣ�������С�������ɣ���ɷ�
//�������е�C / C++�������ɣ�����C++�쳣����ɷ�
//�������е�C / C++�������ɣ���������ʱ��飬���Ĭ��ֵ

//���ݸ�ʽ˵��
//ͨѶ˫������ÿһ֡���ݹ̶��ֽڳ��ȣ�CMD_LENGTH�����������ֽ�CMD_END_BYTE����
//ע�⣬�����Ļ�֡�����������ֽڲ��ܵ���CMD_END_BYTE��������շ�����Ϊ֡���������¿�ʼ������һ֡

#define CMD_LENGTH 6   //һ֡���ݵ��ֽ���
#define MT_ADDR 0x82
#define MCU_ADDR 0x81
#define CMD_END_BYTE 0xFF  //֡���һ���ֽڱ�־����ʾ֡������
//�ɹ����õĴ��ں���
short SendFrame(unsigned char* buff);
short GetFrame(unsigned char* buff);
void InitPort(short ser_mode);
void ClosePort(void);

//�Զ���ķ�����ĺ���
short EnableMT(unsigned char bOn);
short SetSpeed(short nLeft, short nRight);
//#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include "cser.h"

using namespace std;
using namespace cv;

string window_src = "ԭʼͼ��";
string window_hsv = "HSVͼ��";

/** @ Main������ */
int main(int argc, const char** argv)
{
	CvCapture* capture;
	Mat frame;
	Mat hsv;
	unsigned char buff[CMD_LENGTH];
	InitPort();//�򿪴���
	//����һ������ͷ�Ĳ���ͨ�������������������񶼿��ԡ�
	//capture = cvCreateCameraCapture(0);
	capture = cvCaptureFromCAM(0);
	if (capture) //�������ɹ�
	{
		while (true)
		{
			frame = cvQueryFrame(capture); //������ͷ��ȡһ��ͼ��
			if (!frame.empty())  //���ͼ���ȡ�ɹ�
			{
				blur(frame, frame, Size(3, 3));  //ģ����һ�£��൱��PSĥƤ��
				imshow(window_src, frame);   //��ʾ��ȡ��ͼ��
				cvtColor(frame,hsv,CV_BGR2HSV);
				imshow(window_hsv, hsv);

				//�����귢����Ϣ����÷�����Ϣ������Ҫ��stm32һֱ�ȴ�����֡�����յ�����֡����������һ��Ӧ��֡
				buff[0] = 1;
				buff[1] = 2;
				buff[2] = 3;
				buff[3] = 4;
				buff[4] = 5;
				buff[5] = CMD_END_BYTE;
				SendCmd(buff);

				//����PC�����STM32�Ƿ�������֡������������������������귵������
				if (GetCmd(buff) != -1)
				{
					if (buff[0] == 1)
					{
						buff[0] = 1;
						buff[1] = 2;
						buff[2] = 3;
						buff[3] = 4;
						buff[4] = 5;
						buff[5] = CMD_END_BYTE;
						SendCmd(buff);
					}
				}

			}
			else  //���ͼ���ȡ���ɹ������˳�whileѭ��
			{
				printf(" --(!) No captured frame -- Break!"); break;
			}

			//����ȴ��������룬��Ҫ��opencv������а��°����Ż���Ӧ���������ط����°�����û�з�Ӧ�ģ�
			int c = waitKey(1); //�Ӽ��̶�һ��ֵ������������Ϊ0����һֱ�ȴ�ֱ�����̰��£�����ȴ��趨��ʱ��û�м��̰��£������ִ��
			if ((char)c == 27) { break; } //���������esc�������˳�whileѭ����
		}

		cvReleaseCapture(&capture);

	}
	ClosePort();//�رմ���
	return 0;
}

//#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include "cser.h"

using namespace std;
using namespace cv;

string window_src = "原始图像";
string window_hsv = "HSV图像";

/** @ Main主函数 */
int main(int argc, const char** argv)
{
	CvCapture* capture;
	Mat frame;
	Mat hsv;
	unsigned char buff[CMD_LENGTH];
	InitPort();//打开串口
	//建立一个摄像头的捕获通道，下面两个函数好像都可以。
	//capture = cvCreateCameraCapture(0);
	capture = cvCaptureFromCAM(0);
	if (capture) //如果捕获成功
	{
		while (true)
		{
			frame = cvQueryFrame(capture); //从摄像头获取一副图像
			if (!frame.empty())  //如果图像获取成功
			{
				blur(frame, frame, Size(3, 3));  //模糊化一下，相当于PS磨皮？
				imshow(window_src, frame);   //显示获取的图像
				cvtColor(frame,hsv,CV_BGR2HSV);
				imshow(window_hsv, hsv);

				//处理完发送信息并获得返回信息，这里要求stm32一直等待数据帧，接收到数据帧后立即返回一个应答帧
				buff[0] = 1;
				buff[1] = 2;
				buff[2] = 3;
				buff[3] = 4;
				buff[4] = 5;
				buff[5] = CMD_END_BYTE;
				SendCmd(buff);

				//或者PC机检测STM32是否有数据帧发送来，如果有则立即处理完返回数据
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
			else  //如果图像获取不成功，则退出while循环
			{
				printf(" --(!) No captured frame -- Break!"); break;
			}

			//下面等待按键代码，需要在opencv活动窗口中按下按键才会响应！在其他地方按下按键是没有反应的！
			int c = waitKey(1); //从键盘读一个值。如果输入参数为0，则一直等待直到键盘按下；否则等待设定的时间没有键盘按下，则继续执行
			if ((char)c == 27) { break; } //如果按下了esc键，则退出while循环。
		}

		cvReleaseCapture(&capture);

	}
	ClosePort();//关闭串口
	return 0;
}

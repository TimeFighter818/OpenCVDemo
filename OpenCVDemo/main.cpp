//#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include "cser.h"

using namespace std;
using namespace cv;

string window_src = "原始图像+矩形轮廓";
string window_threshold = "二值图像";

Mat imgSrc;
Mat imgHSV;
Mat imgDraw;
Mat imgThresholded;
vector<Mat> hsvSplit;
int nContoursThresh = 100;
int nHSVThresh = 100;
int nHue = 100;
int nSaturation = 100;
int nValue = 100;
int nKey;

RNG rng(12345);
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
CvCapture* capture;

/** @ Main主函数 */
int main(int argc, const char** argv)
{
	unsigned char buff[CMD_LENGTH];
	InitPort(0);//打开串口，使用串口读模式0，也就是每次本程序主动发送一个数据帧，stm32接收到数据帧后立即返回一个应答数据帧
	//InitPort(1);//打开串口，使用串口读模式1，也就是本程序一直等待stm由串口发来一个命令，根据命令处理完毕后返回一个应答数据帧

	//建立一个摄像头的捕获通道，下面两个函数好像都可以。
	//capture = cvCreateCameraCapture(0);
	capture = cvCaptureFromCAM(0);

	if (capture) //如果捕获成功
	{
		while (true)
		{
			imgSrc = cvQueryFrame(capture); //从摄像头获取一副图像


			if (!imgSrc.empty())  //如果图像获取成功
			{
				imgSrc = imread("C:\\Users\\tlan\\Documents\\Projects\\2.jpg", 1);  //调试用，获取一张保存的图片代替摄像头
				blur(imgSrc, imgSrc, Size(3, 3));  //模糊化一下，相当于PS磨皮？
				cvtColor(imgSrc, imgHSV, COLOR_BGR2HSV);   //将图像转换成HSV色彩空间
				//因为我们读取的是彩色图，直方图均衡化需要在HSV空间做
				split(imgHSV, hsvSplit);
				equalizeHist(hsvSplit[2], hsvSplit[2]);
				merge(hsvSplit, imgHSV);

				//假设找黄色木块，设定期望的HSV值，以及范围阈值
				nHue = 22;
				nSaturation = 253;
				nValue = 201;
				nHSVThresh = 75;
				//inRange函数把imgHSV图像根据设定的H+/-阈值、S+/-阈值、V+/-阈值转换成二值图像imgThresholded，如果是在设定区域内则为白色，否则为黑色
				//二值图像保存在imgThresholded中
				inRange(imgHSV, Scalar(nHue - nHSVThresh, nSaturation - nHSVThresh, nValue - nHSVThresh),
					Scalar(nHue + nHSVThresh, nSaturation + nHSVThresh, nValue + nHSVThresh), imgThresholded); //Threshold the image

				//开操作 (去除一些噪点)
				Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
				morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);

				//闭操作 (连接一些连通域)
				morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);

				//显示二值图
				imshow(window_threshold, imgThresholded); //show the thresholded image

				//接下来根据二值图找轮廓
				findContours(imgThresholded, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

				/// Find the rotated rectangles and ellipses for each contour
				vector<RotatedRect> minRect(contours.size());
				//vector<RotatedRect> minEllipse(contours.size());

				//找所有轮廓的最小矩形区域
				for (int i = 0; i < contours.size(); i++)
				{
					minRect[i] = minAreaRect(Mat(contours[i]));
					//if (contours[i].size() > 5)
					//{
					//	minEllipse[i] = fitEllipse(Mat(contours[i]));
					//}
				}

				//在原图上画出彩色的矩形框表示找到的轮廓，Draw contours + rotated rects + ellipses
				//Mat drawing = Mat::zeros(imgThresholded.size(), CV_8UC3);
				imgDraw = imgSrc.clone();
				cout << "Find " << contours.size() << " Contour(s)" << endl;
				for (int i = 0; i< contours.size(); i++)
				{
					Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
					// contour
					//drawContours(drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point());
					// ellipse
					//ellipse(drawing, minEllipse[i], color, 2, 8);
					// rotated rectangle
					cout << "Contour " << dec << i; //在终端窗口显示第i个轮廓
					cout << "; Area=" << minRect[i].size.area(); //在终端窗口显示第i个轮廓的矩形面积

					Point2f rect_points[4]; minRect[i].points(rect_points);
					cout << "; Center: X=" << (rect_points[0].x + rect_points[1].x + rect_points[2].x + rect_points[3].x) / 4; //在终端窗口显示第i个矩形四个顶点的X位置
					cout << " Y= " << (rect_points[0].y + rect_points[1].y + rect_points[2].y + rect_points[3].y) / 4 << endl; //在终端窗口显示第i个矩形四个顶点的Y位置
					for (int j = 0; j < 4; j++)
						line(imgDraw, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
				}

				//显示添加了轮廓线的原图 Show in a window
				namedWindow(window_src, CV_WINDOW_AUTOSIZE);
				imshow(window_src, imgDraw);

				//找最大面积那个矩形区域中心的X，Y位置
				int max_x, max_y;
				int max_i;
				float max_area;
				int nRet;
				max_area = 0;
				for (int i = 0; i < contours.size(); i++)
				{
					if (minRect[i].size.area() > max_area)
					{
						Point2f rect_points[4];
						minRect[i].points(rect_points);
						max_x = (int)((rect_points[0].x + rect_points[1].x + rect_points[2].x + rect_points[3].x) / 4);
						max_y = (int)((rect_points[0].y + rect_points[1].y + rect_points[2].y + rect_points[3].y) / 4);
						max_i = i;
					}
				}

				//在串口模式0下（InitPort(0)情况下），立即发送处理完的数据。例如：在运输任务中，将最大的那个矩形框的中心X，Y坐标发送给STM32。
				buff[0] = MT_ADDR;
				buff[1] = MCU_ADDR;
				
				buff[5] = CMD_END_BYTE; //帧结束字节

				if (max_area == 0)//如果最大面积为0，则说明根本没找到
				{
					buff[2] = 0x01; //命令类型，没有找到
					buff[3] = 0; //命令参数
					buff[4] = 0;	//命令参数
					nRet=SendFrame(buff);  //nRet返回了应答数据帧的第2，3字节，如果没有应答返回是-1
				}
				else
				{

					buff[2] = 0x02; //命令类型，X位置
					buff[3] = (max_x >> 8) & 0xFF; //命令参数
					buff[4] = max_x & 0xFF;	//命令参数

					nRet=SendFrame(buff);

					//注意：连续发两帧数据是否需要延时一会。看stm32能否处理得过来
					buff[2] = 0x03; //命令类型，Y位置
					buff[3] = (max_y >> 8) & 0xFF; //命令参数
					buff[4] = max_y & 0xFF;	//命令参数
					nRet=SendFrame(buff);

				}

				//在串口模式1下（InitPort(1)情况下）
				//PC机检测STM32是否有数据帧发送来，如果有则立即处理完返回数据
				if (GetFrame(buff) != -1)
				{
					buff[0] = MT_ADDR;
					buff[1] = MCU_ADDR;
					buff[5] = CMD_END_BYTE; //帧结束字节
					switch (buff[2]) //发来的数据帧中，第2个字节是命令类型
					{
					case 1:  //例如1代表询问的是货架下格是否有黄色木块
						max_i = 0;
						for (int i = 0; i < contours.size(); i++)
						{
							if (minRect[i].size.area() > 0)
							{
								Point2f rect_points[4];
								minRect[i].points(rect_points);
								max_x = (int)((rect_points[0].x + rect_points[1].x + rect_points[2].x + rect_points[3].x) / 4);
								max_y = (int)((rect_points[0].y + rect_points[1].y + rect_points[2].y + rect_points[3].y) / 4);
								if (max_x > 120 && max_x < 420 && max_y> 340) //假设图像是640*480，则x在120-420区间，y在大于340区间，认为是下格
								{
									max_i = 1;
								}
							}
						}
						buff[2] = 1; //返回下格黄色木块信息
						buff[3] = max_i; //没找到是0，找到了是1
						buff[4] = 0; //未使用
						SendFrame(buff);
						break;
					case 2:  //例如2代表询问的是货架上格是否有黄色木块
						max_i = 0;
						for (int i = 0; i < contours.size(); i++)
						{
							if (minRect[i].size.area() > 0)
							{
								Point2f rect_points[4];
								minRect[i].points(rect_points);
								max_x = (int)((rect_points[0].x + rect_points[1].x + rect_points[2].x + rect_points[3].x) / 4);
								max_y = (int)((rect_points[0].y + rect_points[1].y + rect_points[2].y + rect_points[3].y) / 4);
								if (max_x > 120 && max_x < 420 && max_y < 240) //假设图像是640*480，则x在120-420区间，y在小于240区间，认为是上格
								{
									max_i = 1;
								}
							}
						}
						buff[2] = 2; //返回上格黄色木块信息
						buff[3] = max_i; //没找到是0，找到了是1
						buff[4] = 0; //未使用
						SendFrame(buff);
						break;
					case 3:
						break;
					default:
						break;
					}
				}

			}
			else  //如果图像获取不成功，则退出while循环
			{
				printf(" --(!) No captured frame -- Break!"); 
				//如果要退出while循环结束程序，则注释掉下面这个break
				break;
			}

			//下面等待按键代码，需要在opencv活动窗口中按下按键才会响应！在其他地方按下按键是没有反应的！
			nKey = waitKey(1); //从键盘读一个值。如果输入参数为0，则一直等待直到键盘按下；否则等待设定的时间没有键盘按下，则继续执行
			if (nKey == 27) { break; } //如果按下了esc键，则退出while循环。
		}

		cvReleaseCapture(&capture);

	}
	ClosePort();//关闭串口
	return 0;
}

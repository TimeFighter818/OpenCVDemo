//#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include "cser.h"

using namespace std;
using namespace cv;

string window_src = "ԭʼͼ��+��������";
string window_threshold = "��ֵͼ��";

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

/** @ Main������ */
int main(int argc, const char** argv)
{
	unsigned char buff[CMD_LENGTH];
	InitPort(0);//�򿪴��ڣ�ʹ�ô��ڶ�ģʽ0��Ҳ����ÿ�α�������������һ������֡��stm32���յ�����֡����������һ��Ӧ������֡
	//InitPort(1);//�򿪴��ڣ�ʹ�ô��ڶ�ģʽ1��Ҳ���Ǳ�����һֱ�ȴ�stm�ɴ��ڷ���һ����������������Ϻ󷵻�һ��Ӧ������֡

	//����һ������ͷ�Ĳ���ͨ�������������������񶼿��ԡ�
	//capture = cvCreateCameraCapture(0);
	capture = cvCaptureFromCAM(0);

	if (capture) //�������ɹ�
	{
		while (true)
		{
			imgSrc = cvQueryFrame(capture); //������ͷ��ȡһ��ͼ��


			if (!imgSrc.empty())  //���ͼ���ȡ�ɹ�
			{
				imgSrc = imread("C:\\Users\\tlan\\Documents\\Projects\\2.jpg", 1);  //�����ã���ȡһ�ű����ͼƬ��������ͷ
				blur(imgSrc, imgSrc, Size(3, 3));  //ģ����һ�£��൱��PSĥƤ��
				cvtColor(imgSrc, imgHSV, COLOR_BGR2HSV);   //��ͼ��ת����HSVɫ�ʿռ�
				//��Ϊ���Ƕ�ȡ���ǲ�ɫͼ��ֱ��ͼ���⻯��Ҫ��HSV�ռ���
				split(imgHSV, hsvSplit);
				equalizeHist(hsvSplit[2], hsvSplit[2]);
				merge(hsvSplit, imgHSV);

				//�����һ�ɫľ�飬�趨������HSVֵ���Լ���Χ��ֵ
				nHue = 22;
				nSaturation = 253;
				nValue = 201;
				nHSVThresh = 75;
				//inRange������imgHSVͼ������趨��H+/-��ֵ��S+/-��ֵ��V+/-��ֵת���ɶ�ֵͼ��imgThresholded����������趨��������Ϊ��ɫ������Ϊ��ɫ
				//��ֵͼ�񱣴���imgThresholded��
				inRange(imgHSV, Scalar(nHue - nHSVThresh, nSaturation - nHSVThresh, nValue - nHSVThresh),
					Scalar(nHue + nHSVThresh, nSaturation + nHSVThresh, nValue + nHSVThresh), imgThresholded); //Threshold the image

				//������ (ȥ��һЩ���)
				Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
				morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);

				//�ղ��� (����һЩ��ͨ��)
				morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);

				//��ʾ��ֵͼ
				imshow(window_threshold, imgThresholded); //show the thresholded image

				//���������ݶ�ֵͼ������
				findContours(imgThresholded, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

				/// Find the rotated rectangles and ellipses for each contour
				vector<RotatedRect> minRect(contours.size());
				//vector<RotatedRect> minEllipse(contours.size());

				//��������������С��������
				for (int i = 0; i < contours.size(); i++)
				{
					minRect[i] = minAreaRect(Mat(contours[i]));
					//if (contours[i].size() > 5)
					//{
					//	minEllipse[i] = fitEllipse(Mat(contours[i]));
					//}
				}

				//��ԭͼ�ϻ�����ɫ�ľ��ο��ʾ�ҵ���������Draw contours + rotated rects + ellipses
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
					cout << "Contour " << dec << i; //���ն˴�����ʾ��i������
					cout << "; Area=" << minRect[i].size.area(); //���ն˴�����ʾ��i�������ľ������

					Point2f rect_points[4]; minRect[i].points(rect_points);
					cout << "; Center: X=" << (rect_points[0].x + rect_points[1].x + rect_points[2].x + rect_points[3].x) / 4; //���ն˴�����ʾ��i�������ĸ������Xλ��
					cout << " Y= " << (rect_points[0].y + rect_points[1].y + rect_points[2].y + rect_points[3].y) / 4 << endl; //���ն˴�����ʾ��i�������ĸ������Yλ��
					for (int j = 0; j < 4; j++)
						line(imgDraw, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
				}

				//��ʾ����������ߵ�ԭͼ Show in a window
				namedWindow(window_src, CV_WINDOW_AUTOSIZE);
				imshow(window_src, imgDraw);

				//���������Ǹ������������ĵ�X��Yλ��
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

				//�ڴ���ģʽ0�£�InitPort(0)����£����������ʹ���������ݡ����磺�����������У��������Ǹ����ο������X��Y���귢�͸�STM32��
				buff[0] = MT_ADDR;
				buff[1] = MCU_ADDR;
				
				buff[5] = CMD_END_BYTE; //֡�����ֽ�

				if (max_area == 0)//���������Ϊ0����˵������û�ҵ�
				{
					buff[2] = 0x01; //�������ͣ�û���ҵ�
					buff[3] = 0; //�������
					buff[4] = 0;	//�������
					nRet=SendFrame(buff);  //nRet������Ӧ������֡�ĵ�2��3�ֽڣ����û��Ӧ�𷵻���-1
				}
				else
				{

					buff[2] = 0x02; //�������ͣ�Xλ��
					buff[3] = (max_x >> 8) & 0xFF; //�������
					buff[4] = max_x & 0xFF;	//�������

					nRet=SendFrame(buff);

					//ע�⣺��������֡�����Ƿ���Ҫ��ʱһ�ᡣ��stm32�ܷ���ù���
					buff[2] = 0x03; //�������ͣ�Yλ��
					buff[3] = (max_y >> 8) & 0xFF; //�������
					buff[4] = max_y & 0xFF;	//�������
					nRet=SendFrame(buff);

				}

				//�ڴ���ģʽ1�£�InitPort(1)����£�
				//PC�����STM32�Ƿ�������֡������������������������귵������
				if (GetFrame(buff) != -1)
				{
					buff[0] = MT_ADDR;
					buff[1] = MCU_ADDR;
					buff[5] = CMD_END_BYTE; //֡�����ֽ�
					switch (buff[2]) //����������֡�У���2���ֽ�����������
					{
					case 1:  //����1����ѯ�ʵ��ǻ����¸��Ƿ��л�ɫľ��
						max_i = 0;
						for (int i = 0; i < contours.size(); i++)
						{
							if (minRect[i].size.area() > 0)
							{
								Point2f rect_points[4];
								minRect[i].points(rect_points);
								max_x = (int)((rect_points[0].x + rect_points[1].x + rect_points[2].x + rect_points[3].x) / 4);
								max_y = (int)((rect_points[0].y + rect_points[1].y + rect_points[2].y + rect_points[3].y) / 4);
								if (max_x > 120 && max_x < 420 && max_y> 340) //����ͼ����640*480����x��120-420���䣬y�ڴ���340���䣬��Ϊ���¸�
								{
									max_i = 1;
								}
							}
						}
						buff[2] = 1; //�����¸��ɫľ����Ϣ
						buff[3] = max_i; //û�ҵ���0���ҵ�����1
						buff[4] = 0; //δʹ��
						SendFrame(buff);
						break;
					case 2:  //����2����ѯ�ʵ��ǻ����ϸ��Ƿ��л�ɫľ��
						max_i = 0;
						for (int i = 0; i < contours.size(); i++)
						{
							if (minRect[i].size.area() > 0)
							{
								Point2f rect_points[4];
								minRect[i].points(rect_points);
								max_x = (int)((rect_points[0].x + rect_points[1].x + rect_points[2].x + rect_points[3].x) / 4);
								max_y = (int)((rect_points[0].y + rect_points[1].y + rect_points[2].y + rect_points[3].y) / 4);
								if (max_x > 120 && max_x < 420 && max_y < 240) //����ͼ����640*480����x��120-420���䣬y��С��240���䣬��Ϊ���ϸ�
								{
									max_i = 1;
								}
							}
						}
						buff[2] = 2; //�����ϸ��ɫľ����Ϣ
						buff[3] = max_i; //û�ҵ���0���ҵ�����1
						buff[4] = 0; //δʹ��
						SendFrame(buff);
						break;
					case 3:
						break;
					default:
						break;
					}
				}

			}
			else  //���ͼ���ȡ���ɹ������˳�whileѭ��
			{
				printf(" --(!) No captured frame -- Break!"); 
				//���Ҫ�˳�whileѭ������������ע�͵��������break
				break;
			}

			//����ȴ��������룬��Ҫ��opencv������а��°����Ż���Ӧ���������ط����°�����û�з�Ӧ�ģ�
			nKey = waitKey(1); //�Ӽ��̶�һ��ֵ������������Ϊ0����һֱ�ȴ�ֱ�����̰��£�����ȴ��趨��ʱ��û�м��̰��£������ִ��
			if (nKey == 27) { break; } //���������esc�������˳�whileѭ����
		}

		cvReleaseCapture(&capture);

	}
	ClosePort();//�رմ���
	return 0;
}

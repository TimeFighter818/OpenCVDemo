#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;

static void onMouse(int event, int x, int y, int f, void*);

Mat imgSrc;
Mat imgHSV;
Mat imgGray;
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

/** @function main */
int main(int argc, char** argv)
{

	capture = cvCaptureFromCAM(0);
	if (capture) //�������ɹ�
	{
		namedWindow("Control", CV_WINDOW_NORMAL); //����һ��"Control"����

		cvCreateTrackbar("Hֵ", "Control", &nHue, 179);  //����Hue������ ��0-179��

		cvCreateTrackbar("Sֵ", "Control", &nSaturation, 255); //����Saturation ������(0 - 255)

		cvCreateTrackbar("Vֵ", "Control", &nValue, 255); //����Value������ (0 - 255)
		
		cvCreateTrackbar("HSV��ֵ", "Control", &nHSVThresh, 255); //������ֵ��������ֻҪH��S��Vֵ�������������Χ�ڣ�������Ҫ�ҵ���ɫ
		
		imgSrc = cvQueryFrame(capture); //������ͷ��ȡһ��ͼ��
		if (!imgSrc.empty()) imshow("Original", imgSrc); //show the original image

		setMouseCallback("Original", onMouse, 0);  //��������¼���������������ԭͼ���ϵ������������ɻ���Ǹ����HSVֵ�����ı��������λ�ã�����µ�HSVֵ

		while (true)
		{
			imgSrc = cvQueryFrame(capture); //������ͷ��ȡһ��ͼ��
			if (!imgSrc.empty())  //���ͼ���ȡ�ɹ�
			{
				//imgSrc = imread("C:\\Users\\tlan\\Documents\\Projects\\2.jpg", 1);  //�����ã���ȡһ�ű����ͼƬ��������ͷ
				blur(imgSrc, imgSrc, Size(3, 3));  //ģ����һ�£��൱��PSĥƤ��
				cvtColor(imgSrc, imgHSV, COLOR_BGR2HSV);   //��ͼ��ת����HSVɫ�ʿռ�
				//��Ϊ���Ƕ�ȡ���ǲ�ɫͼ��ֱ��ͼ���⻯��Ҫ��HSV�ռ���
				split(imgHSV, hsvSplit);
				equalizeHist(hsvSplit[2], hsvSplit[2]);
				merge(hsvSplit, imgHSV);
				
				//inRange������imgHSVͼ������趨��H+/-��ֵ��S+/-��ֵ��V+/-��ֵת���ɶ�ֵͼ��imgThresholded����������趨��������Ϊ��ɫ������Ϊ��ɫ
				inRange(imgHSV, Scalar(nHue-nHSVThresh, nSaturation-nHSVThresh, nValue-nHSVThresh),
					Scalar(nHue + nHSVThresh, nSaturation + nHSVThresh, nValue + nHSVThresh), imgThresholded); //Threshold the image

				//������ (ȥ��һЩ���)
				Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
				morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);

				//�ղ��� (����һЩ��ͨ��)
				morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);

				imshow("Thresholded Image", imgThresholded); //show the thresholded image
				imshow("Original", imgSrc); //show the original image


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
				Mat drawing = imgSrc.clone();  
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
						line(drawing, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
				}

				/// Show in a window
				namedWindow("����", CV_WINDOW_AUTOSIZE);
				imshow("����", drawing);

				//����ȴ��������룬��Ҫ��opencv������а��°����Ż���Ӧ���������ط����°�����û�з�Ӧ�ģ�
				nKey = waitKey(30); //�Ӽ��̶�һ��ֵ������������Ϊ0����һֱ�ȴ�ֱ�����̰��£�����ȴ��趨��ʱ��û�м��̰��£������ִ��
				if (nKey == 27) //���������esc�������˳�whileѭ����
					break;
			}
		}
		cvReleaseCapture(&capture);

	}
	return(0);
}

//����¼�����
static void onMouse(int event, int x, int y, int f, void*)
{
	if(event == EVENT_LBUTTONDOWN)  //������������
	{
		Mat image = imgHSV.clone();
		Vec3b hsv = image.at<Vec3b>(y, x);
		int H = hsv.val[0];
		int S = hsv.val[1];
		int V = hsv.val[2];
		setTrackbarPos("Hֵ", "Control", H);
		setTrackbarPos("Sֵ", "Control", S);
		setTrackbarPos("Vֵ", "Control", V);
	}

}



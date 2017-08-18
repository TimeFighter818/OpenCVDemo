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
	if (capture) //如果捕获成功
	{
		namedWindow("Control", CV_WINDOW_NORMAL); //建立一个"Control"窗口

		cvCreateTrackbar("H值", "Control", &nHue, 179);  //创建Hue控制条 （0-179）

		cvCreateTrackbar("S值", "Control", &nSaturation, 255); //创建Saturation 控制条(0 - 255)

		cvCreateTrackbar("V值", "Control", &nValue, 255); //创建Value控制条 (0 - 255)
		
		cvCreateTrackbar("HSV阈值", "Control", &nHSVThresh, 255); //创建阈值控制条，只要H、S、V值落在正负这个范围内，就算是要找的颜色
		
		imgSrc = cvQueryFrame(capture); //从摄像头获取一副图像
		if (!imgSrc.empty()) imshow("Original", imgSrc); //show the original image

		setMouseCallback("Original", onMouse, 0);  //设置鼠标事件处理函数，这样在原图像上单击鼠标左键即可获得那个点的HSV值，并改变控制条的位置，获得新的HSV值

		while (true)
		{
			imgSrc = cvQueryFrame(capture); //从摄像头获取一副图像
			if (!imgSrc.empty())  //如果图像获取成功
			{
				//imgSrc = imread("C:\\Users\\tlan\\Documents\\Projects\\2.jpg", 1);  //调试用，获取一张保存的图片代替摄像头
				blur(imgSrc, imgSrc, Size(3, 3));  //模糊化一下，相当于PS磨皮？
				cvtColor(imgSrc, imgHSV, COLOR_BGR2HSV);   //将图像转换成HSV色彩空间
				//因为我们读取的是彩色图，直方图均衡化需要在HSV空间做
				split(imgHSV, hsvSplit);
				equalizeHist(hsvSplit[2], hsvSplit[2]);
				merge(hsvSplit, imgHSV);
				
				//inRange函数把imgHSV图像根据设定的H+/-阈值、S+/-阈值、V+/-阈值转换成二值图像imgThresholded，如果是在设定区域内则为白色，否则为黑色
				inRange(imgHSV, Scalar(nHue-nHSVThresh, nSaturation-nHSVThresh, nValue-nHSVThresh),
					Scalar(nHue + nHSVThresh, nSaturation + nHSVThresh, nValue + nHSVThresh), imgThresholded); //Threshold the image

				//开操作 (去除一些噪点)
				Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
				morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);

				//闭操作 (连接一些连通域)
				morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);

				imshow("Thresholded Image", imgThresholded); //show the thresholded image
				imshow("Original", imgSrc); //show the original image


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
					cout << "Contour " << dec << i; //在终端窗口显示第i个轮廓
					cout << "; Area=" << minRect[i].size.area(); //在终端窗口显示第i个轮廓的矩形面积

					Point2f rect_points[4]; minRect[i].points(rect_points);
					cout << "; Center: X=" << (rect_points[0].x + rect_points[1].x + rect_points[2].x + rect_points[3].x) / 4; //在终端窗口显示第i个矩形四个顶点的X位置
					cout << " Y= " << (rect_points[0].y + rect_points[1].y + rect_points[2].y + rect_points[3].y) / 4 << endl; //在终端窗口显示第i个矩形四个顶点的Y位置
					for (int j = 0; j < 4; j++)
						line(drawing, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
				}

				/// Show in a window
				namedWindow("轮廓", CV_WINDOW_AUTOSIZE);
				imshow("轮廓", drawing);

				//下面等待按键代码，需要在opencv活动窗口中按下按键才会响应！在其他地方按下按键是没有反应的！
				nKey = waitKey(30); //从键盘读一个值。如果输入参数为0，则一直等待直到键盘按下；否则等待设定的时间没有键盘按下，则继续执行
				if (nKey == 27) //如果按下了esc键，则退出while循环。
					break;
			}
		}
		cvReleaseCapture(&capture);

	}
	return(0);
}

//鼠标事件函数
static void onMouse(int event, int x, int y, int f, void*)
{
	if(event == EVENT_LBUTTONDOWN)  //如果是左键按下
	{
		Mat image = imgHSV.clone();
		Vec3b hsv = image.at<Vec3b>(y, x);
		int H = hsv.val[0];
		int S = hsv.val[1];
		int V = hsv.val[2];
		setTrackbarPos("H值", "Control", H);
		setTrackbarPos("S值", "Control", S);
		setTrackbarPos("V值", "Control", V);
	}

}



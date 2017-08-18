#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

CvPoint pt1 = cvPoint(0, 0);
CvPoint pt2 = cvPoint(0, 0);
bool is_selecting = false;

CvCapture* capture;

static void onMouse(int event, int x, int y, int, void*)
{
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		pt1 = cvPoint(x, y);
		pt2 = cvPoint(x, y);
		is_selecting = true;
		break;
	case CV_EVENT_MOUSEMOVE:
		if (is_selecting)
			pt2 = cvPoint(x, y);
		break;
	case CV_EVENT_LBUTTONUP:
		pt2 = cvPoint(x, y);
		is_selecting = false;
		break;
	}
	return;
}


int main(int argc, const char** argv)
{
	printf("用鼠标左键在图像窗口中画框，以得到目标区域的大致位置.\
		   		   \n然后用a,s,d,w移动所选区域，用1，2,3,5放大或缩小之\
				   		   \n由于opencv中键盘响应函数不能响应键盘中的方向键、shift键和ctrl键\
						   		   \n所以用tab来切换状态（放大和缩小）。\
								   		   \n选择结束后用enter键保存。Esc键退出。");

	//char img_path[80] = ".\\test.png";
	char save_path[80] = ".\\save.jpg";
	char* window = "img";
	IplImage* img;
	IplImage* img_show;
	capture = cvCaptureFromCAM(0);
	if (capture) //如果捕获成功
	{
		img = cvQueryFrame(capture);
		
		img_show = cvCloneImage(img);

		namedWindow(window, CV_WINDOW_AUTOSIZE);
		setMouseCallback(window, onMouse);

		bool shift_on = false;

		char text[80];
		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0);
		while (true)
		{
			img = cvQueryFrame(capture);
			cvCopy(img, img_show);
			cvRectangle(img_show, pt1, pt2, cvScalar(255, 255, 255));
			sprintf(text, "roi = cvRect(%d,%d,%d,%d)", pt1.x, pt1.y, std::abs(pt2.x - pt1.x), std::abs(pt2.y - pt1.y));
			cvPutText(img_show, text, cvPoint(10, 20), &font, cvScalar(0, 0, 255));

			cvShowImage(window, img_show);
			char key = cvWaitKey(10);
			switch (key)
			{
			case '\t':
				shift_on = !shift_on; break;

			case 'a':
				pt1.x--; pt2.x--; break;
			case 's':
				pt1.y++; pt2.y++; break;
			case 'd':
				pt1.x++; pt2.x++; break;
			case 'w':
				pt1.y--; pt2.y--; break;

			case '1':
				if (shift_on) pt1.x--;
				else pt2.x--;
				break;
			case '2':
				if (shift_on) pt2.y++;
				else pt1.y++;
				break;
			case '3':
				if (shift_on) pt2.x++;
				else pt1.x++;
				break;
			case '5':
				if (shift_on) pt1.y--;
				else pt2.y--;
				break;

			case '\r':
				cvSetImageROI(img, cvRect(pt1.x, pt1.y, std::abs(pt2.x - pt1.x), std::abs(pt2.y - pt1.y)));
				cvSaveImage(save_path, img);
				cvResetImageROI(img);
				break;
			};

			if (key == 27) break;
		}

	
	}
	//cvReleaseImage(&img);
	cvReleaseImage(&img_show);
	cvReleaseCapture(&capture);
	return 0;
}

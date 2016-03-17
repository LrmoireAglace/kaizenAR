#include <iostream>
#include "opencv2/opencv.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <sstream>
#define CANNY_MAX 90
#define CANNY_MIN 35

using namespace cv;
using namespace std;

Point point1, point2; /* vertical points of the bounding box */
int drag = 0;
Rect rect; /* bounding box */
Mat img, img_grey, roiImg, roiImg_gray, canny_img, canny_roi; /* roiImg - the part of the image in the bounding box */
int select_flag = 0;
bool go_fast = false;

Mat mytemplate;



///------- template matching -----------------------------------------------------------------------------------------------

Mat TplMatch(Mat &imgg, Mat &mytemplate)
{
	Mat result;
	matchTemplate(imgg, mytemplate, result, CV_TM_SQDIFF_NORMED);
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

	return result;
}


///------- Localizing the best match with minMaxLoc ------------------------------------------------------------------------

Point minmax(Mat &result)
{
	double minVal, maxVal;
	Point  minLoc, maxLoc, matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	matchLoc = minLoc;
	std::cout << "Score: " << minVal << " ," << maxVal << endl;

	return matchLoc;
}


///------- tracking --------------------------------------------------------------------------------------------------------

void track()
{
	if (select_flag)
	{
		//roiImg.copyTo(mytemplate);
		//    select_flag = false;
		go_fast = true;
	}




	//Img conversion to grey
	cvtColor(img, img_grey, COLOR_RGB2GRAY);

	//Convert canny filter
	//Canny(img, canny_img, CANNY_MIN, CANNY_MAX);

	//verifying
	imshow("mytemplate", mytemplate); //waitKey(0);
									  //imshow("Canny", canny_img); //waitKey(0);


									  //matching
	Mat result = TplMatch(img_grey, mytemplate);
	Point match = minmax(result);

	rectangle(img, match, Point(match.x + mytemplate.cols, match.y + mytemplate.rows), CV_RGB(255, 255, 255), 0.5);

	std::cout << "match: " << match << endl;

	/// latest match is the new template
	//Rect ROI = cv::Rect(match.x, match.y, mytemplate.cols, mytemplate.rows);
	//roiImg = img(ROI);
	//roiImg.copyTo(mytemplate);
	//imshow("roiImg", roiImg); //waitKey(0);
}


///------- MouseCallback function ------------------------------------------------------------------------------------------

void mouseHandler(int event, int x, int y, int flags, void *param)
{
	if (event == CV_EVENT_LBUTTONDOWN && !drag)
	{
		/// left button clicked. ROI selection begins
		point1 = Point(x, y);
		drag = 1;
	}

	if (event == CV_EVENT_MOUSEMOVE && drag)
	{
		/// mouse dragged. ROI being selected
		point2 = Point(x, y);
		rectangle(img, point1, point2, CV_RGB(255, 0, 0), 3, 8, 0);
	}

	if (event == CV_EVENT_LBUTTONUP && drag)
	{
		point2 = Point(x, y);
		rect = Rect(point1.x, point1.y, x - point1.x, y - point1.y);
		drag = 0;
		roiImg = img(rect);

		//roiIMG before any change
		imshow("MOUSE roiImg", roiImg); //waitKey(0);

										//apply threshold
		cvtColor(roiImg, roiImg_gray, COLOR_RGB2GRAY);
		threshold(roiImg_gray, roiImg, 120, 255, THRESH_BINARY);
		//imshow("MOUSE roiImg", roiImg);

		//apply canny filter
		//Canny(roiImg, canny_roi, CANNY_MIN, CANNY_MAX);
		//imshow("Canny roi", canny_roi);

		//Create template
		roiImg_gray.copyTo(mytemplate);

	}

	if (event == CV_EVENT_LBUTTONUP)
	{
		/// ROI selected
		select_flag = 1;
		drag = 0;
	}

}



///------- Main() ----------------------------------------------------------------------------------------------------------

int main()
{
	int k;

	///open webcam
	VideoCapture cap(0);
	if (!cap.isOpened())
		return 1;

	/*///Use video file
	VideoCapture cap;
	cap.open("Megamind.avi");
	if (!cap.isOpened())
	{
	cout << "Unable to open video file" << endl;    return -1;
	}*/

	cap >> img;
	//GaussianBlur(img, img, Size(7, 7), 3.0);
	imshow("Rendu", img);

	while (1)
	{
		cap >> img;
		if (img.empty())
			break;

		// Flip the frame horizontally and add blur
		cv::flip(img, img, 1);
		//GaussianBlur(img, img, Size(7, 7), 3.0);

		if (rect.width == 0 && rect.height == 0)
			cvSetMouseCallback("Rendu", mouseHandler, NULL);
		else
			track();

		imshow("Rendu", img);
		//  waitKey(100);   k = waitKey(75);
		k = waitKey(go_fast ? 30 : 100);
		if (k == 27)
			break;
	}

	return 0;
}

#include <windows.h> // windows api for directory creation 
#include <iostream>
#include <vector>

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\stitching\stitcher.hpp>

#include "Stitcher.h"

namespace {
	
	bool searchPoint(const std::vector<cv::Point>& const points, cv::Point point)
	{
		for (int i=0; i<points.size(); ++i)
			if (point == points[i])
				return true;

		return false;
	} // end of searchPoint function

	const cv::Point getTheLastPoint(const cv::Mat& const image, const std::vector<cv::Point>& points, cv::Point startPoint)
	{
		int i=0;
		for (i=startPoint.x; i<image.cols; ++i)
			if (true == searchPoint(points, cv::Point(i, startPoint.y)))
				return i == startPoint.x ?  INVALID_POINT : cv::Point(i-1, startPoint.y);

		return i == image.cols ? cv::Point(i-1, startPoint.y) : INVALID_POINT;
	} // end of getTheLastPoint function


	// morphologic erosion
	void clearOutlierBlackPoints(const cv::Mat& const originalImage, const cv::Mat& const image, std::vector<cv::Point>& points)
	{
		int neighboorNumber = 0;
		cv::Point curr,
				  temp;

		for (int i=0; i<points.size(); ++i) {
			curr = points[i];
			neighboorNumber =0;

			// kenarlara yakýn olanlara bakmýyorum. zaten kenarda ondan zarar gelmez
			if (curr.x <= NEIGHBOOR_DISTANCE ||
				curr.y <= NEIGHBOOR_DISTANCE ||
				curr.x >= image.cols-NEIGHBOOR_DISTANCE ||
				curr.y >= image.rows-NEIGHBOOR_DISTANCE)
				continue;

			temp = cv::Point(curr.x-NEIGHBOOR_DISTANCE, curr.y);
			if (false == searchPoint(points, temp))
				++neighboorNumber;

			temp = cv::Point(curr.x-NEIGHBOOR_DISTANCE, curr.y-NEIGHBOOR_DISTANCE);
			if (false == searchPoint(points, temp))
				++neighboorNumber;

			temp = cv::Point(curr.x, curr.y-NEIGHBOOR_DISTANCE);
			if (false == searchPoint(points, temp))
				++neighboorNumber;

			temp = cv::Point(curr.x+NEIGHBOOR_DISTANCE, curr.y);
			if (false == searchPoint(points, temp))
				++neighboorNumber;

			temp = cv::Point(curr.x+NEIGHBOOR_DISTANCE, curr.y+NEIGHBOOR_DISTANCE);
			if (false == searchPoint(points, temp))
				++neighboorNumber;

			temp = cv::Point(curr.x, curr.y+NEIGHBOOR_DISTANCE);
			if (false == searchPoint(points, temp))
				++neighboorNumber;

			temp = cv::Point(curr.x+NEIGHBOOR_DISTANCE, curr.y-NEIGHBOOR_DISTANCE);
			if (false == searchPoint(points, temp))
				++neighboorNumber;

			temp = cv::Point(curr.x-NEIGHBOOR_DISTANCE, curr.y+NEIGHBOOR_DISTANCE);
			if (false == searchPoint(points, temp))
				++neighboorNumber;

			// sonucu yorumlama kýsmý
			if (neighboorNumber>5)
				points.erase(points.begin() + i);
		}
	} // end of clearOutlierBlackPoints function

	int getHeight(const cv::Mat& const image, BIL496::Rectangle& rect, const std::vector<cv::Point>& points)
	{
		const cv::Point start = rect.startPoint;
		cv::Point temp;

		switch(rect.direction) {
		case BIL496::TOP_DOWN: {
			for (int i=start.y; i<image.rows; ++i) {
				temp =getTheLastPoint(image, points, cv::Point(start.x, i));

				if (start.x > temp.x) {
					rect.width = start.y-i;
					return 0;
				} else 
					continue;
					
			}
		} break;
		case BIL496::BOTTOM_UP: {
			;
		} break;
		default:
			return -1;
		} // end of switch

		return 0;

	} // end of getHeight function

} // end of unnamed namespace

	
const cv::Mat BIL496::Stitcher::stitch(const std::string& const videoPath)
{
	static cv::Mat res,
				   frame,
				   tempImg;

	std::vector<cv::Mat> frames;
	char frameFileName[20];
	int frameIndex =0, frameNumber =0;
	
	static cv::Stitcher stitcher = cv::Stitcher::createDefault(true);
	static cv::Stitcher::Status status;

	// making directory for saving frame that used for stitching operations
	CreateDirectory ("frames", NULL);


	#if defined(DEBUG_MODE)
		// Create a window for display.
		cv::namedWindow( "Display", CV_WINDOW_NORMAL);
	#endif	

	cv::VideoCapture capture(videoPath.data());

	while (true) {
		capture >> frame;

		if (frame.empty()) break;

		if (0 == (frameIndex++ % 20)) {
			++frameNumber;
			sprintf(frameFileName, "frames/frm%d.jpg", frameNumber);
			imwrite(frameFileName, frame);
			tempImg = cv::imread(frameFileName);
			frames.push_back(tempImg);
			#if defined(DEBUG_MODE)
				imshow("Display", tempImg);
				cvWaitKey(0);
			#endif
		}

	} // end of infinite loop 

	if (!frames.empty())
		status = stitcher.stitch(frames, res);

	if (status != cv::Stitcher::OK) {
		std::cerr << "There is problem at the stitching operation\n";
		
		// We can't return local variable.
		return res.clone();
	}
	
	// We can't return local variable. This is a huge problem.
	return res.clone();
}

const cv::Mat BIL496::Stitcher::fixTheEdges(const cv::Mat& const image)
{
	cv::Mat res = image.clone();
	cv::Vec3b rgbVal;
	std::vector<cv::Point> blackPointsBin;
	cv::Point current;
	current.x =0;
	current.y =0;

	Rectangle largeRect;

	largeRect.width =-1;
	largeRect.startPoint =current;
	largeRect.endPoint =current;

	for (int i=0; i<res.rows; ++i)
		for (int j=0; j<res.cols; ++j) {
			current.x = j;
			current.y = i;
			rgbVal = res.at<cv::Vec3b>(current);
			if (rgbVal.val[0] < THERSHOLD_BLACK_POINT &&
				rgbVal.val[1] < THERSHOLD_BLACK_POINT &&
				rgbVal.val[2] < THERSHOLD_BLACK_POINT ) {
				res.at<cv::Vec3b>(current).val[0] = 255;
				res.at<cv::Vec3b>(current).val[1] = 0;
				res.at<cv::Vec3b>(current).val[2] = 0;
				blackPointsBin.push_back(current);
			}
		}
	
	// double erosion
	clearOutlierBlackPoints(image, res, blackPointsBin);
	clearOutlierBlackPoints(image, res, blackPointsBin);
	
	for (int i=0; i<res.rows/2; ++i) {
		// from top
		for (int j=0; j<res.cols; ++j) {
			current.x = j;
			current.y = i;

			if (false == searchPoint(blackPointsBin, current)) {
				cv::line(res, current, cv::Point(current.x+80, current.y), cv::Scalar(0,255,0));
				cv::Point last = getTheLastPoint(res, blackPointsBin, current);
				
				if (INVALID_POINT != last) {
					BIL496::Rectangle tempRect;
					tempRect.direction = BIL496::TOP_DOWN;
					tempRect.startPoint =current;
					tempRect.endPoint =last;
					tempRect.width = -1;

					getHeight(res, tempRect, blackPointsBin);
					cv::line(res, current, cv::Point(current.x, current.y-tempRect.width), cv::Scalar(0,0,255));
					cv::line(res, current, last, cv::Scalar(0,0,255));
				}
					
				
				cv::imshow("image", res); cv::waitKey(0);
				break;
			}
		}

		// form bottom
		for (int j=0; j<res.cols; ++j) {
			current.x = j;
			current.y = res.rows-i;

			if (false == searchPoint(blackPointsBin, current)) {
				cv::line(res, current, cv::Point(current.x+80, current.y), cv::Scalar(0,255,0));
				cv::Point last = getTheLastPoint(res, blackPointsBin, current);
				
				if (INVALID_POINT != last)
					cv::line(res, current, last, cv::Scalar(0,0,255));
				
				cv::imshow("image", res); cv::waitKey(0);
				break;
			}
		}
	}

	cv::imwrite("res.jpg", res);
	
	return res.clone();
}
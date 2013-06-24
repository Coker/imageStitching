
#include <windows.h> // windows api for directory creation 
#include <iostream>
#include <vector>
#include <cmath>

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
		if (true == searchPoint(points, startPoint))
			return INVALID_POINT;
#if 0
		int i=0;
		for (i=startPoint.x; i<image.cols; ++i)
			if (true == searchPoint(points, cv::Point(i, startPoint.y)))
				return i == startPoint.x ?  INVALID_POINT : cv::Point(i-1, startPoint.y);

		// return i == image.cols ? cv::Point(i-1, startPoint.y) : INVALID_POINT;
		return cv::Point(i-1, startPoint.y);
#endif
// #if 0
		int i=0;
		for (i=image.cols-1; i>startPoint.x; --i)
			if (false == searchPoint(points, cv::Point(i, startPoint.y)))
				return cv::Point(i-1, startPoint.y);

		return INVALID_POINT;
// #endif
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
		const cv::Point end = rect.endPoint;
		int i=0;

		switch (rect.direction) {
		case BIL496::TOP_DOWN: {
			for (i=image.rows-1; i>start.y; --i) {
				if (false == searchPoint(points, cv::Point(start.x, i))) {
					rect.width = std::abs(i-start.y);
					break;
				}		
			}

			for (i=image.rows-1; i>start.y; --i) {
				if (false == searchPoint(points, cv::Point(end.x, i))) {
					int tempWidth = std::abs(i-start.y);
					
					if (tempWidth < rect.width)
						rect.width = tempWidth;
					
					return 0;
				}		
			}

			// TODO control the line, implement a function

		} break;
		case BIL496::BOTTOM_UP: {
			for (i=0; i<start.y; ++i)
				if (false == searchPoint(points, cv::Point(start.x, i))) {
					
					rect.width = std::abs(start.y-i);
					break; 
				}

			for (i=0; i<start.y; ++i)
				if (false == searchPoint(points, cv::Point(end.x, i))) {
					int tempWidth = std::abs(i-start.y);
					
					if (tempWidth < rect.width)
						rect.width = tempWidth;

					return 0;
				}
			
			// TODO control the line, implement a function
		} break;
		default:
			break;
		}

	} // end of getHeight function

} // end of unnamed namespace

bool controlTheLine(cv::Point start, cv::Point end, std::vector<cv::Point> points)
{
	for (int i=start.x; i<end.x; ++i)
		if (true == searchPoint(points, cv::Point(start.x+i, start.y)))
			return false;
	

	return true;
}

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
	largeRect.pixelSquare = 0;

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
	// clearOutlierBlackPoints(image, res, blackPointsBin);
	// clearOutlierBlackPoints(image, res, blackPointsBin);
	
	for (int i=0; i<res.rows/2; ++i) {
		// from top
		for (int j=0; j<res.cols; ++j) {
			current.x = j;
			current.y = i;

			if (false == searchPoint(blackPointsBin, current)) {
				cv::Point last = getTheLastPoint(res, blackPointsBin, current);
				cv::Mat tempImage = res.clone();

				if (INVALID_POINT != last) {
					BIL496::Rectangle tempRect;
					tempRect.direction = BIL496::TOP_DOWN;
					tempRect.startPoint =current;
					tempRect.endPoint =last;
					tempRect.width = -1;
					tempRect.pixelSquare = 0;

					getHeight(res, tempRect, blackPointsBin);
					tempRect.pixelSquare = std::abs(tempRect.startPoint.x - tempRect.endPoint.x) * 
										   std::abs(tempRect.width);
					
					if (tempRect.pixelSquare > largeRect.pixelSquare)
						largeRect = tempRect;

					// drawing rectangle
					cv::line(tempImage, tempRect.startPoint, cv::Point(tempRect.startPoint.x, current.y+tempRect.width), cv::Scalar(0,0,255));
					cv::line(tempImage, tempRect.endPoint, cv::Point(tempRect.endPoint.x, current.y+tempRect.width), cv::Scalar(0,0,255));
					cv::line(tempImage, current, last, cv::Scalar(0,0,255));
					cv::line(tempImage, cv::Point(current.x, current.y+tempRect.width), cv::Point(last.x, current.y+tempRect.width), cv::Scalar(0,0,255));
				}
					
				// cv::imshow("image", tempImage); cv::waitKey(0);
				tempImage.release();
				break;
			}
		}

		// from bottom
		for (int j=0; j<res.cols; ++j) {
			current.x = j;
			current.y = res.rows-i-1;

			if (false == searchPoint(blackPointsBin, current)) {
				cv::Point last = getTheLastPoint(res, blackPointsBin, current);
				cv::Mat tempImage = res.clone();

				if (INVALID_POINT != last) {
					BIL496::Rectangle tempRect;
					tempRect.direction = BIL496::BOTTOM_UP;
					tempRect.startPoint =current;
					tempRect.endPoint =last;
					tempRect.width = -1;
					tempRect.pixelSquare = 0;
					
					getHeight(res, tempRect, blackPointsBin);
					tempRect.pixelSquare = std::abs(tempRect.startPoint.x - tempRect.endPoint.x) *
										   std::abs(tempRect.width);

					if (tempRect.pixelSquare > largeRect.pixelSquare)
						largeRect = tempRect;

					// drawing rectangle
					cv::line(tempImage, current, last, cv::Scalar(0,0,255));
					cv::line(tempImage, current, cv::Point(current.x, current.y-tempRect.width), cv::Scalar(0,0,255));
					cv::line(tempImage, last, cv::Point(last.x, last.y-tempRect.width), cv::Scalar(0,0,255));
					cv::line(tempImage, cv::Point(current.x, current.y-tempRect.width), cv::Point(last.x, current.y-tempRect.width), cv::Scalar(0,0,255));
				}
				
				// cv::imshow("image", tempImage); cv::waitKey(0);
				tempImage.release();
				break;
			}
		}
	}
	
	cv::line(res, largeRect.startPoint, cv::Point(largeRect.endPoint.x, largeRect.endPoint.y), cv::Scalar(0,255,0), 1);
	
	if (BIL496::TOP_DOWN == largeRect.direction) {
		cv::line(res, largeRect.startPoint, cv::Point(largeRect.startPoint.x, largeRect.startPoint.y + std::abs(largeRect.width)), cv::Scalar(0,255,0), 1);
		cv::line(res, largeRect.endPoint, cv::Point(largeRect.endPoint.x, largeRect.endPoint.y + std::abs(largeRect.width)), cv::Scalar(0,255,0), 1);
		cv::line(res, cv::Point(largeRect.startPoint.x, largeRect.startPoint.y + std::abs(largeRect.width)),
				 cv::Point(largeRect.endPoint.x, largeRect.endPoint.y + std::abs(largeRect.width)), cv::Scalar(0,255,0), 1);
		
		cv::Mat largeRectImage = res(cv::Rect(largeRect.startPoint.x+1, largeRect.startPoint.y+1, std::abs(largeRect.endPoint.x - largeRect.startPoint.x)-2, largeRect.width-2 ));
		
		cv::imwrite("corroptedImage.jpg", largeRectImage);
	} else {
		cv::line(res, largeRect.startPoint, cv::Point(largeRect.startPoint.x, largeRect.startPoint.y - std::abs(largeRect.width)), cv::Scalar(0,255,0), 1);
		cv::line(res, largeRect.endPoint, cv::Point(largeRect.endPoint.x, largeRect.endPoint.y - std::abs(largeRect.width)), cv::Scalar(0,255,0), 1);
		cv::line(res, cv::Point(largeRect.startPoint.x, largeRect.startPoint.y - std::abs(largeRect.width)),
				 cv::Point(largeRect.endPoint.x, largeRect.endPoint.y - std::abs(largeRect.width)), cv::Scalar(0,255,0), 1);
	
		cv::Mat largeRectImage = res(cv::Rect(largeRect.startPoint.x+1, largeRect.startPoint.y-std::abs(largeRect.width)+1, std::abs(largeRect.endPoint.x - largeRect.startPoint.x)-2, largeRect.width-2 ));
		cv::imwrite("corroptedImage.jpg", largeRectImage);
	}

 	cv::imshow("image", res); cv::waitKey(0); 
	cv::imwrite("res.jpg", res);
	
	return res.clone();
}

#include <windows.h> // windows api for directory creation 
#include <iostream>
#include <vector>

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\stitching\stitcher.hpp>

#include "Stitcher.h"

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
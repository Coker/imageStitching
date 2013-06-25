
#include <iostream>

#include <opencv2\highgui\highgui.hpp>

#include "Stitcher.h"

int main()
{
	std::string path("resources/3.AVI");
	cv::Mat pano = BIL496::Stitcher::stitch(path);
	
	return 0;
}
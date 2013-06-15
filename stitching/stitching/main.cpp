
#include <iostream>

#include <opencv2\highgui\highgui.hpp>

#include "Stitcher.h"

int main()
{
	std::string path("1.AVI");

	cv::Mat pano = BIL496::Stitcher::stitch(path);

	imwrite("pano1.jpg", pano);

	return 0;
}


#include <iostream>

#include <opencv2\highgui\highgui.hpp>

#include "Stitcher.h"

int main()
{
	// std::string path("1.AVI");
	// cv::Mat pano = BIL496::Stitcher::stitch(path);
	// imwrite("pano1.jpg", pano);

	std::string imagePath("pano13.jpg");
	cv::Mat image = cv::imread(imagePath.data());

	// cv::imshow("image", image); cv::waitKey(0);
	BIL496::Stitcher::fixTheEdges(image);

	return 0;
}

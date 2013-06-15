
#if !defined(_STITCHER)
#define _STITCHER

#include <string>

#include <opencv\cv.h>
#include <opencv2\stitching\stitcher.hpp>

namespace BIL496 {
	class Stitcher {
	public:
		static const cv::Mat stitch(const std::string& const videoPath);

	private:
		static const cv::Mat fixTheEdges(const cv::Mat& const image);
	};
}

#endif
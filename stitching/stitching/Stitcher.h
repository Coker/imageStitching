
#if !defined(_STITCHER)
#define _STITCHER

#include <string>

#include <opencv\cv.h>
#include <opencv2\stitching\stitcher.hpp>

namespace BIL496 {

	// this values are heuristic value
	#define THERSHOLD_BLACK_POINT 10
	#define INVALID_POINT cv::Point(-1,-1)
	#define NEIGHBOOR_DISTANCE 10

	class Stitcher {
	public:
		static const cv::Mat stitch(const std::string& const videoPath);

	// private:
		static const cv::Mat fixTheEdges(const cv::Mat& const image);
	};

} // end of namespace BIL496

#endif
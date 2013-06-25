
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
	
	enum DIRECTION {
		BOTTOM_UP =0,
		TOP_DOWN
	};

	typedef struct {
		cv::Point startPoint;
		cv::Point endPoint;
		int width;
		short int direction;
		long int pixelSquare;
	} Rectangle;

	class Stitcher {
	public:
		static const cv::Mat stitch(const std::string& const videoPath);

	private:
		static const cv::Mat fixTheEdges(const cv::Mat& const image);
	};

} // end of namespace BIL496

#endif
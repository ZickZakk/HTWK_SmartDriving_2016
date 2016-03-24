#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#ifndef _CONNECTED_COMPONENT_FINDER_H_
#define _CONNECTED_COMPONENT_FINDER_H_

class ConnectedComponentFinder
{
public:
	vector<cv::RotatedRect> process(const cv::Mat& imSrc);

	void setThreshold(uchar value);
	uchar getThreshold();

	ConnectedComponentFinder();

private:
	uchar thresh;
	uchar max_thresh;
	cv::Mat threshold_output;

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	std::vector<cv::RotatedRect> boundRects;

	bool isSupportedImageType(const cv::Mat& imSrc);
};
#endif

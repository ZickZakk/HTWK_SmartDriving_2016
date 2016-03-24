#include "../htwk_distance_sensor_merger/stdafx.h"
#include "ConnectedComponentFinder.h"

ConnectedComponentFinder::ConnectedComponentFinder()
{
	thresh = 0;
	max_thresh = 255;
}


vector<cv::RotatedRect> ConnectedComponentFinder::process(const cv::Mat &imSrc)
{
	if (!isSupportedImageType(imSrc))
	{
		throw "ConnectedComponentFinder - Invalid Image Type";
	}

	try
	{
		threshold_output.create(imSrc.size(), imSrc.type());
		threshold(imSrc, threshold_output, thresh, max_thresh, cv::THRESH_BINARY);

		contours.clear();
		hierarchy.clear();
		findContours(threshold_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

		/// Approximate contours to polygons + get bounding rects and circles
		boundRects.clear();
		std::vector<cv::RotatedRect> minRect(contours.size());
		for (int i = 0; i < (int) contours.size(); i++)
		{
			minRect[i] = minAreaRect(cv::Mat(contours[i]));
			boundRects = minRect;
		}
	}
	catch (cv::Exception &e)
	{
		const char *err_msg = e.what();
		throw err_msg;
	}

	return boundRects;
}


void ConnectedComponentFinder::setThreshold(uchar value)
{
	thresh = value;
}


uchar ConnectedComponentFinder::getThreshold()
{
	return thresh;
}


bool ConnectedComponentFinder::isSupportedImageType(const cv::Mat &imSrc)
{
	return (imSrc.depth() == CV_8U || imSrc.depth() == CV_16S || imSrc.depth() == CV_32F);
}

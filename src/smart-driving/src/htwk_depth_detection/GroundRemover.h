#include "stdafx.h"
#include <vector>
#include "opencv2/opencv.hpp"

#ifndef _GROUNDREMOVER_H_
#define _GROUNDREMOVER_H_


class GroundRemover
{
public:
	GroundRemover();
	~GroundRemover();

	cv::Mat& process(const cv::Mat& imSrc);
	void setBackground(const cv::Mat& imSrc);

	int getVerLineMedianSize();

private:
	cv::Mat imRes;

	std::vector<unsigned short> vertLineMedian;
	int threshold;

	bool isValidSize(const cv::Mat &imSrc);
	bool isValidType(const cv::Mat& imSrc);
};

#endif /* _GROUNDREMOVER_H_ */

#include "opencv2/opencv.hpp"

#ifndef _NOISEREMOVER_H_
#define _NOISEREMOVER_H_

class NoiseRemover
{
public:
	NoiseRemover();
	~NoiseRemover();

	cv::Mat& process(const cv::Mat& image);

	void setTrimLevel(tUInt8 n);
	tUInt8 getTrimLevel() const;

	cv::Mat getMask() const;

private:
	tUInt8 trimLevel;

	cv::Mat mask;
	cv::Mat imRes;
};
#endif /* _NOISEREMOVER_H_ */

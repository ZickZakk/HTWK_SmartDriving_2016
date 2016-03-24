#include "stdafx.h"
#include "NoiseRemover.h"

NoiseRemover::NoiseRemover() : trimLevel(2)
{
    mask = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
}


NoiseRemover::~NoiseRemover()
{ }

cv::Mat NoiseRemover::getMask() const
{
    return mask;
}


void NoiseRemover::setTrimLevel(tUInt8 n)
{
    trimLevel = n;
}


tUInt8 NoiseRemover::getTrimLevel() const
{
    return trimLevel;
}


cv::Mat &NoiseRemover::process(const cv::Mat &image)
{
    imRes.create(image.rows, image.cols, image.type());

    cv::dilate(image, imRes, mask);
    cv::erode(imRes, imRes, mask);

    for (int i = trimLevel; i > 0; --i)
    {
        cv::dilate(imRes, imRes, mask);
        cv::dilate(imRes, imRes, mask);
        cv::erode(imRes, imRes, mask);
    }

    return imRes;
}

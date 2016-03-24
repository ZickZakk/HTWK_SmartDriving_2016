#include "GroundRemover.h"


GroundRemover::GroundRemover() : threshold(7000)
{ }


GroundRemover::~GroundRemover() { }


void GroundRemover::setBackground(const cv::Mat &imgSrc)
{
    int rows = imgSrc.rows;
    int cols = imgSrc.cols;

    vertLineMedian.clear();
    vertLineMedian.resize(rows);

    for (int r=0; r<rows; r++)
    {
        std::vector<ushort> line;
        for (int c=0; c<cols; c++)
        {
            ushort intensity = imgSrc.at<ushort>(r, c);
            if (0 != intensity)
            {
                line.push_back(intensity);
            }
        }
        std::sort(line.begin(), line.end());
        int median = (line.size() > 0) ? line.size()/2 : 0;
        if (median > 0)
        {
            vertLineMedian.at(r) = line.at(median);
        }
        else
        {
            vertLineMedian.at(r) = 0;
        }
    }
}


cv::Mat& GroundRemover::process(const cv::Mat &imSrc)
{
    int nrow = imSrc.rows;
    int ncol = imSrc.cols;

    if (!isValidSize(imSrc))
    {
        throw "GroundRemover - invalid image size";
    }

    if (!isValidType(imSrc))
    {
        throw "GoundRemover - invalid type";
    }


    imRes.create(imSrc.rows, imSrc.cols, imSrc.type());

    for (int r=0; r<nrow; r++)
    {
        for (int c=0; c<ncol; c++)
        {
            ushort intensity = imSrc.at<ushort>(r,c);

            // if current intensity lower than threshold value
            // remove current intensity
            if ((vertLineMedian.at(r) - intensity) < threshold)
            {
                imRes.at<ushort>(r,c) = 0;
            }
            else
            {
                imRes.at<ushort>(r,c) = intensity;
            }
        }
    }

    return imRes;
}

bool GroundRemover::isValidSize(const cv::Mat &imSrc)
{
    return imSrc.rows == (int)vertLineMedian.size();
}

bool GroundRemover::isValidType(const cv::Mat& imSrc)
{
    return imSrc.type() == CV_16U;
}

int GroundRemover::getVerLineMedianSize()
{
    return (int)vertLineMedian.size();
}

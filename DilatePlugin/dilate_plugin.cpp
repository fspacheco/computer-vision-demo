#include "dilate_plugin.h"

QString DilatePlugin::name()
{
    return "Dilate";
}

void DilatePlugin::edit(const cv::Mat &input, cv::Mat &output)
{
    dilate(input, output, cv::Mat());
}

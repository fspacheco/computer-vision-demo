#include "invert_plugin.h"

QString InvertPlugin::name()
{
    return "Invert";
}

void InvertPlugin::edit(const cv::Mat &input, cv::Mat &output)
{
    bitwise_not(input, output, cv::Mat());
}

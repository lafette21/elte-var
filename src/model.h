#ifndef MODEL_H
#define MODEL_H

#include "types.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <map>

namespace var {

class model {
public:
    model(const std::string& path) {
        _pitch = cv::imread(path);
        _pitchSave = _pitch.clone();

        cv::cvtColor(_pitch, _pitch, cv::COLOR_BGR2RGBA);
    }

    std::map<int, cv::Point2d>& imagePoints() { return _imagePoints; }
    std::map<int, cv::Point2d>& pitchPoints() { return _pitchPoints; }
    std::string& imagePath() { return _imagePath; }
    cv::Mat& image() { return _image; }
    cv::Mat& pitch() { return _pitch; }
    vec2& pitchSize() { return _pitchSize; }

    void load() {
        _image = cv::imread(_imagePath);
        _imageSave = _image.clone();

        cv::cvtColor(_image, _image, cv::COLOR_BGR2RGBA);
    }

private:
    std::map<int, cv::Point2d> _imagePoints;
    std::map<int, cv::Point2d> _pitchPoints;
    std::string _imagePath;
    cv::Mat _image, _pitch, _imageSave, _pitchSave;
    vec2 _pitchSize = {};
};

} // namespace var

#endif // MODEL_H

#ifndef MODEL_H
#define MODEL_H

#include "logging.h"
#include "types.h"

#include <opencv2/calib3d.hpp>
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

    std::map<int, cv::Point2f>& imagePoints() { return _imagePoints; }
    std::map<int, cv::Point2f>& pitchPoints() { return _pitchPoints; }
    std::string& imagePath() { return _imagePath; }
    cv::Mat& image() { return _image; }
    cv::Mat& pitch() { return _pitch; }
    vec2& attackerPos() { return _attackerPos; }
    vec2& defenderPos() { return _defenderPos; }

    void load() {
        _image = cv::imread(_imagePath);
        _imageSave = _image.clone();

        cv::cvtColor(_image, _image, cv::COLOR_BGR2RGBA);
    }

    void save(const std::string& path) {
        try {
            auto [image, _] = generate();

            cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
            cv::imwrite(path, image);
        } catch (const std::exception& ex) {
            logging::error("{}", ex.what());
        }
    }

    std::pair<cv::Mat, cv::Mat> generate() {
        auto image = _image.clone();
        auto pitch = _pitch.clone();

        if (image.empty()) {
            throw std::runtime_error("You should load an image first!");
        }

        if (_imagePoints.size() != _pitchPoints.size()) {
            throw std::runtime_error("You should select the same number of points in both images!");
        }

        if (_imagePoints.size() < 4) {
            throw std::runtime_error("You should select at least four points in both images!");
        }

        if (_attackerPos == vec2{ -1, -1 } or _defenderPos == vec2{ -1, -1 }) {
            throw std::runtime_error("You should select the attacker and defender players coordinate!");
        }

        std::vector<cv::Point2f> srcPoints, dstPoints;

        for (auto itIm = _imagePoints.cbegin(), endIm = _imagePoints.cend(),
            itPi = _pitchPoints.cbegin(), endPi = _pitchPoints.cend();
            itIm != endIm && itPi != endPi; ++itIm, ++itPi
        ) {
            srcPoints.emplace_back(itIm->second);
            dstPoints.emplace_back(itPi->second);
        }

        cv::Mat H = cv::findHomography(srcPoints, dstPoints);

        std::vector<cv::Point2f> imagePlayerPoints = { _attackerPos, _defenderPos };
        std::vector<cv::Point2f> pitchPlayerPoints;

        cv::perspectiveTransform(imagePlayerPoints, pitchPlayerPoints, H);

        std::vector<cv::Point2f> pitchLinePoints = {};

        for (const auto& point : pitchPlayerPoints) {
            const cv::Point2f p1 = { point.x, 0 };
            const cv::Point2f p2 = { point.x, static_cast<float>(pitch.rows) };
            pitchLinePoints.emplace_back(p1);
            pitchLinePoints.emplace_back(p2);
            cv::line(pitch, p1, p2, cv::Scalar(100, 100, 100, 255), 2);
        }

        cv::circle(pitch, pitchPlayerPoints[0], 3, cv::Scalar(0, 0, 255, 255), 2);
        cv::circle(pitch, pitchPlayerPoints[1], 3, cv::Scalar(255, 0, 0, 255), 2);

        std::vector<cv::Point2f> imageLinePoints;

        cv::perspectiveTransform(pitchLinePoints, imageLinePoints, H.inv());

        for (std::size_t i = 0; i < imageLinePoints.size(); i += 2) {
            const auto& p1 = imageLinePoints[i];
            const auto& p2 = imageLinePoints[i + 1];

            cv::Scalar color = i < 1 ? cv::Scalar(255, 0, 0, 255) : cv::Scalar(0, 0, 255, 255);

            cv::line(image, p1, p2, color, 2);
        }

        return { image, pitch };
    }

    void reset() {
        if (not _image.empty()) {
            _attackerPos = { -1, -1 };
            _defenderPos = { -1, -1 };
            _image = _imageSave.clone();
            _imagePoints.clear();
            cv::cvtColor(_image, _image, cv::COLOR_BGR2RGBA);
        }

        _pitch = _pitchSave.clone();
        _pitchPoints.clear();
        cv::cvtColor(_pitch, _pitch, cv::COLOR_BGR2RGBA);
    }

    void draw() {
        if (not _image.empty()) {
            _image = _imageSave.clone();
            const int ratio = _image.cols / 1000 + 3;

            if (_attackerPos != vec2{ -1, -1 }) {
                cv::circle(_image, cv::Point2f(_attackerPos.x(), _attackerPos.y()), ratio, cv::Scalar(0, 0, 255), ratio - 1);
            }

            if (_defenderPos != vec2{ -1, -1 }) {
                cv::circle(_image, cv::Point2f(_defenderPos.x(), _defenderPos.y()), ratio, cv::Scalar(255, 0, 0), ratio - 1);
            }

            for (const auto& [_, value] : _imagePoints) {
                cv::circle(_image, value, ratio, cv::Scalar(153, 51, 102), ratio - 1);
            }

            cv::cvtColor(_image, _image, cv::COLOR_BGR2RGBA);
        }

        _pitch = _pitchSave.clone();
        const int ratio = _pitch.cols / 1000 + 3;

        for (const auto& [_, value] : _pitchPoints) {
            cv::circle(_pitch, value, ratio, cv::Scalar(153, 51, 102), ratio - 1);
        }

        cv::cvtColor(_pitch, _pitch, cv::COLOR_BGR2RGBA);
    }

private:
    std::map<int, cv::Point2f> _imagePoints;
    std::map<int, cv::Point2f> _pitchPoints;
    std::string _imagePath;
    cv::Mat _image, _pitch, _imageSave, _pitchSave;
    vec2 _attackerPos = { -1, -1 };
    vec2 _defenderPos = { -1, -1 };
};

} // namespace var

#endif // MODEL_H

#ifndef MODEL_H
#define MODEL_H

#include "types.h"
#include "utils.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <spdlog/spdlog.h>

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
    vec2& attackerPos() { return _attackerPos; }
    vec2& defenderPos() { return _defenderPos; }

    void load() {
        _image = cv::imread(_imagePath);
        _imageSave = _image.clone();

        cv::cvtColor(_image, _image, cv::COLOR_BGR2RGBA);
    }

    void save(const std::string& path) {
        try {
            const auto [image, _] = generate();

            cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
            cv::imwrite(path, image);
        } catch (const std::exception& ex) {
            spdlog::error("{}", ex.what());
        }
    }

    std::pair<cv::Mat, cv::Mat> generate() {
        PointPairs pointPairs;
        auto image = _image.clone();
        auto pitch = _pitch.clone();

        if (_imagePoints.size() != _pitchPoints.size()) {
            throw std::runtime_error("You should select the same number of points in both images!");
        }

        if (_imagePoints.size() < 4) {
            throw std::runtime_error("You should select at least four points in both images!");
        }

        if (_attackerPos == vec2{ -1, -1 } or _defenderPos == vec2{ -1, -1 }) {
            throw std::runtime_error("You should select the attacker and defender players coordinate!");
        }

        for (auto itIm = _imagePoints.cbegin(), endIm = _imagePoints.cend(),
            itPi = _pitchPoints.cbegin(), endPi = _pitchPoints.cend();
            itIm != endIm && itPi != endPi; ++itIm, ++itPi
        ) {
            pointPairs.push_back(std::make_pair<cv::Point2f, cv::Point2f>(itIm->second, itPi->second));
        }

        cv::Mat H = calcHomography(pointPairs);

        std::vector<cv::Point2f> imagePlayerPoints = {{ _attackerPos.x, _attackerPos.y }, { _defenderPos.x, _defenderPos.y }};
        std::vector<cv::Point2f> pitchPlayerPoints;

        cv::perspectiveTransform(imagePlayerPoints, pitchPlayerPoints, H);

        std::vector<cv::Point2f> pitchLinePoints = {};

        for (const auto& point : pitchPlayerPoints) {
            pitchLinePoints.emplace_back(point.x, 0);
            pitchLinePoints.emplace_back(point.x, pitch.rows);
            cv::line(pitch, cv::Point2f(point.x, 0), cv::Point2f(point.x, pitch.rows), cv::Scalar(100, 100, 100, 255), 2);
        }

        cv::circle(pitch, cv::Point2f(pitchPlayerPoints[0].x, pitchPlayerPoints[0].y), 3, cv::Scalar(255, 0, 0, 255), 2);
        cv::circle(pitch, cv::Point2f(pitchPlayerPoints[1].x, pitchPlayerPoints[1].y), 3, cv::Scalar(0, 0, 255, 255), 2);

        std::vector<cv::Point2f> imageLinePoints;

        cv::perspectiveTransform(pitchLinePoints, imageLinePoints, H.inv());

        for (std::size_t i = 0; i < imageLinePoints.size(); i += 2) {
            const auto& p1 = imageLinePoints[i];
            const auto& p2 = imageLinePoints[i + 1];

            cv::Scalar color = i < 1 ? cv::Scalar(255, 0, 0, 255) : cv::Scalar(0, 0, 255, 255);

            cv::line(image, cv::Point2f(p1.x, p1.y), cv::Point2f(p2.x, p2.y), color, 2);
        }

        return { image, pitch };
    }

    void reset() {
        _imagePoints.clear();
        _pitchPoints.clear();
        _image = _imageSave.clone();
        _pitch = _pitchSave.clone();
        _attackerPos = { -1, -1 };
        _defenderPos = { -1, -1 };
    }

    void draw() {
        cv::cvtColor(_image, _image, cv::COLOR_RGBA2BGR);
        cv::cvtColor(_pitch, _pitch, cv::COLOR_RGBA2BGR);

        if (_attackerPos != vec2{ -1, -1 }) {
            cv::circle(_image, cv::Point2f(_attackerPos.x, _attackerPos.y), 3, cv::Scalar(0, 0, 255), 2);
        }

        if (_defenderPos != vec2{ -1, -1 }) {
            cv::circle(_image, cv::Point2f(_defenderPos.x, _defenderPos.y), 3, cv::Scalar(255, 0, 0), 2);
        }

        for (const auto& [_, value] : _imagePoints) {
            cv::circle(_image, cv::Point2d(value.x, value.y), 3, cv::Scalar(153, 51, 102), 2);
        }

        for (const auto& [_, value] : _pitchPoints) {
            cv::circle(_pitch, cv::Point2d(value.x, value.y), 3, cv::Scalar(153, 51, 102), 2);
        }

        cv::cvtColor(_image, _image, cv::COLOR_BGR2RGBA);
        cv::cvtColor(_pitch, _pitch, cv::COLOR_BGR2RGBA);
    }

private:
    std::map<int, cv::Point2d> _imagePoints;
    std::map<int, cv::Point2d> _pitchPoints;
    std::string _imagePath;
    cv::Mat _image, _pitch, _imageSave, _pitchSave;
    vec2 _pitchSize = {};
    vec2 _attackerPos = { -1, -1 };
    vec2 _defenderPos = { -1, -1 };
};

} // namespace var

#endif // MODEL_H

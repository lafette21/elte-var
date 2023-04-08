#ifndef UTILS_H
#define UTILS_H

#include <opencv2/core.hpp>

#include <vector>

namespace var {

using PointPairs = std::vector<std::pair<cv::Point2f, cv::Point2f>>;

struct NormalizedData {
    cv::Mat T1;
    cv::Mat T2;
    PointPairs pointPairs;
};

NormalizedData normalizeData(const PointPairs& pointPairs) {
    std::size_t ptsNum = pointPairs.size();

    // calculate means (they will be the center of coordinate systems)
    float mean1x = 0.0, mean1y = 0.0, mean2x = 0.0, mean2y = 0.0;

    for (std::size_t i = 0; i < ptsNum; ++i) {
        auto& pp = pointPairs[i];
        mean1x += pp.first.x;
        mean1y += pp.first.y;
        mean2x += pp.second.x;
        mean2y += pp.second.y;
    }

    mean1x /= ptsNum;
    mean1y /= ptsNum;
    mean2x /= ptsNum;
    mean2y /= ptsNum;

    float spread1x = 0.0, spread1y = 0.0, spread2x = 0.0, spread2y = 0.0;

    for (std::size_t i = 0; i < ptsNum; ++i) {
        auto& pp = pointPairs[i];
        spread1x += (pp.first.x - mean1x) * (pp.first.x - mean1x);
        spread1y += (pp.first.y - mean1y) * (pp.first.y - mean1y);
        spread2x += (pp.second.x - mean2x) * (pp.second.x - mean2x);
        spread2y += (pp.second.y - mean2y) * (pp.second.y - mean2y);
    }

    spread1x /= ptsNum;
    spread1y /= ptsNum;
    spread2x /= ptsNum;
    spread2y /= ptsNum;

    cv::Mat offs1 = cv::Mat::eye(3, 3, CV_32F);
    cv::Mat offs2 = cv::Mat::eye(3, 3, CV_32F);
    cv::Mat scale1 = cv::Mat::eye(3, 3, CV_32F);
    cv::Mat scale2 = cv::Mat::eye(3, 3, CV_32F);

    offs1.at<float>(0, 2) = -mean1x;
    offs1.at<float>(1, 2) = -mean1y;

    offs2.at<float>(0, 2) = -mean2x;
    offs2.at<float>(1, 2) = -mean2y;

    const float sqrt2 = static_cast<float>(std::sqrt(2));

    scale1.at<float>(0, 0) = sqrt2 / std::sqrt(spread1x);
    scale1.at<float>(1, 1) = sqrt2 / std::sqrt(spread1y);

    scale2.at<float>(0, 0) = sqrt2 / std::sqrt(spread2x);
    scale2.at<float>(1, 1) = sqrt2 / std::sqrt(spread2y);

    NormalizedData result;
    result.T1 = scale1 * offs1;
    result.T2 = scale2 * offs2;

    for (std::size_t i = 0; i < ptsNum; ++i) {
        cv::Point2f p1;
        cv::Point2f p2;

        auto& pp = pointPairs[i];
        p1.x = sqrt2 * (pp.first.x - mean1x) / std::sqrt(spread1x);
        p1.y = sqrt2 * (pp.first.y - mean1y) / std::sqrt(spread1y);

        p2.x = sqrt2 * (pp.second.x - mean2x) / std::sqrt(spread2x);
        p2.y = sqrt2 * (pp.second.y - mean2y) / std::sqrt(spread2y);

        result.pointPairs.emplace_back(p1, p2);
    }

    return result;
}

cv::Mat calcHomography(PointPairs& pointPairs) {
    NormalizedData normalizedData = normalizeData(pointPairs);
    pointPairs = normalizedData.pointPairs;

    const std::size_t ptsNum = pointPairs.size();
    cv::Mat A(2 * static_cast<int>(ptsNum), 9, CV_32F);

    for (std::size_t i = 0; i < ptsNum; ++i) {
        float u1 = pointPairs[i].first.x;
        float v1 = pointPairs[i].first.y;

        float u2 = pointPairs[i].second.x;
        float v2 = pointPairs[i].second.y;

        int ii = static_cast<int>(i);

        A.at<float>(2 * ii, 0) = u1;
        A.at<float>(2 * ii, 1) = v1;
        A.at<float>(2 * ii, 2) = 1.0f;
        A.at<float>(2 * ii, 3) = 0.0f;
        A.at<float>(2 * ii, 4) = 0.0f;
        A.at<float>(2 * ii, 5) = 0.0f;
        A.at<float>(2 * ii, 6) = -u2 * u1;
        A.at<float>(2 * ii, 7) = -u2 * v1;
        A.at<float>(2 * ii, 8) = -u2;

        A.at<float>(2 * ii + 1, 0) = 0.0f;
        A.at<float>(2 * ii + 1, 1) = 0.0f;
        A.at<float>(2 * ii + 1, 2) = 0.0f;
        A.at<float>(2 * ii + 1, 3) = u1;
        A.at<float>(2 * ii + 1, 4) = v1;
        A.at<float>(2 * ii + 1, 5) = 1.0f;
        A.at<float>(2 * ii + 1, 6) = -v2 * u1;
        A.at<float>(2 * ii + 1, 7) = -v2 * v1;
        A.at<float>(2 * ii + 1, 8) = -v2;
    }

    cv::Mat eVecs(9, 9, CV_32F), eVals(9, 9, CV_32F);
    // std::cout << A << std::endl;
    cv::eigen(A.t() * A, eVals, eVecs);

    // std::cout << eVals << std::endl;
    // std::cout << eVecs << std::endl;

    cv::Mat H(3, 3, CV_32F);
    for (int i = 0; i < 9; ++i) {
        H.at<float>(i / 3, i % 3) = eVecs.at<float>(8, i);
    }

    // std::cout << H << std::endl;

    // normalize
    H = H * (1.0 / static_cast<double>(H.at<float>(2, 2)));
    // std::cout << H << std::endl;

    return normalizedData.T2.inv() * H * normalizedData.T1;
}

} // namespace var

#endif // UTILS_H

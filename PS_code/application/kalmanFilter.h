#pragma once
#include "defines.h"

#include <opencv/cv.h>

#include <opencv2/tracking.hpp>
#include <opencv2/tracking/kalman_filters.hpp>


class kalmanFilter
{

public:
	kalmanFilter(Point_t pt, track_t deltaTime = 0.2, track_t accelNoiseMag = 0.5);
	~kalmanFilter();

	Point_t GetPointPrediction(Point_t pt);
	Point_t Update(Point_t pt, bool dataCorrect);

	

private:

	cv::Ptr<cv::tracking::UnscentedKalmanFilter> m_unscentedKalman;

	std::deque<Point_t> m_initialPoints;
	static const size_t MIN_INIT_VALS = 4;

	Point_t m_lastPointResult;


	bool m_initialized;
	track_t m_deltaTime;
	track_t m_deltaTimeMin;
	track_t m_deltaTimeMax;
	track_t m_lastDist;
	track_t m_deltaStep;
	static const int m_deltaStepsCount = 20;
	track_t m_accelNoiseMag;
	void createUnscented(Point_t xy0, Point_t xyv0);
};
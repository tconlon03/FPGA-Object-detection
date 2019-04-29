#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <memory>
#include <array>

#include "KalmanFilter.h"

// --------------------------------------------------------------------------
///
/// \brief The TrajectoryPoint struct
///
struct TrajectoryPoint
{
	///
	/// \brief TrajectoryPoint
	///
	TrajectoryPoint()
		: m_hasRaw(false)
	{
	}

	///
	/// \brief TrajectoryPoint
	/// \param prediction
	///
	TrajectoryPoint(const Point_t& prediction)
		:
		m_hasRaw(false),
		m_prediction(prediction)
	{
	}

	///
	/// \brief TrajectoryPoint
	/// \param prediction
	/// \param raw
	///
	TrajectoryPoint(const Point_t& prediction, const Point_t& raw)
		:
		m_hasRaw(true),
		m_prediction(prediction),
		m_raw(raw)
	{
	}

	bool m_hasRaw;
	Point_t m_prediction;
	Point_t m_raw;
};

// --------------------------------------------------------------------------
///
/// Trace class holds previous positions of the object
///
class Trace
{
public:
	///
	/// \brief operator []
	/// \param i
	/// \return
	///
	const Point_t& operator[](size_t i) const
	{
		return m_trace[i].m_prediction;
	}

	///
	/// \brief operator []
	/// \param i
	/// \return
	///
	Point_t& operator[](size_t i)
	{
		return m_trace[i].m_prediction;
	}

	///
	/// \brief at
	/// \param i
	/// \return
	///
	const TrajectoryPoint& at(size_t i) const
	{
		return m_trace[i];
	}

	///
	/// \brief size
	/// \return
	///
	size_t size() const
	{
		return m_trace.size();
	}

	///
	/// \brief push_back
	/// \param prediction
	///
	void push_back(const Point_t& prediction)
	{
		m_trace.push_back(TrajectoryPoint(prediction));
	}
	void push_back(const Point_t& prediction, const Point_t& raw)
	{
		m_trace.push_back(TrajectoryPoint(prediction, raw));
	}

	///
	/// \brief pop_front
	/// \param count
	///
	void pop_front(size_t count)
	{
		if (count < size())
		{
			m_trace.erase(m_trace.begin(), m_trace.begin() + count);
		}
		else
		{
			m_trace.clear();
		}
	}

	///
	/// \brief GetRawCount
	/// \param lastPeriod
	/// \return
	///
	size_t GetRawCount(size_t lastPeriod) const
	{
		size_t res = 0;

		size_t i = 0;
		if (lastPeriod < m_trace.size())
		{
			i = m_trace.size() - lastPeriod;
		}
		for (; i < m_trace.size(); ++i)
		{
			if (m_trace[i].m_hasRaw)
			{
				++res;
			}
		}

		return res;
	}

private:
	std::deque<TrajectoryPoint> m_trace;
};

// --------------------------------------------------------------------------
///
/// \brief The track class
///
class track
{
public:
	track(cv::Point point,
		track_t deltaTime,
		track_t accelNoiseMag,
		size_t trackID);
	//probably add built in tracker

	///
	/// \brief CalcDist
	/// Euclidean distance in pixels between objects centres on two N and N+1 frames
	/// \param pt
	/// \return
	///
	track_t CalcDist(const Point_t& pt) const;



	void Update(const Point_t& pnt, bool dataCorrect, size_t max_trace_length, cv::UMat prevFrame, cv::UMat currFrame, int trajLen);

	Trace m_trace;
	size_t m_trackID;
	size_t m_skippedFrames;
	Point_t m_averagePoint;   ///< Average point after LocalTracking
	Point_t m_lastPoint;


private:
	Point_t m_predictionPoint;
	kalmanFilter* m_kalman;
	bool m_outOfTheFrame;
	 
	cv::Ptr<cv::Tracker> m_tracker;
	void CreateExternalTracker();

	void PointUpdate(const Point_t& pt, bool dataCorrect, const cv::Size& frameSize);

	bool m_isStatic = false;
	int m_staticFrames = 0;
	cv::UMat m_staticFrame;
};

typedef std::vector<std::unique_ptr<track>> tracks_t;
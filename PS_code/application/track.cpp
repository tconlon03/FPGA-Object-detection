#include "track.h"

///
/// \brief track
/// \param pt
/// \param region
/// \param deltaTime
/// \param accelNoiseMag
/// \param trackID
/// \param filterObjectSize
/// \param externalTrackerForLost
///
track::track(
	cv::Point point,
	track_t deltaTime,
	track_t accelNoiseMag,
	size_t trackID
)
	:
	m_trackID(trackID),
	m_skippedFrames(0),
	m_predictionPoint(point),
	m_outOfTheFrame(false)
{
	//unscented kalman filter
	m_kalman = new kalmanFilter(m_predictionPoint, deltaTime, accelNoiseMag);
	m_trace.push_back(m_predictionPoint, m_predictionPoint);
}

///
/// \brief CalcDist
/// \param pt
/// \return
///
track_t track::CalcDist(const Point_t& pt) const
{
	Point_t diff = m_predictionPoint - pt;
	return sqrtf(sqr(diff.x) + sqr(diff.y));
}


///
/// \brief Update
/// \param pt
/// \param region
/// \param dataCorrect
/// \param max_trace_length
/// \param prevFrame
/// \param currFrame
///
void track::Update(
	const Point_t& pnt,
	bool dataCorrect,
	size_t max_trace_length,
	cv::UMat prevFrame,
	cv::UMat currFrame,
	int trajLen
)
{
	cv::Point pt(pnt);
	DBOUT("Current Point : " << pt.x << " , " << pt.y);
	PointUpdate(pt, dataCorrect, currFrame.size());
	DBOUT("New Point : " << m_predictionPoint.x << " , " << m_predictionPoint.y);

	if (dataCorrect)
	{
		m_lastPoint = pt;
		m_trace.push_back(m_predictionPoint, pt);

	}
	else
	{
		m_trace.push_back(m_predictionPoint);
	}

	if (m_trace.size() > max_trace_length)
	{
		m_trace.pop_front(m_trace.size() - max_trace_length);
	}
}

///
/// \brief CreateExternalTracker
///
void track::CreateExternalTracker()
{
	if (!m_tracker || m_tracker.empty())
	{
		cv::TrackerKCF::Params params;
		params.compressed_size = 1;
		params.desc_pca = cv::TrackerKCF::GRAY;
		params.desc_npca = cv::TrackerKCF::GRAY;
		params.resize = true;
		params.detect_thresh = 0.5f;
		#if (((CV_VERSION_MAJOR == 3) && (CV_VERSION_MINOR >= 3)) || (CV_VERSION_MAJOR > 3))
			m_tracker = cv::TrackerKCF::create(params);
		#else
			m_tracker = cv::TrackerKCF::createTracker(params);
		#endif
	}
}

///
/// \brief PointUpdate
/// \param pt
/// \param dataCorrect
///
void track::PointUpdate(
	const Point_t& pt,
	bool dataCorrect,
	const cv::Size& frameSize
)
{
	m_kalman->GetPointPrediction(pt);

	if (m_averagePoint.x + m_averagePoint.y > 0)
	{
		if (dataCorrect)
		{
			m_predictionPoint = m_kalman->Update((pt + m_averagePoint) / 2, dataCorrect);
		}
		else
		{
			m_predictionPoint = m_kalman->Update((m_predictionPoint + m_averagePoint) / 2, true);
		}
	}
	else
	{
		m_predictionPoint = m_kalman->Update(pt, dataCorrect);
	}

	auto Clamp = [](track_t& v, int hi) -> bool
	{
		if (v < 0)
		{
			v = 0;
			return true;
		}
		else if (hi && v > hi - 1)
		{
			v = static_cast<track_t>(hi - 1);
			return true;
		}
		return false;
	};
	m_outOfTheFrame = false;
	m_outOfTheFrame |= Clamp(m_predictionPoint.x, frameSize.width);
	m_outOfTheFrame |= Clamp(m_predictionPoint.y, frameSize.height);
}
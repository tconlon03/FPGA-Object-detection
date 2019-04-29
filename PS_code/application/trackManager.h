#pragma once
#include "track.h"

// ----------------------------------------------------------------------

///
/// \brief The TrackerSettings struct
///
struct TrackerSettings
{
	///
	/// \brief m_dt
	/// Time step for Kalman
	///
	track_t m_dt = 1.0f;

	///
	/// \brief m_accelNoiseMag
	/// Noise magnitude for Kalman
	///
	track_t m_accelNoiseMag = 0.1f;

	///
	/// \brief m_distThres
	/// Distance threshold for Assignment problem for tracking::DistCenters or for tracking::DistRects (for tracking::DistJaccard it need from 0 to 1)
	///
	track_t m_distThres = 50;

	///
	/// \brief m_maximumAllowedSkippedFrames
	/// If the object don't assignment more than this frames then it will be removed
	///
	size_t m_maximumAllowedSkippedFrames = 25;

	///
	/// \brief m_maxTraceLength
	/// The maximum trajectory length
	///
	size_t m_maxTraceLength = 50;

	///
	/// \brief m_useAbandonedDetection
	/// Detection abandoned objects
	///
	bool m_useAbandonedDetection = false;

	///
	/// \brief m_minStaticTime
	/// After this time (in seconds) the object is considered abandoned
	///
	int m_minStaticTime = 5;
	///
	/// \brief m_maxStaticTime
	/// After this time (in seconds) the abandoned object will be removed
	///
	int m_maxStaticTime = 25;
};

class trackManager
{
public:
	trackManager(const TrackerSettings& settings);
	~trackManager(void);

	tracks_t tracks;
	std::vector<Point_t> shaft;
	float danger_radius;
	void Update(const regions_t& regions, cv::UMat grayFrame, float fps);

private:

	TrackerSettings m_settings;
	size_t N;
	size_t M;
	size_t m_nextTrackID;
	
	cv::UMat m_prevFrame;
	std::vector<int> distMatrix;
	std::vector<long> assignment;

	void UpdateHungrian(const regions_t& regions, cv::UMat grayFrame, float fps);
};


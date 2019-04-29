#include "logger.h"
#include "defines.h"
#include <opencv2/video/tracking.hpp>
#include <opencv2/tracking/kalman_filters.hpp>
#include <vector>

using namespace std;
using namespace cv::tracking;

class AcceleratedModel : public cv::tracking::UkfSystemModel
{
public:
	AcceleratedModel(track_t deltaTime, bool rectModel)
		:
		cv::tracking::UkfSystemModel(),
		m_deltaTime(deltaTime),
		m_rectModel(rectModel)
	{

	}

	void stateConversionFunction(const cv::Mat& x_k, const cv::Mat& u_k, const cv::Mat& v_k, cv::Mat& x_kplus1)
	{
		track_t x0 = x_k.at<track_t>(0, 0);
		track_t y0 = x_k.at<track_t>(1, 0);
		track_t vx0 = x_k.at<track_t>(2, 0);
		track_t vy0 = x_k.at<track_t>(3, 0);
		track_t ax0 = x_k.at<track_t>(4, 0);
		track_t ay0 = x_k.at<track_t>(5, 0);
		DBOUT("Current velocity x: " << vx0);
		DBOUT("Current velocity y: " << vy0);
		DBOUT("Current position x: " << x0);
		DBOUT("Current position y: " << y0);
		//cout << "Current acceleration x: " << ax0 << endl;
		//cout << "Current acceleration y: " << ay0 << endl;
		
		//use equations of motion to predict new position / displacement
		x_kplus1.at<track_t>(0, 0) = x0 + (vx0 * m_deltaTime + ax0 * sqr(m_deltaTime) / 2);
		x_kplus1.at<track_t>(1, 0) = y0 + (vy0 * m_deltaTime + ay0 * sqr(m_deltaTime) / 2);
		DBOUT("new position x: " << x_kplus1.at<track_t>(0, 0));
		DBOUT("new position y: " << x_kplus1.at<track_t>(1, 0));
		//new velocity
		x_kplus1.at<track_t>(2, 0) = vx0 + ax0 * m_deltaTime;
		x_kplus1.at<track_t>(3, 0) = vy0 + ay0 * m_deltaTime;
		//assume constant acceleration
		x_kplus1.at<track_t>(4, 0) = ax0;
		x_kplus1.at<track_t>(5, 0) = ay0;

		if (m_rectModel)
		{
			x_kplus1.at<track_t>(6, 0) = x_k.at<track_t>(6, 0);
			x_kplus1.at<track_t>(7, 0) = x_k.at<track_t>(7, 0);
		}

		if (v_k.size() == u_k.size())
		{
			x_kplus1 += v_k + u_k;
		}
		else
		{
			x_kplus1 += v_k;
		}
	}

	//correct
	void measurementFunction(const cv::Mat& x_k, const cv::Mat& n_k, cv::Mat& z_k)
	{
		track_t x0 = x_k.at<track_t>(0, 0);
		track_t y0 = x_k.at<track_t>(1, 0);
		track_t vx0 = x_k.at<track_t>(2, 0);
		track_t vy0 = x_k.at<track_t>(3, 0);
		track_t ax0 = x_k.at<track_t>(4, 0);
		track_t ay0 = x_k.at<track_t>(5, 0);

		DBOUT("Current position x correcting: " << x0);
		DBOUT("Current position y correcting: " << y0);

		z_k.at<track_t>(0, 0) = x0 + vx0 * m_deltaTime + ax0 * sqr(m_deltaTime) / 2 + n_k.at<track_t>(0, 0);
		z_k.at<track_t>(1, 0) = y0 + vy0 * m_deltaTime + ay0 * sqr(m_deltaTime) / 2 + n_k.at<track_t>(1, 0);

		if (m_rectModel)
		{
			z_k.at<track_t>(2, 0) = x_k.at<track_t>(6, 0);
			z_k.at<track_t>(3, 0) = x_k.at<track_t>(7, 0);
		}
	}

private:
	track_t m_deltaTime;
	bool m_rectModel;
};

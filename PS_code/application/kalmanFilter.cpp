#include "kalmanFilter.h"
#include "logger.h"

using namespace cv;

kalmanFilter::kalmanFilter(Point_t pt,
	track_t deltaTime, // time increment (lower values makes target more "massive")
	track_t accelNoiseMag
)
	:
	m_initialized(false),
	m_deltaTime(deltaTime),
	m_deltaTimeMin(deltaTime),
	m_deltaTimeMax(2 * deltaTime),
	m_lastDist(0),
	m_accelNoiseMag(accelNoiseMag)
{
	m_deltaStep = (m_deltaTimeMax - m_deltaTimeMin) / m_deltaStepsCount;

	m_initialPoints.push_back(pt);
	m_lastPointResult = pt;

}


kalmanFilter::~kalmanFilter()
{
}


class AcceleratedModel : public cv::tracking::UkfSystemModel
{
public:
	AcceleratedModel(track_t deltaTime)
		:
		cv::tracking::UkfSystemModel(),
		m_deltaTime(deltaTime)
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


		if (v_k.size() == u_k.size())
		{
			x_kplus1 += v_k + u_k;
		}
		else
		{
			x_kplus1 += v_k;
		}
	}

	void measurementFunction(const cv::Mat& x_k, const cv::Mat& n_k, cv::Mat& z_k)
	{
		track_t x0 = x_k.at<track_t>(0, 0);
		track_t y0 = x_k.at<track_t>(1, 0);
		track_t vx0 = x_k.at<track_t>(2, 0);
		track_t vy0 = x_k.at<track_t>(3, 0);
		track_t ax0 = x_k.at<track_t>(4, 0);
		track_t ay0 = x_k.at<track_t>(5, 0);

		//DBOUT("Current position x correcting: " << x0);
		//DBOUT("Current position y correcting: " << y0);

		z_k.at<track_t>(0, 0) = x0 + vx0 * m_deltaTime + ax0 * sqr(m_deltaTime) / 2 + n_k.at<track_t>(0, 0);
		z_k.at<track_t>(1, 0) = y0 + vy0 * m_deltaTime + ay0 * sqr(m_deltaTime) / 2 + n_k.at<track_t>(1, 0);

		//DBOUT("new position x correcting: " << z_k.at<track_t>(0, 0));
		//DBOUT("new position y correcting: " << z_k.at<track_t>(1, 0));
	}

private:
	track_t m_deltaTime;
};


void kalmanFilter::createUnscented(Point_t xy0, Point_t xyv0) {
	int MP = 2;
	int DP = 6;
	int CP = 0;

	cv::Mat processNoiseCov = cv::Mat::zeros(DP, DP, Mat_t(1));
	processNoiseCov.at<track_t>(0, 0) = 1e-2f;
	processNoiseCov.at<track_t>(1, 1) = 1e-2f;
	processNoiseCov.at<track_t>(2, 2) = 1e-1f;
	processNoiseCov.at<track_t>(3, 3) = 1e-1f;
	processNoiseCov.at<track_t>(4, 4) = 1e-1f;
	processNoiseCov.at<track_t>(5, 5) = 1e-1f;

	cv::Mat measurementNoiseCov = cv::Mat::zeros(MP, MP, Mat_t(1));
	measurementNoiseCov.at<track_t>(0, 0) = 1e-6f;
	measurementNoiseCov.at<track_t>(1, 1) = 1e-6f;

	cv::Mat initState(DP, 1, Mat_t(1));
	initState.at<track_t>(0, 0) = xy0.x;
	initState.at<track_t>(1, 0) = xy0.y;
	initState.at<track_t>(2, 0) = xyv0.x;
	initState.at<track_t>(3, 0) = xyv0.y;
	initState.at<track_t>(4, 0) = 0;
	initState.at<track_t>(5, 0) = 0;

	cv::Mat P = 1e-6 * cv::Mat::eye(DP, DP, Mat_t(1));

	m_deltaTime = 0.1f;
	Ptr<AcceleratedModel> model(new AcceleratedModel(m_deltaTime));
	cv::tracking::UnscentedKalmanFilterParams params(DP, MP, CP, 0, 0, model);
	params.dataType = Mat_t(1);
	params.stateInit = initState.clone();
	params.errorCovInit = P.clone();
	params.measurementNoiseCov = measurementNoiseCov.clone();
	params.processNoiseCov = processNoiseCov.clone();

	params.alpha = 1.0;
	params.beta = 2.0;
	params.k = -2.0;

	m_unscentedKalman = createUnscentedKalmanFilter(params);
	m_initialized = true;
}

Point_t kalmanFilter::GetPointPrediction(Point_t pt)
{
	if (m_initialized)
	{
		//m_lastPointResult = pt;
		cv::Mat prediction;

			prediction = m_unscentedKalman->predict();

		m_lastPointResult = Point_t(prediction.at<track_t>(0), prediction.at<track_t>(1));
	}
	return m_lastPointResult;
}

//---------------------------------------------------------------------------
Point_t kalmanFilter::Update(Point_t pt, bool dataCorrect)
{
	if (!m_initialized)
	{
		if (m_initialPoints.size() < MIN_INIT_VALS)
		{
			if (dataCorrect)
			{
				m_initialPoints.push_back(pt);
			}
		}
		if (m_initialPoints.size() == MIN_INIT_VALS)
		{
			track_t kx = 0;
			track_t bx = 0;
			track_t ky = 0;
			track_t by = 0;
			get_lin_regress_params(m_initialPoints, 0, MIN_INIT_VALS, kx, bx, ky, by);
			Point_t xy0(kx * (MIN_INIT_VALS - 1) + bx, ky * (MIN_INIT_VALS - 1) + by);
			Point_t xyv0(kx, ky);

			createUnscented(xy0, xyv0);
			m_lastDist = 0;
		}
	}

	if (m_initialized)
	{
		cv::Mat measurement(2, 1, Mat_t(1));
		if (!dataCorrect)
		{
			DBOUT("Updating predictions");
			measurement.at<track_t>(0) = m_lastPointResult.x;  //update using prediction
			measurement.at<track_t>(1) = m_lastPointResult.y;
		}
		else
		{
			DBOUT("Updating measurements");
			measurement.at<track_t>(0) = pt.x;  //update using measurements
			measurement.at<track_t>(1) = pt.y;
		}
		// Correction
		cv::Mat estimated;
		estimated = m_unscentedKalman->correct(measurement);

		m_lastPointResult.x = estimated.at<track_t>(0);   //update using measurements
		m_lastPointResult.y = estimated.at<track_t>(1);
	}
	else
	{         
		if (dataCorrect)
		{
			m_lastPointResult = pt;
		}
	}
	return m_lastPointResult;
}


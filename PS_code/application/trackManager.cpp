//#include <dlib/optimization/max_cost_assignment.h>
#include <dlib/optimization.h>

#include "logger.h"
#include "trackManager.h"

using namespace dlib;


trackManager::trackManager(const TrackerSettings& settings)
	:
	m_settings(settings),
	m_nextTrackID(0)
{
	shaft.push_back(Point_t(80,330));
	shaft.push_back(Point_t(50, 290));
	danger_radius = 80.00;
}+


trackManager::~trackManager()
{
}

matrix<int> create_matrix_for_min_cost(std::vector<int> &distMatrix, int no_tracks, int no_regions) {
	//distMatrix is a matrix of track_t distances from each detection to each tracked obj.
	//using max cost algorithm so need to invert values - /1
	//algorithm needs ints so multiply up 
	int max = (no_tracks > no_regions) ? no_tracks : no_regions;
	matrix<int> costMatrix(max, max);
	matrix<float> inverseMatrix(no_tracks, no_regions);
	float inverse;
	float min_inverse = 1.00f;
	for (int i = 0; i < costMatrix.nc(); i++) {
		for (int j = 0; j < costMatrix.nr(); j++) {
			costMatrix(i, j) = -1;
		}
	}
	for (int i = 0; i < no_tracks; i++) {
		for (int j = 0; j < no_regions; j++) {
			if (distMatrix[(i*no_regions) + j] > 0) {
				inverse = 1.00 / (distMatrix[(i*no_regions) + j]);
				DBOUT("dist matrix : " << distMatrix[(i*no_regions) + j]);
				DBOUT("inverse : " << inverse);
				inverseMatrix(i, j) = inverse;
			}
			else {
				inverse = 1.00 / 1e-6;
			}
			if (inverse < min_inverse) {
				min_inverse = inverse;
			}
		}
	}
	for (int i = 0; i < no_tracks; i++) {
		for (int j = 0; j < no_regions; j++) {
			DBOUT("cost matrix : " << costMatrix(i, j));
			float cost_f = inverseMatrix(i,j) / min_inverse;
			costMatrix(i, j) = (int)cost_f;
			DBOUT("cost matrix : " << costMatrix(i,j));
		}
	}
	return costMatrix;

}

void trackManager::Update(
	const regions_t& regions,
	cv::UMat grayFrame,  
	float fps
)
{
	UpdateHungrian(regions, grayFrame, fps);

	grayFrame.copyTo(m_prevFrame);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void trackManager::UpdateHungrian(
	const regions_t& regions,
	cv::UMat grayFrame,
	float fps
)
{
	N = tracks.size();	
	M = regions.size();	
	
	std::vector<CvMoments> mu(1);
	float max_velocity;

	dlib::matrix<int> costMatrix(N, M);
	distMatrix.clear();


	if (!tracks.empty())
	{
		const track_t maxPossibledistMatrix = grayFrame.cols * grayFrame.rows;
		track_t maxdistMatrix = 0;
		for (size_t i = 0; i < tracks.size(); i++) {
			//get velocity from track kalman filter
			for (size_t j = 0; j < regions.size(); j++) {
				mu[0] = moments(regions[j], false);
				Point_t pnt = Point_t(mu[0].m10 / mu[0].m00, mu[0].m01 / mu[0].m00);
				auto dist = tracks[i]->CalcDist(pnt);
				distMatrix.push_back(dist);
				if (dist > maxdistMatrix) {
					maxdistMatrix = dist;
				}
			}
		}

		// -----------------------------------
		// Solving assignment problem (tracks and predictions of Kalman filter)
		// -----------------------------------

		costMatrix = create_matrix_for_min_cost(distMatrix, N, M);
		for (long r = 0; r < costMatrix.nr(); ++r)
		{
			// loop over all the columns
			for (long c = 0; c < costMatrix.nc(); ++c)
			{
				DBOUT("VALUE : " << costMatrix(r, c));
			}
		}
		assignment = max_cost_assignment(costMatrix);
		
		//identify assignments to undetected regions
		for (size_t i = 0; i < static_cast<int>(assignment.size()); i++) {
			if (assignment[i] > regions.size() - 1)
				assignment[i] = -1;
		}

		// -----------------------------------
		// clean assignment from pairs with large distance
		// -----------------------------------
		for (size_t i = 0; i < static_cast<int>(tracks.size()); i++)
		{
			if (assignment[i] != -1)
			{
				if (distMatrix[i + assignment[i] * N] > m_settings.m_distThres)
				{
					assignment[i] = -1;
					tracks[i]->m_skippedFrames++;
				}
			}
			else
			{
				// If track have no assigned detect, then increment skipped frames counter.
				tracks[i]->m_skippedFrames++;  
			}
		}

		// -----------------------------------
		// If track didn't get detected for long time, remove it.
		// -----------------------------------
		for (int i = 0; i < static_cast<int>(tracks.size()); i++)
		{
			if (tracks[i]->m_skippedFrames > m_settings.m_maximumAllowedSkippedFrames)
			{
				tracks.erase(tracks.begin() + i);
				assignment.erase(assignment.begin() + i);
				i--;
			}
		}
	}

	// -----------------------------------
	// Search for unassigned detects and start new tracks for them.
	// -----------------------------------
	for (size_t i = 0; i < regions.size(); ++i)
	{	
		if (distance(assignment.begin(), find(assignment.begin(), assignment.end(), i)) > tracks.size() -1 
			|| assignment.size() == 0)
		{
			mu[0] = moments(regions[i], false);
			Point_t pnt = Point_t(mu[0].m10 / mu[0].m00, mu[0].m01 / mu[0].m00);
			tracks.push_back(std::make_unique<track>(pnt,
				m_settings.m_dt,
				m_settings.m_accelNoiseMag,
				m_nextTrackID++));
		}
	}

	// Update Kalman Filters state
	//DBOUT("UPDATING  HUNGARIAN " << assignment.size());


	for (size_t i = 0; i < assignment.size(); i++)
	{
		//DBOUT("UPDATING KALMAN 4");
		// If track updated less than one time, than filter state is not correct.
		if (assignment[i] != -1) // If we have assigned detect, then update using its coordinates,
		{
			//DBOUT("UPDATING KALMAN ");
 			mu[0] = moments(regions[assignment[i]], false);
			Point_t pt = Point_t(mu[0].m10 / mu[0].m00, mu[0].m01 / mu[0].m00);
			tracks[i]->m_skippedFrames = 0;
			tracks[i]->Update(
				pt, true,
				m_settings.m_maxTraceLength,
				m_prevFrame, grayFrame,
				m_settings.m_useAbandonedDetection ? cvRound(m_settings.m_minStaticTime * fps) : 0);
		}
		else				     // if not continue using predictions
		{
			//DBOUT("UPDATING KALMAN 1");
			Point_t pt;
			tracks[i]->Update(pt, false, m_settings.m_maxTraceLength, m_prevFrame, grayFrame, 0);
		}
	}
}

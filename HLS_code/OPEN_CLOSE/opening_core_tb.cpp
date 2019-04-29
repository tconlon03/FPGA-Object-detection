#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "time.h"
#include <hls_opencv.h>
#include "opening_core.h"
#include <opencv2/core/core.hpp>
#include <stdint.h>
#include <iostream>
#include <fstream>


#ifdef _WIN32
#include <windows.h>
#define SYSERROR()  GetLastError()
#else
#include <errno.h>
#define SYSERROR()  errno
#endif

//#define INPUT_IMG "C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\test_vid_3\\2376_mask.png"
//#define INPUT_IMG "C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\test_vid_2\\246_mask.png"
//#define INPUT_IMG "C:\\Users\\Tiarnan\\Pictures\\unedited_mask_1190.png"


using namespace std;

int main(int argc, char* argv[]) {

	GRAY_AXI_STREAM src_axi;
	RLE_AXI_STREAM dst_axi;
	char path_buffer[100];
	char rle_path[20];
	cv::Mat src;
	cv::Mat ref;
	for (int i = 500; i < 1600; i+=5){
		snprintf(path_buffer, sizeof(path_buffer), "C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\test_vid_5\\%d_mask.png", i);
		std::cout << "Frame path : " << path_buffer << endl;
		src = cvLoadImage(path_buffer, 0);
		cv::Mat dst(src.rows, src.cols, CV_8UC1, cv::Scalar(0));

		int sr = src.rows;
		int sc = src.cols;

		snprintf(rle_path, sizeof(rle_path), "rle_%d.txt", i);
		ofstream rlefile;
		rlefile.open(rle_path);

	//	cv::imshow("unedited", src);

		//for (int r = 0; r < src.rows; r++){
		//	for (int c = 0; c < src.cols; c++){
		//		printf("%d", (int)src.at<uchar>(r,c) > 0 ? 1 : 0);
		//	}
		//	printf("\n");
		//}

		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		//			printf("%d", (int)p.data > 0 ? 1 : 0);

		cvMat2AXIvideo(src, src_axi);

		printf("Calling open close now");

		open_and_close(src_axi, dst_axi);

		rle_run run;
		while(!dst_axi.empty()){
			dst_axi.read(run);
			int start = (int)run.data.s;
			int end = (int)run.data.e;
			int last = (int)run.data._last_run;
			int y = (int) run.data.y;
			rlefile << start;
			rlefile << ",";
			rlefile << end;
			rlefile << ",";
			rlefile << last;
			rlefile << ",";
			rlefile << y << endl;
		}
		rlefile.close();
	}
/*
	printf("Calling show now");
	cv::imshow("edited", dst);

	cv::Mat diffImage(240, 426, CV_8UC1, cv::Scalar(0));
	cv::absdiff(ref, dst, diffImage);

	imshow("Difference in hardware and reference is", diffImage);

	printf("Waiting on key press now");

	cv::waitKey(0);
	// Closes all the frames
	cv::destroyAllWindows();
	*/

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "time.h"
#include <hls_opencv.h>
#include "build_gaussian_core.h"
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

//#define INPUT_VID "C:\\Users\\Tiarnan\\Pictures\\test_vid_mog_bg.avi"
#define INPUT_VID "C:\\Users\\Tiarnan\\Pictures\\test_2_320.avi"
#define INPUT_IMG "C:\\Users\\Tiarnan\\Pictures\\redImage.png"

using namespace std;

int main(int argc, char* argv[]) {

	hls::stream<YUV_pixel> src_axi;
	hls::stream<bool> dst_axi;
	hls::stream<pixel_k_gaussian> MOG_in;
	hls::stream<pixel_k_gaussian> MOG_out;
#ifdef __SYNTHESIS__
	pixel_k_gaussian *MOG = (pixel_k_gaussian*)calloc(sizeof(pixel_k_gaussian) * 320 * 240);
	bool *out_val = (char*)calloc(sizeof(char) * 320 * 240);
#else
	static pixel_k_gaussian MOG[320*240] = { 0 };
	//static short prev_y[320*240] = { 0 };
	//static short prev_bgr[320*240][3] = { 0 };
	static unsigned char out_val[320][240];
#endif
	cv::VideoCapture cap(INPUT_VID);

	  // Check if camera opened successfully
	if(!cap.isOpened()){
		printf( "Error opening video stream or file" );
		return -1;
	}

	/*
	bool ret = cap.set(CV_CAP_PROP_FRAME_WIDTH,320);
	if (!ret){
		printf("failed to set video width");
		printf("Video width is %d", cap.get(CV_CAP_PROP_FRAME_WIDTH));
	}

	ret = cap.set(CV_CAP_PROP_FRAME_HEIGHT,240);
	if (!ret){
			printf("failed to set video height");
			printf("Video height is %d", cap.get(CV_CAP_PROP_FRAME_HEIGHT));
	}*/

	//false is U true is V
	bool u = false;
	int frame_count = 0;
	std::string frame_text;
	std::string base_text = "frame : ";

	std::ofstream of("ref.txt");

	if(of.is_open()) {
		while(frame_count < 1101 ){

			cv::Mat frame;
			cv::Mat frame_yuv;
			// Capture frame-by-frame
			cap >> frame;

			// If the frame is empty, break immediately
			if (frame.empty())
			  break;
			frame_count++;

			//if (frame_count < 200) {
			//	continue;
			//}

			cv::cvtColor(frame, frame_yuv, cv::COLOR_BGR2YUV);

			for (int idxRows = 0; idxRows < frame.rows; idxRows++ ){
				for (int idxCols = 0; idxCols < frame.cols; idxCols++ ){
					YUV_pixel valIn;
					short data;
					short Y = frame_yuv.at<cv::Vec3b>(idxRows, idxCols)[0];
					short U = frame_yuv.at<cv::Vec3b>(idxRows, idxCols)[1];
					short V = frame_yuv.at<cv::Vec3b>(idxRows, idxCols)[2];
					if (idxRows == 0 && idxCols == 0){
						//printf("breaking here");
					}
					// Y is going to be lsb and U/V is msb
					if (u) {
						data = (U << 8) | Y;
					} else{
						data = (V << 8) | Y;
					}
					u = !u;
					valIn.data = data;
					valIn.keep = 1; valIn.strb = 1; valIn.id = 0; valIn.dest = 0;
					//these may be changed below
					valIn.user = 0; valIn.last = 0;
					if (idxCols == 0 && idxRows == 0) {
						valIn.user = 1;
					} else if (idxCols == frame.cols - 1) {
						valIn.last = 1;
					}
					src_axi << valIn;
				}
			}

			build_gaussian(src_axi, dst_axi, MOG, MOG, BG_THRESHOLD, LEARNING_RATE, 1);

			//take data out of output stream;
			for (int idxRows = 0; idxRows < frame.rows; idxRows++ ){
				for (int idxCols = 0; idxCols < frame.cols; idxCols++ ){
					bool valOut;
					while(dst_axi.empty()){};
					dst_axi.read(valOut);
					out_val[idxRows][idxCols] = (valOut == true) ? 255 : 0;
				}
			}

			cv::Mat mask(240, 320, CV_8UC1, cv::Scalar(0));
			for (int idxRows = 0; idxRows < frame_yuv.rows; idxRows++ ){
				for (int idxCols = 0; idxCols < frame_yuv.cols; idxCols++ ){
					mask.at<uchar>(idxRows, idxCols) = out_val[idxRows][idxCols];
				}
			}

			// Display the resulting framec
			char numstr[21]; // enough to hold all numbers up to 64-bits
			sprintf(numstr, "%d", frame_count);
			frame_text = base_text + numstr;
			cv::putText(frame,frame_text,cv::Point(10,30),cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(128));
			cv::imshow( "Frame", frame );
			cv::imshow( "Mask", mask );

			char c=(char)cv::waitKey(5);
				if(c==27)
				  break;

				if (frame_count == 314){
					frame_text = "";
				}

		}
	} else {
		std::cerr<<"Failed to open file : "<<SYSERROR()<<std::endl;
		return -1;
	}
	of.close();
	//myfile.close();
	// When everything done, release the video capture object
	cap.release();
	// Closes all the frames
	cv::destroyAllWindows();

	return 0;
}

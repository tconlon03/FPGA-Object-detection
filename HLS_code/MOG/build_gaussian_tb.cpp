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
#define INPUT_VID "C:\\Users\\Tiarnan\\Pictures\\Matlab_test\\test_vid_1.avi"
//#define INPUT_VID "C:\\Users\\Tiarnan\\Downloads\\test_vid_4.avi"

using namespace std;

int main(int argc, char* argv[]) {

	hls::stream<YUV_pixel> src_axi;
	hls::stream<YUV_pixel> vid_axi;
	//GRAY_AXI_STREAM dst_axi;
	hls::stream<bool> dst_axi;
	hls::stream<pixel_k_gaussian> MOG_in;
	hls::stream<pixel_k_gaussian> MOG_out;
#ifdef __SYNTHESIS__
	pixel_k_gaussian *MOG = (pixel_k_gaussian*)calloc(sizeof(pixel_k_gaussian) * 640 * 480);
	bool *out_val = (char*)calloc(sizeof(char) * 640 * 480);
#else
	static pixel_k_gaussian MOG[640*480] = { 0 };
	//static short prev_y[640*480] = { 0 };
	//static short prev_bgr[640*480][3] = { 0 };
	static unsigned char out_val[480][640];
#endif
	cv::VideoCapture cap(INPUT_VID);

// Check if camera opened successfully
	if(!cap.isOpened()){
		printf( "Error opening video stream or file" );
		return -1;
	}

	/*
	bool ret = cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
	if (!ret){
		printf("failed to set video width");
		printf("Video width is %d", cap.get(CV_CAP_PROP_FRAME_WIDTH));
	}

	ret = cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
	if (!ret){
			printf("failed to set video height");
			printf("Video height is %d", cap.get(CV_CAP_PROP_FRAME_HEIGHT));
	}*/
	printf("input fourcc : %d", cap.get(CV_CAP_PROP_FOURCC));

	cv::VideoWriter output("C:\\Users\\Tiarnan\\Downloads\\test_vid_4_out.avi", CV_FOURCC('C','V','I','D'),
            30, cv::Size(480,640));
	cv::VideoWriter output_mask("C:\\Users\\Tiarnan\\Downloads\\test_vid_4_out_mask.avi", cap.get(CV_CAP_PROP_FOURCC),
            cap.get(CV_CAP_PROP_FPS),
            cv::Size((int)cap.get(CV_CAP_PROP_FRAME_HEIGHT), (int)cap.get(CV_CAP_PROP_FRAME_WIDTH)));
	if(!output.isOpened()){
		printf( "Error opening video file" );
		return -1;
	}
	if(!output_mask.isOpened()){
		printf( "Error opening video mask file" );
		return -1;
	}

	//false is U true is V
	bool u = false;
	int frame_count = 0;
	std::string frame_text;
	std::string base_text = "frame : ";

	std::ofstream of("ref.txt");

	while(frame_count < 80){

		cv::Mat frame;
		cv::Mat frame_yuv;
		// Capture frame-by-frame
		cap >> frame;

		// If the frame is empty, break immediately
		if (frame.empty())
		  break;
		frame_count++;

		if (frame_count < 70) {
			continue;
		}

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

		build_gaussian(src_axi, dst_axi, MOG, vid_axi, MOG, BG_THRESHOLD, LEARNING_RATE, 1);

		//take data out of output stream;
		for (int idxRows = 0; idxRows < frame.rows; idxRows++ ){
			for (int idxCols = 0; idxCols < frame.cols; idxCols++ ){
				bool valOut;
				while(dst_axi.empty()){};
				valOut = dst_axi.read();
				if (valOut == true){
					out_val[idxRows][idxCols] =  255;
				} else {
					out_val[idxRows][idxCols] =  0;
				}
			}
		}

		cv::Mat mask(480, 640, CV_8UC1, cv::Scalar(0));
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
		//cv::putText(mask,frame_text,cv::Point(10,30),cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(128));
		output.write(frame);
		output_mask.write(mask);
		cv::imshow( "Frame", frame );
		cv::imshow( "Mask", mask );

		char c=(char)cv::waitKey(5);
		if(c==27)
		  break;

		if (frame_count == 314){
			frame_text = "";
		}

		/*if (frame_count == 350) {
			cv::imwrite("C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\short_350_frame_v1.png", frame);
			cv::imwrite("C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\short_350_mask_v1.png", mask);
		}
		if (frame_count == 380) {
			cv::imwrite("C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\short_380_frame_v1.png", frame);
			cv::imwrite("C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\short_380_mask_v1.png", mask);
		}
		if (frame_count == 400) {
			cv::imwrite("C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\short_400_frame_v1.png", frame);
			cv::imwrite("C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\short_400_mask_v1.png", mask);
		}
		if (frame_count == 420) {
			cv::imwrite("C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\short_420_frame_v1.png", frame);
			cv::imwrite("C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\short_420_mask_v1.png", mask);
		}*/
	}
	//myfile.close();
	// When everything done, release the video capture object
	cap.release();
	output.release();
	output_mask.release();
	// Closes all the frames
	cv::destroyAllWindows();

	return 0;
}

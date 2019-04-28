#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "time.h"
#include <hls_opencv.h>
#include "blob_analysis_core.h"
#include <opencv2/core/core.hpp>
#include <stdint.h>
#include <iostream>
#include <fstream>

#define INPUT_RLE "C:\\Users\\Tiarnan\\Documents\\Final Year Project\\rle_300_vid2.txt"
#define OG_FRAME "C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\2_300_frame.png"
#define OUT_FRAME "C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\2_300_identified.png"
using namespace std;

int main(int argc, char* argv[]) {
	//vector<blob> blob_vector;
	blob objects[50];
	blob b0 = {};
	objects[0] = b0;
	blob b1 = {};
	objects[1] = b1;
	blob b2 = {};
	objects[2] = b2;
	blob b3 = {};
	objects[3] = b3;
	blob b4 = {};
	objects[4] = b4;
	blob b5 = {};
	objects[5] = b5;
	blob b6 = {};
	objects[6] = b6;
	blob b7 = {};
	objects[7] = b7;
	blob b8 = {};
	objects[8] = b8;
	blob b9 = {};
	objects[9] = b9;
	blob b10 = {};
	objects[10] = b10;
	blob b11 = {};
	objects[11] = b11;
	blob b12 = {};
	objects[12] = b12;
	blob b13 = {};
	objects[13] = b13;
	blob b14 = {};
	objects[14] = b14;
	blob b15 = {};
	objects[15] = b15;
	blob b16 = {};
	objects[16] = b16;
	blob b17 = {};
	objects[17] = b17;
	blob b18 = {};
	objects[18] = b18;
	blob b19 = {};
	objects[19] = b19;
	blob b20 = {};
	objects[20] = b20;
	blob b21 = {};
	objects[21] = b21;
	blob b22 = {};
	objects[22] = b22;
	blob b23 = {};
	objects[23] = b23;
	blob b24 = {};
	objects[24] = b24;
	blob b25 = {};
	objects[25] = b25;
	blob b26 = {};
	objects[26] = b26;
	blob b27 = {};
	objects[27] = b27;
	blob b28 = {};
	objects[28] = b28;
	blob b29 = {};
	objects[29] = b29;
	blob b30 = {};
	objects[30] = b30;
	blob b31 = {};
	objects[31] = b31;
	blob b32 = {};
	objects[32] = b32;
	blob b33 = {};
	objects[33] = b33;
	blob b34 = {};
	objects[34] = b34;
	blob b35 = {};
	objects[35] = b35;
	blob b36 = {};
	objects[36] = b36;
	blob b37 = {};
	objects[37] = b37;
	blob b38 = {};
	objects[38] = b38;
	blob b39 = {};
	objects[39] = b39;
	blob b40 = {};
	objects[40] = b40;
	blob b41 = {};
	objects[41] = b41;
	blob b42 = {};
	objects[42] = b42;
	blob b43 = {};
	objects[43] = b43;
	blob b44 = {};
	objects[44] = b44;
	blob b45 = {};
	objects[45] = b45;
	blob b46 = {};
	objects[46] = b46;
	blob b47 = {};
	objects[47] = b47;
	blob b48 = {};
	objects[48] = b48;
	blob b49 = {};
	objects[49] = b49;
	blob_port objects_port[50];
	cout << "blob size : " << sizeof(objects[0]) << endl;
	cout << "blob port size : " << sizeof(objects_port[0]) << endl;
	for (int i = 0; i < 50; i++){
	    memcpy(&objects_port[i], &objects[i], sizeof(blob));
	}
 	RLE_AXI_STREAM rle_stream;
	rle_run run;
	std::ifstream rlefile(INPUT_RLE);
	string line;
	if (rlefile.is_open()){
	    while ( getline (rlefile,line) ){
	    	std::stringstream ss(line);
	    	int i;
	    	int count = 0;
			while (ss >> i)
			{
				if (ss.peek() == ','){
					ss.ignore();
				}
				if (count == 0){
					run.data.s = i;
				} else if (count == 1){
					run.data.e = i;
				} else if (count == 2){
					run.data._last_run = i;
				} else if (count == 3){
					run.data.y = i;
				}
				count = count +1;
			}
			run.data.no = 0;
			rle_stream << run;
	    }
	    rlefile.close();
	}
	else cout << "Could not open file";

	blob_analysis(rle_stream, objects_port);

	for (int i=0; i<50; i++){
		//masks
		unsigned short *start = (unsigned short *)&objects_port[i];
		unsigned char id = (unsigned char)*(start);
		unsigned short cp_x = *(start + 1);
		unsigned short cp_y = *(start + 2);
		unsigned short area = *(start + 3);
		unsigned short max_x = *(start + 4);
		unsigned short min_x = *(start + 5);
		unsigned short max_y = *(start + 6);
		unsigned short min_y = *(start + 7);
		//coord coord_mask = 0x1FF;
		//unsigned char uchar_mask = 0xFF;
		//short short_mask = 0xFFFF;
		//unsigned char id = objects_port[i] & uchar_mask;
		objects[i].id = id;
		cout << "id : " << id << endl;
		//coord cp_x = (objects_port[i] >> 16) & coord_mask;
		cout << "cp_x : " << cp_x << endl;
		//coord cp_y = (objects_port[i] >> 32) & coord_mask;
		cout << "cp_y : " << cp_y << endl;
		objects[i].cp.x = cp_x;
		objects[i].cp.y = cp_y;
		//short area = (objects_port[i] >> 48) & short_mask;
		cout << "area : " << area << endl;
		objects[i].area = area;
		//coord max_x = (objects_port[i] >> 64) & coord_mask;
		cout << "max x : " << max_x << endl;
		//coord min_x = (objects_port[i] >> 80) & coord_mask;
		cout << "minx : " << min_x << endl;
		//coord max_y = (objects_port[i] >> 96) & coord_mask;
		cout << "max y : " << max_y << endl;
		//coord min_y = (objects_port[i] >> 112) & coord_mask;
		cout << "min_y : " << min_y << endl;
		objects[i].max_x = max_x;
		objects[i].min_x = min_x;
		objects[i].max_y = max_y;
		objects[i].min_y = min_y;
	}

	cv::Mat frame = cvLoadImage(OG_FRAME, 1);

	for (int i = 0 ; i < 50; i++){
		if (objects[i].id > 0 && objects[i].area > 200){
			cv::Point tl;
			cv::Point br;
			printf("Top left x : %d",(int)objects[i].min_x );
			printf("Top left y : %d",(int)objects[i].min_y );
			printf("Bottom right x : %d",(int)objects[i].max_x );
			printf("Bottom right y : %d",(int)objects[i].max_y );
			tl.x = (int)objects[i].min_x;
			tl.y = (int)objects[i].min_y;
			br.x = (int)objects[i].max_x;
			br.y = (int)objects[i].max_y;
			cv::rectangle(frame, tl, br, cv::Scalar(0, 255, 0), 2);
		}
	}

	cv::imshow("edited", frame);
	cv::imwrite(OUT_FRAME, frame);
	cv::waitKey(0);
	return 0;
}

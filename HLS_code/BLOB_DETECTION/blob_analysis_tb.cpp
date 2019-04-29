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

//#define INPUT_RLE "C:\\Users\\Tiarnan\\Documents\\Final Year Project\\rle_300_vid2.txt"
//#define INPUT_RLE "C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\test_vid_4\\rle_382.txt"
//#define OG_FRAME "C:\\Users\\Tiarnan\\Documents\\Final Year Project\\frame_450.png"
//#define OG_FRAME "C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\test_vid_3\\2333.png"
//#define OUT_FRAME "C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\2_300_identified.png"
//#define OUT_FRAME "C:\\Users\\Tiarnan\\Documents\\Final Year Project\\frame_2333_identified.png"
using namespace std;

int main(int argc, char* argv[]) {
	char rle_path[100];
	char og_frame_path[100];
	char out_frame_path[100];
	//loop over test videos
    for (int i = 900; i < 2116; i+=5){
		//vector<blob> blob_vector;
		blob objects[100];
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
		blob b50 = {};
		objects[50] = b50;
		blob b51 = {};
		objects[51] = b51;
		blob b52 = {};
		objects[52] = b52;
		blob b53 = {};
		objects[53] = b53;
		blob b54 = {};
		objects[54] = b54;
		blob b55 = {};
		objects[55] = b55;
		blob b56 = {};
		objects[56] = b56;
		blob b57 = {};
		objects[57] = b57;
		blob b58 = {};
		objects[58] = b58;
		blob b59 = {};
		objects[59] = b59;
		blob b60 = {};
		objects[60] = b60;
		blob b61 = {};
		objects[61] = b61;
		blob b62 = {};
		objects[62] = b62;
		blob b63 = {};
		objects[63] = b63;
		blob b64 = {};
		objects[64] = b64;
		blob b65 = {};
		objects[65] = b65;
		blob b66 = {};
		objects[66] = b66;
		blob b67 = {};
		objects[67] = b67;
		blob b68 = {};
		objects[68] = b68;
		blob b69 = {};
		objects[69] = b69;
		blob b70 = {};
		objects[70] = b70;
		blob b71 = {};
		objects[71] = b71;
		blob b72 = {};
		objects[72] = b72;
		blob b73 = {};
		objects[73] = b73;
		blob b74 = {};
		objects[74] = b74;
		blob b75 = {};
		objects[75] = b75;
		blob b76 = {};
		objects[76] = b76;
		blob b77 = {};
		objects[77] = b77;
		blob b78 = {};
		objects[78] = b78;
		blob b79 = {};
		objects[79] = b79;
		blob b80 = {};
		objects[80] = b80;
		blob b81 = {};
		objects[81] = b81;
		blob b82 = {};
		objects[82] = b82;
		blob b83 = {};
		objects[83] = b83;
		blob b84 = {};
		objects[84] = b84;
		blob b85 = {};
		objects[85] = b85;
		blob b86 = {};
		objects[86] = b86;
		blob b87 = {};
		objects[87] = b87;
		blob b88 = {};
		objects[88] = b88;
		blob b89 = {};
		objects[89] = b89;
		blob b90 = {};
		objects[90] = b90;
		blob b91 = {};
		objects[91] = b91;
		blob b92 = {};
		objects[92] = b92;
		blob b93 = {};
		objects[93] = b93;
		blob b94 = {};
		objects[94] = b94;
		blob b95 = {};
		objects[95] = b95;
		blob b96 = {};
		objects[96] = b96;
		blob b97 = {};
		objects[97] = b97;
		blob b98 = {};
		objects[98] = b98;
		blob b99 = {};
		objects[99] = b99;
		blob_port objects_port[100];
		cout << "blob size : " << sizeof(objects[0]) << endl;
		//cout << "blob port size : " << sizeof(objects_port[0]) << endl;
		for (int j = 0; j < 100; j++){
			memcpy(&objects_port[j], &objects[j], sizeof(blob));
		}
		RLE_AXI_STREAM rle_stream;
		rle_run run;
		snprintf(rle_path, sizeof(rle_path), "C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\test_vid_1_var1\\rle_%d.txt", i);
		snprintf(og_frame_path, sizeof(og_frame_path), "C:\\Users\\Tiarnan\\Pictures\\bg_core_output\\test_vid_1_var1\\%d.png", i);
		snprintf(out_frame_path, sizeof(out_frame_path), "C:\\Users\\Tiarnan\\Documents\\Final Year Project\\test_vid_1_id_var1\\%d.png", i);
		std::ifstream rlefile(rle_path);
		string line;
		if (rlefile.is_open()){
			while ( getline (rlefile,line) ){
				std::stringstream ss(line);
				int j;
				int count = 0;
				while (ss >> j)
				{
					if (ss.peek() == ','){
						ss.ignore();
					}
					if (count == 0){
						run.data.s = j;
					} else if (count == 1){
						run.data.e = j;
					} else if (count == 2){
						run.data._last_run = j;
					} else if (count == 3){
						run.data.y = j;
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

		for (int j=0; j<100; j++){
			//masks
			unsigned short *start = (unsigned short *)&objects_port[j];
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
			objects[j].id = id;
			//cout << "id : " << id << endl;
			//coord cp_x = (objects_port[i] >> 16) & coord_mask;
			//cout << "cp_x : " << cp_x << endl;
			//coord cp_y = (objects_port[i] >> 32) & coord_mask;
			//cout << "cp_y : " << cp_y << endl;
			objects[j].cp.x = cp_x;
			objects[j].cp.y = cp_y;
			//short area = (objects_port[i] >> 48) & short_mask;
			//cout << "area : " << area << endl;
			objects[j].area = area;
			//coord max_x = (objects_port[i] >> 64) & coord_mask;
			//cout << "max x : " << max_x << endl;
			//coord min_x = (objects_port[i] >> 80) & coord_mask;
			//cout << "minx : " << min_x << endl;
			//coord max_y = (objects_port[i] >> 96) & coord_mask;
			//cout << "max y : " << max_y << endl;
			//coord min_y = (objects_port[i] >> 112) & coord_mask;
			//cout << "min_y : " << min_y << endl;
			objects[j].max_x = max_x;
			objects[j].min_x = min_x;
			objects[j].max_y = max_y;
			objects[j].min_y = min_y;
		}

		cv::Mat frame = cvLoadImage(og_frame_path, 1);

		printf("\n%d ", i);
		for (int j = 0 ; j < 100; j++){
			if (objects[j].id > 0 && objects[j].area > 350 && objects[j].area < 5000){
				cv::Point tl;
				cv::Point br;
				tl.x = (int)objects[j].min_x;
				tl.y = (int)objects[j].min_y;
				br.x = (int)objects[j].max_x;
				br.y = (int)objects[j].max_y;
				//if w > 2.5*h
				if (((int)objects[j].max_x - (int)objects[j].min_x) > 2.5*((int)objects[j].max_y - (int)objects[j].min_y)){
					continue;
				}
				printf("%d ", (int)objects[j].min_x );
				printf("%d ",(int)objects[j].min_y );
				printf("%d ",(int)objects[j].max_x );
				printf("%d ",(int)objects[j].max_y );
				cv::rectangle(frame, tl, br, cv::Scalar(0, 255, 0), 2);
			}
		}

		//cv::imshow("edited", frame);
		cv::imwrite(out_frame_path, frame);
	}
	cv::waitKey(0);
	return 0;
}

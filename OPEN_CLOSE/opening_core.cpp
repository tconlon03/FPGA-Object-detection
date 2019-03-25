#include "opening_core.h"
#include <stdint.h>

void open_and_close(GRAY_AXI_STREAM& INPUT_STREAM, RLE_AXI_STREAM& OUTPUT_STREAM)//, int rows, int cols)
{
	#pragma HLS INTERFACE axis port=INPUT_STREAM
	#pragma HLS INTERFACE axis port=OUTPUT_STREAM
	#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS
	GRAY_AXI_STREAM OUTPUT_IMG_STREAM;
#pragma HLS STREAM variable=OUTPUT_IMG_STREAM depth=1024 dim=1

	GRAY_IMAGE img_in(240, 320);
	GRAY_IMAGE  img_out(240, 320);
#pragma HLS STREAM variable=img_in depth=1024 dim=1
#pragma HLS STREAM variable=img_out depth=1024 dim=1
	//	#pragma HLS dataflow
	hls::AXIvideo2Mat(INPUT_STREAM, img_in);
	hls::Window<3, 3, unsigned char> kernel_3;
	hls::Window<8, 8, unsigned char> kernel_8;
	hls::Window<20, 20, unsigned char> kernel_25;
	//close
	hls::Dilate<0,1>(img_in, img_out , kernel_3);
	hls::Erode<0,1>(img_out, img_out , kernel_3);
	//open
	hls::Erode<0,1>(img_out, img_out , kernel_3);
	hls::Dilate<0,1>(img_out, img_out , kernel_3);
	//close
	hls::Dilate<0,1>(img_out, img_out , kernel_25);
	hls::Erode<0,1>(img_out, img_out , kernel_25);
	hls::Mat2AXIvideo(img_out, OUTPUT_IMG_STREAM);
	/* encode image in rle */
	for (int r = 0; r < 240; r++){
		ap_uint<8> prev_pixel = 0;
		coord start = 0;
		coord end = 0;
		rle_run run;
		run.data.s = img_out.cols;
		for (int c = 0; c < img_out.cols; c++){
			while(OUTPUT_IMG_STREAM.empty()){}
			GRAY_PIXEL p;
			OUTPUT_IMG_STREAM.read(p);
			if (p.data != prev_pixel){
				if (p.data > 0){
					start = c;
					if (run.data.s < img_out.cols){
						run.data._last_run = 0;
						OUTPUT_STREAM.write(run);
					}
				} else {
					end = c - 1;
					run.data.s = start;
					run.data.e = end;
					run.data.no = 0;
					run.data.y = r;
					//r.tlast cannot be set yet
				}
			}
			if (c == img_out.cols - 1){
				//EDGE CASE WHERE RUN CONTINUES TO END
				if (run.data.s != start && p.data > 0){
					run.data.s = start;
					run.data.e = end;
					run.data.no = 0;
					run.data.y = r;
				}
				if(run.data.s != img_out.cols){
					run.data._last_run = 1;
					OUTPUT_STREAM.write(run);
				}
			}
			prev_pixel = (uint_8) p.data;
		}
	}
}


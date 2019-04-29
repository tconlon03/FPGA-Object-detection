#include "blob_analysis_core.h"
#include <stdint.h>

/*Types defined in blob_analysis_core.h*/

using namespace hls;

/*
 * Top function - blob analysis
 * Receive input mask as stream two lines at a time encoded using rle
 * and compute the neighbouring areas of the lines to 
 * identify objects.
 * stream_in - input mask from MOG Foreground detection
 * objects - array of bounding boxes and centre points in the image
 */
void blob_analysis(RLE_AXI_STREAM &rle_stream, blob_port objects_port[100]){
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS
#pragma HLS INTERFACE bram port=objects_port
#pragma HLS INTERFACE axis port=rle_stream
	blob * objects_ptr[100];
	blob objects[100];
#pragma HLS ARRAY_MAP variable=objects_ptr instance=array1 horizontal
#pragma HLS ARRAY_MAP variable=objects instance=array1 horizontal

	for (int i=0; i<100; i++){
#pragma HLS PIPELINE
		objects[i].id = objects_port[i] & 0xFF;
		objects[i].cp.x = (objects_port[i] >> 8) & 0x1FF;
		objects[i].cp.y = (objects_port[i] >> 17) & 0x1FF;
		objects[i].area = (objects_port[i] >> 26) & 0xFFFF;
		objects[i].max_x = (objects_port[i] >> 42) & 0x1FF;
		objects[i].min_x = (objects_port[i] >> 51) & 0x1FF;
		objects[i].max_y = (objects_port[i] >> 60) & 0x1FF;
		objects[i].min_y = (objects_port[i] >> 69) & 0x1FF;
		objects_ptr[i] = &(objects[i]);
	}
	//create blobs so they are in scope
	coord cur_line = 0;
	rle_line previous;
	rle_line current;
	unsigned char * next_ob_id_p;
	unsigned char next_ob_id = 1;
	next_ob_id_p = &next_ob_id;
	while(!rle_stream.empty()){
	//for (int l = 0; l < 240; l++){
		//printf("l : %d", l);
		while(rle_stream.empty()){};
		rle_run run;
		uint_8 run_count = 0;
		run.data._last_run = 0;
		while(!run.data._last_run){
		//for (int r = 0; r < 120; r++){
			run = rle_stream.read();
			current.runs[run_count] = run;
			run_count = run_count + 1;
		}
		current.no_runs = run_count;
		if (cur_line > 0){
			identify_update_objects(current, previous, next_ob_id_p, objects_ptr);
		} else {
			//first line is current - mark runs
			for (int i = 0; i < current.no_runs; i++){
			//for (int i = 0; i < 120; i++){
				current.runs[i].data.no = next_ob_id;
				create_blob(objects_ptr[next_ob_id - 1], next_ob_id_p, current.runs[i]);
				next_ob_id = next_ob_id + 1;
			}
		}
		previous = current;
		cur_line = cur_line + 1;
	}
	for (int i = 0; i < 100; i++){
#pragma HLS PIPELINE
		unsigned char *start = (unsigned char *)&objects[i];
		unsigned char *start_port = (unsigned char *)&objects_port[i];
		for (int j = 0; j < sizeof(blob); j ++){
			unsigned char byte = (unsigned char)*(start + j);
			*(start_port + j) = byte;
		}
	}
}

/*
 * Iterate over the two lines passed in and identify what objects
 * the runs belong to. Update the object properties as we go.
 * current - line we are identifying objects in
 * previous - check if runs in current match runs in previous
 * 			  to get object id.
 */
void identify_update_objects(rle_line &current, rle_line &previous, unsigned char * ob_id, blob * (&objects_ptr)[100]){
	for (int i = 0; i < current.no_runs; i++){
	//for (int i = 0; i < 120; i++){
		unsigned char match = 0;
		for (int j = 0; j < previous.no_runs; j++){
		//for (int j = 0; j < 120; j++){
			if ((current.runs[i].data.s <= previous.runs[j].data.e &&
				current.runs[i].data.s >= previous.runs[j].data.s) ||
				(current.runs[i].data.e <= previous.runs[j].data.e &&
				current.runs[i].data.e >= previous.runs[j].data.s) ||
				(current.runs[i].data.s < previous.runs[j].data.s &&
				current.runs[i].data.e > previous.runs[j].data.e)) {
				match = match + 1;
				if (match == 1) {
					//update object parameters
					current.runs[i].data.no = previous.runs[j].data.no;
					//area
					char idx = (char)current.runs[i].data.no - 1;
					if (idx >= 0 && idx < 100){
						blob* b = objects_ptr[idx];
						b->area = b->area + (current.runs[i].data.e - current.runs[i].data.s);
						if (current.runs[i].data.e > b->max_x){
							b->max_x = current.runs[i].data.e;
						}
						if (current.runs[i].data.s < b->min_x){
							b->min_x = current.runs[i].data.s;
						}
						if (current.runs[i].data.y > b->max_y){
							b->max_y = current.runs[i].data.y;
						}
						coord w = b->max_x - b->min_x;
						coord h = b->max_y - b->min_y;
						point centre;
						centre.x = b->min_x + (w/2);
						centre.y = b->min_y + (h/2);
						b->cp = centre;
					}
					//bounding box created at end
				} else if (match > 1) {
					//check if matched with a new object
					//merge objects
					//idx of object already matched with
					current.runs[i].data.no = previous.runs[j].data.no;
					char idx_merge = (char)current.runs[i].data.no - 1;
					char idx = (char)previous.runs[j].data.no -1;
					if (idx == idx_merge){
						break;
					}
					//copy features to blob 1
					blob* b = objects_ptr[idx];
					blob* b_merge = objects_ptr[idx_merge];
					b->area = b->area + b_merge->area;
					b->max_x = b->max_x >= b_merge->max_x ? b->max_x : b_merge->max_x;
					b->max_y = b->max_y >= b_merge->max_y ? b->max_y : b_merge->max_y;
					b->min_x = b->min_x <= b_merge->min_x ? b->min_x : b_merge->min_x;
					b->min_y = b->min_y <= b_merge->min_y ? b->min_y : b_merge->min_y;
					coord w = b->max_x - b->min_x;
					coord h = b->max_y - b->min_y;
					point centre;
					centre.x = b->min_x + (w/2);
					centre.y = b->min_y + (h/2);
					b->cp = centre;
					objects_ptr[idx_merge] = objects_ptr[idx];
				}
			}
		}
		if (match == 0) {
			//create a new object
			if(*ob_id < 100){
				current.runs[i].data.no = *ob_id;
				create_blob(objects_ptr[*(ob_id) - 1], ob_id, current.runs[i]);
				*ob_id = *(ob_id) + 1;
			}
		}
	}
}

void create_blob(blob * b, unsigned char * ob_id, rle_run current){
	b->id = *ob_id;
	b->min_y = current.data.y;
	b->max_y = current.data.y;
	b->min_x = current.data.s;
	b->max_x = current.data.e;
	coord w = current.data.e - current.data.s + 1;
	point centre;
	centre.x = current.data.s + (w/2);
	centre.y = current.data.y;
	b->cp = centre;
	b->area = w;
}

#include "build_gaussian_core.h"
#include <stdint.h>

/*Types defined in build_gaussian_core.h*/

using namespace hls;


/*
 * Top function - Two stages: building MOG and Background Subtraction
 * 	MOG:Receives YUV pixel in AXI Stream and extracts
 * 		luminosity value. If this is the first frame create a new gaussian
 * 		for the pixel with weight one and mean = pixel val. Otherwise find
 * 		the gaussians that match the pixel and update these. create a new gaussian
 * 		if  suitable one does not exist and remove the lowest weighted one.
 * 	Background Subtraction: Order models wrt weight/sd - Backgrounds should occur
 * 		frequently and without much change. Chose fist B models as bg where sum of
 * 		weight is less than T. If pixel matches at least one bg gaussian it is bg.
 * 	stream_in - frame of YUV pixels in frame
 * 	stream_out - mask for input to the morphological core
 * 				use bool for testing and GRAY_AXI_STREAM for real implementation.
 * 	MOG - A max of five gaussians and control variables
 * 	vid_out - YUV stream of black pr white pixels
 * 	bg_thresh - change the bg threshold through AXI Lite
 * 	learning_rate - change the learning_rate through AXI Lite
 * 	min_var - change the min variance a gaussian can have through AXI Lite
 *
 */
//void build_gaussian(hls::stream<YUV_pixel> &stream_in , GRAY_AXI_STREAM &stream_out , pixel_k_gaussian MOG[640*480], hls::stream<YUV_pixel> &vid_out, pixel_k_gaussian MOG_out[640*480], weight_vals bg_thresh, weight_vals learning_rate, unsigned char min_var){
void build_gaussian(hls::stream<YUV_pixel> &stream_in , hls::stream<bool> &stream_out , pixel_k_gaussian MOG[640*480], hls::stream<YUV_pixel> &vid_out, pixel_k_gaussian MOG_out[640*480], weight_vals bg_thresh, weight_vals learning_rate, unsigned char min_var){
#pragma HLS DATA_PACK variable=MOG struct_level
#pragma HLS DATA_PACK variable=MOG_out struct_level
//#pragma HLS DATA_PACK variable=MOG->data
//#pragma HLS DATA_PACK variable=MOG_out->data
//#pragma HLS INTERFACE axis port=MOG_out //used for testing
//#pragma HLS INTERFACE axis port=MOG //used for testing
#pragma HLS INTERFACE s_axilite port=bg_thresh bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=learning_rate bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=min_var bundle=CRTL_BUS
#pragma HLS INTERFACE m_axi depth=800 port=MOG offset=slave bundle=MOG_MASTER
#pragma HLS INTERFACE m_axi depth=800 port=MOG_out offset=slave bundle=MOG_MASTER
#pragma HLS INTERFACE axis port=stream_in
#pragma HLS INTERFACE axis port=stream_out
#pragma HLS INTERFACE axis port=vid_out
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS
	static int frame_count = 0;
	calc_t BG_THRESH = bg_thresh;
	frame_count = frame_count + 1;
//#pragma HLS DATAFLOW
	const int pixels = 320 * 240;
	for (int idx = 0; idx < pixels; idx++){
#pragma HLS PIPELINE II=10
		bool p_is_bg = false;
		unsigned char match = 3;
		YUV_pixel pix = stream_in.read();
		unsigned char y = (pix.data & 0xFF);
		YUV_pixel out_p;
		out_p.dest = pix.dest;
		out_p.last = pix.last;
		out_p.user = pix.user;
		out_p.id = pix.id;
		out_p.strb = pix.strb;
		out_p.keep = pix.keep;
		out_p.data = 0x0000;
		pixel_k_gaussian pg = MOG[idx]; //= *((pixel_k_gaussian*)MOG + idx);
		//std::cout << "Size of pkg : " << sizeof(pixel_k_gaussian) << std::endl;
		//unsigned char * pg_ptr = (unsigned char *)&pg;
		//for (int j = 0; j < sizeof(pixel_k_gaussian); j ++){
		//	unsigned char byte = (unsigned char)*(MOG + sizeof(pixel_k_gaussian)*idx + j);
		//	*(pg_ptr + j) = byte;
		//}
		//memcpy(&pg, MOG + sizeof(pixel_k_gaussian)*idx, sizeof(pixel_k_gaussian));
		//order gaussians by fitness
		sort_gaussians(pg);
		// check pixel for match with a gaussian - changhe to going up and break
		for (int g = 0; g <= 2; g++ ){
		//#pragma HLS UNROLL
			int m = find_match(pg.data.k_lum[g], y);
			//int m = find_match(pg.k_lum[g], y);
			match = (m == 1) ? hls::min(match, (unsigned char)g) : match;
		}
		//update matched gaussian with best fitness
		if (match < 3){
			for (int g = 0; g <= 2; g++ ){
			//#pragma HLS UNROLL
				if (match == g){
					//update_gaussian(pg.k_lum[g],y, true, learning_rate, min_var, frame_count);
					update_gaussian(pg.data.k_lum[g],y, true, learning_rate, min_var, frame_count);
				} else {
					update_gaussian(pg.data.k_lum[g],y, false, learning_rate, min_var, frame_count);
					//update_gaussian(pg.k_lum[g],y, false, learning_rate, min_var, frame_count);
				}
			}
		} else{
			//create a gaussian to model y and remove the lowest weighted gaussian, idx = 2
			//calc new weight
			calc_t msumtot = calc_t(pg.data.k_lum[0].matchsum) + calc_t(pg.data.k_lum[1].matchsum);
			//calc_t msumtot = calc_t(pg.k_lum[0].matchsum) + calc_t(pg.k_lum[1].matchsum);
			weight_vals w;
			if (msumtot == 0) {
				w = weight_vals (1);
			} else {
				w = weight_vals(calc_t(1)/msumtot);
			}
			lum_gaussian l = create_new_gaussian(y, var_vals (30), w);
			//pg.k_lum[2] = l;
			pg.data.k_lum[2] = l;
		}
		normalise_weights(pg);
		if (frame_count > 50 ){
			// start background subtraction
			sort_gaussians(pg);
			calc_t sum = 0.0;
			//int g = 0;
			for (ap_uint<4> g = 0; g < 3; g ++ ){
			//while (sum < BG_THRESH){
				sum = sum + pg.data.k_lum[g].weight;
				//sum = sum + pg.k_lum[g].weight;
				//if (find_match(pg.k_lum[g], y )){
				if (find_match(pg.data.k_lum[g], y )){
					p_is_bg = true;
					out_p.data = 0x8000;
					break;
				}
				if (sum > BG_THRESH){
					break;
				}
			}
			stream_out.write(!p_is_bg);
			//gp.data = !p_is_bg;
		} else {
			stream_out.write(false);
		}
		vid_out.write(out_p);
		//stream_out.write(gp);
		//pg.last = pg.last + 1;
		//if(idx == 1){
		//	std::cout << "last : " << (int)pg.last <<std::endl;
		//}
		//pix_out.data = 0xFF80;
		//unsigned char * pg_ptr = (unsigned char *)&pg;
		//for (int j = 0; j < sizeof(pixel_k_gaussian); j ++){
		//	unsigned char byte = (unsigned char)*(pg_ptr + j);
		//	*(MOG + sizeof(pixel_k_gaussian)*idx + j) = byte;
		//}
		//memcpy(MOG + 26*idx, &pg, sizeof(pixel_k_gaussian));
		MOG_out[idx] = pg;
	}
}

/*
 * This function checks whether the current pixel matches to
 * any of the pixels past gaussians. A match is defined as being
 * within 2.5 standard deviations
 * 	l - input gaussian of pixel. This function is
 * 		called in a loop so all gaussians are checked.
 * 	y - luminosity value of current pixel
 * 	return val - 1 if match 0 otherwise
 */
int find_match (lum_gaussian l, unsigned char y){
	if (y >= l.mean) {
		if ((y - l.mean) < calc_t(sqrt(l.var)*calc_t(2.5))){
			if (l.matchsum > 0) {
				return 1;
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	}
	else {
		if ((l.mean - y) < calc_t(sqrt(l.var)*calc_t(2.5))){
			if (l.matchsum > 0) {
				return 1;
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	}
}

/*
 * This function a gaussian, mean, var and weight based on
 * previous values.
 * 	l - input gaussian of pixel. This function is
 * 		called in a loop so all gaussians are checked.
 * 	y - luminosity value of current pixel
 * 	matched - is this the gaussian that the current pixel matches
 */
void update_gaussian(lum_gaussian &l, unsigned char y, bool matched, weight_vals learn_rate, unsigned char min_var, int frame_count){
	calc_t learning_rate = frame_count < 50 ? calc_t (0.5/frame_count) : (calc_t)learn_rate;
	weight_vals weight_out;
	weight_vals weight_in = l.weight;
	if (matched){
		mean_vals mean_out;
		mean_vals mean_in = l.mean;
		var_vals var_out;
		var_vals var_in = l.var;
		unsigned char matchsum_in = l.matchsum;
		unsigned char matchsum_out = (matchsum_in == 255) ? matchsum_in : matchsum_in + 1;
		calc_t mean_var_lr = 0.5;
		//make sure outdated gaussians get updated quicklly but heavily weighted arent overly affected by outliers
		if (weight_in != 0){
			mean_var_lr = calc_t (learning_rate/weight_in) > mean_var_lr ? mean_var_lr :  calc_t (learning_rate/weight_in);
		} else {
			mean_var_lr = 0;
		}
		//update mean
		mean_out = mean_in + mean_var_lr * (y - mean_in);
		//mean_out = ((mean_in * num_frames) + y ) / num_frames + 1;
		//update sd
		var_out = var_in + mean_var_lr * (((y-mean_in)*(y-mean_in)) - var_in);
		if (var_out < min_var){
			var_out = min_var;
		}
		//calc_t Sn_prev = (sd_in * sd_in) * num_frames;
		//calc_t Sn_curr = Sn_prev + (y - mean_in)*(y - mean_out);
		//sd_out = hls::sqrt((calc_t)(Sn_curr/num_frames+1));
		//update weight
		weight_out = weight_in - learning_rate * weight_in + learning_rate;
		//weight_out = (gaus_val)(1-LEARNING_RATE)*weight_in + LEARNING_RATE;
		//update l
		l.mean = mean_out;
		l.var = var_out;
		l.matchsum = matchsum_out;
	} else {
		weight_out = weight_in - learning_rate * weight_in;
	}
	l.weight = weight_out;
}

/*
 * This function sorts the gaussians in order wrt w/sd
 * highest ratio is in position 0
 * Background models should have a high weight and low variance
 * 	pg - contains all gaussians for a given pixel
 */
void sort_gaussians(pixel_k_gaussian &pg){
	//bool swap = true;
	//while ( swap ){
		//swap = false;
	for (int g = 0; g < 2; g ++ ){
		lum_gaussian l_curr = pg.data.k_lum[g];
		lum_gaussian l_next = pg.data.k_lum[g+1];
		//lum_gaussian l_curr = pg.k_lum[g];
		//lum_gaussian l_next = pg.k_lum[g+1];
		calc_t ratio_next;
		calc_t ratio_curr;
		if (l_curr.matchsum > 0){
			if (l_curr.var != 0){
				ratio_curr = calc_t(calc_t(l_curr.weight)/calc_t(hls::sqrt(l_curr.var)));
			} else {
				ratio_curr = calc_t(l_curr.weight);
			}
		} else {
			ratio_curr = calc_t(0);
		}
		if (l_next.matchsum > 0){
			if (l_next.var != 0){
				ratio_next = calc_t(calc_t(l_next.weight)/calc_t(hls::sqrt(l_next.var)));
			} else {
				ratio_next = calc_t(l_next.weight);
			}
		} else {
			ratio_next = calc_t(0);
		}
		if (ratio_next > ratio_curr){
			//swap them
			//swap = true;
			lum_gaussian temp = l_next;
			pg.data.k_lum[g+1] = pg.data.k_lum[g];
			//pg.k_lum[g+1] = pg.k_lum[g];
			pg.data.k_lum[g] = temp;
			//pg.k_lum[g] = temp;
		}
	}
	for (int g = 0; g < 2; g ++ ){
			lum_gaussian l_curr = pg.data.k_lum[g];
			//lum_gaussian l_curr = pg.k_lum[g];
			lum_gaussian l_next = pg.data.k_lum[g+1];
			//lum_gaussian l_next = pg.k_lum[g+1];
			calc_t ratio_next;
			calc_t ratio_curr;
			if (l_curr.matchsum > 0){
				if (l_curr.var != 0){
					ratio_curr = calc_t(calc_t(l_curr.weight)/calc_t(hls::sqrt(l_curr.var)));
				} else {
					ratio_curr = calc_t(l_curr.weight);
				}
			} else {
				ratio_curr = calc_t(0);
			}
			if (l_next.matchsum > 0){
				if (l_next.var != 0){
					ratio_next = calc_t(calc_t(l_next.weight)/calc_t(hls::sqrt(l_next.var)));
				} else {
					ratio_next = calc_t(l_next.weight);
				}
			} else {
				ratio_next = calc_t(0);
			}
			if (ratio_next > ratio_curr){
				//swap them
				//swap = true;
				lum_gaussian temp = l_next;
				pg.data.k_lum[g+1] = pg.data.k_lum[g];
				//pg.k_lum[g+1] = pg.k_lum[g];
				pg.data.k_lum[g] = temp;
				//pg.k_lum[g] = temp;
			}
		}
}


/*
 * This function normalises the weights to a sum of one
 * 	pg - contains all gaussians for a given pixel
 */
void normalise_weights(pixel_k_gaussian &pg){
	//calc_t sum =  pg.k_lum[0].weight + pg.k_lum[1].weight + pg.k_lum[2].weight;
	calc_t sum =  pg.data.k_lum[0].weight + pg.data.k_lum[1].weight + pg.data.k_lum[2].weight;
	for (int g = 0; g < 3; g ++ ){
		pg.data.k_lum[g].weight = (weight_vals)(pg.data.k_lum[g].weight / sum);
		//pg.k_lum[g].weight = (weight_vals)(pg.k_lum[g].weight / sum);
	}
}


/*
 * This function creates a new gaussian based on params
 *	y - pixel luiminance needs cast to mean_sd_vals
 *	var - variance inital value
 *	weight - weight inital value
 *	return val - lum_gaussian struct
 */
lum_gaussian create_new_gaussian(unsigned char y, var_vals var, weight_vals weight){
	lum_gaussian l;
	l.mean = mean_vals (y);
	l.var = var;
	l.weight = weight;
	l.matchsum = 1;
	return l;
}

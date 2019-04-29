#include <hls_video.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <hls_math.h>
#include <ap_fixed.h>
#include <ap_int.h>

typedef hls::Mat<480, 640, HLS_8UC1> GRAY_IMAGE;
typedef ap_axiu<16,2,5,6> YUV_pixel;
typedef ap_axiu<8,2,5,6> GRAY_pixel;
typedef hls::stream< YUV_pixel > AXI_STREAM;
typedef hls::stream< GRAY_pixel> GRAY_AXI_STREAM;
typedef ap_ufixed<10, 8, AP_RND> mean_vals;
typedef ap_ufixed<10, 1, AP_RND> weight_vals;
typedef ap_ufixed<14, 14>var_vals;
typedef ap_fixed<32,16> calc_t;
struct lum_gaussian{
	mean_vals mean;
	var_vals var;
	weight_vals weight;
	unsigned char matchsum;
}__attribute__((packed, aligned(2)));
typedef struct lum_gaussian lum_gaussian;
struct pixel_k_gaussian{
	struct {
	lum_gaussian k_lum[3];
	} data;
	unsigned char last;
}__attribute__((packed, aligned(2)));
typedef struct pixel_k_gaussian pixel_k_gaussian;
//typedef pixel_k_gaussian k_luminosity_gaussian[1920*1080];


//slow learning rate
const calc_t LEARNING_RATE (0.05);
const calc_t BG_THRESHOLD (0.97);

//function declaration
void update_gaussian(lum_gaussian &l, unsigned char y, bool matched, weight_vals learn_rate, unsigned char min_var, int frame_count);
int find_match(lum_gaussian l, unsigned char y);
void sort_gaussians(pixel_k_gaussian &pg);
void normalise_weights(pixel_k_gaussian &pg);
lum_gaussian create_new_gaussian(unsigned char y, var_vals var, weight_vals weight);
//void build_gaussian(hls::stream<YUV_pixel> &stream_in , GRAY_AXI_STREAM &stream_out , pixel_k_gaussian MOG[640*480], hls::stream<YUV_pixel> &vid_out, pixel_k_gaussian MOG_out[640*480], weight_vals bg_thresh, weight_vals learning_rate, unsigned char min_var);
void build_gaussian(hls::stream<YUV_pixel> &stream_in , hls::stream<bool> &stream_out , pixel_k_gaussian MOG[640*480], hls::stream<YUV_pixel> &vid_out, pixel_k_gaussian MOG_out[640*480], weight_vals bg_thresh, weight_vals learning_rate, unsigned char min_var);

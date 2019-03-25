#include <hls_video.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

typedef hls::Mat<240, 320, HLS_8UC1> GRAY_IMAGE;
typedef ap_axiu<8,1,1,1> GRAY_PIXEL;
typedef hls::stream< GRAY_PIXEL > GRAY_AXI_STREAM;


#include <hls_video.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <hls_math.h>
#include <ap_fixed.h>
#include <ap_int.h>

typedef ap_ufixed<9, 9> coord;
typedef ap_ufixed<8, 8> uint_8;
typedef ap_ufixed<16, 16> uint_16;
typedef ap_ufixed<1, 1> uint_1;

/* start is inclusive, end is inclusive  */
struct rle_run {
	struct{
		coord s;
		coord e;
		uint_8 no;
		coord y;
		uint_1 _last_run;
	} data;
	bool last;
};
typedef rle_run rle_run;

typedef hls::stream< rle_run > RLE_AXI_STREAM;

void open_and_close(GRAY_AXI_STREAM& INPUT_STREAM, RLE_AXI_STREAM& OUTPUT_STREAM);

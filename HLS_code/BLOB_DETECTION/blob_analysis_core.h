
#include <hls_video.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <hls_math.h>
#include <ap_fixed.h>
#include <ap_int.h>

typedef ap_uint<9> coord;
typedef ap_uint<8> uint_8;
typedef ap_uint<1> uint_1;

struct point {
	coord x;
	coord y;
};
typedef point point;

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

/* start is inclusive, end is exclusive  */
struct rle_line {
	uint_8 no_runs;
	rle_run runs[20];
};
typedef rle_line rle_line;

/* top left, top right, bottom left, bottom right*/
/*struct bounding_box {
	point tl;
	point tr;
	point bl;
	point br;
};
typedef bounding_box bounding_box;
*/

struct blob{
	unsigned char id;
	point cp;
	short area;
	coord max_x;
	coord min_x;
	coord max_y;
	coord min_y;
} __attribute__((packed, alligned(1))); //128 bits due to alignment issues


typedef ap_uint<128> blob_port;

//typedef blob blob;

//function declaration
void blob_analysis(RLE_AXI_STREAM &rle_stream, blob_port objects_port[100]);
void identify_update_objects(rle_line &current, rle_line &previous, unsigned char * ob_id, blob * (&objects_ptr)[100]);
void create_blob(blob * b, unsigned char * ob_id, rle_run current);

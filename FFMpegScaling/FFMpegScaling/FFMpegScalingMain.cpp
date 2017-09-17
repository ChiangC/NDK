extern "C"{
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
};

#define MAX_FRAME_NUM 100

static void fill_yuv_image(uint8_t *data[4], int linesize[4],
	int width, int height, int frame_index)
{
	int x, y;

	/* Y */
	for (y = 0; y < height; y++)
	for (x = 0; x < width; x++)
		data[0][y * linesize[0] + x] = x + y + frame_index * 3;

	/* Cb and Cr */
	for (y = 0; y < height / 2; y++) {
		for (x = 0; x < width / 2; x++) {
			data[1][y * linesize[1] + x] = 128 + y + frame_index * 2;
			data[2][y * linesize[2] + x] = 64 + x + frame_index * 5;
		}
	}

}

static int read_yuv_from_src_file(uint8_t *src_data[4], int src_linesize[4], int src_w, int src_h, int color_plane, FILE *src_file){
	int frame_height = color_plane == 0 ? src_h : src_h / 2;
	int frame_width  = color_plane == 0 ? src_w : src_w / 2;
	int frame_size   = frame_width * frame_height;
	int frame_stride = src_linesize[color_plane];

	if (frame_width == frame_stride){
		//宽度和跨度相等，像素信息连续存放
		fread_s(src_data[color_plane], frame_size, 1, frame_size, src_file);
	}
	else{
		//宽度小于跨度，像素信息保存空间之间存在间隔
		int row_idx;
		for (row_idx = 0; row_idx < frame_height; row_idx++){
			fread_s(src_data[color_plane] + row_idx * frame_stride, frame_width, 1, frame_width, src_file);
		}
	}
	return frame_size;
}

int main(int argc, char **argv){
	uint8_t *src_data[4], *dst_data[4];
	int src_linesize[4], dst_linesize[4];
	int src_w, src_h, dst_w, dst_h;
	enum AVPixelFormat src_pix_fmt = AV_PIX_FMT_YUV420P;
	enum AVPixelFormat dst_pix_fmt = AV_PIX_FMT_YUV420P;
	//enum AVPixelFormat dst_pix_fmt = AV_PIX_FMT_RGB24;
	const char *src_filename = NULL;
	const char *src_size = NULL;
	const char *dst_size = NULL;
	const char *dst_filename = NULL;
	FILE *src_file, *dst_file;
	int dst_bufsize;
	struct SwsContext *sws_ctx;
	int i, ret;

	if (argc != 5) {
		fprintf(stderr, "Usage: %s output_file output_size\n"
			"API example program to show how to scale an image with libswscale.\n"
			"This program generates a series of pictures, rescales them to the given "
			"output_size and saves them to an output file named output_file\n."
			"\n", argv[0]);
		exit(1);
	}

	src_filename = argv[1];
	src_size = argv[2];
	dst_filename = argv[3];
	dst_size = argv[4];

	fopen_s(&src_file, src_filename, "rb+");
	if (!src_file){
		printf("Error: Failed to open input file\n");
		exit(1);
	}

	fopen_s(&dst_file, dst_filename, "wb+");
	if (!dst_file){
		printf("Error: Failed to open output file\n");
		exit(1);
	}

	if (av_parse_video_size(&src_w, &src_h, src_size) < 0){
		printf("Error: parsing input size failed.\n");
		goto end;
	}

	if (av_parse_video_size(&dst_w, &dst_h, dst_size) < 0){
		printf("Error: parsing output size failed.\n");
		goto end;
	}

	/* create scaling context */
	sws_ctx = sws_getContext(src_w, src_h, src_pix_fmt,
							 dst_w, dst_h, dst_pix_fmt,
							 SWS_BILINEAR, NULL, NULL, NULL);

	if (!sws_ctx) {
		fprintf(stderr,
			"Impossible to create scale context for the conversion "
			"fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
			av_get_pix_fmt_name(src_pix_fmt), src_w, src_h,
			av_get_pix_fmt_name(dst_pix_fmt), dst_w, dst_h);
		ret = AVERROR(EINVAL);
		goto end;
	}

	/* allocate source and destination image buffers */
	if ((ret = av_image_alloc(src_data, src_linesize,
							  src_w, src_h, src_pix_fmt, 32)) < 0){
		fprintf(stderr, "Could not allocate source image\n");
		goto end;
	}

	/* buffer is going to be written to rawvideo file, no alignment */
	if ( (ret = av_image_alloc(dst_data, dst_linesize,
							   dst_w, dst_h, dst_pix_fmt, 1))< 0){
		fprintf(stderr, "Could not allocate destination image.\n");
		goto end;
	}

	dst_bufsize = ret;

	for (i = 0; i < MAX_FRAME_NUM; i++){
		/* generate synthetic video */
		//fill_yuv_image(src_data, src_linesize, src_w, src_h, i);
		read_yuv_from_src_file(src_data, src_linesize, src_w, src_h, 0, src_file);
		read_yuv_from_src_file(src_data, src_linesize, src_w, src_h, 1, src_file);
		read_yuv_from_src_file(src_data, src_linesize, src_w, src_h, 2, src_file);

		/* convert to destination format */
		sws_scale(sws_ctx, (const uint8_t * const*)src_data,
				  src_linesize, 0, src_h, dst_data, dst_linesize);

		/* write scaled image to file */
		fwrite(dst_data[0], 1, dst_bufsize, dst_file);
	}
	
	fprintf(stderr, "Scaling succeeded. Play the output file with the command:\n"
		"ffplay -f rawvideo -pix_fmt %s -video_size %dx%d %s\n",
		av_get_pix_fmt_name(dst_pix_fmt), dst_w, dst_h, dst_filename);


end:
	fclose(src_file);
	fclose(dst_file);
	av_freep(&src_data[0]);
	av_freep(&dst_data[0]);
	sws_freeContext(sws_ctx);

	return 0;
}
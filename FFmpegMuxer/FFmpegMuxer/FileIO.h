#ifndef _FILE_IO_H
#define _FILE_IO_H

typedef struct _FileIO
{
	const char *input_filename;
	const char *output_filename;
	int frame_width;
	int frame_height;
}FileIO;

#endif


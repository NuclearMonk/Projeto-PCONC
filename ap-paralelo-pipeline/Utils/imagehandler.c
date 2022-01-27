/**
 * @file imagehandler.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-01-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "imagehandler.h"
#include <stdlib.h>
#include "help.h"
#include "filehandler.h"
#include "general.h"


gdImagePtr read_png_file(char *imgs_path, char *img_name)
{
	char *file = img_path_generator(imgs_path, "", img_name);
	FILE *fp = fopen(file, "rb");
	if (NULL == fp)
	{
		help(FILE_READ_FAIL, file);
		freeNew(file);

		return NULL;
	}

	gdImagePtr read_img = gdImageCreateFromPng(fp);
	fcloseNew(fp);
	if (NULL == read_img)
	{
		help(FILE_READ_FAIL, file);
		freeNew(file);

		return NULL;
	}

	freeNew(file);

	return read_img;
}

inline thread_args *create_thread_args(int thread_id,char *imgs_path,int* pipe_read,int* pipe_write,gdImagePtr watermark,int* ret_pipe)
{
	thread_args *targs = (thread_args *) malloc(sizeof(thread_args));
	if (NULL == targs)
	{
		help(ALLOCATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}
	targs->thread_id= thread_id;
	targs->imgs_path = imgs_path;
	targs->pipe_read=pipe_read;
	targs->pipe_write= pipe_write;
	targs->watermark = watermark;
	targs->ret_pipe=ret_pipe;
	return targs;
}

inline gdImagePtr resize_image(gdImagePtr in_img, int new_width)
{

	gdImagePtr out_img = NULL;
	int width = 0, height = 0, new_height = 0;
	width = in_img->sx;
	height = in_img->sy;
	new_height = new_width * 1.0 / width * height;

	gdImageSetInterpolationMethod(in_img, GD_BILINEAR_FIXED);
	out_img = gdImageScale(in_img, new_width, new_height);
	if (NULL == out_img)
	{
		return NULL;
	}

	return out_img;
}

inline gdImagePtr thumb_image(gdImagePtr in_img, int size)
{
	gdImagePtr out_img = NULL, aux_img = NULL;

	int width = 0, height = 0;
	int new_height = 0, new_width = 0;
	// Get the image's width and height

	width = in_img->sx;
	height = in_img->sy;

	if (height > width)
	{
		new_width = size;
		new_height = new_width * 1.0 / width * height;
	}
	else
	{
		new_height = size;
		new_width = new_height * 1.0 / height * width;
	}

	gdImageSetInterpolationMethod(in_img, GD_BILINEAR_FIXED);
	aux_img = gdImageScale(in_img, new_width, new_height);
	if (NULL == aux_img)
	{
		return NULL;
	}

	gdRect crop_area;
	crop_area.height = size;
	crop_area.width = size;
	crop_area.x = 0;
	crop_area.y = 0;

	out_img = gdImageCrop(aux_img, &crop_area);
	gdImageDestroy(aux_img);

	if (NULL == out_img)
	{
		return NULL;
	}

	return out_img;
}

inline void save_image(gdImagePtr image, char *img_final_path, char *subdirectory, char *img_name)
{
	char *out_file = img_path_generator(img_final_path, subdirectory, img_name);
	FILE *fp = fopen(out_file, "w");
	if (NULL == fp)
	{
		help(FILE_WRITE_FAIL, out_file);

		exit(EXIT_FAILURE);
	}
	gdImagePng(image, fp);
	freeNew(out_file);
	fcloseNew(fp);
}

inline gdImagePtr add_watermark(gdImagePtr original_img, gdImagePtr watermark)
{
	gdImagePtr out_img = NULL;

	int width = 0, height = 0;

	width = watermark->sx;
	height = watermark->sy;

	out_img = gdImageClone(original_img);

	gdImageCopy(out_img, watermark, 0, 0, 0, 0, width, height);

	return out_img;
}

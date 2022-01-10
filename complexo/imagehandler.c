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

struct ImageSet{
	char *imgs_path;
	char **filenames_array;
	gdImagePtr* image_array;
	unsigned int array_length;
	unsigned int start_index;
	unsigned int thread_count;
	gdImagePtr watermark;
};


void *process_image_set_1(void *args)
{
	image_set *set = (image_set *)args;
	gdImagePtr image = NULL, out_image = NULL;
	for (unsigned int i = set->start_index; i < set->array_length; i += set->thread_count)
	{
		printf("%s\n", set->filenames_array[i]);
		image = read_png_file(set->imgs_path, set->filenames_array[i]);
		if (NULL == image)
		{
			continue;
		}

		out_image = resize_image(image, 640);
		gdImageDestroy(image);
		if (NULL == out_image) {
			help(ERR_RESIZE, set->filenames_array[i]);
		} else {
			save_image(out_image, set->imgs_path, RESIZE_DIR, set->filenames_array[i]);
			gdImageDestroy(out_image);
		}
	}

	free(args);

	return NULL;
}
void *process_image_set_2(void *args)
{
	image_set *set = (image_set *)args;
	gdImagePtr image = NULL, out_image = NULL;
	for (unsigned int i = set->start_index; i < set->array_length; i += set->thread_count)
	{
		printf("%s\n", set->filenames_array[i]);
		image = read_png_file(set->imgs_path, set->filenames_array[i]);
		if (NULL == image)
		{
			continue;
		}

		out_image = thumb_image(image, 640);
		if (NULL == out_image) {
			help(ERR_THUMB, set->filenames_array[i]);
		} else {
			save_image(out_image, set->imgs_path, THUMB_DIR, set->filenames_array[i]);
			gdImageDestroy(out_image);
		}

		gdImageDestroy(image);
	}

	free(args);

	return NULL;
}
void *process_image_set_3(void *args)
{
	image_set *set = (image_set *)args;
	gdImagePtr image = NULL, out_image = NULL;
	for (unsigned int i = set->start_index; i < set->array_length; i += set->thread_count)
	{
		printf("%s\n", set->filenames_array[i]);
		image = read_png_file(set->imgs_path, set->filenames_array[i]);
		if (NULL == image)
		{
			continue;
		}

		out_image = add_watermark(image, set->watermark);
		if (NULL == out_image) {
			help(ERR_WATER, set->filenames_array[i]);
		} else {
			save_image(out_image, set->imgs_path, WATER_DIR, set->filenames_array[i]);
			gdImageDestroy(out_image);
		}

		gdImageDestroy(image);
	}

	free(args);

	return NULL;
}


gdImagePtr read_png_file(char *imgs_path, char *img_name)
{

	FILE *fp = NULL;
	gdImagePtr read_img = NULL;
	char *file = imgPathGenerator(imgs_path, "", img_name);
	fp = fopen(file, "rb");

	if (NULL == fp)
	{
		help(FILE_READ_FAIL, file);
		free(file);

		return NULL;
	}

	read_img = gdImageCreateFromPng(fp);
	fclose(fp);

	if (NULL == read_img)
	{
		help(FILE_READ_FAIL, file);
		free(file);

		return NULL;
	}

	free(file);

	return read_img;
}

image_set *create_image_set(char *imgs_path, char **array, unsigned int array_length,
							unsigned int start_index, unsigned int thread_count, gdImagePtr watermark)
{
	image_set *img_set = (image_set *)malloc(sizeof(image_set));
	if (NULL == img_set)
	{
		help(ALLOCATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}

	img_set->imgs_path = imgs_path;
	img_set->filenames_array = array;
	img_set->array_length = array_length;
	img_set->start_index = start_index;
	img_set->thread_count = thread_count;
	img_set->watermark = watermark;

	return img_set;
}

gdImagePtr resize_image(gdImagePtr in_img, int new_width)
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

gdImagePtr thumb_image(gdImagePtr in_img, int size)
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

	if (!out_img)
	{
		return NULL;
	}

	return out_img;
}

void save_image(gdImagePtr image, char *img_final_path, char *subdirectory, char *img_name)
{
	char *out_file = imgPathGenerator(img_final_path, subdirectory, img_name);
	FILE *fp = fopen(out_file, "w");
	if (NULL == fp)
	{
		help(FILE_WRITE_FAIL, out_file);

		return;
	}
	gdImagePng(image, fp);
	free(out_file);
	fclose(fp);
}

gdImagePtr add_watermark(gdImagePtr in_img, gdImagePtr watermark){

	gdImagePtr out_img = NULL;

	int width = 0, height = 0;

	width = watermark->sx;
	height = watermark->sy;

	out_img =  gdImageClone (in_img);

	gdImageCopy(out_img, watermark, 0, 0, 0, 0, width, height);

	return out_img;
}

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
	char *path;
	char **array;
	unsigned int array_length;
	unsigned int start_index;
	unsigned int thread_count;
	gdImagePtr watermark;
};

/**
 * @brief Create a image set object
 *
 * @param path The root path of the target directory
 * @param array The array that holds the filenames
 * @param array_lenght the number of files to handle
 * @param start_index the index the thread should start counting on
 * @param thread_count the number of avaiable threads doing the task
 * @param watermark the image to be used as the watermark
 * @return image_set* the image_set to be passed to the arguments of the thread
 */
image_set *create_image_set(char *path, char **array, unsigned int array_lenght, unsigned int start_index,
							unsigned int thread_count, gdImagePtr watermark)
{
	image_set *img_set = (image_set *)malloc(sizeof(image_set));
	if (NULL == img_set)
	{
		help(ALLOCATTIONION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}
	img_set->path = path;
	img_set->array = array;
	img_set->array_length = array_lenght;
	img_set->start_index = start_index;
	img_set->thread_count = thread_count;
	img_set->watermark = watermark;
	return img_set;
}

/**
 * @brief Resizes an image
 *
 * @param in_img a pointer to a gdImage to be resized
 * @param new_width the new width to change the image
 * @return gdImagePtr the new image, NULL if the scaling failed
 */
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

/**
 * @brief Returns a scalled and cropped to square shape version of the provided image
 *
 * @param in_img
 * @param size
 * @return gdImagePtr the output image
 */
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

/**
 * @brief reads a png file to a gdImage
 *
 * @param file_name the name of the file to open
 * @return gdImagePtr pointer to the created image
 */
gdImagePtr read_png_file(char *path, char *file_name)
{

	FILE *fp = NULL;
	gdImagePtr read_img = NULL;
	char *file = file_path(path, "", file_name);
	fp = fopen(file, "rb");

	if (!fp)
	{
		help(FILE_READ_FAIL, file);

		return NULL;
	}

	read_img = gdImageCreateFromPng(fp);

	fclose(fp);

	if (NULL == read_img)
	{
		help(FILE_READ_FAIL, file);

		return NULL;
	}
	free(file);
	return read_img;
}

/**
 * @brief Saves an image o the specified directory and file
 *
 * @param image the gdImage to save
 * @param directory the path to the destination directory
 * @param filename the final filename
 */
void save_image(gdImagePtr image, char *path, char *subdirectory, char *filename)
{
	char *out_file = file_path(path, subdirectory, filename);
	FILE *fp = fopen(out_file, "w");
	if (!fp)
	{
		help(FILE_WRITE_FAIL, out_file);

		return;
	}
	gdImagePng(image, fp);
	free(out_file);
	fclose(fp);
}

/**
 * @brief
 * Funcao de invocação para os threads de  processamento paralelo
 * Cada thread executa as transformações necessárias sobre um subconjunto do array dados
 * começando na posicao correspondent ao seu thread id e dando leapfrog saltando threadcount imagens de cada vez
 * isto é feito assim para diminuir as discrepancias de trabalho entre threads tal que nenhum thread trabalhe em mais do que uma image que todos os outros
 * @param args
 * Um pointer void que aponta para um struct de tipo image_set
 * @return void*
 */
void *process_image_set(void *args)
{
	gdImagePtr image = NULL, out_image = NULL;
	image_set *set = (image_set *)args;
	for (unsigned int i = set->start_index; i < set->array_length; i += set->thread_count)
	{
		printf("%s\n", set->array[i]);
		image = read_png_file(set->path, set->array[i]);
		if (NULL == image)
		{
			continue;
		}
		out_image = resize_image(image, 640);
		if (NULL != out_image)
		{
			save_image(out_image, set->path, RESIZE_DIR, set->array[i]);
			gdImageDestroy(out_image);
		}
		out_image = thumb_image(image, 640);
		if (NULL != out_image)
		{
			save_image(out_image, set->path, THUMB_DIR, set->array[i]);
			gdImageDestroy(out_image);
		}
		out_image = add_watermark(image, set->watermark);
		if (NULL != out_image)
		{
			save_image(out_image, set->path, WATER_DIR, set->array[i]);
			gdImageDestroy(out_image);
		}
		gdImageDestroy(image);
	}

	free(args);
	return NULL;
}

/**
 * @brief Additively adds a watermark to the image
 *
 * @param in_img
 * @param watermark
 * @return gdImagePtr
 */
gdImagePtr  add_watermark(gdImagePtr in_img, gdImagePtr watermark){

	gdImagePtr out_img = NULL;

	int width = 0, height = 0;

	width = watermark->sx;
	height = watermark->sy;

	out_img =  gdImageClone (in_img);

	gdImageCopy(out_img, watermark, 0, 0, 0, 0, width, height);

	return out_img;
}

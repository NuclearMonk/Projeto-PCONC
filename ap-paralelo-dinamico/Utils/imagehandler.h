/**
 * @file imagehandler.h
 * @author Manuel Soares, Eduardo Faustino
 * @brief
 * @version 0.1
 * @date 2022-01-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef PROJETO_PCONC_IMAGEHANDLER_H
#define PROJETO_PCONC_IMAGEHANDLER_H

#include <gd.h>
#include "stats.h"

typedef struct
{
	int thread_id;
	char *imgs_path;
	int*pipe;
	gdImagePtr watermark;
	int* ret_pipe;
} ThreadParams;

/**
 * @brief Create a image set object
 *
 * @param imgs_path The root path of the target directory
 * @param array The filenames_array that holds the filenames
 * @param array_length the number of files to handle
 * @param start_index the index the thread should start counting on
 * @param thread_count the number of avaiable threads doing the task
 * @param watermark the image to be used as the watermark
 *
 * @return ThreadParams* the ThreadParams to be passed to the arguments of the thread
 */
ThreadParams *create_ThreadParams(int thread_id, char *imgs_path, int* pipe, gdImagePtr watermark,int* ret_pipe); // __attribute__((nonnull));

/**
 * @brief reads a png file to a gdImage
 *
 * @param imgs_path
 * @param file_name the name of the file to open
 *
 * @return gdImagePtr pointer to the created image
 */
gdImagePtr read_png_file(char *imgs_path, char *file_name) __attribute__((nonnull));

/**
 * @brief Resizes an image
 *
 * @param in_img a pointer to a gdImage to be resized
 * @param new_width the new width to change the image
 *
 * @return gdImagePtr the new image, NULL if the scaling failed
 */
gdImagePtr resize_image(gdImagePtr in_img, int new_width) __attribute__((nonnull));

/**
 * @brief Returns a scaled and cropped to square shape version of the provided image
 *
 * @param in_img
 * @param size
 *
 * @return gdImagePtr the output image
 */
gdImagePtr thumb_image(gdImagePtr in_img, int size) __attribute__((nonnull));

/**
 * @brief Saves an image o the specified imgs_path and file
 *
 * @param image the gdImage to save
 * @param img_final_path the imgs_path to the destination imgs_path
 * @param filename the final filename
 */
void save_image(gdImagePtr image, char *img_final_path, char *subdirectory, char *img_name) __attribute__((nonnull));

/**
 * @brief Add a watermark to the image
 *
 * @param original_img the original imagem
 * @param watermark the watermark image
 *
 * @return the new image
 */
gdImagePtr add_watermark(gdImagePtr original_img, gdImagePtr watermark) __attribute__((nonnull));


#endif //PROJETO_PCONC_IMAGEHANDLER_H

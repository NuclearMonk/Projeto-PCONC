/**
 * @file filehandler.h
 * @author Manuel Soares, Eduardo Faustino
 * @brief
 * @version 0.1
 * @date 2022-01-06
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef PROJETO_PCONC_FILEHANDLER_H
#define PROJETO_PCONC_FILEHANDLER_H

#define RESIZE_DIR "Resize/"
#define THUMB_DIR "Thumbnail/"
#define WATER_DIR "Watermark/"

/**
 * @brief Lists all PNG files in a directory.
 * Given an images files path, this function looks for a file named "img-process-list.txt" file in which every line has
 * a file image name (with extension), and returns the number of files found, also filling an array (allocated
 * internally) with the names of the files found.
 *
 * @param imgs_path the path of the images to explore
 * @param img_names a pointer (initialized to NULL) to a char** array where to store the image file names
 *
 * @return the number of files found
 */
int list_pngs(char *imgs_path, char ***img_names) __attribute__((nonnull));

/**
 * @brief Creates all output directories
 *
 * @param output_path the base output path
 */
void create_output_directories(char *output_path) __attribute__((nonnull));

/**
 * @brief Builds a string based on the provided base path, a subdirectory and the file name.
 * Its output is of the way "path/subdirectory/filename".
 *
 * @param imgs_path the base path
 * @param subdirectory the subdirectory
 * @param filename the file name
 *
 * @return the full file path
 */
char *img_path_generator(char *imgs_path, char *subdirectory, char *img_name) __attribute__((nonnull));

#endif //PROJETO_PCONC_FILEHANDLER_H

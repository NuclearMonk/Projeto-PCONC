/**
 * @file filehandler.c
 * @author Manuel Soares,Eduardo Faustino
 * @brief
 * @version 0.1
 * @date 2022-01-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "filehandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "help.h"
#include "utils.h"

static inline bool imgExistsInOutputDirs(char *imgs_path, char* img_file_name) __attribute__((nonnull));
static inline bool create_directory(char *dir_path) __attribute__((nonnull));

inline int list_pngs(char *imgs_path, char ***img_names) {
	char img_name[256];
	char *imgs_file_path = img_path_generator(imgs_path, "", "img-process-list.txt");
	FILE *file = fopen(imgs_file_path, "r");
	freeNew(imgs_file_path);
	if (NULL == file) {
		help(FILE_NOT_FOUND, NULL);

		exit(EXIT_FAILURE);
	}

	int filecount = 0;
	while (1 == fscanf(file, "%255s ", img_name))
	{
		if (imgExistsInOutputDirs(imgs_path, img_name)) {
			printf("File \"%s\" already exists - ignoring it.\n", img_name);
		} else {
			++filecount;
		}
	}
	if (0 == filecount) {
		fcloseNew(file);

		return 0;
	}

	rewind(file);

	*img_names = (char **) malloc(filecount * sizeof(char *));
	if (NULL == *img_names) {
		help(ALLOCATION_FAIL, NULL);
		fcloseNew(file);

		exit(EXIT_FAILURE);
	}

	for (int i = 0; (1 == fscanf(file, "%255s ", img_name)) && (i < filecount); /*Nada*/) {
		if (imgExistsInOutputDirs(imgs_path, img_name)) {
			continue;
		}

		int string_length = strlen(img_name) + 1;
		(*img_names)[i] = (char *) malloc(string_length * sizeof(char));
		if (NULL == (*img_names)[i]) {
			help(ALLOCATION_FAIL, NULL);
			for (int j = 0; j < i; ++j) {
				freeNew((*img_names)[i]);
			}

			exit(EXIT_FAILURE);
		}
		strncpy((*img_names)[i], img_name, string_length);

		++i;
	}

	fcloseNew(file);

	return filecount;
}

inline void create_output_directories(char *output_path)
{
	char folders[3][11] = {RESIZE_DIR, THUMB_DIR, WATER_DIR};
	for (int i = 0; i < 3; ++i) {
		char *operation_result_path = (char *) malloc((strlen(output_path) + 1 + (strlen(folders[i]) + 1)) * sizeof(char));
		if (NULL == operation_result_path) {
			help(ALLOCATION_FAIL, NULL);

			exit(EXIT_FAILURE);
		}
		sprintf(operation_result_path, "%s/%s", output_path, folders[i]);
		bool create_dir_success = create_directory(operation_result_path);
		freeNew(operation_result_path);
		if (!create_dir_success) {
			help(DIR_CREATION_FAIL, NULL);

			exit(EXIT_FAILURE);
		}
	}
}

inline char *img_path_generator(char *imgs_path, char *subdirectory, char *img_name)
{
	int filename_len = strlen(imgs_path) + 1 + strlen(subdirectory) + strlen(img_name) + 1;
	char *file_path = (char *) malloc(filename_len * sizeof(char));
	if (NULL == file_path)
	{
		help(ALLOCATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}
	sprintf(file_path, "%s/%s%s", imgs_path, subdirectory, img_name);

	return file_path;
}


/**
 * @brief Checks if an image file exists already in the 3 output directories.
 *
 * @param imgs_path the images main path
 * @param img_file_name the complete name of the image file
 *
 * @return true if the file exists in all the directories, false otherwise
 */
static inline bool imgExistsInOutputDirs(char *imgs_path, char *img_file_name)
{
	char folders[3][11] = {RESIZE_DIR, THUMB_DIR, WATER_DIR};
	for (int i = 0; i < 3; ++i) {
		char *img_file_path = img_path_generator(imgs_path, folders[i], img_file_name);
		bool file_exists = (0 == access(img_file_path, F_OK)) ? true : false;
		freeNew(img_file_path);
		if (!file_exists) {
			return false;
		}
	}

	return true;
}

/**
 * @brief Checks if a given directory exists, and in case it doesn't, the function creates it.
 *
 * @param dir_path the complete directory dir_path
 *
 * @return true if the directory exists, false if its creation failed
 */
static inline bool create_directory(char *dir_path)
{
	DIR *dir = opendir(dir_path);
	if (NULL == dir)
	{
		if (mkdir(dir_path, 0777) != 0)
		{
			return false;
		}
	}
	else
	{
		closedir(dir);
	}

	return true;
}

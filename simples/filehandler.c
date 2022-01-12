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

static bool imgExistsInOutputDirs(char *imgs_path, char* img_file_name) __attribute__((nonnull(1,2)));
static bool create_directory(char *path) __attribute__((nonnull));

int list_pngs(char *imgs_path, char ***img_names)
{
	char img_name[256];
	char *imgs_file_path = imgPathGenerator(imgs_path, "", "img-process-list.txt");
	FILE *file = fopen(imgs_file_path, "r");
	free(imgs_file_path);
	if (NULL == file)
	{
		help(FILE_NOT_FOUND, NULL);

		exit(EXIT_FAILURE);
	}

	int filecount = 0;
	while (1 == fscanf(file, "%255s ", img_name))
	{
		if (imgExistsInOutputDirs(imgs_path, img_name))
		{
			printf("File \"%s\" already exists - ignoring it.\n", img_name);
		}
		else
		{
			++filecount;
		}
	}
	if (0 == filecount)
	{
		fclose(file);

		return 0;
	}

	rewind(file);

	*img_names = (char **)malloc(filecount * sizeof(char *));
	if (NULL == *img_names)
	{
		help(ALLOCATION_FAIL, NULL);
		fclose(file);

		exit(EXIT_FAILURE);
	}

	int i = 0;
	while ((1 == fscanf(file, "%255s ", img_name)) && (i < filecount))
	{
		if (imgExistsInOutputDirs(imgs_path, img_name))
		{
			continue;
		}

		int string_length = strlen(img_name) + 1;
		(*img_names)[i] = (char *)malloc(string_length * sizeof(char));
		if (NULL == (*img_names)[i])
		{
			help(ALLOCATION_FAIL, NULL);

			exit(EXIT_FAILURE);
		}
		strncpy((*img_names)[i], img_name, string_length);

		++i;
	}

	fclose(file);

	return filecount;
}

/**
 * @brief Checks if an image file exists already in the 3 output directories
 *
 * @param imgs_path the main program path
 * @param img_file_name the name of the file image, including extension
 *
 * @return true if the file exists in all the directories, false otherwise
 */
static bool imgExistsInOutputDirs(char *imgs_path, char *img_file_name)
{
	char *img_file_path = imgPathGenerator(imgs_path, RESIZE_DIR, img_file_name);
	bool file_exists = (0 == access(img_file_path, F_OK)) ? true : false;
	free(img_file_path);
	if (!file_exists)
	{
		return false;
	}

	img_file_path = imgPathGenerator(imgs_path, THUMB_DIR, img_file_name);
	file_exists = (0 == access(img_file_path, F_OK)) ? true : false;
	free(img_file_path);
	if (!file_exists)
	{
		return false;
	}

	img_file_path = imgPathGenerator(imgs_path, WATER_DIR, img_file_name);
	file_exists = (0 == access(img_file_path, F_OK)) ? true : false;
	free(img_file_path);
	if (!file_exists)
	{
		return false;
	}

	return true;
}

/**
 * @brief Checks if a given directory exists, if it doesn't it creates it
 *
 * @param path string with the target directory path
 *
 * @return true if the directory exists, false if the creation of the directory failed
 */
static bool create_directory(char *path)
{
	DIR *dir = opendir(path);
	if (NULL == dir)
	{
		if (mkdir(path, 0777) != 0)
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

void create_output_directories(char *output_path)
{
	char *resize_result_path = (char *)malloc((strlen(output_path) + 1 + (strlen(RESIZE_DIR) + 1)) * sizeof(char));
	if (NULL == resize_result_path)
	{
		help(ALLOCATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}
	sprintf(resize_result_path, "%s/%s", output_path, RESIZE_DIR);
	if (!create_directory(resize_result_path))
	{
		help(DIR_CREATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}

	char *thumb_result_path = (char *)malloc((strlen(output_path) + 1 + (strlen(THUMB_DIR) + 1)) * sizeof(char));
	if (NULL == thumb_result_path)
	{
		help(ALLOCATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}
	sprintf(thumb_result_path, "%s/%s", output_path, THUMB_DIR);
	if (!create_directory(thumb_result_path))
	{
		help(DIR_CREATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}

	char *water_result_path = (char *)malloc((strlen(output_path) + 1 + (strlen(WATER_DIR) + 1)) * sizeof(char));
	if (NULL == water_result_path)
	{
		help(ALLOCATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}
	sprintf(water_result_path, "%s/%s", output_path, WATER_DIR);
	if (!create_directory(water_result_path))
	{
		help(DIR_CREATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}
	free(resize_result_path);
	free(thumb_result_path);
	free(water_result_path);
}

inline char *imgPathGenerator(char *imgs_path, char *subdirectory, char *img_name)
{
	int filename_len = strlen(imgs_path) + 1 + strlen(subdirectory) + strlen(img_name) + 1;
	char *file_path = (char *)malloc(filename_len * sizeof(char));
	if (NULL == file_path)
	{
		help(ALLOCATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}
	sprintf(file_path, "%s/%s%s", imgs_path, subdirectory, img_name);

	return file_path;
}

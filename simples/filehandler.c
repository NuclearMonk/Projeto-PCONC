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



int list_pngs(char *imgs_path, char ***img_names)
{
	char img_name[256];
	char *imgs_file_path = imgPathGenerator(imgs_path, "", "img-process-list.txt");
	FILE *file = fopen(imgs_file_path, "r");
	free(imgs_file_path);
	if (NULL == file) {
		help(FILE_NOT_FOUND, NULL);

		exit(EXIT_FAILURE);
	}

	int filecount = 0;
	while (1 == fscanf(file, "%255s ", img_name)) {
		if (imgExistsInOutputDirs(imgs_path, img_name)) {
			printf("File \"%s\" already exists - ignoring it.\n", img_name);
		} else {
			++filecount;
		}
	}
	if (0 == filecount) {
		fclose(file);

		return 0;
	}

	rewind(file);

	*img_names = (char**)malloc(filecount * sizeof(char *));
	if (NULL == *img_names) {
		help(ALLOCATION_FAIL, NULL);
		fclose(file);

		return -2;
	}

	int i = 0;
	while ((1 == fscanf(file, "%255s ", img_name)) && (i < filecount)) {
		if (imgExistsInOutputDirs(imgs_path, img_name)) {
			continue;
		}

		int string_length = strlen(img_name) + 1;
		(*img_names)[i] = (char*)malloc(string_length * sizeof(char));
		strncpy((*img_names)[i], img_name, string_length);

		++i;
	}

	fclose(file);

	return filecount;
}

bool imgExistsInOutputDirs(char *imgs_path, char* img_file_name) {
	char *img_file_path = imgPathGenerator(imgs_path, RESIZE_DIR, img_file_name);
	bool file_exists = (0 == access(img_file_path, F_OK)) ? true : false;
	free(img_file_path);
	if (!file_exists) {
		return false;
	}

	img_file_path = imgPathGenerator(imgs_path, THUMB_DIR, img_file_name);
	file_exists = (0 == access(img_file_path, F_OK)) ? true : false;
	free(img_file_path);
	if (!file_exists) {
		return false;
	}

	img_file_path = imgPathGenerator(imgs_path, WATER_DIR, img_file_name);
	file_exists = (0 == access(img_file_path, F_OK)) ? true : false;
	free(img_file_path);
	if (!file_exists) {
		return false;
	}

	return true;
}

bool create_directory(char *path)
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
	sprintf(resize_result_path, "%s/%s", output_path, RESIZE_DIR);
	if (!create_directory(resize_result_path))
	{
		help(DIR_CREATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}

	char *thumb_result_path = (char *)malloc((strlen(output_path) + 1 + (strlen(THUMB_DIR) + 1)) * sizeof(char));
	sprintf(thumb_result_path, "%s/%s", output_path, THUMB_DIR);
	if (!create_directory(thumb_result_path))
	{
		help(DIR_CREATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}

	char *water_result_path = (char *)malloc((strlen(output_path) + 1 + (strlen(WATER_DIR) + 1)) * sizeof(char));
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
	sprintf(file_path, "%s/%s%s", imgs_path, subdirectory, img_name);

	return file_path;
}

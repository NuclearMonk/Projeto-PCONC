/**hopefully my virtual assistant will improve it even more when it really starts to assist. maybe would publish it on XDA if I see it's worth it (if it goes deep enough in the system to be worthy XD)
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



int list_pngs(char *base_path, char ***filenames)
{
	char filename[256];
	char *imgs_file_path = file_path(base_path, "", "img-process-list.txt");
	FILE *file = fopen(imgs_file_path, "r");
	free(imgs_file_path);
	if (NULL == file) {
		help(FILE_NOT_FOUND, NULL);

		exit(EXIT_FAILURE);
	}

	int filecount = 0;
	while (1 == fscanf(file, "%255s ", filename)) {
		if (!imgExistsInOutputDirs(base_path, filename)) {
			++filecount;
		} else {
			printf("File \"%s\" already exists - ignoring it.\n", filename);
		}
	}
	if (0 == filecount) {
		fclose(file);

		return 0;
	}

	rewind(file);

	*filenames = malloc(filecount * sizeof(char *));
	if (NULL == *filenames) {
		help(ALLOCATION_FAIL, NULL);
		fclose(file);

		return -2;
	}
	for (int i = 0; 1 == fscanf(file, "%255s ", filename); ++i) {
		if (imgExistsInOutputDirs(base_path, filename)) {
			continue;
		}
		int string_length = strlen(filename) + 1;
		(*filenames)[i] = malloc(string_length * sizeof(char));
		strncpy((*filenames)[i], filename, string_length);
	}

	fclose(file);

	return filecount;
}

bool imgExistsInOutputDirs(char *base_path, char* img_file_name) {
	char *img_file_path = file_path(base_path, RESIZE_DIR, img_file_name);
	bool file_exists = (0 == access(img_file_path, F_OK)) ? true : false;
	free(img_file_path);
	if (!file_exists) {
		return false;
	}

	img_file_path = file_path(base_path, THUMB_DIR, img_file_name);
	file_exists = (0 == access(img_file_path, F_OK)) ? true : false;
	free(img_file_path);
	if (!file_exists) {
		return false;
	}

	img_file_path = file_path(base_path, WATER_DIR, img_file_name);
	file_exists = (0 == access(img_file_path, F_OK)) ? true : false;
	free(img_file_path);
	if (!file_exists) {
		return false;
	}

	return true;
}

bool create_directory(char *base_path)
{
	DIR *dir = opendir(base_path);
	if (NULL == dir)
	{
		if (mkdir(base_path, 0777) != 0)
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

void create_output_directories(char *base_path)
{
	char *resize_result_path = (char *)malloc((strlen(base_path) + 1 + (strlen(RESIZE_DIR) + 1)) * sizeof(char));
	sprintf(resize_result_path, "%s/%s", base_path, RESIZE_DIR);
	if (!create_directory(resize_result_path))
	{
		help(DIR_CREATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}

	char *thumb_result_path = (char *)malloc((strlen(base_path) + 1 + (strlen(THUMB_DIR) + 1)) * sizeof(char));
	sprintf(thumb_result_path, "%s/%s", base_path, THUMB_DIR);
	if (!create_directory(thumb_result_path))
	{
		help(DIR_CREATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}

	char *water_result_path = (char *)malloc((strlen(base_path) + 1 + (strlen(WATER_DIR) + 1)) * sizeof(char));
	sprintf(water_result_path, "%s/%s", base_path, WATER_DIR);
	if (!create_directory(water_result_path))
	{
		help(DIR_CREATION_FAIL, NULL);

		exit(EXIT_FAILURE);
	}
	free(resize_result_path);
	free(thumb_result_path);
	free(water_result_path);
}

inline char *file_path(char *base_path, char *subdirectory, char *filename)
{
	int filename_len = strlen(base_path) + 1 + strlen(subdirectory) + strlen(filename) + 1;
	char *file_path = (char *)malloc(filename_len * sizeof(char));
	sprintf(file_path, "%s/%s%s", base_path, subdirectory, filename);
	return file_path;
}

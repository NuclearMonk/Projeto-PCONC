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
#include <sys/stat.h>
#include <dirent.h>
#include "help.h"



/**
 * @brief verifica a extençao de um ficheiro dado o seu nome
 *        Retorna falso se o ficheiro for composto apenas pela extenção, mesmo que a extensão seja a correta
 * @param filename nome do ficheiro a verificar
 * @param extention extenção contra a qual estamos a comparar
 * @return true
 * @return false
 */
bool check_file_ext(char *filename, char *extention)
{
    const char *ext_dot = strrchr(filename, '.');
    if (NULL == ext_dot)
    {
        return false;
    }
    if (ext_dot == filename)
    {
        return false;
    }
    if (strcasecmp(ext_dot, extention) == 0)
    {
        return true;
    }
    return false;
}

/**
 * @brief
 * Dado o string para um path retorna o numero de ficheiros png dentro desse path, retorna tambem um array com os nomes desses ficheiros
 * @param path o path a explorar
 * @param filenames um ponteiro onde guardar o array de string com o nome de todos os ficheiros .png
 * @return int o numero de ficheiros png encontrado
 */
int list_pngs(char *path, char ***filenames)
{
    char filename[256];
	char *imgs_file_path = file_path(path, "", "img-process-list.txt");
	FILE *file = fopen(imgs_file_path, "r");
    free(imgs_file_path);
	if (NULL == file) {
		return -1;
	}

	int filecount = 0;
	while (1 == fscanf(file, "%255s ", filename)) {
		++filecount;
	}
	if (0 == filecount) {
		help(FILE_NOT_FOUND, NULL);
		fclose(file);

		return 0;
	}

	rewind(file);

	*filenames = malloc(filecount * sizeof(char *));
	if (NULL == *filenames) {
		help(ALLOCATTIONION_FAIL, NULL);
		fclose(file);

		return -2;
	}
	for (int i = 0; 1 == fscanf(file, "%255s ", filename); ++i) {
		int string_length = strlen(filename) + 1;
		(*filenames)[i] = malloc(string_length * sizeof(char));
		strncpy((*filenames)[i], filename, string_length);
	}

	fclose(file);

	return filecount;
}

/**
 * @brief Checks if a given directory exists, if it doesn't it creates it
 *
 * @param path string with the target directory path
 * @return true if the directory exists
 * @return false if the creation of the directory failed
 */
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

/**
 * @brief Create all output directories
 *
 * @param path the base path
 */
void create_output_directories(char *path)
{
    char *resize_result_path = (char *)malloc((strlen(path) + 1 + (strlen(RESIZE_DIR) + 1)) * sizeof(char));
    char *thumb_result_path = (char *)malloc((strlen(path) + 1 + (strlen(THUMB_DIR) + 1)) * sizeof(char));
    char *water_result_path = (char *)malloc((strlen(path) + 1 + (strlen(WATER_DIR) + 1)) * sizeof(char));
    sprintf(resize_result_path, "%s/%s", path, RESIZE_DIR);
    sprintf(thumb_result_path, "%s/%s", path, THUMB_DIR);
    sprintf(water_result_path, "%s/%s", path, WATER_DIR);
    if (!create_directory(resize_result_path))
    {
        help(DIR_CREATION_FAIL, NULL);
    }
    if (!create_directory(thumb_result_path))
    {
        help(DIR_CREATION_FAIL, NULL);
    }
    if (!create_directory(water_result_path))
    {
        help(DIR_CREATION_FAIL, NULL);
    }
    free(resize_result_path);
    free(thumb_result_path);
    free(water_result_path);
}


/**
 * @brief Builds a string based on the provided base path, a subdirectory and the file name
 * path/subdirectory/filename
 * @param path the base path
 * @param subdirectory the target subdirectory
 * @param filename the target filename
 * @return char* the full filepath
 */
inline char *file_path(char *path, char *subdirectory, char *filename)
{
    int filename_len = strlen(path) + 1 + strlen(subdirectory) + strlen(filename) + 1;
    char *file_path = (char *)malloc(filename_len * sizeof(char));
    sprintf(file_path, "%s/%s%s", path, subdirectory, filename);
    return file_path;
}

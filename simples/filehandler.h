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

#include <stdbool.h>

#define RESIZE_DIR "Resize/"
#define THUMB_DIR "Thumbnail/"
#define WATER_DIR "Watermark/"

/**
 * @brief
 * Dado o string para um imgs_path retorna o numero de ficheiros png dentro desse imgs_path, retorna tambem um
 * filenames_array com os nomes desses ficheiros
 *
 * @param imgs_path o imgs_path a explorar
 * @param filenames um ponteiro onde guardar o filenames_array de string com o nome de todos os ficheiros .png
 *
 * @return int o numero de ficheiros png encontrado, -1 se não foi possível abrir o ficheiro da lista de imagens, ou -2
 * se houve um erro a alocar memória
 */
int list_pngs(char *imgs_path, char ***filenames) __attribute__((nonnull));

/**
 * @brief Checks if an image file exists already in the 3 output directories
 *
 * @param imgs_path the main program path
 * @param img_file_name the name of the file image, including extension
 *
 * @return true if the file exists in all the directories, false otherwise
 */
bool imgExistsInOutputDirs(char *imgs_path, char* img_file_name) __attribute__((nonnull(1,2)));

/**
 * @brief Checks if a given directory exists, if it doesn't it creates it
 *
 * @param path string with the target directory path
 *
 * @return true if the directory exists, false if the creation of the directory failed
 */
bool create_directory(char *path) __attribute__((nonnull));

/**
 * @brief Create all output directories
 *
 * @param output_path the base path
 */
void create_output_directories(char *output_path) __attribute__((nonnull));

/**
 * @brief Builds a string based on the provided base path, a subdirectory and the file name
 * path/subdirectory/filename
 *
 * @param imgs_path the base path
 * @param subdirectory the target subdirectory
 * @param filename the target filename
 *
 * @return char* the full filepath
 */
char *imgPathGenerator(char *imgs_path, char *subdirectory, char *img_name) __attribute__((nonnull, returns_nonnull));

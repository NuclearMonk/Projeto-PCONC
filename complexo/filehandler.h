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

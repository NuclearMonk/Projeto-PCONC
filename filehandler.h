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
 * @brief verifica a extençao de um ficheiro dado o seu nome
 *        Retorna falso se o ficheiro for composto apenas pela extenção, mesmo que a extensão seja a correta
 * @param filename nome do ficheiro a verificar
 * @param extention extenção contra a qual estamos a comparar
 * @return true 
 * @return false 
 */
bool check_file_ext(char *filename, char *extention);

/**
 * @brief Builds a string based on the provided base path, a subdirectory and the file name
 * path/subdirectory/filename
 * @param path the base path
 * @param subdirectory the target subdirectory
 * @param filename the target filename
 * @return char* the full filepath
 */
char *file_path(char *path, char *subdirectory, char *filename);

/**
 * @brief 
 * Dado o string para um path retorna o numero de ficheiros png dentro desse path, retorna tambem um array com os nomes desses ficheiros
 * @param path o path a explorar
 * @param filenames um ponteiro onde guardar o array de string com o nome de todos os ficheiros .png
 * @return int o numero de ficheiros png encontrado
 */
int list_pngs(char *path, char ***filenames);


/**
 * @brief Create all output directories
 * 
 * @param path the base path
 */
void create_output_directories(char *path);
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

#include <gd.h>

typedef struct ImageSet image_set;


image_set *create_image_set(char *path, char **array, unsigned int array_lenght, unsigned int start_index, unsigned int thread_count, gdImagePtr watermark);

gdImagePtr resize_image(gdImagePtr in_img, int new_width);


gdImagePtr thumb_image(gdImagePtr in_img, int size);

/**
 * @brief reads a png file to a gdImage
 * 
 * @param file_name the name of the file to open
 * @return gdImagePtr pointer to the created image
 */
gdImagePtr read_png_file(char *path, char *file_name);

/**
 * @brief Saves an image o the specified directory and file
 * 
 * @param image the gdImage to save
 * @param directory the path to the destination directory
 * @param filename the final filename
 */
void save_image(gdImagePtr image, char *path, char *subdirectory, char *filename);

/**
 * @brief Additively adds a watermark to the image
 * 
 * @param in_img 
 * @param watermark 
 * @return gdImagePtr 
 */
gdImagePtr  add_watermark(gdImagePtr in_img, gdImagePtr watermark);

/**
 * @brief 
 * Funcao de invocação para os threads de  processamento paralelo
 * Cada thread executa as transformações necessárias sobre um subconjunto do array dados
 * começando na posicao correspondent ao seu thread id e dando leapfrog saltando threadcount imagens de cada vez
 * isto é feito assim para diminuir as discrepancias de trabalho entre threads tal que nenhum thread trabalhe em mais do que uma image que todos os outros
 * @param args
 * Um pointer void que aponta para um struct de tipo image_set
 * @return void* 
 */
void *process_image_set(void *args);


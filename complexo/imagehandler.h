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
#include "stats.h"

typedef struct ImageSet image_set;


/**
 * @brief Create a image set object
 *
 * @param imgs_path The root path of the target directory
 * @param array The filenames_array that holds the filenames
 * @param array_length the number of files to handle
 * @param start_index the index the thread should start counting on
 * @param thread_count the number of avaiable threads doing the task
 * @param watermark the image to be used as the watermark
 *
 * @return image_set* the image_set to be passed to the arguments of the thread
 */
image_set *create_image_set(char *imgs_path, char **array,gdImagePtr * image_array, unsigned int array_length, unsigned int start_index,
							unsigned int thread_count, gdImagePtr watermark,timer_data* thread_timers,timer_data* image_timers) __attribute__((nonnull, returns_nonnull));



gdImagePtr * create_image_array(int count);

/**
 * @brief reads a png file to a gdImage
 *
 * @param imgs_path
 * @param file_name the name of the file to open
 *
 * @return gdImagePtr pointer to the created image
 */
gdImagePtr read_png_file(char *imgs_path, char *file_name) __attribute__((nonnull));

/**
 * @brief
 * Funcao de invocação para os threads de  processamento complexo
 * Cada thread executa as transformações necessárias sobre um subconjunto do filenames_array dados
 * começando na posicao correspondent ao seu thread id e dando leapfrog saltando threadcount imagens de cada vez
 * isto é feito assim para diminuir as discrepancias de trabalho entre threads tal que nenhum thread trabalhe em mais do
 * que uma image que todos os outros
 *
 * @param args Um pointer void que aponta para um struct de tipo image_set
 *
 * @return nada
 */
void *process_image_set_1(void *args) __attribute__((nonnull));

/**
 * Read process_image_set_1()'s documentation.
 *
 * @param args
 * @return
 */
void *process_image_set_2(void *args) __attribute__((nonnull));

/**
 * Read process_image_set_1()'s documentation.
 *
 * @param args
 * @return
 */
void *process_image_set_3(void *args) __attribute__((nonnull));

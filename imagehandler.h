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


/**
 * @brief Create a image set object
 *
 * @param base_path The root path of the target directory
 * @param array The array that holds the filenames
 * @param array_length the number of files to handle
 * @param start_index the index the thread should start counting on
 * @param thread_count the number of avaiable threads doing the task
 * @param watermark the image to be used as the watermark
 *
 * @return image_set* the image_set to be passed to the arguments of the thread
 */
image_set *create_image_set(char *base_path, char **array, unsigned int array_length, unsigned int start_index,
							unsigned int thread_count, gdImagePtr watermark);

/**
 * @brief Resizes an image
 *
 * @param in_img a pointer to a gdImage to be resized
 * @param new_width the new width to change the image
 *
 * @return gdImagePtr the new image, NULL if the scaling failed
 */
gdImagePtr resize_image(gdImagePtr in_img, int new_width);

/**
 * @brief Returns a scalled and cropped to square shape version of the provided image
 *
 * @param in_img
 * @param size
 *
 * @return gdImagePtr the output image
 */
gdImagePtr thumb_image(gdImagePtr in_img, int size);

/**
 * @brief reads a png file to a gdImage
 *
 * @param base_path
 * @param file_name the name of the file to open
 *
 * @return gdImagePtr pointer to the created image
 */
gdImagePtr read_png_file(char *base_path, char *file_name);

/**
 * @brief Saves an image o the specified base_path and file
 *
 * @param image the gdImage to save
 * @param base_path the base_path to the destination base_path
 * @param filename the final filename
 */
void save_image(gdImagePtr image, char *base_path, char *subdirectory, char *filename);

/**
 * @brief
 * Funcao de invocação para os threads de  processamento paralelo
 * Cada thread executa as transformações necessárias sobre um subconjunto do array dados
 * começando na posicao correspondent ao seu thread id e dando leapfrog saltando threadcount imagens de cada vez
 * isto é feito assim para diminuir as discrepancias de trabalho entre threads tal que nenhum thread trabalhe em mais do que uma image que todos os outros
 *
 * @param args
 *
 * @return Um pointer void que aponta para um struct de tipo image_set
 */
void *process_image_set(void *args);

/**
 * @brief Additively adds a watermark to the image
 *
 * @param in_img
 * @param watermark
 *
 * @return gdImagePtr
 */
gdImagePtr add_watermark(gdImagePtr in_img, gdImagePtr watermark);


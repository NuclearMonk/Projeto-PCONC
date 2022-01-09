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
 * @param imgs_path The root path of the target directory
 * @param array The filenames_array that holds the filenames
 * @param array_length the number of files to handle
 * @param start_index the index the thread should start counting on
 * @param thread_count the number of avaiable threads doing the task
 * @param watermark the image to be used as the watermark
 *
 * @return image_set* the image_set to be passed to the arguments of the thread
 */
image_set *create_image_set(char *imgs_path, char **array, unsigned int array_length, unsigned int start_index,
							unsigned int thread_count, gdImagePtr watermark) __attribute__((nonnull, returns_nonnull));


/**
 * @brief 
 * 
 * @param imgs_path the path to the target directory
 * @param filenames an array filled with the filenames of the files to be opened
 * @param file_count the number of entries in the filenames array
 * @return gdImagePtr* Array with pointers to the corresponding gdImages that were just opened
 */
gdImagePtr * read_all_png_files(char* imgs_path,char** filenames, int file_count);

/**
 * @brief Frees an array of images
 * 
 * @param images the array of images
 * @param image_count the number of entries in the array
 */
void free_all_images(gdImagePtr* images, int image_count);

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
void *process_image_set(void *args) __attribute__((nonnull));

/**
 * @brief Resizes an image
 *
 * @param in_img a pointer to a gdImage to be resized
 * @param new_width the new width to change the image
 *
 * @return gdImagePtr the new image, NULL if the scaling failed
 */
gdImagePtr resize_image(gdImagePtr in_img, int new_width) __attribute__((nonnull));

/**
 * @brief Saves an image o the specified imgs_path and file
 *
 * @param image the gdImage to save
 * @param img_final_path the imgs_path to the destination imgs_path
 * @param filename the final filename
 */
void save_image(gdImagePtr image, char *img_final_path, char *subdirectory, char *img_name) __attribute__((nonnull));

/**
 * @brief Additively adds a watermark to the image
 *
 * @param in_img
 * @param watermark
 *
 * @return gdImagePtr
 */
gdImagePtr add_watermark(gdImagePtr in_img, gdImagePtr watermark) __attribute__((nonnull, returns_nonnull));

/**
 * @brief Returns a scalled and cropped to square shape version of the provided image
 *
 * @param in_img
 * @param size
 *
 * @return gdImagePtr the output image
 */
gdImagePtr thumb_image(gdImagePtr in_img, int size) __attribute__((nonnull));

/**
 * @file imagehandler.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "imagehandler.h"

#include <stdlib.h>
#include "help.h"
#include "filehandler.h"

struct ImageSet{
    char *path;
    char **array;
    unsigned int array_lenght;
    unsigned int start_index;
    unsigned int thread_count;
};


image_set *create_image_set(char *path, char **array, unsigned int array_lenght, unsigned int start_index, unsigned int thread_count)
{
    image_set *thread_args = (image_set *)malloc(sizeof(image_set));
    if (NULL == thread_args)
    {
        help(ALLOCATTIONION_FAIL, NULL);
    }
    thread_args->path = path;
    thread_args->array = array;
    thread_args->array_lenght = array_lenght;
    thread_args->start_index = start_index;
    thread_args->thread_count = thread_count;
    return thread_args;
}

/**
 * @brief Resizes an image
 * 
 * @param in_img a pointer to a gdImage to be resized
 * @param new_width the new width to change the image
 * @return gdImagePtr the new image, NULL if the scaling failed
 */
gdImagePtr resize_image(gdImagePtr in_img, int new_width)
{

    gdImagePtr out_img;
    int width, heigth, new_heigth;

    width = in_img->sx;
    heigth = in_img->sy;
    new_heigth = (int)new_width * 1.0 / width * heigth;

    gdImageSetInterpolationMethod(in_img, GD_BILINEAR_FIXED);
    out_img = gdImageScale(in_img, new_width, new_heigth);
    if (NULL == out_img)
    {
        return NULL;
    }

    return (out_img);
}


gdImagePtr thumb_image(gdImagePtr in_img, int size)
{

    gdImagePtr out_img, aux_img;

    int width, heigth;
    int new_heigth, new_width;
    // Get the image's width and height

    width = in_img->sx;
    heigth = in_img->sy;

    if (heigth > width)
    {
        new_width = size;
        new_heigth = (int)new_width * 1.0 / width * heigth;
    }
    else
    {
        new_heigth = size;
        new_width = (int)new_heigth * 1.0 / heigth * width;
    }

    gdImageSetInterpolationMethod(in_img, GD_BILINEAR_FIXED);
    aux_img = gdImageScale(in_img, new_width, new_heigth);
    if (NULL == aux_img)
    {
        return NULL;
    }

    gdRect crop_area;
    crop_area.height = size;
    crop_area.width = size;
    crop_area.x = 0;
    crop_area.y = 0;

    out_img = gdImageCrop(aux_img, &crop_area);
    gdImageDestroy(aux_img);

    if (!out_img)
    {
        return NULL;
    }

    return out_img;
}

/**
 * @brief reads a png file to a gdImage
 * 
 * @param file_name the name of the file to open
 * @return gdImagePtr pointer to the created image
 */
gdImagePtr read_png_file(char *path, char *file_name)
{

    FILE *fp;
    gdImagePtr read_img;
    char *file = file_path(path, "", file_name);
    fp = fopen(file, "rb");

    if (!fp)
    {
        help(FILE_READ_FAIL, file);
        return NULL;
    }

    read_img = gdImageCreateFromPng(fp);

    fclose(fp);

    if (read_img == NULL)
    {
        help(FILE_READ_FAIL, file);
        return NULL;
    }
    free(file);
    return read_img;
}

/**
 * @brief Saves an image o the specified directory and file
 * 
 * @param image the gdImage to save
 * @param directory the path to the destination directory
 * @param filename the final filename
 */
void save_image(gdImagePtr image, char *path, char *subdirectory, char *filename)
{
    char *out_file = file_path(path, subdirectory, filename);
    FILE *fp = fopen(out_file, "w");
    if (!fp)
    {
        help(FILE_WRITE_FAIL, out_file);
        return;
    }
    gdImagePng(image, fp);
    free(out_file);
    fclose(fp);
}

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
void *process_image_set(void *args)
{
    gdImagePtr image, out_image = NULL;
    image_set *set = (image_set *)args;
    for (unsigned int i = set->start_index; i < set->array_lenght; i += set->thread_count)
    {
        printf("%s\n", set->array[i]);
        image = read_png_file(set->path, set->array[i]);
        if (NULL == image)
        {
            continue;
        }
        out_image = resize_image(image, 640);
        if (NULL != out_image)
        {
            save_image(out_image, set->path, RESIZE_DIR, set->array[i]);
            gdImageDestroy(out_image);
        }
        out_image = thumb_image(image, 640);
        if (NULL != out_image)
        {
            save_image(out_image, set->path, THUMB_DIR, set->array[i]);
            gdImageDestroy(out_image);
        }
        gdImageDestroy(image);
    }

    free(args);
    return NULL;
}
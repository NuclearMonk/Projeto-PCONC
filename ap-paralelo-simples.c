/******************************************************************************
 * Programacao Concorrente
 * MEEC 2021/2022
 *
 * Projecto - Parte 1
 *                           ap-paralelo-simples.c
 * 
 * Compilacao: make ap-paralelo-simples
 * Author: MAnuel Soares, Eduardo Faustino
 *****************************************************************************/
#pragma region INCLUDES
#include <gd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#pragma endregion

#pragma region ERROR CODES
#define INVALID_ARGS 1
#define DIR_NOT_FOUND 2
#define DIR_CREATION_FAIL 3
#define ALLOCATTIONION_FAIL 4
#define NO_FILES_FOUND 5
#define FILE_READ_FAIL 6
#define FILE_WRITE_FAIL 7
#pragma endregion

#define RESIZE_DIR "Resize/"
#define THUMB_DIR "Thumbnail/"
#define WATER_DIR "Watermark/"

/**
 * @brief Throws an error and exits the program
 *
 * 
 * @param error_id Error codes defined in by macro at the top of this file
 */
void help(int error_id, char *extra_info)
{
    switch (error_id)
    {
    case INVALID_ARGS:
        fprintf(stderr, "Invalid Arguments\nProper Argument usage is\n./ap-paralelo-simples TARGER_DIRECTORY MAX_THREADS\n");
        exit(EXIT_FAILURE);
    case DIR_NOT_FOUND:
        fprintf(stderr, "Target Directory Not Found\n");
        exit(EXIT_FAILURE);
    case DIR_CREATION_FAIL:
        fprintf(stderr, "Final Directory Creations Failed\n");
        exit(EXIT_FAILURE);
    case ALLOCATTIONION_FAIL:
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    case NO_FILES_FOUND:
        fprintf(stderr, "No Files Found In Target Directory\n");
        exit(EXIT_FAILURE);
    case FILE_READ_FAIL:
        fprintf(stderr, "%s : Failed to read file\n", extra_info);
        break;
    case FILE_WRITE_FAIL:
        fprintf(stderr, "%s : Failed to write file\n", extra_info);
        break;
    default:
        fprintf(stderr, "Unkown Error\n");
        exit(EXIT_FAILURE);
    }
}

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
    unsigned int filecount = 0;
    unsigned int aux = 0;
    struct dirent *de;
    DIR *dir = opendir(path);
    if (NULL == dir)
    {
        help(DIR_NOT_FOUND, NULL);
    }
    while ((de = readdir(dir)) != NULL)
    {
        if (check_file_ext(de->d_name, ".png"))
        {
            filecount++;
        }
    }
    if (filecount == 0)
        return 0;
    rewinddir(dir);
    (*filenames) = (char **)malloc(filecount * sizeof(char *));
    if (NULL == (*filenames))
    {
        help(ALLOCATTIONION_FAIL, NULL);
    }
    while ((de = readdir(dir)) != NULL)
    {
        if (check_file_ext(de->d_name, ".png"))
        {
            (*filenames)[aux] = (char *)malloc((strlen(de->d_name) + 1) * sizeof(char));
            if (NULL == (*filenames)[aux])
            {
                help(ALLOCATTIONION_FAIL, NULL);
            }
            strcpy((*filenames)[aux],de->d_name);
            aux++;
        }
    }
    closedir(dir);
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
 * @brief struct de image set para passar todos os dados necessarios para cada thread poder processar um conjunto de imagens
 * 
 */
typedef struct ImageSet
{
    char* path;
    char **array;
    unsigned int array_lenght;
    unsigned int start_index;
    unsigned int thread_count;
} image_set;

/**
 * @brief Create a image set object
 * 
 * @param array pointer to array of string with all the png file names
 * @param array_lenght the lenght of the array of string
 * @param start_index  the start index for the array
 * @param thread_count the threeadcount being used
 * @return image_set* 
 */
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
gdImagePtr read_png_file(char *file_name)
{

    FILE *fp;
    gdImagePtr read_img;

    fp = fopen(file_name, "rb");

    if (!fp)
    {
        help(FILE_READ_FAIL, file_name);
        return NULL;
    }

    read_img = gdImageCreateFromPng(fp);

    fclose(fp);

    if (read_img == NULL)
    {
        help(FILE_READ_FAIL, file_name);
        return NULL;
    }
    return read_img;
}

/**
 * @brief Saves an image o the specified directory and file
 * 
 * @param image the gdImage to save
 * @param directory the path to the destination directory
 * @param filename the final filename
 */
void save_image(gdImagePtr image,char *path, char *subdirectory, char *filename)
{
    int filename_len = strlen(path)+1+strlen(subdirectory) +strlen(filename) + 1;
    char *out_file = (char *)malloc(filename_len * sizeof(char));
    sprintf(out_file, "%s/%s%s",path,subdirectory, filename);
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
        image = read_png_file(set->array[i]);
        if (NULL == image)
        {
            continue;
        }
        out_image = resize_image(image, 640);
        if (NULL != out_image)
        {
            save_image(out_image,set->path, RESIZE_DIR, set->array[i]);
            gdImageDestroy(out_image);
        }
        out_image = thumb_image(image, 640);
        if (NULL != out_image)
        {
            save_image(out_image,set->path, THUMB_DIR, set->array[i]);
            gdImageDestroy(out_image);
        }
        gdImageDestroy(image);
    }

    free(args);
    return NULL;
}

int main(int argc, char *argv[])
{
    unsigned int input_files_count = 0;
    char **input_files_names = NULL;
    int max_threads = 0;
    char * path = (char*)malloc((strlen(argv[1])+1)*sizeof(char));
    strcpy(path,argv[1]);
    if (argc != 3)
        help(INVALID_ARGS, NULL);
    max_threads = atoi(argv[2]);
    input_files_count = list_pngs(path, &input_files_names);
    if (input_files_count == 0)
    {
        help(NO_FILES_FOUND, NULL);
    }
    create_output_directories(path);
    pthread_t *threads = (pthread_t *)malloc(max_threads * sizeof(pthread_t));
    if (NULL == threads)
        help(ALLOCATTIONION_FAIL, NULL);

    for (int i = 0; i < max_threads; i++)
    {
        pthread_create(&(threads[i]), NULL, process_image_set, create_image_set(path,input_files_names, input_files_count, i, max_threads));
    }
    for (int i = 0; i < max_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    for (unsigned int i = 0; i < input_files_count; i++)
    {
        free(input_files_names[i]);
    }
    free(path);
    free(input_files_names);
    free(threads);
    return 0;
}
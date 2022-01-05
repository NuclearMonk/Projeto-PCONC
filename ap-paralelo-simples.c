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
#pragma endregion

/**
 * @brief Throws an error and exits the program
 *
 * 
 * @param error_id Error codes defined in by macro at the top of this file
 */
void help(int error_id)
{
    switch (error_id)
    {
    case INVALID_ARGS:
        fprintf(stderr, "Invalid Arguments\nProper Argument usage is\n./ap-paralelo-simples TARGER_DIRECTORY MAX_THREADS\n");
        exit(EXIT_FAILURE);
    case DIR_NOT_FOUND:
        fprintf(stderr, "Target Directory Not Found");
        exit(EXIT_FAILURE);
    default:
        fprintf(stderr, "Unkown Error");
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
    if (ext_dot == NULL)
        return false;
    if (ext_dot == filename)
        return false;
    if (strcasecmp(ext_dot, extention) == 0)
        return true;
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
    if (dir == NULL)
    {
        help(DIR_NOT_FOUND);
    }
    while ((de = readdir(dir)) != NULL)
    {
        if (check_file_ext(de->d_name, ".png"))
            filecount++;
    }
    if (filecount == 0)
        return 0;
    rewinddir(dir);
    (*filenames) = (char **)malloc(filecount * sizeof(char *));
    while ((de = readdir(dir)) != NULL)
    {
        if (check_file_ext(de->d_name, ".png"))
        {
            (*filenames)[aux] = (char *)malloc((strlen(de->d_name) + 1) * sizeof(char));
            strcpy((*filenames)[aux], de->d_name);
            aux++;
        }
    }
    closedir(dir);
    return filecount;
}

/**
 * @brief struct de image set para passar todos os dados necessarios para cada thread poder processar um conjunto de imagens
 * 
 */
typedef struct ImageSet
{
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
image_set *create_image_set(char **array, unsigned int array_lenght, unsigned int start_index, unsigned int thread_count)
{
    image_set *thread_args = (image_set *)malloc(sizeof(image_set));
    thread_args->array = array;
    thread_args->array_lenght = array_lenght;
    thread_args->start_index = start_index;
    thread_args->thread_count = thread_count;
    return thread_args;
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
    image_set *set = (image_set *)args;
    for (unsigned int i = set->start_index; i < set->array_lenght; i += set->thread_count)
    {
        /*TODO Implement Image Transforming Function Calls*/
        printf("%u %s\n", set->start_index, set->array[i]);
    }
    free(args);
    return NULL;
}

int main(int argc, char *argv[])
{
    unsigned int input_files_count = 0;
    char **input_files_names = NULL;
    int max_threads = 0;

    if (argc != 3)
        help(INVALID_ARGS);

    max_threads = atoi(argv[2]);
    input_files_count = list_pngs(argv[1], &input_files_names);
    pthread_t *threads = (pthread_t *)malloc(max_threads * sizeof(pthread_t));

    for (int i = 0; i < max_threads; i++)
    {
        pthread_create(&(threads[i]), NULL, process_image_set, create_image_set(input_files_names, input_files_count, i, max_threads));
    }
    for (int i = 0; i < max_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    for (unsigned int i = 0; i < input_files_count; i++)
    {
        free(input_files_names[i]);
    }
    free(input_files_names);
    free(threads);
    return 0;
}
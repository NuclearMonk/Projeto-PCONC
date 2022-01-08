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
#include <string.h>
#include <pthread.h>
#include "help.h"
#include "filehandler.h"
#include "imagehandler.h"
#pragma endregion

int main(int argc, char *argv[])
{
	if (3 != argc) {
		help(INVALID_ARGS, NULL);

		return EXIT_FAILURE;
	}

	unsigned int input_files_count = 0;
	char **input_files_names = NULL;
	int max_threads = 0;
	char *path = (char *)malloc((strlen(argv[1]) + 1) * sizeof(char));
	gdImagePtr watermark = read_png_file(".", "watermark.png");

	strcpy(path, argv[1]);
	printf("%s\n", path);

	max_threads = atoi(argv[2]);
	input_files_count = list_pngs(path, &input_files_names);
	if (0 == input_files_count)
	{
		help(NO_FILES_FOUND, NULL);
		gdImageDestroy(watermark);
		free(path);
		free(input_files_names);

		return EXIT_FAILURE;
	}

	max_threads = max_threads < input_files_count ? max_threads : input_files_count;
	printf("using %d Threads\n", max_threads);
	create_output_directories(path);
	pthread_t *threads = (pthread_t *)malloc(max_threads * sizeof(pthread_t));
	if (NULL == threads) {
		help(ALLOCATTIONION_FAIL, NULL);
		gdImageDestroy(watermark);
		free(path);
		free(input_files_names);
		free(threads);

		return EXIT_FAILURE;
	}

	for (int i = 0; i < max_threads; ++i)
	{
		pthread_create(&(threads[i]), NULL, process_image_set,
					   create_image_set(path, input_files_names, input_files_count, i, max_threads,watermark));
	}
	for (int i = 0; i < max_threads; ++i)
	{
		pthread_join(threads[i], NULL);
	}

	for (unsigned int i = 0; i < input_files_count; ++i)
	{
		free(input_files_names[i]);
	}

	gdImageDestroy(watermark);
	free(path);
	free(input_files_names);
	free(threads);

	return 0;
}

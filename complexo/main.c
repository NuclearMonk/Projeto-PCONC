/******************************************************************************
 * Programacao Concorrente
 * MEEC 2021/2022
 *
 * Projecto - Parte 1
 *                           ap-complexo-simples.c
 *
 * Compilacao: make ap-complexo-simples
 * Author: MAnuel Soares, Eduardo Faustino
 *****************************************************************************/
#pragma region INCLUDES
#include <gd.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
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
	struct timespec timeStart,timeStop;
	
	int input_files_count = 0;
	char **input_files_names = NULL;
	int max_threads = 0;
	char *base_path = (char *)malloc((strlen(argv[1]) + 1) * sizeof(char));
	
	clock_gettime(CLOCK_REALTIME,&timeStart);
	strcpy(base_path, argv[1]);
	printf("Imgs path: %s\n", base_path);

	gdImagePtr watermark = read_png_file(base_path, "watermark.png");
	max_threads = atoi(argv[2]) ;
	max_threads = max_threads<0 ? INT_MAX : max_threads;
	input_files_count = list_pngs(base_path, &input_files_names);
	if (0 == input_files_count)
	{
		help(NO_FILES_FOUND, NULL);
		free(base_path);
		gdImageDestroy(watermark);
		free(input_files_names);

		return EXIT_FAILURE;
	}

	max_threads = max_threads < input_files_count ? max_threads : input_files_count;
	printf("using %d Threads\n", max_threads);
	create_output_directories(base_path);
	pthread_t *threads = (pthread_t *)malloc(max_threads * sizeof(pthread_t));
	if (NULL == threads) {
		help(ALLOCATION_FAIL, NULL);
		free(base_path);
		gdImageDestroy(watermark);
		free(input_files_names);
		free(threads);

		return EXIT_FAILURE;
	}

	for (int i = 0; i < max_threads; ++i)
	{
		pthread_create(&(threads[i]), NULL, process_image_set,
					   create_image_set(base_path, input_files_names, input_files_count, i, max_threads, watermark));
	}
	for (int i = 0; i < max_threads; ++i)
	{
		pthread_join(threads[i], NULL);
	}

	for (int i = 0; i < input_files_count; ++i)
	{
		free(input_files_names[i]);
	}

	free(base_path);
	gdImageDestroy(watermark);
	free(input_files_names);
	free(threads);
	clock_gettime(CLOCK_REALTIME,&timeStop);
	printf("start:%ld.%ld\nend:%ld.%ld\n",timeStart.tv_sec,timeStart.tv_nsec,timeStop.tv_sec,timeStop.tv_nsec);
	return 0;
}

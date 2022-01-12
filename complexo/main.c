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
#include "stats.h"
#pragma endregion

int main(int argc, char *argv[])
{
	if (3 != argc)
	{
		help(INVALID_ARGS, NULL);

		return EXIT_FAILURE;
	}
	timer_data timer;

	int input_files_count = 0;
	char **input_files_names = NULL;
	int max_threads = 0;
	char *base_path = (char *)malloc((strlen(argv[1]) + 1) * sizeof(char));

	clock_gettime(CLOCK_REALTIME, &(timer.start));
	strcpy(base_path, argv[1]);
	printf("Imgs path: %s\n", base_path);

	gdImagePtr watermark = read_png_file(base_path, "watermark.png");
	max_threads = atoi(argv[2]);
	max_threads = max_threads < 0 ? INT_MAX : max_threads;
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

	if (NULL == threads)
	{
		help(ALLOCATION_FAIL, NULL);
		free(base_path);
		gdImageDestroy(watermark);
		free(input_files_names);
		free(threads);

		return EXIT_FAILURE;
	}

	///////////////////////////////////////////////////
	gdImagePtr *image_array = create_image_array(input_files_count);
	image_set **thread_data = (image_set **)malloc(max_threads * sizeof(image_set *));
	timer_data* thread_timers = ALLOCATE_TIMERS(max_threads)
	timer_data* image_timers = ALLOCATE_TIMERS(input_files_count)
	FILE * fp = fopen("stats.csv","w");
	fprintf(fp,"name,start time,end time\n");
	for (int i = 0; i < max_threads; ++i)
	{
		thread_data[i] = create_image_set(base_path, input_files_names, image_array, input_files_count, i, max_threads,
										  watermark,thread_timers,image_timers);
		pthread_create(&(threads[i]), NULL, process_image_set_1, thread_data[i]);
	}
	for (int i = 0; i < max_threads; ++i)
	{
		pthread_join(threads[i], NULL);
	}
	fprintf(fp,"name,Resize start time,Resize end time\n");
	for (int i = 0; i < input_files_count; i++)
	{
		fprintf(fp,"%s,%ld.%ld,%ld.%ld\n",input_files_names[i],image_timers[i].start.tv_sec,image_timers[i].start.tv_nsec,image_timers[i].end.tv_sec,image_timers[i].end.tv_nsec);
	}
	fprintf(fp,"Thread,Thread start time,Thread end time\n");
	for (int i = 0; i < max_threads; i++)
	{
		fprintf(fp,"Thread %d,%ld.%ld,%ld.%ld\n",i,thread_timers[i].start.tv_sec,thread_timers[i].start.tv_nsec,thread_timers[i].end.tv_sec,thread_timers[i].end.tv_nsec);
	}
	
	for (int i = 0; i < max_threads; ++i)
	{
		pthread_create(&(threads[i]), NULL, process_image_set_2,
					   thread_data[i]);
	}
	for (int i = 0; i < max_threads; ++i)
	{
		pthread_join(threads[i], NULL);
	}
	fprintf(fp,"name,Thumbnail start time,Thumbnail end time\n");
	for (int i = 0; i < input_files_count; i++)
	{
		fprintf(fp,"%s,%ld.%ld,%ld.%ld\n",input_files_names[i],image_timers[i].start.tv_sec,image_timers[i].start.tv_nsec,image_timers[i].end.tv_sec,image_timers[i].end.tv_nsec);
	}
	fprintf(fp,"Thread,Thread start time,Thread end time\n");
	for (int i = 0; i < max_threads; i++)
	{
		fprintf(fp,"Thread %d,%ld.%ld,%ld.%ld\n",i,thread_timers[i].start.tv_sec,thread_timers[i].start.tv_nsec,thread_timers[i].end.tv_sec,thread_timers[i].end.tv_nsec);
	}
	for (int i = 0; i < max_threads; ++i)
	{
		pthread_create(&(threads[i]), NULL, process_image_set_3,
					   thread_data[i]);
	}
	for (int i = 0; i < max_threads; ++i)
	{
		pthread_join(threads[i], NULL);
	}
	///////////////////////////////////////////////////

	free(image_array);
	free(thread_data);
	free(base_path);
	gdImageDestroy(watermark);
	free(threads);
	clock_gettime(CLOCK_REALTIME, &(timer.end));
	fprintf(fp,"name,Watermark start time,Watermark end time\n");
	for (int i = 0; i < input_files_count; i++)
	{
		fprintf(fp,"%s,%ld.%ld,%ld.%ld\n",input_files_names[i],image_timers[i].start.tv_sec,image_timers[i].start.tv_nsec,image_timers[i].end.tv_sec,image_timers[i].end.tv_nsec);
		free(input_files_names[i]);
	}
	free(input_files_names);
	fprintf(fp,"Thread,Thread start time,Thread end time\n");
	for (int i = 0; i < max_threads; i++)
	{
		fprintf(fp,"Thread %d,%ld.%ld,%ld.%ld\n",i,thread_timers[i].start.tv_sec,thread_timers[i].start.tv_nsec,thread_timers[i].end.tv_sec,thread_timers[i].end.tv_nsec);
	}

	free(image_timers);
	free(thread_timers);
	fprintf(fp,"Total,%ld.%ld,%ld.%ld\n",timer.start.tv_sec,timer.start.tv_nsec,timer.end.tv_sec,timer.end.tv_nsec);
	fclose(fp);
	return 0;
}

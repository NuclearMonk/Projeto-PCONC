/******************************************************************************
 * Programação Concorrente
 * LEEC 2021/2022
 *
 * Projeto - Parte A
 * 		Paralelo simples
 *
 * Compilação: make paralelo-simples
 * Autores:
 * - Eduardo Faustino, Nº 102298
 * - Manuel Soares, Nº 96267
 *****************************************************************************/
#pragma region INCLUDES
#include "Utils/help.h"
#include "Utils/filehandler.h"
#include "Utils/imagehandler.h"
#include "Utils/general.h"
#include <gd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#pragma endregion

static void *process_image_set(void *args);

int main(int argc, char *argv[])
{
	if (3 != argc) {
		help(INVALID_ARGS, NULL);

		return EXIT_FAILURE;
	}
	int ret_var = EXIT_SUCCESS;

	printf("----- Running ap-paralelo-simples -----");

	// Things to deallocate in the end
	char *base_path = NULL;
	char **input_files_names = NULL;
	pthread_t *threads = NULL;
	timer_data* thread_timers = NULL;
	timer_data* image_timers = NULL;
	gdImagePtr watermark = NULL;
	char *stats_csv_path = NULL;
	FILE *stats_csv_file = NULL;
	// Things to deallocate in the end

	timer_data timer;
	clock_gettime(CLOCK_REALTIME, &(timer.start));

	base_path = (char *) malloc((strlen(argv[1]) + 1) * sizeof(char));
	strcpy(base_path, argv[1]);
	printf("Imgs path: %s\n", base_path);

	int input_files_count = list_pngs(base_path, &input_files_names);
	if (0 == input_files_count) {
		help(NO_FILES_FOUND, NULL);
		ret_var = EXIT_FAILURE;

		goto endMain;
	}

	int max_threads = atoi(argv[2]);
	if (max_threads < 0 || max_threads > input_files_count) {
		max_threads = input_files_count;
	}
	printf("using %d Threads\n", max_threads);
	create_output_directories(base_path);

	threads = (pthread_t *) malloc(max_threads * sizeof(pthread_t));
	thread_timers = ALLOCATE_TIMERS(max_threads);
	image_timers = ALLOCATE_TIMERS(input_files_count);
	watermark = read_png_file(base_path, "watermark.png");
	if (NULL == threads || NULL == thread_timers || NULL == image_timers || NULL == watermark) {
		help(ALLOCATION_FAIL, "sdlfkj");
		ret_var = EXIT_FAILURE;

		goto endMain;
	}

	stats_csv_path = img_path_generator(base_path, "", "stats.csv");
	stats_csv_file = fopen(stats_csv_path, "w");
	freeNew(stats_csv_path);
	if (NULL == stats_csv_file) {
		help(FILE_WRITE_FAIL, "stats.csv");
		ret_var = EXIT_FAILURE;

		goto endMain;
	}

	/////////////////////////////////////////////////////////////
	for (int i = 0; i < max_threads; ++i)
	{
		pthread_create(&(threads[i]), NULL, process_image_set,
					   create_image_set(base_path, input_files_names, NULL, input_files_count, i, max_threads, watermark,
										thread_timers, image_timers));
	}
	for (int i = 0; i < max_threads; ++i)
	{
		pthread_join(threads[i], NULL);
	}

	fprintf(stats_csv_file, "Thread no.,start time,end time\n");
	for (int i = 0; i < max_threads; ++i)
	{
		fprintf(stats_csv_file,"Thread %d,%ld.%ld,%ld.%ld\n", i, thread_timers[i].start.tv_sec,
				thread_timers[i].start.tv_nsec, thread_timers[i].end.tv_sec, thread_timers[i].end.tv_nsec);
	}
	fprintf(stats_csv_file, "File name,start time,end time\n"); \
	for (int i = 0; i < input_files_count; ++i)
	{
		fprintf(stats_csv_file,"%s,%ld.%ld,%ld.%ld\n", input_files_names[i], image_timers[i].start.tv_sec,
				image_timers[i].start.tv_nsec,image_timers[i].end.tv_sec,image_timers[i].end.tv_nsec);
	}
	/////////////////////////////////////////////////////////////

	fprintf(stats_csv_file, "\n");
	clock_gettime(CLOCK_REALTIME, &(timer.end));
	fprintf(stats_csv_file,"Total,%ld.%ld,%ld.%ld\n", timer.start.tv_sec, timer.start.tv_nsec, timer.end.tv_sec,
			timer.end.tv_nsec);

	// Main freeNew() calls
	// Labels for all errors to come to and this way we don't need to remember to free things all over the place.
	endMain:
	fcloseNew(stats_csv_file);
	freeNew(image_timers);
	freeNew(thread_timers);
	gdImageDestroy(watermark);
	watermark = NULL;
	freeNew(threads);
	if (NULL != input_files_names) {
		for (int i = 0; i < input_files_count; ++i) {
			freeNew(input_files_names[i]);
		}
	}
	freeNew(input_files_names);
	freeNew(base_path);

	return ret_var;
}

/**
 * @brief Thread function.
 *
 * @param args a pointer to a struct of type image_set
 *
 * @return NULL
 */
static void *process_image_set(void *args)
{
	// Explanation of how the thread works:
	// Each thread executes the necessary transformations over a set of given 'filename_array's, beginning in the position
	// corresponding to its thread ID, and jumping 'threadcount' images each time.
	//	This is done like this to reduce the discrepancies of work between the threads such that no thread works more than
	// any other (preferably - if the number of images to process cannot divide the number of threads, there will be at
	// least one thread with more work than the others).

	image_set *set = (image_set *)args;

	clock_gettime(CLOCK_REALTIME, &(set->thread_timers[set->start_index].start));
	for (unsigned int i = set->start_index; i < set->array_length; i += set->thread_count)
	{
		clock_gettime(CLOCK_REALTIME, &(set->image_timers[i].start));
		printf("%s\n", set->filenames_array[i]);
		gdImagePtr image = read_png_file(set->imgs_path, set->filenames_array[i]);
		if (NULL == image)
		{
			continue;
		}

		gdImagePtr out_image = resize_image(image, 640);
		if (NULL == out_image) {
			help(ERR_RESIZE, set->filenames_array[i]);

			continue;
		} else {
			save_image(out_image, set->imgs_path, RESIZE_DIR, set->filenames_array[i]);
			gdImageDestroy(out_image);
			out_image = NULL;
		}

		out_image = thumb_image(image, 640);
		if (NULL == out_image) {
			help(ERR_THUMB, set->filenames_array[i]);

			continue;
		} else {
			save_image(out_image, set->imgs_path, THUMB_DIR, set->filenames_array[i]);
			gdImageDestroy(out_image);
			out_image = NULL;
		}

		out_image = add_watermark(image, set->watermark);
		if (NULL == out_image) {
			help(ERR_WATER, set->filenames_array[i]);

			continue;
		} else {
			save_image(out_image, set->imgs_path, WATER_DIR, set->filenames_array[i]);
			gdImageDestroy(out_image);
			out_image = NULL;
		}

		gdImageDestroy(image);
		clock_gettime(CLOCK_REALTIME, &(set->image_timers[i].end));
	}
	clock_gettime(CLOCK_REALTIME, &(set->thread_timers[set->start_index].end));

	freeNew(args);

	return NULL;
}

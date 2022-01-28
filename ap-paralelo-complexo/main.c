/******************************************************************************
 * Programação Concorrente
 * LEEC 2021/2022
 *
 * Projeto - Parte A
 * 		Paralelo complexo
 *
 * Compilação: make paralelo-complexo
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
#include <stdbool.h>
#pragma endregion

#define FREE_MEMORY {fcloseNew(stats_csv_file);\
	freeNew(image_timers);\
	freeNew(thread_timers);\
	freeNew(thread_data);\
	if(NULL!=watermark){gdImageDestroy(watermark);}\
	watermark = NULL;\
	freeNew(threads);\
	freeNew(image_array);\
	if (NULL != input_files_names) {for (int i = 0; i < input_files_count; ++i) {freeNew(input_files_names[i]);}}\
	freeNew(input_files_names);\
	freeNew(base_path);}

#define joinAndWriteFile(free_data) \
	fprintf(stats_csv_file, "Thread no.,start time,end time\n"); \
	for (int i = 0; i < max_threads; ++i) { \
		pthread_join(threads[i], NULL); \
		fprintf(stats_csv_file, "Thread %d,%ld.%ld,%ld.%ld\n", i, thread_timers[i].start.tv_sec, \
		thread_timers[i].start.tv_nsec, thread_timers[i].end.tv_sec, thread_timers[i].end.tv_nsec); \
        if (free_data) { \
            freeNew(thread_data[i]); \
        } \
	} \
	fprintf(stats_csv_file, "File name,start time,end time\n"); \
	for (int i = 0; i < input_files_count; ++i) { \
		fprintf(stats_csv_file, "%s,%ld.%ld,%ld.%ld\n", input_files_names[i], image_timers[i].start.tv_sec, \
		image_timers[i].start.tv_nsec, image_timers[i].end.tv_sec, image_timers[i].end.tv_nsec); \
	} \

static void *process_image_set_resize(void *args) __attribute__((nonnull));
static void *process_image_set_thumb(void *args) __attribute__((nonnull));
static void *process_image_set_watermark(void *args) __attribute__((nonnull));

int main(int argc, char *argv[])
{
	if (3 != argc)
	{
		help(INVALID_ARGS, NULL);

		return EXIT_FAILURE;
	}
	int ret_var = EXIT_SUCCESS;

	printf("----- Running ap-paralelo-complexo -----");

	/* Things to free/close in the end */
	char *base_path = NULL;
	char **input_files_names = NULL;
	pthread_t *threads = NULL;
	gdImagePtr *image_array = NULL;
	ThreadParams **thread_data = NULL;
	timer_data* thread_timers = NULL;
	timer_data* image_timers = NULL;
	gdImagePtr watermark = NULL;
	char *stats_csv_path = NULL;
	FILE *stats_csv_file = NULL;
	/***********************************/

	timer_data timer;
	clock_gettime(CLOCK_REALTIME, &(timer.start));

	base_path = (char *) malloc((strlen(argv[1]) + 1) * sizeof(char));
	strcpy(base_path, argv[1]);
	printf("Imgs path: %s\n", base_path);

	int input_files_count = list_pngs(base_path, &input_files_names);
	if (0 == input_files_count) {
		help(NO_FILES_FOUND, NULL);
		ret_var = EXIT_FAILURE;
		FREE_MEMORY
		return ret_var;
	}

	int max_threads = atoi(argv[2]);
	if (max_threads <= 0 || max_threads > input_files_count) {
		max_threads = input_files_count;
	}
	printf("Using %d Threads\n", max_threads);
	create_output_directories(base_path);

	threads = (pthread_t *) malloc(max_threads * sizeof(pthread_t));
	thread_timers = ALLOCATE_TIMERS(max_threads);
	image_timers = ALLOCATE_TIMERS(input_files_count);
	image_array = create_image_array(input_files_count);
	thread_data = (ThreadParams **) malloc(max_threads * sizeof(ThreadParams *));
	watermark = read_png_file(base_path, "watermark.png");
	if (NULL == image_array || NULL == thread_data || NULL == thread_timers || NULL == image_timers ||
			NULL == watermark || NULL == threads) {
		help(ALLOCATION_FAIL, "sdlfkj");
		ret_var = EXIT_FAILURE;

		FREE_MEMORY
		return ret_var;
	}

	stats_csv_path = img_path_generator(base_path, "", "stats.csv");
	stats_csv_file = fopen(stats_csv_path, "w");
	freeNew(stats_csv_path);
	if (NULL == stats_csv_file) {
		help(FILE_WRITE_FAIL, "stats.csv");
		ret_var = EXIT_FAILURE;

		FREE_MEMORY
		return ret_var;
	}

	/////////////////////////////////////////////////////////////
	// Threads for the resize operation.
	for (int i = 0; i < max_threads; ++i) {
		thread_data[i] = create_image_set(base_path, input_files_names, image_array, input_files_count, i, max_threads,
										  watermark, thread_timers, image_timers);
		pthread_create(&(threads[i]), NULL, process_image_set_resize, thread_data[i]);
	}
	fprintf(stats_csv_file, "----- Resize operation -----\n");
	joinAndWriteFile(false)

	// Threads for the thumbnail insertion operation.
	for (int i = 0; i < max_threads; ++i) {
		pthread_create(&(threads[i]), NULL, process_image_set_thumb, thread_data[i]);
	}
	fprintf(stats_csv_file, "----- Thumbnail operation -----\n");
	joinAndWriteFile(false)

	// Threads for the watermark insertion operation.
	for (int i = 0; i < max_threads; ++i) {
		pthread_create(&(threads[i]), NULL, process_image_set_watermark, thread_data[i]);
	}
	fprintf(stats_csv_file, "----- Watermark operation -----\n");
	joinAndWriteFile(true)
	/////////////////////////////////////////////////////////////

	fprintf(stats_csv_file, "\n");
	clock_gettime(CLOCK_REALTIME, &(timer.end));
	fprintf(stats_csv_file, "Total,%ld.%ld,%ld.%ld\n", timer.start.tv_sec, timer.start.tv_nsec, timer.end.tv_sec,
			timer.end.tv_nsec);

	FREE_MEMORY
	return ret_var;
}

//////////////////////////////////////////////////////
// Explanation of how the threads work:
// Each thread executes the necessary transformations over a set of given 'filename_array's, beginning in the position
// corresponding to its thread ID, and jumping 'threadcount' images each time.
//	This is done like this to reduce the discrepancies of work between the threads such that no thread works more than
// any other (preferably - if the number of images to process cannot divide the number of threads, there will be at
// least one thread with more work than the others).

/**
 * @brief Thread function to do the resize operation.
 *
 * @param args a pointer to a struct of type ThreadParams
 *
 * @return NULL
 */
static void *process_image_set_resize(void *args)
{
	ThreadParams *set = (ThreadParams *) args;
	clock_gettime(CLOCK_REALTIME, &(set->thread_timers[set->start_index].start));
	for (unsigned int i = set->start_index; i < set->array_length; i += set->thread_count)
	{
		clock_gettime(CLOCK_REALTIME, &(set->image_timers[i].start));
		printf("%s\n", set->filenames_array[i]);
		set->image_array[i] = read_png_file(set->imgs_path, set->filenames_array[i]);
		if (NULL == set->image_array[i]) {
			continue;
		}

		gdImagePtr out_image = resize_image(set->image_array[i], 640);
		if (NULL == out_image) {
			help(ERR_RESIZE, set->filenames_array[i]);

			continue;
		} else {
			save_image(out_image, set->imgs_path, RESIZE_DIR, set->filenames_array[i]);
			gdImageDestroy(out_image);
			out_image = NULL;
		}
		clock_gettime(CLOCK_REALTIME, &(set->image_timers[i].end));
	}
	clock_gettime(CLOCK_REALTIME, &(set->thread_timers[set->start_index].end));

	return NULL;
}

/**
 * @brief Thread function to do the thumbnail insertion operation.
 *
 * @param args a pointer to a struct of type ThreadParams
 *
 * @return NULL
 */
static void *process_image_set_thumb(void *args)
{
	ThreadParams *set = (ThreadParams *)args;
	clock_gettime(CLOCK_REALTIME, &(set->thread_timers[set->start_index].start));
	for (unsigned int i = set->start_index; i < set->array_length; i += set->thread_count)
	{
		clock_gettime(CLOCK_REALTIME, &(set->image_timers[i].start));
		printf("%s\n", set->filenames_array[i]);
		gdImagePtr image = set->image_array[i];
		if (NULL == image)
		{
			continue;
		}

		gdImagePtr out_image = thumb_image(image, 640);
		if (NULL == out_image)
		{
			help(ERR_THUMB, set->filenames_array[i]);

			continue;
		}
		else
		{
			save_image(out_image, set->imgs_path, THUMB_DIR, set->filenames_array[i]);
			gdImageDestroy(out_image);
			out_image = NULL;
		}
		clock_gettime(CLOCK_REALTIME, &(set->image_timers[i].end));
	}
	clock_gettime(CLOCK_REALTIME, &(set->thread_timers[set->start_index].end));

	return NULL;
}

/**
 * @brief Thread function to do the watermark insertion operation.
 *
 * @param args a pointer to a struct of type ThreadParams
 *
 * @return NULL
 */
static void *process_image_set_watermark(void *args)
{
	ThreadParams *set = (ThreadParams *)args;
	clock_gettime(CLOCK_REALTIME, &(set->thread_timers[set->start_index].start));
	for (unsigned int i = set->start_index; i < set->array_length; i += set->thread_count)
	{
		clock_gettime(CLOCK_REALTIME, &(set->image_timers[i].start));
		printf("%s\n", set->filenames_array[i]);
		gdImagePtr image = set->image_array[i];
		if (NULL == image)
		{
			continue;
		}

		gdImagePtr out_image = add_watermark(image, set->watermark);
		if (NULL == out_image)
		{
			help(ERR_WATER, set->filenames_array[i]);

			continue;
		}
		else
		{
			save_image(out_image, set->imgs_path, WATER_DIR, set->filenames_array[i]);
			gdImageDestroy(out_image);
		}
		gdImageDestroy(image);
		image = NULL;
		clock_gettime(CLOCK_REALTIME, &(set->image_timers[i].end));
	}
	clock_gettime(CLOCK_REALTIME, &(set->thread_timers[set->start_index].end));

	return NULL;
}

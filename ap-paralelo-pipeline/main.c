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

#define _GNU_SOURCE

#include "Utils/help.h"
#include "Utils/filehandler.h"
#include "Utils/imagehandler.h"
#include "Utils/general.h"
#include "Utils/stats.h"
#include <gd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>

#pragma endregion

#define NUM_THREAD_TYPES 3
#define TRANSF_TYPE_WATER 0
#define TRANSF_TYPE_THUMB 1
#define TRANSF_TYPE_RESIZE 2

#define FREE_MEMORY                                     \
	{                                                   \
		fcloseNew(stats_csv_file);                      \
		if (NULL != watermark)                          \
		{                                               \
			gdImageDestroy(watermark);                  \
		}                                               \
		watermark = NULL;                               \
		freeNew(threads);                               \
		if (NULL != input_files_names)                  \
		{                                               \
			for (int i = 0; i < input_files_count; ++i) \
			{                                           \
				freeNew(input_files_names[i]);          \
			}                                           \
		}                                               \
		freeNew(input_files_names);                     \
		freeNew(base_path);                             \
	}

static void *process_image_set(void *args) __attribute__((nonnull));

// Cada índice é dado pelas constants começadas por TRANSF_TYPE_.
int images_to_process_each_type[NUM_THREAD_TYPES] = {0};
// Um mutex para cada tipo de transformação.
pthread_mutex_t mutex_lock[NUM_THREAD_TYPES];


int pipe_logs[2];

typedef struct {
	int thread_index;
	int img_index;
	int operation;
	struct timespec start_time;
	struct timespec end_time;
} __attribute__((aligned(64))) PipeData;


int main(int argc, char *argv[])
{
	if (3 != argc)
	{
		help(INVALID_ARGS, NULL);

		return EXIT_FAILURE;
	}

	printf("----- Running ap-paralelo-dinamico -----");
	/* Things to free/close in the end */
	char *base_path = NULL;
	char **input_files_names = NULL;
	pthread_t *threads = NULL;
	gdImagePtr watermark = NULL;
	char *stats_csv_path = NULL;
	FILE *stats_csv_file = NULL;
	int pipe_w[2];
	int pipe_t[2];
	int pipe_r[2];
	int pipe_threads_finalizadas[2];
	/***********************************/

	timer_data timer;
	clock_gettime(CLOCK_REALTIME, &(timer.start));
	base_path = (char *)malloc((strlen(argv[1]) + 1) * sizeof(char));
	strcpy(base_path, argv[1]);
	printf("Imgs path: %s\n", base_path);

	int input_files_count = list_pngs(base_path, &input_files_names);
	if (0 == input_files_count)
	{
		help(NO_FILES_FOUND, NULL);
		FREE_MEMORY

		return EXIT_FAILURE;
	}
	images_to_process_each_type[TRANSF_TYPE_WATER] = input_files_count;
	images_to_process_each_type[TRANSF_TYPE_THUMB] = input_files_count;
	images_to_process_each_type[TRANSF_TYPE_RESIZE] = input_files_count;

	int max_type_threads = atoi(argv[2]);
	if ((max_type_threads <= 0) || (max_type_threads > input_files_count))
	{
		max_type_threads = input_files_count;
	}
	printf("Using %d Threads\n", max_type_threads);
	create_output_directories(base_path);

	threads = (pthread_t *)malloc(max_type_threads * NUM_THREAD_TYPES * sizeof(pthread_t));
	watermark = read_png_file(base_path, "watermark.png");
	if ((NULL == watermark) || (NULL == threads)) {
		help(ALLOCATION_FAIL, "sdlfkj");
		FREE_MEMORY

		return EXIT_FAILURE;
	}

	if ((pipe(pipe_w) < 0) || (pipe(pipe_r) < 0) || (pipe(pipe_t) < 0) ||
		(pipe2(pipe_threads_finalizadas, O_NONBLOCK) < 0) ||
		pipe(pipe_logs)) {
		help(ERR_CREATING_PIPE, NULL);
		FREE_MEMORY

		return EXIT_FAILURE;
	}

	{
		// Iniciar as threads, max_type_threads de cada tipo.
		int thread_num = 0;
		for (int i = 0; i < max_type_threads; ++i) {
			ThreadParams *thread_data = create_ThreadParams(thread_num, input_files_names, base_path, pipe_w, pipe_t,
															watermark,
															pipe_threads_finalizadas, TRANSF_TYPE_WATER);
			pthread_create(&(threads[thread_num]), NULL, process_image_set, thread_data);
			++thread_num;

			thread_data = create_ThreadParams(thread_num, input_files_names, base_path, pipe_t, pipe_r, watermark,
											  pipe_threads_finalizadas,
											  TRANSF_TYPE_THUMB);
			pthread_create(&(threads[thread_num]), NULL, process_image_set, thread_data);
			++thread_num;

			thread_data = create_ThreadParams(thread_num, input_files_names, base_path, pipe_r, NULL, watermark,
											  pipe_threads_finalizadas,
											  TRANSF_TYPE_RESIZE);
			pthread_create(&(threads[thread_num]), NULL, process_image_set, thread_data);
			++thread_num;
		}
	}
	for (int i = 0; i < input_files_count; ++i) {
		if (write(pipe_w[1], &i, sizeof(i)) != sizeof(i)) {
			help(ERR_USING_PIPE, NULL);
			FREE_MEMORY

			return EXIT_FAILURE;
		}
	}

	stats_csv_path = img_path_generator(base_path, "", "stats.csv");
	stats_csv_file = fopen(stats_csv_path, "w");
	freeNew(stats_csv_path);
	if (NULL == stats_csv_file)
	{
		help(FILE_WRITE_FAIL, "stats.csv");

		FREE_MEMORY
		return EXIT_FAILURE;
	}

	struct timespec *images_start_times = malloc(input_files_count * sizeof(*images_start_times));
	for (int valid_packets_received = 0; valid_packets_received < ((4 * input_files_count) + (max_type_threads * NUM_THREAD_TYPES)); /*Nada*/) { // 4 logs por cada ficheiro
		PipeData pipe_data;
		read(pipe_logs[0], &pipe_data, sizeof(pipe_data));

		if (pipe_data.thread_index < 0) {
			if (0 != pipe_data.end_time.tv_sec) {
				fprintf(stats_csv_file, "%s op %d,%ld.%ld,%ld.%ld\n", input_files_names[pipe_data.img_index],
						pipe_data.operation, pipe_data.start_time.tv_sec, pipe_data.start_time.tv_nsec,
						pipe_data.end_time.tv_sec, pipe_data.end_time.tv_nsec);
				if (TRANSF_TYPE_RESIZE == pipe_data.operation) {
					// Se for a última transformação, usar o tempo de término dela para o da imagem também.
					fprintf(stats_csv_file, "%s,%ld.%ld,%ld.%ld\n", input_files_names[pipe_data.img_index],
							images_start_times[pipe_data.img_index].tv_sec,
							images_start_times[pipe_data.img_index].tv_nsec, pipe_data.end_time.tv_sec,
							pipe_data.end_time.tv_nsec);
				}
				++valid_packets_received;
			} else if (TRANSF_TYPE_WATER == pipe_data.operation) {
				// Se for a primeira transformação, guardar o tempo de começo dela para o da imagem também.
				images_start_times[pipe_data.img_index] = pipe_data.start_time;
				++valid_packets_received;
			}
		} else {
			printf("closed thread: %d \n", pipe_data.thread_index);

			fprintf(stats_csv_file, "thread %d,%ld.%ld,%ld.%ld\n", pipe_data.thread_index, pipe_data.start_time.tv_sec,
					pipe_data.start_time.tv_nsec, pipe_data.end_time.tv_sec, pipe_data.end_time.tv_nsec);
			++valid_packets_received;
		}
	}
	close(pipe_threads_finalizadas[1]); // Aqui é fechado apenas porque não vai ser mais usado
	freeNew(images_start_times);

	for (int i = 0; i < NUM_THREAD_TYPES; ++i) {
		pthread_mutex_destroy(&mutex_lock[i]);
	}

	fprintf(stats_csv_file, "\n");
	clock_gettime(CLOCK_REALTIME, &(timer.end));
	fprintf(stats_csv_file, "Total,%ld.%ld,%ld.%ld\n", timer.start.tv_sec, timer.start.tv_nsec, timer.end.tv_sec,
			timer.end.tv_nsec);

	FREE_MEMORY
	return EXIT_SUCCESS;
}

/**
 * @brief Thread function to do the image operations.
 *
 * @param args a pointer to a struct of type ThreadParams
 *
 * @return NULL
 */
static void *process_image_set(void *args) {
	ThreadParams *targs = (ThreadParams *) args;

	int transf_type = targs->transf_type;
	struct timespec zero_time = {0};
	struct timespec curr_time = {0};

	clock_gettime(CLOCK_REALTIME, &curr_time);
	PipeData pipe_data_main = {0};
	pipe_data_main.thread_index = targs->thread_index;
	pipe_data_main.start_time = curr_time;

	int img_index = 0;
	while (true) {
		pthread_mutex_lock(&mutex_lock[transf_type]);
		if (0 == images_to_process_each_type[transf_type]) {
			pthread_mutex_unlock(&mutex_lock[transf_type]);

			break;
		}
		--images_to_process_each_type[transf_type];
		pthread_mutex_unlock(&mutex_lock[transf_type]);

		read(targs->pipe_self[0], &img_index, sizeof(img_index));

		// Send the start time for the transformation.
		clock_gettime(CLOCK_REALTIME, &curr_time);
		PipeData pipe_data = {0};
		pipe_data.thread_index = -1;
		pipe_data.img_index = img_index;
		pipe_data.operation = transf_type;
		pipe_data.start_time = curr_time;
		pipe_data.end_time = zero_time;
		write(pipe_logs[1], &pipe_data, sizeof(pipe_data));

		char *filename = targs->imgs_array[img_index];
		printf("thread %d type %d: %s\n", targs->thread_index, transf_type, filename);
		gdImagePtr image = read_png_file(targs->imgs_path, filename);
		if (NULL == image) {
			help(FILE_NOT_FOUND, filename);

			continue;
		}

		gdImagePtr out_image = NULL;
		if (TRANSF_TYPE_WATER == transf_type) {
			out_image = add_watermark(image, targs->watermark);
			if (NULL == out_image) {
				help(ERR_WATER, filename);

				continue;
			}
			save_image(out_image, targs->imgs_path, WATER_DIR, filename);
		} else if (TRANSF_TYPE_THUMB == transf_type) {
			out_image = thumb_image(image, 640);
			if (NULL == out_image) {
				help(ERR_THUMB, filename);

				continue;
			}
			save_image(out_image, targs->imgs_path, THUMB_DIR, filename);
		} else if (TRANSF_TYPE_RESIZE == transf_type) {
			out_image = resize_image(image, 640);
			if (NULL == out_image) {
				help(ERR_RESIZE, filename);

				continue;
			}
			save_image(out_image, targs->imgs_path, RESIZE_DIR, filename);
		} else {
			break;
		}

		gdImageDestroy(out_image);
		out_image = NULL;

		// Send now also the end time for the transformation.
		clock_gettime(CLOCK_REALTIME, &curr_time);
		pipe_data.end_time = curr_time;
		write(pipe_logs[1], &pipe_data, sizeof(pipe_data));

		if (transf_type != TRANSF_TYPE_RESIZE) {
			write(targs->pipe_next[1], &img_index, sizeof(img_index));
		}
	}

	clock_gettime(CLOCK_REALTIME, &curr_time);
	pipe_data_main.end_time = curr_time;
	write(pipe_logs[1], &pipe_data_main, sizeof(pipe_data_main));

	return NULL;
}

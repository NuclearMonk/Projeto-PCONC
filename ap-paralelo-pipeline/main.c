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

#define FREE_MEMORY                                                       \
	{                                                                     \
		fcloseNew(stats_csv_file);                                        \
		if (NULL != watermark)                                            \
		{                                                                 \
			gdImageDestroy(watermark);                                    \
		}                                                                 \
		watermark = NULL;                                                 \
		if (NULL != input_files_names)                                    \
		{                                                                 \
			for (int i_macro = 0; i_macro < input_files_count; ++i_macro) \
			{                                                             \
				freeNew(input_files_names[i_macro]);                      \
			}                                                             \
		}                                                                 \
		freeNew(input_files_names);                                       \
		freeNew(base_path);                                               \
	}

static void *process_image_set(void *thread_args) __attribute__((nonnull));

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
	gdImagePtr watermark = NULL;
	FILE *stats_csv_file = NULL;
	int pipe_w[2];
	int pipe_t[2];
	int pipe_r[2];
	/***********************************/

	timer_data main_timer;
	clock_gettime(CLOCK_REALTIME, &(main_timer.start));
	base_path = (char *)malloc((strlen(argv[1]) + 1) * sizeof(base_path));
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

	watermark = read_png_file(base_path, "watermark.png");
	if (NULL == watermark) {
		help(ALLOCATION_FAIL, "sdlfkj");
		FREE_MEMORY

		return EXIT_FAILURE;
	}

	if ((pipe(pipe_w) < 0) || (pipe(pipe_r) < 0) || (pipe(pipe_t) < 0) || (pipe(pipe_logs) < 0)) {
		help(ERR_CREATING_PIPE, NULL);
		FREE_MEMORY

		return EXIT_FAILURE;
	}

	{
		// Iniciar as threads, max_type_threads de cada tipo, e iniciá-las como detached. Como não é necessário fazer
		// join delas, podemos criá-las como detached para não ser preciso fazer join (caso não sejam detached e não
		// façamos join, vai haver fuga de memória).
		int thread_num = 0;
		for (int i = 0; i < max_type_threads; ++i) {
			pthread_t threads = 0;
			ThreadParams *thread_data = create_ThreadParams(thread_num, input_files_names, base_path, pipe_w, pipe_t,
															watermark, TRANSF_TYPE_WATER);
			pthread_create(&threads, NULL, process_image_set, thread_data);
			pthread_detach(threads);
			++thread_num;

			thread_data = create_ThreadParams(thread_num, input_files_names, base_path, pipe_t, pipe_r, NULL,
											  TRANSF_TYPE_THUMB);
			pthread_create(&threads, NULL, process_image_set, thread_data);
			pthread_detach(threads);
			++thread_num;

			thread_data = create_ThreadParams(thread_num, input_files_names, base_path, pipe_r, NULL, NULL,
											  TRANSF_TYPE_RESIZE);
			pthread_create(&threads, NULL, process_image_set, thread_data);
			pthread_detach(threads);
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

	char *stats_csv_path = img_path_generator(base_path, "", "stats.csv");
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
		if (sizeof(pipe_data) != read(pipe_logs[0], &pipe_data, sizeof(pipe_data))) {
			break;
		}

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
	freeNew(images_start_times);

	int array_size = sizeof(mutex_lock) / sizeof(*mutex_lock);
	for (int i = 0; i < array_size; ++i) {
		pthread_mutex_destroy(&(mutex_lock[i]));
	}

	clock_gettime(CLOCK_REALTIME, &(main_timer.end));
	fprintf(stats_csv_file, "\nTotal,%ld.%ld,%ld.%ld\n", main_timer.start.tv_sec, main_timer.start.tv_nsec,
			main_timer.end.tv_sec, main_timer.end.tv_nsec);

	FREE_MEMORY

	return EXIT_SUCCESS;
}

/**
 * @brief Thread function to do the image operations.
 *
 * @param thread_args a pointer to a struct of type ThreadParams
 *
 * @return NULL
 */
static void *process_image_set(void *thread_args) {
	ThreadParams *args = (ThreadParams *) thread_args;

	int transf_type = args->transf_type;
	struct timespec zero_time = {0};
	struct timespec curr_time = {0};

	clock_gettime(CLOCK_REALTIME, &curr_time);
	PipeData pipe_data_thread = {0};
	pipe_data_thread.thread_index = args->thread_index;
	pipe_data_thread.start_time = curr_time;

	int img_index = 0;
	while (true) {
		pthread_mutex_lock(&mutex_lock[transf_type]);
		if (0 == images_to_process_each_type[transf_type]) {
			pthread_mutex_unlock(&mutex_lock[transf_type]);

			break;
		}
		--images_to_process_each_type[transf_type];
		pthread_mutex_unlock(&mutex_lock[transf_type]);

		if (sizeof(img_index) != read(args->pipe_self[0], &img_index, sizeof(img_index))) {
			help(ERR_USING_PIPE, "read image index on thread");

			continue;
		}

		// Send the start time for the transformation.
		clock_gettime(CLOCK_REALTIME, &curr_time);
		PipeData pipe_data = {0};
		pipe_data.thread_index = -1;
		pipe_data.img_index = img_index;
		pipe_data.operation = transf_type;
		pipe_data.start_time = curr_time;
		pipe_data.end_time = zero_time;
		if (sizeof(pipe_data) != write(pipe_logs[1], &pipe_data, sizeof(pipe_data))) {
			help(ERR_USING_PIPE, "send transformation start time");

			continue;
		}

		char *filename = args->imgs_array[img_index];
		printf("thread %d type %d: %s\n", args->thread_index, transf_type, filename);
		gdImagePtr image = read_png_file(args->imgs_path, filename);
		if (NULL == image) {
			help(FILE_NOT_FOUND, filename);

			continue;
		}

		gdImagePtr out_image = NULL;
		if (TRANSF_TYPE_WATER == transf_type) {
			out_image = add_watermark(image, args->watermark);
			gdImageDestroy(image);
			image = NULL;
			if (NULL == out_image) {
				help(ERR_WATER, filename);

				continue;
			}
			save_image(out_image, args->imgs_path, WATER_DIR, filename);
		} else if (TRANSF_TYPE_THUMB == transf_type) {
			out_image = thumb_image(image, 640);
			gdImageDestroy(image);
			image = NULL;
			if (NULL == out_image) {
				help(ERR_THUMB, filename);

				continue;
			}
			save_image(out_image, args->imgs_path, THUMB_DIR, filename);
		} else if (TRANSF_TYPE_RESIZE == transf_type) {
			out_image = resize_image(image, 640);
			gdImageDestroy(image);
			image = NULL;
			if (NULL == out_image) {
				help(ERR_RESIZE, filename);

				continue;
			}
			save_image(out_image, args->imgs_path, RESIZE_DIR, filename);
		} else {
			break;
		}

		gdImageDestroy(out_image);
		out_image = NULL;

		// Send now also the end time for the transformation.
		clock_gettime(CLOCK_REALTIME, &curr_time);
		pipe_data.end_time = curr_time;
		if (sizeof(pipe_data) != write(pipe_logs[1], &pipe_data, sizeof(pipe_data))) {
			help(ERR_USING_PIPE, "send transformation times");

			continue;
		}

		if (transf_type != TRANSF_TYPE_RESIZE) {
			if (sizeof(img_index) != write(args->pipe_next[1], &img_index, sizeof(img_index))) {
				help(ERR_USING_PIPE, "send index to next transformation");
			}
		}
	}

	freeNew(thread_args);

	clock_gettime(CLOCK_REALTIME, &curr_time);
	pipe_data_thread.end_time = curr_time;
	if (sizeof(pipe_data_thread) != write(pipe_logs[1], &pipe_data_thread, sizeof(pipe_data_thread))) {
		help(ERR_USING_PIPE, "thread closing");
	}

	return NULL;
}

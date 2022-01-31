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
#include <unistd.h>
#pragma endregion

#define NUM_THREAD_TYPES 3

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

static void *process_image_set_resize(void *args) __attribute__((nonnull));
static void *process_image_set_thumb(void *args) __attribute__((nonnull));
static void *process_image_set_watermark(void *args) __attribute__((nonnull));

int pipe_t[2];
int pipe_r[2];

int threads_running_each_type[3] = {0, 0, 0};
pthread_mutex_t mutex_lock;
int max_threads_each_type = 0;

int main(int argc, char *argv[])
{
	if (3 != argc)
	{
		help(INVALID_ARGS, NULL);

		return EXIT_FAILURE;
	}
	int ret_var = EXIT_SUCCESS;

	printf("----- Running ap-paralelo-dinamico -----");
	/* Things to free/close in the end */
	char *base_path = NULL;
	char **input_files_names = NULL;
	pthread_t *threads = NULL;
	gdImagePtr watermark = NULL;
	char *stats_csv_path = NULL;
	FILE *stats_csv_file = NULL;
	int pipe_w[2];
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
		ret_var = EXIT_FAILURE;
		FREE_MEMORY
		return ret_var;
	}
	int max_type_threads = atoi(argv[2]);
	if ((max_type_threads <= 0) || (max_type_threads > input_files_count))
	{
		max_type_threads = input_files_count;
	}
	max_threads_each_type = max_type_threads;
	printf("Using %d Threads\n", max_type_threads);
	create_output_directories(base_path);

	threads = (pthread_t *)malloc(max_type_threads * NUM_THREAD_TYPES * sizeof(pthread_t));
	watermark = read_png_file(base_path, "watermark.png");
	if ((NULL == watermark) || (NULL == threads)) {
		help(ALLOCATION_FAIL, "sdlfkj");
		ret_var = EXIT_FAILURE;
		FREE_MEMORY
		return ret_var;
	}

	if ((pipe(pipe_w) < 0) || (pipe(pipe_r) < 0) || (pipe(pipe_t) < 0) || (pipe(pipe_threads_finalizadas) < 0)) {
		help(ERR_CREATING_PIPE, NULL);
		ret_var = EXIT_FAILURE;
		FREE_MEMORY

		return ret_var;
	}

	{
		// Iniciar as threads, max_type_threads de cada tipo.
		int thread_num = 0;
		threads_running_each_type[0] = max_type_threads;
		threads_running_each_type[1] = max_type_threads;
		threads_running_each_type[2] = max_type_threads;
		for (int i = 0; i < max_type_threads; ++i) {
			ThreadParams *thread_data = create_ThreadParams(thread_num, base_path, pipe_w, watermark,
															pipe_threads_finalizadas);
			pthread_create(&(threads[thread_num]), NULL, process_image_set_watermark, thread_data);
			++thread_num;
		}
		for (int i = 0; i < max_type_threads; ++i) {
			ThreadParams *thread_data = create_ThreadParams(thread_num, base_path, pipe_t, watermark,
															pipe_threads_finalizadas);
			pthread_create(&(threads[thread_num]), NULL, process_image_set_thumb, thread_data);
			++thread_num;
		}
		for (int i = 0; i < max_type_threads; ++i) {
			ThreadParams *thread_data = create_ThreadParams(thread_num, base_path, pipe_r, watermark,
															pipe_threads_finalizadas);
			pthread_create(&(threads[thread_num]), NULL, process_image_set_resize, thread_data);
			++thread_num;
		}
	}
	int size_sent = input_files_count * sizeof(char *);
	if (write(pipe_w[1], input_files_names, size_sent) != size_sent) {
		help(ERR_USING_PIPE, NULL);
		ret_var = EXIT_FAILURE;
		FREE_MEMORY

		return ret_var;
	}
	// Fechar os pipes para quando as threads chegarem ao fim deles pipe, o read() não ficar preso e sair porque
	// encontrou um caracter EOF, colocado lá pelo close().
	close(pipe_w[1]);

	for (int i = 0; i < max_type_threads * NUM_THREAD_TYPES; ++i) {
		int thread_to_close;
		if (sizeof(int) != read(pipe_threads_finalizadas[0], &thread_to_close, sizeof(int))) {
			break;
		}
		pthread_join(threads[thread_to_close], NULL);
		printf("closed thread: %d \n",thread_to_close);
	}
	close(pipe_threads_finalizadas[1]); // Aqui é fechado apenas porque não vai ser mais usado

	pthread_mutex_destroy(&mutex_lock);

	stats_csv_path = img_path_generator(base_path, "", "stats.csv");
	stats_csv_file = fopen(stats_csv_path, "w");
	freeNew(stats_csv_path);
	if (NULL == stats_csv_file)
	{
		help(FILE_WRITE_FAIL, "stats.csv");
		ret_var = EXIT_FAILURE;

		FREE_MEMORY
		return ret_var;
	}
	fprintf(stats_csv_file, "\n");
	clock_gettime(CLOCK_REALTIME, &(timer.end));
	fprintf(stats_csv_file, "Total,%ld.%ld,%ld.%ld\n", timer.start.tv_sec, timer.start.tv_nsec, timer.end.tv_sec,
			timer.end.tv_nsec);

	FREE_MEMORY
	return ret_var;
}

/**
 * @brief Thread function to do the resize operation.
 *
 * @param args a pointer to a struct of type ThreadParams
 *
 * @return NULL
 */
static void *process_image_set_resize(void *args) {
	char *filename = NULL;
	ThreadParams *targs = (ThreadParams *) args;
	while (sizeof(char *) == read(pipe_r[0], &filename, sizeof(char *)))
	{
		printf("thread %d resize: %s\n", targs->thread_id, filename);
		gdImagePtr image = read_png_file(targs->imgs_path, filename);
		if (NULL == image) {
			help(FILE_NOT_FOUND, filename);

			continue;
		}

		gdImagePtr out_image = resize_image(image, 640);
		if (NULL == out_image) {
			help(ERR_RESIZE, filename);

			continue;
		}

		save_image(out_image, targs->imgs_path, RESIZE_DIR, filename);
		gdImageDestroy(out_image);
		out_image = NULL;
	}
	write(targs->ret_pipe[1], &(targs->thread_id), sizeof(int));

	return NULL;
}

/**
 * @brief Thread function to do the thumbnail insertion operation.
 *
 * @param args a pointer to a struct of type ThreadParams
 *
 * @return NULL
 */
static void *process_image_set_thumb(void *args) {
	char *filename = NULL;
	ThreadParams *targs = (ThreadParams *) args;
	while (sizeof(char *) == read(pipe_t[0], &filename, sizeof(char *)))
	{
		printf("thread %d thumbnail: %s\n", targs->thread_id, filename);
		gdImagePtr image = read_png_file(targs->imgs_path, filename);
		if (NULL == image) {
			help(FILE_NOT_FOUND, filename);

			continue;
		}

		gdImagePtr out_image = thumb_image(image, 640);
		if (NULL == out_image) {
			help(ERR_THUMB, filename);

			continue;
		}

		save_image(out_image, targs->imgs_path, THUMB_DIR, filename);
		gdImageDestroy(out_image);
		out_image = NULL;

		write(pipe_r[1], &filename, sizeof(char *));
	}
	pthread_mutex_lock(&mutex_lock);
	--(threads_running_each_type[1]);
	if (0 == threads_running_each_type[1]) {
		close(pipe_r[1]);
	}
	pthread_mutex_unlock(&mutex_lock);

	write(targs->ret_pipe[1], &(targs->thread_id), sizeof(int));

	return NULL;
}

/**
 * @brief Thread function to do the watermark insertion operation.
 *
 * @param args a pointer to a struct of type ThreadParams
 *
 * @return NULL
 */
static void *process_image_set_watermark(void *args) {
	char *filename = NULL;
	ThreadParams *targs = (ThreadParams *) args;
	while (sizeof(char *) == read(targs->pipe[0], &filename, sizeof(char *)))
	{
		printf("thread %d watermark: %s\n", targs->thread_id, filename);
		gdImagePtr image = read_png_file(targs->imgs_path, filename);
		if (NULL == image) {
			help(FILE_NOT_FOUND, filename);

			continue;
		}

		gdImagePtr out_image = add_watermark(image, targs->watermark);
		if (NULL == out_image) {
			help(ERR_WATER, filename);

			continue;
		}

		save_image(out_image, targs->imgs_path, WATER_DIR, filename);
		gdImageDestroy(out_image);
		out_image = NULL;

		write(pipe_t[1], &filename, sizeof(char *));
	}

	pthread_mutex_lock(&mutex_lock);
	--(threads_running_each_type[0]);
	if (0 == threads_running_each_type[0]) {
		close(pipe_t[1]);
	}
	pthread_mutex_unlock(&mutex_lock);

	write(targs->ret_pipe[1], &(targs->thread_id), sizeof(int));

	return NULL;
}

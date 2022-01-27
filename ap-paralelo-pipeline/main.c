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
#include <unistd.h>
#include <errno.h>
#pragma endregion

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

int main(int argc, char *argv[])
{
	if (3 != argc)
	{
		help(INVALID_ARGS, NULL);

		return EXIT_FAILURE;
	}
	int ret_var = EXIT_SUCCESS;

	printf("----- Running ap-paralelo-dinamico -----");
	/* Things to deallocate in the end */
	char *base_path = NULL;
	char **input_files_names = NULL;
	pthread_t *threads = NULL;
	gdImagePtr watermark = NULL;
	char *stats_csv_path = NULL;
	FILE *stats_csv_file = NULL;
	int pipe_w[2];
	int pipe_t[2];
	int pipe_r[2];
	int pipe_threads[2];
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
	int max_threads = atoi(argv[2]);
	if (max_threads <= 0 || max_threads > input_files_count)
	{
		max_threads = input_files_count;
	}
	printf("Using %d Threads of each type\n", max_threads);
	create_output_directories(base_path);

	threads = (pthread_t *)malloc(max_threads * 3 * sizeof(pthread_t)); //3 times because we have 3 types of threads
	watermark = read_png_file(base_path, "watermark.png");
	if (NULL == watermark || NULL == threads)
	{
		help(ALLOCATION_FAIL, "sdlfkj");
		ret_var = EXIT_FAILURE;
		FREE_MEMORY
		return ret_var;
	}

	if (pipe(pipe_w) < 0 || pipe(pipe_r) < 0 || pipe(pipe_t) < 0 || pipe(pipe_threads) < 0)
	{
		help(ERR_CREATING_PIPE, NULL);
		ret_var = EXIT_FAILURE;
		FREE_MEMORY
		return ret_var;
	}
	for (int i = 0; i < max_threads * 3; i += 3)
	{
		thread_args *thread_data = create_thread_args(i, base_path, pipe_w, pipe_t, watermark, pipe_threads);
		pthread_create(&(threads[i]), NULL, process_image_set_watermark, thread_data);
		thread_data = create_thread_args(i + 1, base_path, pipe_t, pipe_r, watermark, pipe_threads);
		pthread_create(&(threads[i + 1]), NULL, process_image_set_thumb, thread_data);
		thread_data = create_thread_args(i + 2, base_path, pipe_r, NULL, watermark, pipe_threads);
		pthread_create(&(threads[i + 2]), NULL, process_image_set_resize, thread_data);
	}
	if (write(pipe_w[1], input_files_names, input_files_count * sizeof(char *)) < 0)
	{
		help(ERR_USING_PIPE, NULL);
		ret_var = EXIT_FAILURE;
		FREE_MEMORY
		return ret_var;
	}
	close(pipe_w[1]);
	int thread_to_close;
	int total_threads = 0;
	int watermark_threads = max_threads;
	int thumb_threads = max_threads;
	while ((total_threads != max_threads * 3) && (read(pipe_threads[0], &thread_to_close, sizeof(int)) == sizeof(int)))
	{
		switch (thread_to_close % 3)
		{
		case 0:
			watermark_threads--;
			if (watermark_threads == 0)
				close(pipe_t[1]);
			break;
		case 1:
			thumb_threads--;
			if (thumb_threads == 0)
				close(pipe_r[1]);
			break;
		default:
			break;
		}
		total_threads++;
		pthread_join(threads[thread_to_close], NULL);
		printf("closed thread: %d \n", thread_to_close);
	}
	close(pipe_threads[1]);
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
 * @param args a pointer to a struct of type thread_args
 *
 * @return NULL
 */
static void *process_image_set_resize(void *args)
{
	char *filename = NULL;
	thread_args *targs = (thread_args *)args;
	while (read(targs->pipe_read[0], &filename, sizeof(char *)) == sizeof(char *))
	{
		printf("thread %d resize: %s\n", targs->thread_id, filename);
		gdImagePtr image = read_png_file(targs->imgs_path, filename);
		if (NULL == image)
		{
			help(ERR_RESIZE, filename);
			continue;
		}
		else
		{
			gdImagePtr out_image = resize_image(image, 640);
			if (NULL != out_image)
			{
				save_image(out_image, targs->imgs_path, RESIZE_DIR, filename);
				gdImageDestroy(out_image);
				gdImageDestroy(image);
				out_image = NULL;
			}
			else
			{
				help(ERR_RESIZE, filename);
				continue;
			}
		}
	}
	write(targs->ret_pipe[1], &(targs->thread_id), sizeof(int));
	free(targs);
	return NULL;
}

/**
 * @brief Thread function to do the thumbnail insertion operation.
 *
 * @param args a pointer to a struct of type thread_args
 *
 * @return NULL
 */
static void *process_image_set_thumb(void *args)
{
	char *filename = NULL;
	thread_args *targs = (thread_args *)args;
	while (read(targs->pipe_read[0], &filename, sizeof(char *)) == sizeof(char *))
	{
		printf("thread %d thumbnail: %s\n", targs->thread_id, filename);
		gdImagePtr image = read_png_file(targs->imgs_path, filename);
		if (NULL == image)
		{
			help(ERR_THUMB, filename);
			continue;
		}
		else
		{
			gdImagePtr out_image = thumb_image(image, 200);
			if (NULL != out_image)
			{
				save_image(out_image, targs->imgs_path, THUMB_DIR, filename);
				gdImageDestroy(out_image);
				gdImageDestroy(image);
				out_image = NULL;
				if (write(targs->pipe_write[1], &filename, sizeof(char *)) != sizeof(char *))
				{
					help(ERR_USING_PIPE, "THUMB");
					continue;
				}
			}
			else
			{
				help(ERR_THUMB, filename);
				continue;
			}
		}
	}
	write(targs->ret_pipe[1], &(targs->thread_id), sizeof(int));
	free(targs);
	return NULL;
}

/**
 * @brief Thread function to do the watermark insertion operation.
 *
 * @param args a pointer to a struct of type thread_args
 *
 * @return NULL
 */
static void *process_image_set_watermark(void *args)
{
	char *filename = NULL;
	thread_args *targs = (thread_args *)args;
	while (read(targs->pipe_read[0], &filename, sizeof(char *)) == sizeof(char *))
	{
		printf("thread %d watermark: %s\n", targs->thread_id, filename);
		gdImagePtr image = read_png_file(targs->imgs_path, filename);
		if (NULL == image)
		{
			help(ERR_WATER, filename);
			continue;
		}
		else
		{
			gdImagePtr out_image = add_watermark(image, targs->watermark);
			if (NULL != out_image)
			{
				save_image(out_image, targs->imgs_path, WATER_DIR, filename);
				gdImageDestroy(out_image);
				gdImageDestroy(image);
				out_image = NULL;
				if (write(targs->pipe_write[1], &filename, sizeof(char *)) != sizeof(char *))
				{
					help(ERR_USING_PIPE, "Watermark");
					continue;
				}
			}
			else
			{
				help(ERR_WATER, filename);
				continue;
			}
		}
	}
	write(targs->ret_pipe[1], &(targs->thread_id), sizeof(int));
	free(targs);
	return NULL;
}

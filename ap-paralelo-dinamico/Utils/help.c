/**
 * @file help.c
 * @author Manuel Soares,Eduardo David Faustino
 * @brief
 * @version 0.1
 * @date 2022-01-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "help.h"
#include "stdlib.h"
#include <stdio.h>

inline void help(int error_id, char *extra_info)
{
	switch (error_id)
	{
		case INVALID_ARGS:
			fprintf(stderr, "Invalid Arguments\nProper Argument usage is\n./ap-complexo-simples TARGER_DIRECTORY MAX_THREADS\n");
			
			break;
		case FILE_NOT_FOUND:
			fprintf(stderr, "Target File Not Found\n");

			break;
		case DIR_CREATION_FAIL:
			fprintf(stderr, "Final Directory Creations Failed\n");

			break;
		case ALLOCATION_FAIL:
			fprintf(stderr, "Memory Allocation Failed\n");

			break;
		case NO_FILES_FOUND:
			fprintf(stderr, "No Files Found In Target List or No files to process in said list\n");

			break;
		case FILE_READ_FAIL:
			fprintf(stderr, "%s : Failed to read file\n", extra_info);

			break;
		case FILE_WRITE_FAIL:
			fprintf(stderr, "%s : Failed to write file\n", extra_info);

			break;
		case ERR_RESIZE:
			fprintf(stderr, "Could not resize the image: %s\n", extra_info);

			break;
		case ERR_THUMB:
			fprintf(stderr, "Could not create thumbnail for image: %s\n", extra_info);

			break;
		case ERR_WATER:
			fprintf(stderr, "Could not add watermark to the image: %s\n", extra_info);

			break;
		case ERR_WRITING_CSV:
			fprintf(stderr, "Could not write the CSV file successfully\n");
			break;
		case ERR_CREATING_PIPE:
			fprintf(stderr, "Could not create the pipes successfully\n");
			break;
		default:
			fprintf(stderr, "How did we end up here\nMy guess is a programmer made a big oopsie\n ¯\\_(ツ)_/¯\n");

			break;
	}
}

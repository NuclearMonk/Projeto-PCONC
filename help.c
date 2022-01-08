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

/**
 * @brief Throws an error and exits the program
 *
 *
 * @param error_id Error codes defined in by macro at the top of this file
 */
void help(int error_id, char *extra_info)
{
    switch (error_id)
    {
    case INVALID_ARGS:
        fprintf(stderr, "Invalid Arguments\nProper Argument usage is\n./ap-paralelo-simples TARGER_DIRECTORY MAX_THREADS\n");
        exit(EXIT_FAILURE);
    case FILE_NOT_FOUND:
        fprintf(stderr, "Target File Not Found\n");
        exit(EXIT_FAILURE);
    case DIR_CREATION_FAIL:
        fprintf(stderr, "Final Directory Creations Failed\n");
        exit(EXIT_FAILURE);
    case ALLOCATTIONION_FAIL:
        fprintf(stderr, "Memory Allocation Failed\n");
        exit(EXIT_FAILURE);
    case NO_FILES_FOUND:
        fprintf(stderr, "No Files Found In Target Directory\n");
        exit(EXIT_FAILURE);
    case FILE_READ_FAIL:
        fprintf(stderr, "%s : Failed to read file\n", extra_info);
        break;
    case FILE_WRITE_FAIL:
        fprintf(stderr, "%s : Failed to write file\n", extra_info);
        break;
    default:
        fprintf(stderr, "Unkown Error\n");
        exit(EXIT_FAILURE);
    }
}

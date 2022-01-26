/**
 * @file help.h
 * @author Manuel Soares, Eduardo Faustino
 * @brief
 * @version 0.1
 * @date 2022-01-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef PROJETO_PCONC_HELP_H
#define PROJETO_PCONC_HELP_H

#pragma region ERROR CODES
#define INVALID_ARGS 1
#define FILE_NOT_FOUND 2
#define DIR_CREATION_FAIL 3
#define ALLOCATION_FAIL 4
#define NO_FILES_FOUND 5
#define FILE_READ_FAIL 6
#define FILE_WRITE_FAIL 7
#define ERR_RESIZE 9
#define ERR_THUMB 10
#define ERR_WATER 11
#define ERR_WRITING_CSV 12
#pragma endregion


/**
 * @brief Throws an error and exits the program
 *
 * @param error_id error codes defined in help.h
 * @param extra_info information that will be printed depending on error_id
 */
void help(int error_id, char *extra_info);

#endif //PROJETO_PCONC_HELP_H

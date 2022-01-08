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

#pragma region ERROR CODES
#define INVALID_ARGS 1
#define FILE_NOT_FOUND 2
#define DIR_CREATION_FAIL 3
#define ALLOCATION_FAIL 4
#define NO_FILES_FOUND 5
#define FILE_READ_FAIL 6
#define FILE_WRITE_FAIL 7
#pragma endregion


/**
 * @brief Throws an error and exits the program
 *
 * @param error_id Error codes defined in by macro at the top of this file
 * @param extra_info info that will be printed depending on error_id
 */
void help(int error_id, char *extra_info);

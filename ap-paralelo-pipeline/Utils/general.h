/**
 * @file filehandler.c
 * @author Manuel Soares,Eduardo Faustino
 * @brief
 * @version 0.1
 * @date 2022-01-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef PROJETO_PCONC_GENERAL_H
#define PROJETO_PCONC_GENERAL_H

#define freeNew(PTR) free(PTR); PTR = NULL
#define fcloseNew(FILE) if(NULL != FILE) fclose(FILE); FILE = NULL


#endif //PROJETO_PCONC_GENERAL_H

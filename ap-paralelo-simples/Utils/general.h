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

#define freeNew(ptr)  if(NULL!=ptr)free(ptr); ptr = NULL
#define fcloseNew(file) if(NULL!=file)fclose(file); file = NULL

#endif //PROJETO_PCONC_GENERAL_H

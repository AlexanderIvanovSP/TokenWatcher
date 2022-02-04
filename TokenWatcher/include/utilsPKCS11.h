/*************************************************************************
* Rutoken                                                                *
* Copyright (c) 2003-2021, Aktiv-Soft JSC. All rights reserved.          *
* Подробная информация:  http://www.rutoken.ru                           *
*------------------------------------------------------------------------*
* Пример работы с Рутокен при помощи библиотеки PKCS#11 на языке C       *
*------------------------------------------------------------------------*
* Заголовочный файл утилитарных функций для работы с PKCS#11             *
*************************************************************************/

#ifndef UTILSPKCS11_H
#define UTILSPKCS11_H

#include "commonPKCS11.h"

extern CK_FUNCTION_LIST_PTR functionList;                 // Указатель на список функций PKCS#11, хранящийся в структуре CK_FUNCTION_LIST
extern CK_FUNCTION_LIST_EXTENDED_PTR functionListEx;      // Указатель на список функций расширения PKCS#11, хранящийся в структуре CK_FUNCTION_LIST_EXTENDED
extern BOOL pkcs11LibState;

/* Функция для инициализации библиотеки PKCS#11 */
int init_pkcs11(char* pkcsPath);
/* Функция для деинициализации библиотеки PKCS#11 */
int free_pkcs11();

#endif // UTILS_h

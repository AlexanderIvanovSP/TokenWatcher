/*************************************************************************
* Rutoken                                                                *
* Copyright (c) 2003-2021, Aktiv-Soft JSC. All rights reserved.          *
* Подробная информация:  http://www.rutoken.ru                           *
*------------------------------------------------------------------------*
* Пример работы с Рутокен при помощи библиотеки PKCS#11 на языке C       *
*------------------------------------------------------------------------*
* Утилитарные функции для работы с PKCS#11                               *
*************************************************************************/

#include "utilsPKCS11.h"

#define PKCS11_LOADED 1
#define PKCS11_NOT_LOADED 0

CK_FUNCTION_LIST_PTR functionList;                 // Указатель на список функций PKCS#11, хранящийся в структуре CK_FUNCTION_LIST
CK_FUNCTION_LIST_EXTENDED_PTR functionListEx;      // Указатель на список функций расширения PKCS#11, хранящийся в структуре CK_FUNCTION_LIST_EXTENDED
static HMODULE module;
BOOL pkcs11LibState = PKCS11_NOT_LOADED;

int init_pkcs11(char* pkcsPath)
{

	CK_C_GetFunctionList getFunctionList;              // Указатель на функцию C_GetFunctionList
	CK_C_EX_GetFunctionListExtended getFunctionListEx; // Указатель на функцию C_EX_GetFunctionListExtended

	/* Параметры для инициализации библиотеки: разрешаем использовать объекты синхронизации операционной системы */
	CK_C_INITIALIZE_ARGS initArgs = { NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, CKF_OS_LOCKING_OK, NULL_PTR };

	CK_RV rv;					   // Код возврата PKCS#11 функций
	int errorCode = 1;                                 // Флаг ошибки

	/*************************************************************************
	* Выполнить действия для начала работы с библиотекой PKCS#11             *
	*************************************************************************/
	printf("Initialization...\n");

	/*************************************************************************
	* Загрузить библиотеку                                                   *
	*************************************************************************/
	module = LoadLibrary(pkcsPath);
	CHECK(" LoadLibrary", module != NULL, exit);

	/*************************************************************************
	* Получить адрес функции запроса структуры с указателями на функции      *
	*************************************************************************/
	getFunctionList = (CK_C_GetFunctionList)GetProcAddress(module, "C_GetFunctionList");
	CHECK(" GetProcAddress (C_GetFunctionList)", getFunctionList != NULL, unload_pkcs11);

	/*************************************************************************
	* Получить адрес функции запроса структуры с указателями на функции      *
	* расширения стандарта PKCS#11                                           *
	*************************************************************************/
	getFunctionListEx = (CK_C_EX_GetFunctionListExtended)GetProcAddress(module, "C_EX_GetFunctionListExtended");
	CHECK(" GetProcAddress (C_EX_GetFunctionListExtended)", getFunctionList != NULL, unload_pkcs11);

	/*************************************************************************
	* Получить структуру с указателями на функции                            *
	*************************************************************************/
	rv = getFunctionList(&functionList);
	CHECK_AND_LOG(" Get function list", rv == CKR_OK, rvToStr(rv), unload_pkcs11);

	/*************************************************************************
	* Получить структуру с указателями на функции расширения стандарта       *
	*************************************************************************/
	rv = getFunctionListEx(&functionListEx);
	CHECK_AND_LOG(" Get function list extended", rv == CKR_OK, rvToStr(rv), unload_pkcs11);

	/*************************************************************************
	* Инициализировать библиотеку                                            *
	*************************************************************************/
	rv = functionList->C_Initialize(&initArgs);
	CHECK_AND_LOG(" C_Initialize", rv == CKR_OK, rvToStr(rv), unload_pkcs11);

	errorCode = 0;
	pkcs11LibState = PKCS11_LOADED;

	/*************************************************************************
	* Выгрузить библиотеку из памяти                                         *
	*************************************************************************/
unload_pkcs11:
	if (errorCode)
		CHECK_RELEASE(" FreeLibrary", FreeLibrary(module), errorCode);
exit:
	return errorCode;
}

int free_pkcs11()
{
	CK_RV rv = CKR_FUNCTION_REJECTED;
	int errorCode = 1;

	if (functionList) {
		printf("\nFinalizing... \n");
		__try {
			rv = functionList->C_Finalize(NULL_PTR);
		}
		__finally {
			CHECK_RELEASE_AND_LOG(" C_Finalize", rv == CKR_OK, rvToStr(rv), errorCode);
		}

	}

	CHECK_RELEASE(" FreeLibrary", FreeLibrary(module), errorCode);
	pkcs11LibState = PKCS11_NOT_LOADED;

	return errorCode;
}
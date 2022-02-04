/*
* Модуль реализует функционал чтения конфигурационного файла
*/
#include "config.h"

#define CFG_SECTION_CONNECT "CONNECTION"
#define CFG_KEY_IP "IP"
#define CFG_KEY_PORT "PORT"
#define CFG_KEY_HTTP_PORT "HTTP_PORT"
#define CFG_KEY_OSI_LEVEL "OSI_LEVEL"
#define CFG_KEY_HTTP_MODE "HTTP_MODE"

#define CFG_SECTION_DEBUG "DEBUG"
#define CFG_KEY_PKCS11_MODE "PKCS11_MODE"
#define CFG_KEY_LOG "LOG"
#define CFG_KEY_LOG_T "LOG_T"
#define CFG_KEY_REPORT "REPORT"
#define CFG_KEY_SHORT_LOG "SHORT_LOG"
#define CFG_KEY_SHORT_LOG_T "SHORT_LOG_T"

#define DEFAULT_LOG_T "d"
#define DEFAULT_SHORT_LOG_T "d"
#define DEFAULT_SHORT_LOG "no"
#define DEFAULT_LOG_MODE "no"
#define DEFAULT_PKCS11_MODE 0
#define DEFAULT_REPORT_MODE "no"
#define DEFAULT_OSI_LEVEL 4
#define DEFAULT_HTTP_MODE "no"

char cfgPath[MAX_PATH];
/// <summary>
/// Функция осуществляет чтение опции из конфигурационного файла
/// </summary>
/// <param name="section">Имя секции</param>
/// <param name="key">Имя запрашиваемого параметра</param>
/// <param name="out">Буфер для записи считанного значения</param>
/// <returns></returns>
static DWORD readConfigIni(char* section, char* key, char* out) {

	return GetPrivateProfileString(
		section,
		key,
		"",
		out,
		MAX_SZ_STR_CFG,
		cfgPath
	);
}
/// <summary>
/// Функция возвращает значение периода логирования
/// d - по дням
/// h - по часам
/// по умолчанию - "d"
/// </summary>
/// <param name="out">Буфер для записи считанного значения</param>
void getLogMode_T(char* out) {
	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_LOG_T, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_LOG_T);

	return;
}
/// <summary>
/// Функция возвращает значение периода Short-логирования
/// d - по дням
/// h - по часам
/// по умолчанию - "d"
/// </summary>
/// <param name="out">Буфер для записи считанного значения</param>
void getShortLogMode_T(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_SHORT_LOG_T, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_SHORT_LOG_T);

	return;
}
/// <summary>
/// Функция возвращает режим Short-логирования
/// yes - включить логирование
/// no - отключить логирование
/// по умолчанию - "no"
/// </summary>
/// <param name="out">Буфер для записи считанного значения</param>
void getShortLogMode(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_SHORT_LOG, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_SHORT_LOG);

	return;
}
/// <summary>
/// Функция возвращает адрес сервера 1с
/// по умолчанию - "localhost"
/// </summary>
/// <param name="out">Буфер для записи считанного значения</param>
void getIP(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_IP, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_ADDR);

	return;
}
/// <summary>
/// Функция возвращает порт сервера 1с
/// по умолчанию - "27015"
/// </summary>
/// <param name="out">Буфер для записи считанного значения</param>
void getPORT(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_PORT, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_PORT);

	return;
}
/// <summary>
/// Функция возвращает режим http сервера
/// yes - включить
/// no - отключить
/// по умолчанию - "no"
/// </summary>
/// <param name="out">Буфер для записи считанного значения</param>
void getHttpMode(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_HTTP_MODE, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_HTTP_MODE);

	return;
}
/// <summary>
/// Функция возвращает формат передаваемых пакетов на сервер 1с
/// 4 - datagram
/// 7 - http
/// по умолчанию - 4
/// </summary>
/// <returns>Формат пакетов</returns>
UINT getOsiLevel() {

	return GetPrivateProfileInt(CFG_SECTION_CONNECT, CFG_KEY_OSI_LEVEL, DEFAULT_OSI_LEVEL, cfgPath);
}
/// <summary>
/// Функция возвращает путь для поиска библитеки pkcs11
/// 1 - rtpkcs11.dll в каталоге * .exe файла службы
/// 2 - rtpkcs11ecp.dll в каталоге * .exe файла службы
/// 3 - rtpkcs11ecp.dll в каталоге Windows
/// 0 - rtpkcs11.dll в каталоге Windows
/// по умолчанию - rtpkcs11.dll в каталоге Windows
/// </summary>
/// <returns>Путь для поиска библитеки pkcs11</returns>
UINT getPkcs11DllMode() {

	return GetPrivateProfileInt(CFG_SECTION_DEBUG, CFG_KEY_PKCS11_MODE, DEFAULT_PKCS11_MODE, cfgPath);
}
/// <summary>
/// Функция возвращает режим логирования
/// yes - включить логирование
/// no - отключить логирование
/// по умолчанию - "no"
/// </summary>
/// <param name="out">Буфер для записи считанного значения</param>
void getLogMode(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_LOG, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_LOG_MODE);

	return;
}
/// <summary>
/// Функция возвращает режим работы с ошибками
/// yes - включить отправку ошибок на сервер 1С
/// no - отключить отправку ошибок на сервер 1С
/// по умолчанию - "no"
/// </summary>
/// <param name="out">Буфер для записи считанного значения</param>
void getReportMode(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_REPORT, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_REPORT_MODE);

	return;
}
/// <summary>
/// Функция возвращает порт разворачиваемого http сервера
/// Сервер всегда предоставляет данные по последнему подключенному токену
/// по умолчанию - "8000"
/// </summary>
/// <param name="out">Буфер для записи считанного значения</param>
void getHttpPort(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_HTTP_PORT, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_HTTP_PORT);

	return;
}
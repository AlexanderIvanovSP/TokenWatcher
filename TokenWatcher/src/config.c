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

void getLogMode_T(char* out) {
	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_LOG_T, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_LOG_T);

	return;
}

void getShortLogMode_T(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_SHORT_LOG_T, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_SHORT_LOG_T);

	return;
}

void getShortLogMode(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_SHORT_LOG, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_SHORT_LOG);

	return;
}

void getIP(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_IP, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_ADDR);

	return;
}

void getPORT(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_PORT, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_PORT);

	return;
}

void getHttpMode(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_HTTP_MODE, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_HTTP_MODE);

	return;
}

UINT getOsiLevel() {

	return GetPrivateProfileInt(CFG_SECTION_CONNECT, CFG_KEY_OSI_LEVEL, DEFAULT_OSI_LEVEL, cfgPath);
}

UINT getPkcs11DllMode() {

	return GetPrivateProfileInt(CFG_SECTION_DEBUG, CFG_KEY_PKCS11_MODE, DEFAULT_PKCS11_MODE, cfgPath);
}

void getLogMode(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_LOG, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_LOG_MODE);

	return;
}


void getReportMode(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_REPORT, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_REPORT_MODE);

	return;
}

void getHttpPort(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_HTTP_PORT, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_HTTP_PORT);

	return;
}
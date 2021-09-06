#include "config.h"

#define NAME_CONFIG_FILE "config.ini"

#define CFG_SECTION_CONNECT "CONNECTION"
#define CFG_KEY_IP "IP"
#define CFG_KEY_PORT "PORT"
#define CFG_KEY_HTTP_PORT "HTTP_PORT"
#define CFG_KEY_OSI_LEVEL "OSI_LEVEL"

#define CFG_SECTION_DEBUG "DEBUG"
#define CFG_KEY_PKCS11_MODE "PKCS11_MODE"
#define CFG_KEY_LOG "LOG"
#define CFG_KEY_REPORT "REPORT"

#define DEFAULT_LOG_MODE "no"
#define DEFAULT_PKCS11_MODE 0
#define DEFAULT_REPORT_MODE "no"
#define DEFAULT_OSI_LEVEL 7


static DWORD readConfigIni(char* section, char* key, char* out) {

	char cfgPath[MAX_PATH] = { 0 };

	GetModuleFileName(GetModuleHandle(NULL), cfgPath, MAX_PATH);
	*strrchr(cfgPath, '\\') = '\0';
	strcat_s(cfgPath, MAX_PATH, "\\");
	strcat_s(cfgPath, MAX_PATH, NAME_CONFIG_FILE);

	return GetPrivateProfileString(
		section,
		key,
		"",
		out,
		MAX_SZ_STR_CFG,
		cfgPath
	);
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

UINT getOsiLevel() {

	char cfgPath[MAX_PATH] = { 0 };

	GetModuleFileName(GetModuleHandle(NULL), cfgPath, MAX_PATH);
	*strrchr(cfgPath, '\\') = '\0';
	strcat_s(cfgPath, MAX_PATH, "\\");
	strcat_s(cfgPath, MAX_PATH, NAME_CONFIG_FILE);

	return GetPrivateProfileInt(CFG_SECTION_CONNECT, CFG_KEY_OSI_LEVEL, DEFAULT_OSI_LEVEL, cfgPath);
}

UINT getPkcs11DllMode() {

	char cfgPath[MAX_PATH] = { 0 };

	GetModuleFileName(GetModuleHandle(NULL), cfgPath, MAX_PATH);
	*strrchr(cfgPath, '\\') = '\0';
	strcat_s(cfgPath, MAX_PATH, "\\");
	strcat_s(cfgPath, MAX_PATH, NAME_CONFIG_FILE);

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


void offLogMode(char* out) {

	strcpy_s(out, MAX_SZ_STR_CFG, "no");

	return;
}
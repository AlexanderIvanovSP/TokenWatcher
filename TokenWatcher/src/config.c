/*
* ������ ��������� ���������� ������ ����������������� �����
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
/// ������� ������������ ������ ����� �� ����������������� �����
/// </summary>
/// <param name="section">��� ������</param>
/// <param name="key">��� �������������� ���������</param>
/// <param name="out">����� ��� ������ ���������� ��������</param>
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
/// ������� ���������� �������� ������� �����������
/// d - �� ����
/// h - �� �����
/// �� ��������� - "d"
/// </summary>
/// <param name="out">����� ��� ������ ���������� ��������</param>
void getLogMode_T(char* out) {
	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_LOG_T, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_LOG_T);

	return;
}
/// <summary>
/// ������� ���������� �������� ������� Short-�����������
/// d - �� ����
/// h - �� �����
/// �� ��������� - "d"
/// </summary>
/// <param name="out">����� ��� ������ ���������� ��������</param>
void getShortLogMode_T(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_SHORT_LOG_T, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_SHORT_LOG_T);

	return;
}
/// <summary>
/// ������� ���������� ����� Short-�����������
/// yes - �������� �����������
/// no - ��������� �����������
/// �� ��������� - "no"
/// </summary>
/// <param name="out">����� ��� ������ ���������� ��������</param>
void getShortLogMode(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_SHORT_LOG, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_SHORT_LOG);

	return;
}
/// <summary>
/// ������� ���������� ����� ������� 1�
/// �� ��������� - "localhost"
/// </summary>
/// <param name="out">����� ��� ������ ���������� ��������</param>
void getIP(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_IP, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_ADDR);

	return;
}
/// <summary>
/// ������� ���������� ���� ������� 1�
/// �� ��������� - "27015"
/// </summary>
/// <param name="out">����� ��� ������ ���������� ��������</param>
void getPORT(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_PORT, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_PORT);

	return;
}
/// <summary>
/// ������� ���������� ����� http �������
/// yes - ��������
/// no - ���������
/// �� ��������� - "no"
/// </summary>
/// <param name="out">����� ��� ������ ���������� ��������</param>
void getHttpMode(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_HTTP_MODE, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_HTTP_MODE);

	return;
}
/// <summary>
/// ������� ���������� ������ ������������ ������� �� ������ 1�
/// 4 - datagram
/// 7 - http
/// �� ��������� - 4
/// </summary>
/// <returns>������ �������</returns>
UINT getOsiLevel() {

	return GetPrivateProfileInt(CFG_SECTION_CONNECT, CFG_KEY_OSI_LEVEL, DEFAULT_OSI_LEVEL, cfgPath);
}
/// <summary>
/// ������� ���������� ���� ��� ������ ��������� pkcs11
/// 1 - rtpkcs11.dll � �������� * .exe ����� ������
/// 2 - rtpkcs11ecp.dll � �������� * .exe ����� ������
/// 3 - rtpkcs11ecp.dll � �������� Windows
/// 0 - rtpkcs11.dll � �������� Windows
/// �� ��������� - rtpkcs11.dll � �������� Windows
/// </summary>
/// <returns>���� ��� ������ ��������� pkcs11</returns>
UINT getPkcs11DllMode() {

	return GetPrivateProfileInt(CFG_SECTION_DEBUG, CFG_KEY_PKCS11_MODE, DEFAULT_PKCS11_MODE, cfgPath);
}
/// <summary>
/// ������� ���������� ����� �����������
/// yes - �������� �����������
/// no - ��������� �����������
/// �� ��������� - "no"
/// </summary>
/// <param name="out">����� ��� ������ ���������� ��������</param>
void getLogMode(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_LOG, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_LOG_MODE);

	return;
}
/// <summary>
/// ������� ���������� ����� ������ � ��������
/// yes - �������� �������� ������ �� ������ 1�
/// no - ��������� �������� ������ �� ������ 1�
/// �� ��������� - "no"
/// </summary>
/// <param name="out">����� ��� ������ ���������� ��������</param>
void getReportMode(char* out) {

	if (!readConfigIni(CFG_SECTION_DEBUG, CFG_KEY_REPORT, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_REPORT_MODE);

	return;
}
/// <summary>
/// ������� ���������� ���� ���������������� http �������
/// ������ ������ ������������� ������ �� ���������� ������������� ������
/// �� ��������� - "8000"
/// </summary>
/// <param name="out">����� ��� ������ ���������� ��������</param>
void getHttpPort(char* out) {

	if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_HTTP_PORT, out))
		strcpy_s(out, MAX_SZ_STR_CFG, DEFAULT_HTTP_PORT);

	return;
}
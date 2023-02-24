#include "check.h"

bool IsWindows81orHigher() {
	NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);
	OSVERSIONINFOEXW osInfo;

	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

	if (NULL != RtlGetVersion)
	{
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);
		return (osInfo.dwMajorVersion >= 6 && osInfo.dwMinorVersion >= 3) || osInfo.dwMajorVersion >= 10;
	}

	return false;
}
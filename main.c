#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tlhelp32.h>

DWORD pid(const char* file) {
  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);
  HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
  if(Process32First(hsnap, &entry)) {
    while(Process32Next(hsnap, &entry)) {
      if(!strcmp(entry.szExeFile, file)) {
	return entry.th32ProcessID;
      }
    }
  }
  return 0;
}

int main() {
  const char* dll = "C:\\Users\\22noa\\Desktop\\dll\\intern.dll";
  DWORD id = 0;
  id = pid("ac_client.exe");
  if(id != 0) {
    printf("got pid\n");
    HANDLE hproc = OpenProcess(PROCESS_ALL_ACCESS, 0, id);
    if(hproc) {
      FARPROC lib = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
      if(lib) {
	printf("got loadlib lib\n");
      }	
      printf("got process handle\n");
      void* alloc_mem = VirtualAllocEx(hproc, 0, sizeof(dll), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
      if(alloc_mem) {
	printf("got allocated memory\n");
	DWORD protection;
	VirtualProtectEx(hproc, alloc_mem, sizeof(dll), PAGE_EXECUTE_READWRITE, &protection);
	WriteProcessMemory(hproc, alloc_mem, dll, strlen(dll) + 1, 0);

	VirtualProtectEx(hproc, alloc_mem, sizeof(alloc_mem), protection, &protection);
	printf("wrote to mem \n");
      }
      HANDLE thread = CreateRemoteThread(hproc, 0, 0, (LPTHREAD_START_ROUTINE)lib, alloc_mem, 0, 0);
      if(thread) {
	printf("thread created\n");
	WaitForSingleObject(thread, 30000);
	unsigned int exit_code;
	GetExitCodeThread(thread, &exit_code);

	printf("exit code: %d", exit_code);
	CloseHandle(thread);
      }
      CloseHandle(hproc);
    }
  }
  getchar();
  return 0;
}

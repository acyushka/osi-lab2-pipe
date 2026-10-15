#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

static void close_if_valid(HANDLE h) {
    if (h && h != INVALID_HANDLE_VALUE) CloseHandle(h);
}

int main(void) {
    char filename[MAX_PATH];

    printf("Enter file name: ");
    fflush(stdout);
    if (!fgets(filename, sizeof(filename), stdin)) {
        fprintf(stderr, "failed to read file name\n");
        return 1;
    }
    filename[strcspn(filename, "\r\n")] = 0;

    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ,
                               &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        print_error("CreateFileA(input)");
        return 1;
    }

    HANDLE hRead = NULL, hWrite = NULL;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        print_error("CreatePipe");
        close_if_valid(hFile);
        return 1;
    }

    if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0)) {
        print_error("SetHandleInformation");
        close_if_valid(hRead);
        close_if_valid(hWrite);
        close_if_valid(hFile);
        return 1;
    }

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = hFile;
    si.hStdOutput = hWrite;
    si.hStdError  = hWrite;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(NULL, "child.exe", NULL, NULL, TRUE,
                        0, NULL, NULL, &si, &pi)) {
        print_error("CreateProcessA");
        close_if_valid(hRead);
        close_if_valid(hWrite);
        close_if_valid(hFile);
        return 1;
    }

    CloseHandle(hWrite);
    CloseHandle(hFile);

    char buf[4096];
    DWORD n;
    BOOL ok = TRUE;
    while (ok) {
        ok = ReadFile(hRead, buf, sizeof(buf), &n, NULL);
        if (!ok) {
            DWORD err = GetLastError();
            if (err == ERROR_BROKEN_PIPE) break;
            print_error("ReadFile");
            break;
        }
        if (n == 0) break;
        if (fwrite(buf, 1, n, stdout) != n) {
            perror("fwrite");
            break;
        }
    }
    CloseHandle(hRead);

    DWORD wait = WaitForSingleObject(pi.hProcess, INFINITE);
    if (wait == WAIT_FAILED) {
        print_error("WaitForSingleObject");
    }

    DWORD code = 0;
    if (!GetExitCodeProcess(pi.hProcess, &code)) {
        print_error("GetExitCodeProcess");
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (code != 0) {
        fprintf(stderr, "child terminated with code %lu\n", code);
        return 1;
    }
    return 0;
}
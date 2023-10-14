// In Visual Studio, make sure it's a windows GUI project (not console)
// Go to Project tab -> <Project Name> Properties->Linker->System
// Change SubSystem to Windows (/SUBSYSTEM:WINDOWS)

#include <Windows.h>
#include <WinUser.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define WM_TRAYICON ( WM_USER + 1 )
#define ID_SYSTEM_TRAY_EXIT 3000

NOTIFYICONDATA nid;
HMENU TrayIconMenu;
HANDLE hThread;

// Keystroke Simulator
void KeystrokeSimulatorLoop()
{
    INPUT ip;

    // Set up a generic keyboard event.
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0; // hardware scan code for key
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    const WORD values[] = { VK_LSHIFT, VK_RSHIFT };
    int num = sizeof(values) / sizeof(values[0]);
    int selector = 0;

    // virtual-key code for the left-shift key
    ip.ki.wVk = values[0];

    // Initialize random number generator
    srand((unsigned)time(NULL));

    // Simulate user input if no user input for 1 minute (60,000 milliseconds)
    const DWORD max_inactive_time_milliseconds = 60000;

    // max inactive time - a random number from 1 to 10 seconds
    // Randomized to make it look realistic
    DWORD random_inactive_time_milliseconds = max_inactive_time_milliseconds - (((rand() % 10) + 1) * 1000);

    // Keep looping until user closes console window
    while (1)
    {
        // Get time in milliseconds since last user input
        LASTINPUTINFO last_user_input_info;
        last_user_input_info.cbSize = sizeof(last_user_input_info);
        GetLastInputInfo(&last_user_input_info);

        // Current time in milliseconds - time of last user input in milliseconds
        DWORD milliseconds_since_last_input = GetTickCount() - last_user_input_info.dwTime;

        // Only simulate user input if needed (ie: user hasn't been active)
        // so that it doesn't interrup the user with random key presses while the user is working and this script is running
        if (milliseconds_since_last_input >= random_inactive_time_milliseconds)
        {
            // User hasn't pressed anything in the required session timeout time
            ip.ki.dwFlags = 0; // 0 for key press
            SendInput(1, &ip, sizeof(INPUT));

            // Release the Left Shift key
            ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
            SendInput(1, &ip, sizeof(INPUT));

            // Toggle the key being pressed so it looks realistic (not the same key every time)
            selector = (selector + 1) % num;
            ip.ki.wVk = values[selector];

            // Update to a new random time for the next run
            random_inactive_time_milliseconds = max_inactive_time_milliseconds - (((rand() % 10) + 1) * 1000);

            Sleep(random_inactive_time_milliseconds);
        }
        else
        {
            // User has pressed something within the required session timeout time
            // Calculate the difference from the time the last key was pressed, to the new timeout time
            Sleep(random_inactive_time_milliseconds - milliseconds_since_last_input);
        }
    }
}

// Safely exit
void ExitFunction()
{
    CloseHandle(hThread);
    Shell_NotifyIcon(NIM_DELETE, &nid);
    PostQuitMessage(0);
}

// When the tray icon is clicked
void TrayIconClicked(HWND* hwnd)
{
    POINT curPoint;
    GetCursorPos(&curPoint);
    SetForegroundWindow(*hwnd);
    UINT clicked = TrackPopupMenu(TrayIconMenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, *hwnd, NULL);

    if (clicked == ID_SYSTEM_TRAY_EXIT)
    {
        ExitFunction();
    }
}

// When window closes
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_TRAYICON:
        switch (lParam)
        {
        case WM_LBUTTONUP:
            TrayIconClicked(&hwnd);
            break;
        case WM_RBUTTONUP:
            TrayIconClicked(&hwnd);
            break;
        }
        break;

    case WM_DESTROY:
        ExitFunction();
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Create the main window (must be created even though you don't want it)
void InitializeWindow(WNDCLASSW* wc, HINSTANCE* hInstance, HWND* hwnd)
{
    // Window properties
    wc->style = CS_HREDRAW | CS_VREDRAW;
    wc->cbClsExtra = 0;
    wc->cbWndExtra = 0;
    wc->lpszClassName = TEXT("Window");
    wc->hInstance = *hInstance;
    wc->hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc->lpszMenuName = NULL;
    wc->lpfnWndProc = WndProc;
    wc->hCursor = LoadCursor(NULL, IDC_ARROW);
    wc->hIcon = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassW(wc);
    *hwnd = CreateWindow(wc->lpszClassName, TEXT("Window"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 350, 250, NULL, NULL, *hInstance, NULL);

    ShowWindow(*hwnd, SW_HIDE);
    UpdateWindow(*hwnd);
}

// Create the tray icon
void CreateTrayIcon(HWND* hwnd)
{
    // Tray Icon properties
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = *hwnd;
    nid.uID = 100;
    nid.uVersion = NOTIFYICON_VERSION;
    nid.uCallbackMessage = WM_TRAYICON;
    wcscpy_s(nid.szTip, sizeof(nid.szTip), L"Keystroke Simulator");
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.hIcon = LoadIcon(NULL, IDI_WINLOGO);

    Shell_NotifyIcon(NIM_ADD, &nid);

    TrayIconMenu = CreatePopupMenu();
    AppendMenu(TrayIconMenu, MF_STRING, ID_SYSTEM_TRAY_EXIT, TEXT("Exit"));
}

// Main GUI Loop
void GuiLoop(MSG* msg)
{
    while (GetMessage(msg, NULL, 0, 0))
    {
        DispatchMessage(msg);
    }
}

// Main function (code enters here)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    MSG msg;
    HWND hwnd;
    WNDCLASSW wc;
    DWORD threadID;

    InitializeWindow(&wc, &hInstance, &hwnd);
    CreateTrayIcon(&hwnd);

    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeystrokeSimulatorLoop, NULL, 0, &threadID);

    GuiLoop(&msg);

    return (int)msg.wParam;
}

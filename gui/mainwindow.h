#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

bool    RegisterMainWindowClass(HINSTANCE hInst);
HWND    CreateMainWindow(HINSTANCE hInst, int nCmdShow);
int     RunMessageLoop();
int     RunApp(HINSTANCE hInst, int nCmdShow);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

#endif
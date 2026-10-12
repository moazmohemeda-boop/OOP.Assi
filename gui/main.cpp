#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "mainwindow.h"

// Single translation unit: avoids "multiple definition" errors from stb
// when Image_Class.h defines the stb implementations.
#define BPS_UNITY
#include "mainwindow.cpp"
#include "filters.cpp"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    return RunApp(hInst, nCmdShow);
}

// Works with or without -mwindows
int main()
{
    return RunApp(GetModuleHandleA(nullptr), SW_SHOWDEFAULT);
}
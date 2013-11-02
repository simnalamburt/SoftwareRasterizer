#include "pch.h"
#include "SystemClass.h"



int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    SystemClass window(hInstance, 800, 600, false);
    window.Run(nCmdShow);
}

#include <windows.h>

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    MessageBoxA(0, "some text in the box.", "title",
        MB_OK | MB_ICONINFORMATION);
    return 0;
}
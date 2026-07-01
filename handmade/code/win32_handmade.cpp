#include <windows.h>


WNDPROC Wndproc;

LRESULT CALLBACK MainWindowCallback(
    HWND Window,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam)
{
    LRESULT Result = 0;
    switch (Message)
    {
    case WM_SIZE:
    {
        OutputDebugStringA("WM_SIZE\n");
    }break;

    case WM_DESTROY:
    {
        OutputDebugStringA("WM_DESTROY\n");
    }break;

    case WM_CLOSE:
    {
        OutputDebugStringA("WM_CLOSE\n");
    }break;

    case WM_ACTIVATEAPP:
    {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
    }break;

    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        HDC DeviceContext= BeginPaint(Window,&paint );
        int X = paint.rcPaint.left; 
        int Y = paint.rcPaint.top;
        int Height = paint.rcPaint.bottom - paint.rcPaint.top;
        int Width = paint.rcPaint.right - paint.rcPaint.left;
        static DWORD Operation = WHITENESS;
        PatBlt(DeviceContext, X, Y, Width, Height, Operation);
        if (Operation == WHITENESS) {
            Operation = BLACKNESS;
        }

        else {
            Operation = WHITENESS;
        }
        EndPaint(Window, &paint);
    }break; 

    default:
    {
        OutputDebugStringA("default\n");
        Result = DefWindowProc(Window, Message, WParam, LParam);
    }break;
    }
    return Result;

}
int CALLBACK
WinMain(HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode)
{
    WNDCLASS WindowClass = {};

    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;       //can be same as winmain one

    // HICON hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";


    if (RegisterClass(&WindowClass))
    {
        HWND WindowHandle =
            CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
        if (WindowHandle) {
            MSG Message;
            for (;;) {
                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                if (MessageResult > 0) {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else {
                    break;

                }

            }
        }
        else {
            //todo
        }
    }
    else {
        //
    }
    return 0;
}
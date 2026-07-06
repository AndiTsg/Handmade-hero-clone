#include <windows.h>

// no longer needed : #define  local_persist static
#define global_variable static 

//this one means only for this file or translation unit or source file
#define  internal static

//this is glbal for now
global_variable BOOL Running;

global_variable BITMAPINFO BitmapInfo; //we fill out the bitmapinfo ourselves
global_variable void* BitmapMemory;
global_variable int BitmapHeight;
global_variable int BitmapWidth;

// following are no longer needed bcoz stretchdibit : global_variable HBITMAP BitmapHandle; global_variable HDC BitmapDeviceContext;

internal void
Win32ResizeDIBSection(int Width, int Height)   // this is for resizing our dib section that we can draw into and 
// also to create if it hasnt been before
{
    if (BitmapMemory) { //to free the memory for when we want to allocate again
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);//MEM_RELEASE frees too but if we used decommit it would leave the pages reserved(way of saying dont track but we may need the memory later)
    }
    BitmapHeight = Height;
    BitmapWidth = Width;
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight; //we choose top down so we make negative
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    /* the following are no longer needed bc bitmap info set to static .BitmapInfo.biSizeImage = 0;  BitmapInfo.biXPelsPerMeter = 0;   BitmapInfo.biYPelsPerMeter = 0; BitmapInfo.biClrUsed = 0;BitmapInfo.biXPelsPerMeter = 0;BitmapInfo.biClrImportant=0*/

    int BytesPerPixel = 4;
    int BitmapMemorySize = (Width * Height) * BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

}


internal void
Win32UpdateWindow(HDC DeviceContext, RECT* WindowRect, int  X, int  Y, int  Width, int  Height) {


    int WindowWidth = WindowRect->right - WindowRect->left;
    int WindowHeight = WindowRect->bottom - WindowRect->top;

    StretchDIBits(DeviceContext,
        /*
        X,  Y, Width, Height   //   destination rectangle
        X, Y, Width, Height    //   source rectangle, it is the same as src since we're drawing to the window wc is destination
        */
        0, 0, BitmapWidth, BitmapHeight,
        0, 0, WindowWidth, WindowHeight,
        BitmapMemory,
        &BitmapInfo,
        DIB_RGB_COLORS, SRCCOPY);
}


WNDPROC Wndproc;

LRESULT CALLBACK
Win32MainWindowCallback(
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
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);   // this is to get drawable area of a window

        int Height = ClientRect.bottom - ClientRect.top;
        int Width = ClientRect.right - ClientRect.left;
        Win32ResizeDIBSection(Width, Height);

        OutputDebugStringA("WM_SIZE\n");
    }break;

    case WM_DESTROY:
    {
        //this case will be treated as an error in case unexpectedly closed
        Running = false;
        OutputDebugStringA("WM_DESTROY\n");
    }break;

    case WM_CLOSE:
    {
        //this case will be treated as a pop message
        Running = false;
        //DestroyWindow(Window);
        //PostQuitMessage(0);
        OutputDebugStringA("WM_CLOSE\n");
    }break;

    case WM_ACTIVATEAPP:
    {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
    }break;

    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        HDC DeviceContext = BeginPaint(Window, &paint);
        int X = paint.rcPaint.left;
        int Y = paint.rcPaint.top;
        int Height = paint.rcPaint.bottom - paint.rcPaint.top;
        int Width = paint.rcPaint.right - paint.rcPaint.left;

        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
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
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;       //can be same as winmain one

    // HICON hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";


    if (RegisterClass(&WindowClass))
    {
        HWND WindowHandle =
            CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
        if (WindowHandle) {
            Running = true;
            while (Running) {
                MSG Message;
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
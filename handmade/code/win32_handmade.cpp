#include <windows.h>

#define  local_persist static
#define global_variable static 

//this one means only for this file or translation unit or source file
#define  intenal static

//this is glbal for now
global_variable BOOL Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void* BitMapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void
Win32ResizeDIBSection(int Width,int Height)   // this is for resizing our dib section that we can draw into and 
                                              // also to create if it hasnt been before
{
    BitmapInfo.bmiHeader.biSize =sizeof(BitmapInfo.bmiHeader) ;
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    /* no longer needed bc bitmap info set to static .BitmapInfo.biSizeImage = 0;  BitmapInfo.biXPelsPerMeter = 0;   BitmapInfo.biYPelsPerMeter = 0; BitmapInfo.biClrUsed = 0;BitmapInfo.biXPelsPerMeter = 0;BitmapInfo.biClrImportant=0*/
        
    
    //todo bullet proof this

    HBITMAP BitmapHandle = CreateDIBSection    //the return value HBITMAP is the handle for our created dibsection
                            (DeviceContext, &BitMapInfo, 
                            DIB_RGB_COLORS, &BitMapMemory ,0,0);
}


internal void
Win32UpdateWindow(HDC DeviceContext, int  X, int  Y, int  Width, int  Height)() {
    int StretchDIBits(DeviceContext,  
                     X,  Y, Width, Height   //   destination rectangle
                     X, Y, Width, Height    //   source rectangle, it is the same as src since we're drawing to th ewindow wc iis destination
                     const VOID * lpBits,
                     const BITMAPINFO * lpbmi,
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
       BOOL GetClientRect(window, &ClientRect){     // this is to get drawable area of a window
           int Height = ClientRect.rcPaint.bottom - ClientRect.rcPaint.top;
           int Width = ClientRect.rcPaint.right - ClientRect.rcPaint.left;
           Win32ResizeDIBSection(Width, Height);
        }
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
        HDC DeviceContext= BeginPaint(Window,&paint );
        int X = paint.rcPaint.left; 
        int Y = paint.rcPaint.top;
        int Height = paint.rcPaint.bottom - paint.rcPaint.top;
        int Width = paint.rcPaint.right - paint.rcPaint.left;
        Win32UpdateWindow(Window,X, Y, Width, Height);
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
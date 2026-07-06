#include <windows.h>
#include <stdint.h>

#define  local_persist static
#define global_variable static 

  // for uint8_t ,wc we use to get 8bit integer
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uin64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t in64;



//this one means only for this file or translation unit or source file
#define  internal static

//this is global for now
global_variable BOOL Running;

global_variable BITMAPINFO BitmapInfo; //we fill out the bitmapinfo ourselves
global_variable void* BitmapMemory;
global_variable int BitmapHeight;
global_variable int BitmapWidth;
global_variable int BytesPerPixel = 4;

// following are no longer needed bcoz we are using stretchdibit : global_variable HBITMAP BitmapHandle; global_variable HDC BitmapDeviceContext;


internal void
RenderColors(int XOffset, int YOffset) {
    int Width = BitmapWidth;
    int Height = BitmapHeight;
    int Pitch = Width * BytesPerPixel;
    uint8* Row = (uint8*)BitmapMemory;
    for (int Y = 0; Y < BitmapHeight; Y++) {
        uint32* Pixel = (uint32*)Row;   //so that we get 1/4th of a pixel to write its colors
        for (int X = 0; X < BitmapWidth; X++) {
            
            uint8 Blue = X + XOffset;
            uint8 Green = Y + YOffset;
            
            *Pixel = ((Green << 8) | Blue);
            Pixel++; //u can write *pixel++ instead at the top
        }
        Row += Pitch;
    }
}


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

    int BitmapMemorySize = (Width * Height) * BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}


internal void
Win32UpdateWindow(HDC DeviceContext, RECT* ClientRect, int  X, int  Y, int  Width, int  Height) {


    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;

    StretchDIBits(DeviceContext,
        /*
        X,  Y, Width, Height   //   destination rectangle
        X, Y, Width, Height    //   source rectangle, it is the same as src since we're drawing to the window wc is destination
        */
        0, 0, WindowWidth, WindowHeight,
        0, 0,  BitmapWidth,   BitmapHeight,
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
        HWND Window =
            CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
        if (Window) {
            Running = true;
           int XOffset = 0;
           int YOffset = 0;
            while (Running) { 
                
                MSG Message;
                while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {   //while instead of if because we dont wanna repaint for every message. and we need to clear the queue
                    if (Message.message == WM_QUIT) {
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }  


                RenderColors(XOffset, YOffset);

                HDC DeviceContext = GetDC(Window );
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(DeviceContext, &ClientRect,0, 0 , WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);
                
                
                XOffset++;



                

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
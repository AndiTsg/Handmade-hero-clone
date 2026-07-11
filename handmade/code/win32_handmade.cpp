#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#define  local_persist static
#define global_variable static 

  // for uint8_t ,wc we use to get 8bit integer
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

//this one means only for this file or translation unit or source file
#define  internal static

//this is global for now
global_variable BOOL Running;

struct Win32_Offscreen_Buffer {
BITMAPINFO Info; //we fill out the bitmapinfo ourselves
void *Memory;
int Height;
int Width;
int Pitch;
int BytesPerPixel; 
};
global_variable Win32_Offscreen_Buffer Buffer1;


struct Win32_Window_Dimension {
    int Width;
    int Height;
};

internal
Win32_Window_Dimension Get_Window_Dimension(HWND Window){

    Win32_Window_Dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);   // this is to get drawable area of a window

    Result.Height = ClientRect.bottom - ClientRect.top;
    Result.Width = ClientRect.right - ClientRect.left;
    
    return (Result);

}

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState) //ig makes easier to make multiple func w same signature
typedef  X_INPUT_GET_STATE(x_input_get_state); //create a type wc is a function (so that we can create pointer to it)
X_INPUT_GET_STATE(XInputGetStateStub) {   //default func
    return 0;
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub; //initializing
#define XInputGetState XInputGetState_ // XInputGetState is no longer the one in the import library


#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef  X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
    return 0;
 }
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_
// following are no longer needed bcoz we are using stretchdibit : global_variable HBITMAP BitmapHandle; global_variable HDC BitmapDeviceContext;

internal void 
Win32LoadXInput(void) {
    HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
    if (XInputLibrary){
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

internal void
RenderColors(Win32_Offscreen_Buffer *Buffer, int BlueOffset, int GreenOffset) {    //when this gets called every update
    
    uint8* Row = (uint8*)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; Y++) {
        uint32* Pixel = (uint32*)Row;   //so that we get 1/4th of a pixel to write its colors
        for (int X = 0; X < Buffer->Width; X++) {    // iterates through each pixels of a row
            
            uint8 Blue = X + BlueOffset;
            uint8 Green = Y + GreenOffset;   //YOffset is getting incremented every update so creates motion
            
            *Pixel = ((Green << 8) | Blue);
            Pixel++; //u can write *pixel++ instead at the top
        }
        Row += Buffer->Pitch;
    }
}

internal void
Win32ResizeDIBSection(Win32_Offscreen_Buffer* Buffer,int Width, int Height)   // this is for resizing our dib section that we can draw into and 
// also to create if it hasnt been before
{
    if (Buffer->Memory) { //to free the memory for when we want to allocate again
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);//MEM_RELEASE frees too but if we used decommit it would leave the pages reserved(way of saying dont track but we may need the memory later)
    }
    Buffer->BytesPerPixel = 4;
    Buffer->Height = Height;
    Buffer->Width = Width;
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; //we choose top down so we make negative
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    /* the following are no longer needed bc bitmap info set to static .BitmapInfo.biSizeImage = 0;  BitmapInfo.biXPelsPerMeter = 0;   BitmapInfo.biYPelsPerMeter = 0; BitmapInfo.biClrUsed = 0;BitmapInfo.biXPelsPerMeter = 0;BitmapInfo.biClrImportant=0*/

    int BitmapMemorySize = (Width * Height) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;  //pitch is in bytes i think

}

internal void
Win32UpdateWindow(Win32_Offscreen_Buffer Buffer,int WindowWidth, int WindowHeight, HDC DeviceContext, int  X, int  Y, int  Width, int  Height) {

    StretchDIBits(DeviceContext,  //is used to transfer from backbuffer to window
        /*
        X,  Y, Width, Height   //   destination rectangle
        X, Y, Width, Height    //   source rectangle, it is the same as src since we're drawing to the window wc is destination
         */
        0, 0, WindowWidth, WindowHeight,
        0, 0,  Buffer.Width, Buffer.Height,
        Buffer.Memory,
        &Buffer.Info,
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

        Win32_Window_Dimension Dimension = Get_Window_Dimension(Window);
        Win32UpdateWindow(Buffer1 ,Dimension.Width , Dimension.Height, DeviceContext, X, Y, Width, Height);
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

    Win32ResizeDIBSection(&Buffer1, 1200, 720);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;       //can be same as winmain one

    // HICON hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";


    if (RegisterClass(&WindowClass))
    {
        HWND Window =
            CreateWindowEx(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
        if (Window) {
            Win32LoadXInput();
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

                for (DWORD ControllerIndex = 0; 
                    ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++)
                {
                    XINPUT_STATE ControllerState;
                    // Simply get the state of the controller from XInput.
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS){     // XInputGetState is a func that gives us the -
                                 //enter here if controller is plugged in                                                                // state of the controller we pass by index.
                        XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;                          // we pass it address of ControllerState that -
                                                                                                 // we initialized so that the function fills it for us
                        bool Up = ( Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool Down  = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool Left  = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool Back  = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool LeftShoulder  = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder  = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);
                        

                        int16 StickX = Pad->sThumbLX;
                        int16 StickY = Pad->sThumbLY;

                    }
                    else{
                        //controller not plugged in

                        }

                }
                XINPUT_VIBRATION Vibration;
                Vibration.wLeftMotorSpeed = 60000;
                Vibration.wRightMotorSpeed = 60000;
                XInputSetState(0, &Vibration);

// after we draw to the bitmap we draw to the screen. then the XOffset++ makes each pixel position have +1 more value of the color
// when this repeats every run of while(running) ,creates animation 
                RenderColors(&Buffer1, XOffset, YOffset);
                
                HDC DeviceContext = GetDC(Window);
                Win32_Window_Dimension Dimension = Get_Window_Dimension(Window);
                Win32UpdateWindow(Buffer1, Dimension.Width, Dimension.Height, DeviceContext ,0, 0 , Dimension.Width, Dimension.Height);
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
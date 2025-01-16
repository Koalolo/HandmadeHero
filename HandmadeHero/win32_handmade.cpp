
#include <windows.h>
#include <stdint.h>
#include <xinput.h>



#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_buffer
{
	BITMAPINFO Info;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct win32_window_dimension
{
	int Width;
	int Height;
	int p;
};

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex,  XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateSub)
{
	return (0);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateSub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex,  XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateSub)
{
	return (0);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateSub;
#define XInputSetState XInputSetState_

internal void
Win32LoadXInput(void)
{
	HMODULE XInputLibrary = LoadLibraryW(L"xinput1_3.dll");
	if (XInputLibrary)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
	}
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Dimension;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Dimension.Height = ClientRect.bottom - ClientRect.top;
	Dimension.Width = ClientRect.right - ClientRect.left;

	return (Dimension);

}

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;

internal void
RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset)
{

	uint8* Row = (uint8*)Buffer->Memory;
	for (int y = 0; y < Buffer->Height; ++y)
	{
		uint32* Pixel = (uint32*)Row;
		for (int x = 0; x < Buffer->Width; ++x)
		{
			
			uint8 Blue = (x/2) + XOffset;
			uint8 Green = (y/2) + YOffset;
			uint8 Red = 0;

			*Pixel++ = ((Red << 16) |(Green << 8)| Blue);

		}

		Row += Buffer->Pitch;
	}
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer* Buffer, int Width, int Height)
{
	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	
	int BitmapMemorySize = Buffer->BytesPerPixel * (Width * Height);
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	Buffer->Pitch = Width * Buffer->BytesPerPixel;

	RenderWeirdGradient(Buffer, 0, 0);
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, 
							HDC DeviceContext, 
							int WindowWidth, int WindowHeight)
{
	StretchDIBits(DeviceContext,
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory,
		&Buffer->Info,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(
	HWND Window,
	UINT Message,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT result = 0;
	
	switch (Message)
	{
		case WM_SIZE:
		{
			
				
		}break;
		case WM_CLOSE:
		{
			GlobalRunning = false;
			//PostQuitMessage(0);
			OutputDebugStringA("WM_CLOSE\n");
		}break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_CLOSE\n");
		}break;
		case WM_DESTROY:
		{
			GlobalRunning = false;
			OutputDebugStringA("WM_DESTROY\n");
		}break;
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32 VKCode = wParam;
			bool wasDown = ((lParam & (1 << 30)) != 0);
			bool isDown = ((lParam & (1 << 31)) == 0);

			if (wasDown != isDown)
			{
				if (VKCode == 'Z')
				{

				}
				else if (VKCode == 'S')
				{

				}
				else if (VKCode == 'Q')
				{

				}
				else if (VKCode == 'D')
				{

				}
				else if (VKCode == 'A')
				{

				}
				else if (VKCode == 'E')
				{

				}
				else if (VKCode == VK_UP)
				{

				}
				else if (VKCode == VK_DOWN	)
				{

				}
				else if (VKCode == VK_LEFT)
				{

				}
				else if (VKCode == VK_RIGHT)
				{

				}
				else if (VKCode == VK_ESCAPE)
				{
					if (isDown)
					{
						OutputDebugStringA("isDown ");
					}
					if (wasDown)
					{
						OutputDebugStringA("wasDown\n");
					}

				}
				else if (VKCode == VK_SPACE)
				{

				}		
			}
		}break;
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC DeviceContext;
			DeviceContext = BeginPaint(Window, &paint);
			
			win32_window_dimension Dimension = Win32GetWindowDimension(Window);

			Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
			EndPaint(Window, &paint);
			
		}break;
		default:
		{
			result = DefWindowProc(Window, Message, wParam, lParam);
		}break;			
	}

	return (result);
}

int __stdcall 
WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR     CommandLine,
	int       nShowCmd)
{
	OutputDebugStringA("Started\n");

	Win32LoadXInput();

	WNDCLASSEX WindowClass = {};

	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	//WindowClass.hIcon = ;
	//WindowClass.hCursor = ;
	WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//WindowClass.lpszMenuName = ;
	WindowClass.lpszClassName = TEXT("HandmadeHeroWindowsClass");

	if (RegisterClassEx(&WindowClass))
	{
		HWND Window =
			CreateWindowEx(
				0,
				WindowClass.lpszClassName,
				TEXT("Handmade Hero"),
				WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				Instance,
				0);
		if (Window)
		{	
			int XOffset = 0;
			int YOffset = 0;
			int yDirection = 0;

			HDC DeviceContext = GetDC(Window);

			GlobalRunning = true;

			while ( GlobalRunning )
			{
				
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}



				for (DWORD ControllerIndex = 0 ; 
					ControllerIndex < XUSER_MAX_COUNT ; 
					++ControllerIndex)
				{
					XINPUT_STATE ControllerState;
					if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
					{
						XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;

						bool DPadUp = (Pad->wButtons && XINPUT_GAMEPAD_DPAD_UP);
						bool DPadDown = (Pad->wButtons && XINPUT_GAMEPAD_DPAD_DOWN);
						bool DPadLeft = (Pad->wButtons && XINPUT_GAMEPAD_DPAD_LEFT);
						bool DPadRight = (Pad->wButtons && XINPUT_GAMEPAD_DPAD_RIGHT);
						bool Start = (Pad->wButtons && XINPUT_GAMEPAD_START);
						bool Back = (Pad->wButtons && XINPUT_GAMEPAD_BACK);
						bool LeftShoulder = (Pad->wButtons && XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool RightShoulder = (Pad->wButtons && XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool AButton = (Pad->wButtons && XINPUT_GAMEPAD_A);
						bool BButton = (Pad->wButtons && XINPUT_GAMEPAD_B);
						bool XButton = (Pad->wButtons && XINPUT_GAMEPAD_X);
						bool YButton = (Pad->wButtons && XINPUT_GAMEPAD_Y);

						int16 StickX = Pad->sThumbLX;
						int16 StickY = Pad->sThumbLY;

						if (StickX)
						{
							XOffset += -StickX/16000;
						}
						if (StickY)
						{
							YOffset += StickY / 16000;
						}

						if (YButton)
						{
							/*XINPUT_VIBRATION Vibration;
							Vibration.wLeftMotorSpeed = 20000;
							Vibration.wRightMotorSpeed = 20000;
							XInputSetState(0, &Vibration);*/
						}
					}
					else
					{

					}

				}
				
				RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);
		
				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
				ReleaseDC(Window,DeviceContext);

				//++XOffset;		
							
			}		
		}
		else
		{
			// TODO
		}

	}
	else
	{
		// TODO
	}

	return (0);
}
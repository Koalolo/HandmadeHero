
#include <windows.h>
#include <stdint.h>

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
};

win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Dimension;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Dimension.Height = ClientRect.bottom - ClientRect.top;
	Dimension.Width = ClientRect.right - ClientRect.left;

	return (Dimension);

}

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{

	uint8* Row = (uint8*)Buffer.Memory;
	for (int y = 0; y < Buffer.Height; ++y)
	{
		uint32* Pixel = (uint32*)Row;
		for (int x = 0; x < Buffer.Width; ++x)
		{
			uint8 Blue = (x + XOffset);
			uint8 Green= (y + YOffset);

			*Pixel++ = ((Green << 8)| Blue);

		}

		Row += Buffer.Pitch;
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

	RenderWeirdGradient(*Buffer, 0, 0);
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight,
						win32_offscreen_buffer Buffer,
						int w, int y)
{
	StretchDIBits(DeviceContext,
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer.Width, Buffer.Height,
		Buffer.Memory,
		&Buffer.Info,
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
			Running = false;
			//PostQuitMessage(0);
			OutputDebugStringA("WM_CLOSE\n");
		}break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_CLOSE\n");
		}break;
		case WM_DESTROY:
		{
			Running = false;
			OutputDebugStringA("WM_DESTROY\n");
		}break;
		
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC DeviceContext;
			DeviceContext = BeginPaint(Window, &paint);
			
			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int height = paint.rcPaint.bottom - paint.rcPaint.top;
			int width = paint.rcPaint.right - paint.rcPaint.left;

			win32_window_dimension Dimension = Win32GetWindowDimension(Window);

			Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
										GlobalBackBuffer, 
										x, y);
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
	WNDCLASSEX WindowClass = {};

	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.style = CS_HREDRAW|CS_VREDRAW;
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

			Running = true;
			while ( Running )
			{
				
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						Running = false;
					}
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				
				RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

				HDC DeviceContext = GetDC(Window);
				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, 
											GlobalBackBuffer, 0, 0);
				ReleaseDC(Window,DeviceContext);

				++XOffset;
		
				YOffset += 1;			
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

#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <math.h>

  
#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.1459265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

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

global_variable bool32 GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

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
	return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateSub;
#define XInputSetState XInputSetState_

internal void
Win32LoadXInput(void)
{
	HMODULE XInputLibrary = LoadLibraryW(L"xinput1_4.dll");
	if (!XInputLibrary)
	{
		HMODULE XInputLibrary = LoadLibraryW(L"xinput9_1_0.dll");
	}
	if (!XInputLibrary)
	{
		HMODULE XInputLibrary = LoadLibraryW(L"xinput1_3.dll");
	}
	if (XInputLibrary)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		if (!XInputGetState) {XInputGetState = XInputGetStateSub;}
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
		if (!XInputSetState) { XInputSetState = XInputSetStateSub; }
	}
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter )
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
	HMODULE DSoundLibrary = LoadLibraryW(L"dsound.dll");

	if (DSoundLibrary)
	{
		direct_sound_create* DirectSoundCreate = 
			(direct_sound_create*)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

		LPDIRECTSOUND DirectSound;
		if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
		{
			WAVEFORMATEX WaveFormat = {};
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nChannels = 2;
			WaveFormat.nSamplesPerSec = SamplesPerSecond;
			WaveFormat.wBitsPerSample = 16;
			WaveFormat.nBlockAlign = WaveFormat.nChannels * WaveFormat.wBitsPerSample / 8;
			WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
			WaveFormat.cbSize = 0;

			if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
			{
				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
				
				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
				{
					
					if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
					{
						OutputDebugString(L"PrimaryBuffer was created.\n");
					}
					else
					{

					}
				}
				else
				{

				}
			}
			else
			{

			}
		
			DSBUFFERDESC BufferDescription = {};
			BufferDescription.dwSize = sizeof(BufferDescription);
			BufferDescription.dwFlags = 0;
			BufferDescription.dwBufferBytes = BufferSize;
			BufferDescription.lpwfxFormat = &WaveFormat;
		
			HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
			if (SUCCEEDED(Error))
			{
				OutputDebugString(L"GlobalSecondaryBuffer was created.\n");
			}
			else
			{

			}
		}
		else
		{
		}
		
		//BufferDescription.dwBufferBytes = BufferSize;
		
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
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

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
			bool32 wasDown = ((lParam & (1 << 30)) != 0);
			bool32 isDown = ((lParam & (1 << 31)) == 0);

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
			bool32 altKeyWasDown = ((lParam & (1 << 29)) != 0);
			if (VKCode == VK_F4 && altKeyWasDown)
			{
				GlobalRunning = false;
			}
		}break;
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC DeviceContext = BeginPaint(Window, &paint);	
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

struct win32_sound_output
{
	int SamplesPerSecond;
	int ToneHz;
	int16 ToneVolume;
	int16 MaxToneVolume;
	uint32 RunningSampleIndex;
	int WavePeriod;
	int BytesPerSample;
	int SecondaryBufferSize;
	real32 tSine;
	int LatencySampleCount;
};

void
Win32FillSoundBuffer(win32_sound_output* SoundOutput, DWORD BytesToLock, DWORD BytesToWrite)
{
	VOID* Region1;
	DWORD Region1Size;
	VOID* Region2;
	DWORD Region2Size;

	if (SUCCEEDED(GlobalSecondaryBuffer->Lock(BytesToLock, BytesToWrite,
		&Region1, &Region1Size,
		&Region2, &Region2Size, 0)))
	{

		int16* SampleOut = (int16*)Region1;
		DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
		for (DWORD SampleIndex = 0;
			SampleIndex < Region1SampleCount;
			++SampleIndex)
		{
			
			//real32 t = 2.0f * Pi32 * (real32)SoundOutput->RunningSampleIndex / (real32)SoundOutput->WavePeriod;
			real32 SineValue = sinf(SoundOutput->tSine);
			int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
			//int16 SampleValue = ((RunningSampleIndex++ / HalfWavePeriod) % 2) ? ToneVolume : -(ToneVolume);
			*SampleOut++ = SampleValue;
			*SampleOut++ = SampleValue;

			SoundOutput->tSine += 2.0f * Pi32 * 1.0f / (real32)SoundOutput->WavePeriod;
			++SoundOutput->RunningSampleIndex;
		}

		DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
		SampleOut = (int16*)Region2;

		for (DWORD SampleIndex = 0;
			SampleIndex < Region2SampleCount;
			++SampleIndex)
		{
			
			real32 t = 2.0f * Pi32 * (real32)SoundOutput->RunningSampleIndex / (real32)SoundOutput->WavePeriod;
			real32 SineValue = sinf(SoundOutput->tSine);
			int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
			//int16 SampleValue = ((RunningSampleIndex++ / HalfWavePeriod) % 2) ? ToneVolume : -(ToneVolume);
			*SampleOut++ = SampleValue;
			*SampleOut++ = SampleValue;

			SoundOutput->tSine += 2.0f * Pi32 * 1.0f / (real32)SoundOutput->WavePeriod;
			++SoundOutput->RunningSampleIndex;
		}
		
		GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
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

			win32_sound_output SoundOutput = {};
			SoundOutput.SamplesPerSecond = 48000;
			SoundOutput.ToneHz = 220;
			SoundOutput.ToneVolume = 1000;
			SoundOutput.MaxToneVolume = 1000;
			SoundOutput.RunningSampleIndex = 0;
			SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
			SoundOutput.BytesPerSample = sizeof(int16) * 2;
			SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
			SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
			
			Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
			Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.LatencySampleCount* SoundOutput.BytesPerSample);
			bool32 isSoundPlaying = false;
			GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

			GlobalRunning = true;

			while (GlobalRunning)
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
					ControllerIndex < XUSER_MAX_COUNT; 
					++ControllerIndex)
				{
					XINPUT_STATE ControllerState;
					if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
					{
						XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;

						bool32 DPadUp = (Pad->wButtons && XINPUT_GAMEPAD_DPAD_UP);
						bool32 DPadDown = (Pad->wButtons && XINPUT_GAMEPAD_DPAD_DOWN);
						bool32 DPadLeft = (Pad->wButtons && XINPUT_GAMEPAD_DPAD_LEFT);
						bool32 DPadRight = (Pad->wButtons && XINPUT_GAMEPAD_DPAD_RIGHT);
						bool32 Start = (Pad->wButtons && XINPUT_GAMEPAD_START);
						bool32 Back = (Pad->wButtons && XINPUT_GAMEPAD_BACK);
						bool32 LeftShoulder = (Pad->wButtons && XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool32 RightShoulder = (Pad->wButtons && XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool32 AButton = (Pad->wButtons && XINPUT_GAMEPAD_A);
						bool32 BButton = (Pad->wButtons && XINPUT_GAMEPAD_B);
						bool32 XButton = (Pad->wButtons && XINPUT_GAMEPAD_X);
						bool32 YButton = (Pad->wButtons && XINPUT_GAMEPAD_Y);

						int16 StickX = Pad->sThumbLX;
						int16 StickY = Pad->sThumbLY;				
						
						SoundOutput.ToneHz = 880 + (int)(440.0f)*((real32)StickY / 30000.0f);
						SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;

						XOffset += -StickX / 8192;
						YOffset += StickY / 8192;

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

				DWORD PlayCursor;
				DWORD WriteCursor;
				if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
				{
					DWORD ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % 
										SoundOutput.SecondaryBufferSize;
					DWORD BytesToWrite;
					DWORD TargetCursor = (PlayCursor + 
										(SoundOutput.LatencySampleCount*SoundOutput.BytesPerSample)) %
											SoundOutput.SecondaryBufferSize;
					
					if (ByteToLock > TargetCursor)
					{
						BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
						BytesToWrite += TargetCursor;
					}
					else
					{
						BytesToWrite = TargetCursor - ByteToLock;
					}
						
					Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite);
				}	
		
				if (!isSoundPlaying)
				{
					isSoundPlaying = true;
					GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
				}

				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);						
				
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
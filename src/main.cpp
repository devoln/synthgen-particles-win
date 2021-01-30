

#include <math.h>
#include "util.h"
#include "demo.h"
#include "shader.h"
#include "music.h"

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>
#include <mmsystem.h>

GL gl;

enum {MUSIC_NUMSAMPLES = 60'000'000};


static short music[MUSIC_NUMSAMPLES+22];

#ifdef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

int main()
{
	HDC hDC;
	HGLRC context;
	HWND wnd = CreateGLWindow(&hDC, &context);
	Demo demo;

	MusicInit(music);
	sndPlaySoundA(reinterpret_cast<const char*>(&music), SND_ASYNC|SND_MEMORY|SND_LOOP);
    
    do
    {
		demo.Step();
		demo.RenderFrame();
		
		SwapBuffers(hDC);
		if(AppProcessMessages(wnd)) return 0;
	}
	while(!GetAsyncKeyState(VK_ESCAPE));
}

// Make main() work even with /SUBSYSTEM:WINDOWS linker flag
extern "C" void mainCRTStartup();
extern "C" void WinMainCRTStartup() {mainCRTStartup();}

// Make main() work without default CRT
#ifdef LINK_TO_MSVCRT
extern "C" int _fltused = 0;
extern "C" void mainCRTStartup()
{
	ExitProcess(main());
}
#endif

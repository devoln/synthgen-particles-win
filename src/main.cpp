

#include <math.h>
#include "util.h"
#include "demo.h"
#include "shader.h"
#include "music.h"

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>
#include <mmsystem.h>

#ifdef _DEBUG
#define DEBUG_GL
#endif


extern "C" int _fltused = 0;

// /CRINKLER
// /CRINKLER /HASHTRIES:300 /COMPMODE:SLOW /ORDERTRIES:4000  /UNALIGNCODE /REPORT:.\exe\out.html
//Первое намного быстрее, и не сильно уступает в сжатии





const GL gl;

#define MUSIC_NUMSAMPLES 60000000
#define MUSIC_NUMCHANNELS 1


static short music[MUSIC_NUMSAMPLES+22];

extern "C" void WinMainCRTStartup()
{
	//Sleep(15000);
	
	HDC hDC;
	HGLRC context;
	CreateGLWindow(&hDC, &context);
	Demo demo;
	demo.Init();

	MusicInit(music);
	sndPlaySoundA((const char*)&music, SND_ASYNC|SND_MEMORY|SND_LOOP);
    
    do
    {
		demo.Step();
		demo.RenderFrame();
		
		SwapBuffers(hDC);
		AppProcessMessages();
	}
	while(!GetAsyncKeyState(VK_ESCAPE));

    ExitProcess(0);
}

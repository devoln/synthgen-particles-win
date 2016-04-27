#pragma once

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>

class Demo
{
	int minSizeOrderLoc, maxSizeOrderLoc;
	int timeLoc, rateLoc, lifeTimeLoc, viewportLoc;
	int screenWidth, screenHeight;

	unsigned progId, bufId;
	LARGE_INTEGER prevTime, freq;

	bool paused;
	float speed;
	float time, lifeTime, rate;
public:
	void Init();
	void Step();
	void RenderFrame();
};


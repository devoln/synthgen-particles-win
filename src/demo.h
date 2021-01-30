#pragma once

class Demo
{
public:
	Demo();
	void Step();
	void RenderFrame();

private:
	int screenWidth, screenHeight;

	int minSizeOrderLoc, maxSizeOrderLoc;
	int timeLoc, rateLoc, lifeTimeLoc, viewportLoc;

	unsigned progId, bufId;
	unsigned long long startTime, prevTime, freq;

	bool paused = false;
	float speed = 0.5f;
	float time = 0, lifeTime = 5, rate = 10000;
};

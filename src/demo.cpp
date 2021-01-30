#include "demo.h"
#include "util.h"
#include "shader.h"

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>
#include <GL/gl.h>

#include <cmath>
constexpr float PI = float(3.14159265358979323846);

constexpr float fovy = 60;
constexpr float znear = 0.1f;
constexpr float zfar = 100;
constexpr float f = 1.732f; //1 / tanf(fovy / 360 * PI);
float gPerspectiveMatrix[16] = {
	f, 0, 0, 0,
	0, f, 0, 0,
	0, 0, (zfar+znear)/(znear-zfar), -1,
	0, 0, (2*zfar*znear)/(znear-zfar), 0
};

Demo::Demo()
{
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	ParticleEffectDesc effect;

	effect.ColorFunction =
		"vec3 mainColor = mix( f3rand(vec2(floor(birthTime), 42.1)), f3rand(vec2(floor(birthTime)+1.0, 42.1)), fract(birthTime) )*4.0;"
		"vec3 startColor = f3rand(vec2(birthTime, 342.1))*0.1;"
		"vec3 finalColor = f3rand(vec2(birthTime, 39.6));"
		"return vec4( mix(startColor, finalColor, factor)*mainColor, (1.0-factor)*(1.0-factor) );";

	effect.SizeFunction =
		"float scale = exp(mix(MinSizeOrder, MaxSizeOrder, frand(vec2(birthTime, 5.3))));"
		"float startSize = frand(vec2(birthTime, 85.3))*0.04*scale;"
		"float finalSize = frand(vec2(birthTime, 123.5))*0.1*scale;"
		"return mix(startSize, finalSize, pow(factor, 0.7));";

	effect.PositionFunction =
		"return origin + startVelocity*t + vec3(0.0, 0.58*t*t*0.5, 0.0);";

	effect.EmitterPositionFunction =
		"return vec3(sin(t*5.0)*3.1-1.5, -1.0+sin(t*4.0)*2.6, sin(t*1.6+0.23f)*2.5-1.0);";

	effect.EmitterRotationFunction =
		"return vec3(5.2, -5., -3.)*t;";

	effect.StartParametersFunction =
		"float speed = mix(0.5, 3.35, frand(vec2(t, 0.0)));"
		"vec3 rotation = mix(vec3(-0.7,-0.3,-0.7), vec3(0.7,0.3,0.7), f3rand(vec2(t, 12.3)));"
		"velocity = RotationEulerMatrix(rotation)*vec3(speed, 0.0, 0.0);"
		"origin = (f3rand(vec2(t, 10.0))*2.0-1.0)*0.2;";

	progId = effect.ToShader();

	timeLoc = gl.GetUniformLocation(progId, "Time");
	rateLoc = gl.GetUniformLocation(progId, "Rate");
	lifeTimeLoc = gl.GetUniformLocation(progId, "LifeTime");

	viewportLoc = gl.GetUniformLocation(progId, "ViewportSize");
	gl.Uniform2f(viewportLoc, float(screenWidth), float(screenHeight));

	minSizeOrderLoc = gl.GetUniformLocation(progId, "MinSizeOrder");
	maxSizeOrderLoc = gl.GetUniformLocation(progId, "MaxSizeOrder");

	gl.GenBuffers(1, &bufId);
	gl.BindBuffer(GL::ARRAY_BUFFER, bufId);
	gl.BufferData(GL::ARRAY_BUFFER, 4, nullptr, GL::STATIC_DRAW);
	gl.EnableVertexAttribArray(0);
	gl.VertexAttribPointer(0, 1, GL_FLOAT, false, 4, nullptr);

	glEnable(GL::POINT_SPRITE);
	glEnable(GL::PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);


	gPerspectiveMatrix[0] *= float(screenHeight) / screenWidth;

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(gPerspectiveMatrix);
	glMatrixMode(GL_MODELVIEW);

	gl.Uniform1f(rateLoc, rate);
	gl.Uniform1f(lifeTimeLoc, lifeTime);

	gl.Uniform1f(minSizeOrderLoc, 0.0f);
	gl.Uniform1f(maxSizeOrderLoc, 2.5f);

	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	prevTime = startTime = li.QuadPart;
	QueryPerformanceFrequency(&li);
	freq = li.QuadPart;
}

void Demo::Step()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	const auto curTime = li.QuadPart;

	if(GetAsyncKeyState('I')) time = 0;

	static const float rates[] = {2000, 2000, 10000, 10000, 100000, 100000, 200000, 500000, 1000000};
	static const float lifeTimes[] = {2, 4, 2, 5, 2, 4, 5, 5, 10};
	static const float minSizeOrders[] = {1.0, 1.0, 0.0, 0.0, -1.0, -1.0, -1.5, -2.0, -2.5};
	for(int i=0; i<9; i++)
	{
		if(!GetAsyncKeyState('1' + i)) continue;
		rate = rates[i];
		lifeTime = lifeTimes[i];
		float minSizeOrder = minSizeOrders[i];
		float maxSizeOrder = minSizeOrder + 2.5f;
		gl.Uniform1f(rateLoc, rate);
		gl.Uniform1f(lifeTimeLoc, lifeTime);
		gl.Uniform1f(minSizeOrderLoc, minSizeOrder);
		gl.Uniform1f(maxSizeOrderLoc, maxSizeOrder);
		break;
	}

	float delta = float(int(curTime - prevTime)) / int(freq);

	if(GetAsyncKeyState(VK_SHIFT)) speed += speed * delta;
	if(GetAsyncKeyState(VK_CONTROL)) speed -= speed * delta;
	if(speed > 0 && GetAsyncKeyState(VK_LEFT) ||
		speed < 0 && GetAsyncKeyState(VK_RIGHT)) speed = -speed;

	paused = (GetKeyState('P') & 1) != 0;
	if(GetAsyncKeyState(VK_UP)) delta *= 5;
	else if(GetAsyncKeyState(VK_DOWN)) delta *= -5;
	else if(paused) delta = 0;

	float tt = float(int(curTime - startTime)) / int(freq);
	float s = cosf(tt*0.23f+0.9f)*1.5f-0.5f;
	time += delta*speed*(1.0f + s + s*s*0.5f + s*s*s*0.167f + s*s*s*s*0.04f);
	if(time < 0) time = 0;
	prevTime = curTime;
}

void Demo::RenderFrame()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -7);
	glRotatef(time*20.0f, 0.1f, 0.9f, 0);
	gl.Uniform1f(timeLoc, time);

	glDrawArrays(GL_POINTS, 0, int(min(time, lifeTime)*rate));
}

#include "demo.h"
#include "util.h"
#include "shader.h"

#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>

void Demo::Init()
{
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	ParticleEffectDesc effect;
	effect.ColorFunction = "vec3 mainColor = mix( f3rand(vec2(floor(birthTime), 42.1)), f3rand(vec2(floor(birthTime)+1.0, 42.1)), fract(birthTime) )*4.0;"
		"vec3 startColor = f3rand(vec2(birthTime, 342.1))*0.1;"
		"vec3 finalColor = f3rand(vec2(birthTime, 39.6));"
		"return vec4( mix(startColor, finalColor, factor)*mainColor, (1.0-factor)*(1.0-factor) );"
		/*SomeColorFunction*/;
	effect.EmitterPositionFunction = SomeEmitterTrajectoryFunction;
	effect.EmitterRotationFunction = "return vec3(5.2, -5., -3.)*t;";//SomeEmitterRotationFunction;
	effect.PositionFunction = SomeParticleTrajectoryFunction;
	effect.SizeFunction = SomeSizeFunction;
	effect.StartParametersFunction = SomeEmitterFunction;

	progId = effect.ToShader();

	timeLoc = gl.GetUniformLocation(progId, "Time");
	rateLoc = gl.GetUniformLocation(progId, "Rate");
	lifeTimeLoc = gl.GetUniformLocation(progId, "LifeTime");

	viewportLoc = gl.GetUniformLocation(progId, "ViewportSize");
	gl.Uniform2f(viewportLoc, (float)screenWidth, (float)screenHeight);

	minSizeOrderLoc = gl.GetUniformLocation(progId, "MinSizeOrder");
	maxSizeOrderLoc = gl.GetUniformLocation(progId, "MaxSizeOrder");

	gl.GenBuffers(1, &bufId);
	gl.BindBuffer(GL_ARRAY_BUFFER, bufId);
	gl.BufferData(GL_ARRAY_BUFFER, 4, nullptr, GL_STATIC_DRAW);

	//glVertexPointer(1, GL_FLOAT, 4, nullptr);
	gl.VertexAttribPointer(0, 1, GL_FLOAT, false, 4, nullptr);

	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);


	//glClearColor(0, 0, 1, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (double)screenWidth/screenHeight, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);

	rate = 10000;
	lifeTime = 5;
	gl.Uniform1f(rateLoc, rate);
	gl.Uniform1f(lifeTimeLoc, lifeTime);

	gl.Uniform1f(minSizeOrderLoc, 0.0f);
	gl.Uniform1f(maxSizeOrderLoc, 2.5f);

	QueryPerformanceCounter(&prevTime);
	QueryPerformanceFrequency(&freq);
	time = 0;
	speed = 0.5f;
	paused = false;
}

void Demo::Step()
{
	LARGE_INTEGER curTime;
	QueryPerformanceCounter(&curTime);
	if(GetAsyncKeyState('I')) time = 0.0f;

	static const float rates[] = {2000, 2000, 10000, 10000, 100000, 100000, 200000, 500000, 1000000};
	static const float lifeTimes[] = {2, 4, 2, 4, 2, 4, 5, 5, 10};
	static const float minSizeOrders[] = {1.0, 1.0, 0.0, 0.0, -1.0, -1.0, -1.5, -2.0, -2.0};
	for(int i=0; i<9; i++)
	{
		if(!GetAsyncKeyState('1'+i)) continue;
		rate = rates[i];
		lifeTime = lifeTimes[i];
		float minSizeOrder = minSizeOrders[i];
		float maxSizeOrder = minSizeOrder+2.0f;
		gl.Uniform1f(rateLoc, rate);
		gl.Uniform1f(lifeTimeLoc, lifeTime);
		gl.Uniform1f(minSizeOrderLoc, minSizeOrder);
		gl.Uniform1f(maxSizeOrderLoc, maxSizeOrder);
		break;
	}

	float delta = float(double(curTime.QuadPart-prevTime.QuadPart)/freq.QuadPart);

	if(GetAsyncKeyState(VK_SHIFT)) speed += speed*delta;
	if(GetAsyncKeyState(VK_CONTROL)) speed -= speed*delta;
	if(speed>0 && GetAsyncKeyState(VK_LEFT) || speed<0 && GetAsyncKeyState(VK_RIGHT)) speed = -speed;

	paused = (GetKeyState('P') & 1) !=0;
	if(GetAsyncKeyState(VK_UP)) delta*=5;
	else if(GetAsyncKeyState(VK_DOWN)) delta*=-5;
	else if(paused) delta=0;

	float tt = float(double(curTime.QuadPart)/freq.QuadPart);
	float s = cosf(tt*0.23f+0.9f)*1.5f-0.5f;
	time += delta*speed*(1.0f + s + s*s*0.5f + s*s*s*0.167f + s*s*s*s*0.04f);
	if(time<0) time=0;
	prevTime = curTime;
}

void Demo::RenderFrame()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -7);
	glRotatef(time*20.0f, 0.1f, 0.9f, 0);
	gl.Uniform1f(timeLoc, time);

	//glEnableClientState(GL_VERTEX_ARRAY);
	gl.EnableVertexAttribArray(0);
	glDrawArrays(GL_POINTS, 0, int(min(time, lifeTime)*rate));
	//glDisableClientState(GL_VERTEX_ARRAY);
	gl.DisableVertexAttribArray(0);
}


#pragma once

static const char* const VertexShaderTemplate = 
	"#version 130\r\n"

	"#define ParticleID gl_VertexID\r\n"

	"uniform float MinSizeOrder,MaxSizeOrder;"

	"uniform float Time,Rate,LifeTime;"
	"uniform vec2 ViewportSize;"

	"varying vec4 vColor;"

	"float frand(vec2 seed){return fract(sin(dot(seed,vec2(12.9898,78.233)))*43758.5453);}"
	"vec3 f3rand(vec2 seed){return vec3(frand(seed),frand(seed+vec2(1.2345,0.)),frand(seed+vec2(0.,1.2345)));}"

	"mat3 RotationEulerMatrix(vec3 rotRads)"
	"{"
		"vec3 cos_r=cos(rotRads),sin_r=-sin(rotRads);"
		"return mat3(cos_r.y*cos_r.z,-sin_r.z*cos_r.x+cos_r.z*sin_r.y*sin_r.x,sin_r.x*sin_r.z+cos_r.z*sin_r.y*cos_r.x,"
			"cos_r.y*sin_r.z,cos_r.z*cos_r.x+sin_r.y*sin_r.z*sin_r.x,-sin_r.x*cos_r.z+sin_r.x*sin_r.z*cos_r.x,"
			"-sin_r.y, cos_r.y*sin_r.x, cos_r.y*cos_r.x);"
	"}"

	"vec4 GetColor(float factor,float t,float birthTime){%s}"
	"float GetSize(float factor,float t,float birthTime){%s}"
	"vec3 GetPosition(float t,vec3 origin,vec3 startVelocity,float birthTime){%s}"
	"vec3 GetEmitterPosition(float t){%s}"
	"vec3 GetEmitterRotationAngles(float t){%s}"
	"void GetStartParameters(float t,out vec3 origin,out vec3 velocity){%s}"


    "void main()"
    "{"
		//Идентификация частицы и вычисление основных параметров
		"float minTLT=min(Time,LifeTime);" //Сколько времени назад ни одной из обрабатываемых в кадре частиц ещё не существовало
		"float birthTime=(floor((Time-minTLT)*Rate)+float(ParticleID))/Rate;"
		"float t=Time-birthTime;"
		"float leftLT=LifeTime+birthTime-Time;"
		"float factor=1.0-leftLT/LifeTime;"

		//Состояние эмиттера в момент рождения рассматриваемой частицы
		"vec3 emitterPosition=GetEmitterPosition(birthTime);"
		"vec3 emitterRotation=GetEmitterRotationAngles(birthTime);"

		//Начальная координата и скорость рассматриваемой частицы относительно эмиттера
		"vec3 origin,startVelocity;"
		"GetStartParameters(birthTime,origin,startVelocity);"

		//Вычисляем текущие параметры рассматриваемой частицы
		"vec3 pos=emitterPosition+RotationEulerMatrix(emitterRotation)*GetPosition(t,origin,startVelocity,birthTime);"
		"vec4 color=GetColor(factor,t,birthTime);"
		"float size=GetSize(factor,t,birthTime);"


		//Отображение частицы на экран
		"vec4 eyePos=gl_ModelViewMatrix*vec4(pos,1.);"
		"vec4 projVoxel=gl_ProjectionMatrix*vec4(vec2(size),eyePos.zw);"
		"gl_PointSize=0.25/projVoxel.w*dot(ViewportSize,projVoxel.xy);"
		"gl_Position=gl_ProjectionMatrix*eyePos;"
		"vColor=color;"
    "}\r\n";

static const char* const FragmentShaderCode = 
	"#version 130\r\n"
	"varying vec4 vColor;"
    "void main()"
    "{"
		"vec2 coord=gl_PointCoord.xy-vec2(0.5);"
		"float factor=1.0-dot(coord, coord)*4.0;"
		"gl_FragColor=vColor*factor;"
    "}\r\n";


static const char* const ParticleKinematics = "return origin + startVelocity*t + Acceleration*t*t*0.5;";


static const char* const EmitterRotation = "return EmitterRotationVelocity*t+EmitterRotationAcceleration*t*t/2;";



static const char* const SomeEmitterTrajectoryFunction = "return vec3(sin(t*5.0)*3.1-1.5, -1.0+sin(t*4.0)*2.6, sin(t*1.6+0.23f)*2.5-1.0);";

static const char* const SomeEmitterRotationFunction = "return vec3(8.0,29.5,-1.0)*t*cos(t*3.0);";

static const char* const SomeColorFunction =
		"vec3 mainColor = mix( f3rand(vec2(floor(birthTime*4.0), 42.1)), f3rand(vec2(floor(birthTime*4.0+1.0), 58.3)), fract(birthTime*4.0) )*8.0;"
		"vec3 startColor = f3rand(vec2(birthTime, 342.1))*0.03;"
		"vec3 finalColor = f3rand(vec2(birthTime, 39.6));"
		"return vec4( mix(startColor, finalColor, factor)*mainColor, (1.0-factor)*(1.0-factor) );";

static const char* const SomeSizeFunction =
	"float scale = exp(mix(MinSizeOrder, MaxSizeOrder, frand(vec2(birthTime, 5.3))));"
	"float startSize = frand(vec2(birthTime, 85.3))*0.04*scale;"
	"float finalSize = frand(vec2(birthTime, 123.5))*0.1*scale;"
	"return mix(startSize, finalSize, pow(factor, 0.7));";

static const char* const SomeParticleTrajectoryFunction = "return origin + startVelocity*t + vec3(0.0, 0.58*t*t*0.5, 0.0);";

static const char* const SomeEmitterFunction =
		"float speed = mix(0.5, 3.35, frand(vec2(t, 0.0)));"
		"vec3 rotation = mix(vec3(-0.7,-0.3,-0.7), vec3(0.7,0.3,0.7), f3rand(vec2(t, 12.3)));"
		"velocity = RotationEulerMatrix(rotation)*vec3(speed, 0.0, 0.0);"
		"origin = (f3rand(vec2(t, 10.0))*2.0-1.0)*0.2;";


struct ParticleEffectDesc
{
	const char* ColorFunction;
	const char* SizeFunction;
	const char* PositionFunction;
	const char* EmitterPositionFunction;
	const char* EmitterRotationFunction;
	const char* StartParametersFunction;

	unsigned ToShader() const;
};


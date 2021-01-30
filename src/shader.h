#pragma once

constexpr const char* VertexShaderTemplate = 
	"#version 130\n"

	"#define ParticleID gl_VertexID\n"

	"uniform float MinSizeOrder,MaxSizeOrder;"

	"uniform float Time,Rate,LifeTime;"
	"uniform vec2 ViewportSize;"

	"out vec4 vColor;"

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
		// Particle identification and calculation of its basic parameters
		"float minTLT=min(Time,LifeTime);" // The age of the oldest alive particle
		"float birthTime=(floor((Time-minTLT)*Rate)+float(ParticleID))/Rate;"
		"float t=Time-birthTime;"
		"float leftLT=LifeTime+birthTime-Time;"
		"float factor=1.0-leftLT/LifeTime;"

		// The emitter state when the current particle was born
		"vec3 emitterPosition=GetEmitterPosition(birthTime);"
		"vec3 emitterRotation=GetEmitterRotationAngles(birthTime);"

		// Start position and velocity relative to the emitter
		"vec3 origin,startVelocity;"
		"GetStartParameters(birthTime,origin,startVelocity);"

		// Current particle parameters
		"vec3 pos=emitterPosition+RotationEulerMatrix(emitterRotation)*GetPosition(t,origin,startVelocity,birthTime);"
		"vec4 color=GetColor(factor,t,birthTime);"
		"float size=GetSize(factor,t,birthTime);"


		// Render the particle as a point
		"vec4 eyePos=gl_ModelViewMatrix*vec4(pos,1.);"
		"vec4 projVoxel=gl_ProjectionMatrix*vec4(vec2(size),eyePos.zw);"
		"gl_PointSize=0.25/projVoxel.w*dot(ViewportSize,projVoxel.xy);"
		"gl_Position=gl_ProjectionMatrix*eyePos;"
		"vColor=color;"
    "}\n";

static const char* const FragmentShaderCode = 
	"#version 130\n"
	"in vec4 vColor;"
    "void main()"
    "{"
		"vec2 coord=gl_PointCoord.xy-vec2(0.5);"
		"float factor=1.0-dot(coord, coord)*4.0;"
		"gl_FragColor=vColor*factor;"
    "}\n";

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

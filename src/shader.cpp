#define _CRT_SECURE_NO_WARNINGS
#include "shader.h"
#include "util.h"
#include <cstdio>

static char vertexShaderCode[8192];

unsigned ParticleEffectDesc::ToShader() const
{
	sprintf(vertexShaderCode, VertexShaderTemplate,
		ColorFunction,
		SizeFunction,
		PositionFunction,
		EmitterPositionFunction,
		EmitterRotationFunction,
		StartParametersFunction);
	return CompileShaderProgram(vertexShaderCode, FragmentShaderCode);
}

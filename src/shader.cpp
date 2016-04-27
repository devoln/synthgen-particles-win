#include "shader.h"
#include "util.h"
#define _NO_CRT_STDIO_INLINE
#include <stdio.h>

static char vertexShaderCode[8192];

unsigned ParticleEffectDesc::ToShader() const
{
	sprintf(vertexShaderCode, VertexShaderTemplate, ColorFunction, SizeFunction,
		PositionFunction, EmitterPositionFunction, EmitterRotationFunction, StartParametersFunction);
	auto result = CompileShaderProgram(vertexShaderCode, FragmentShaderCode);



	return result;
}

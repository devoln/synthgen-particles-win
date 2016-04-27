#pragma once

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>
#include <GL/GL.h>
#include <GL/glext.h>

struct GL
{
	GL() {}
	void InitExtensions() const;

	PFNGLCREATEPROGRAMPROC CreateProgram;
	PFNGLCREATESHADERPROC CreateShader;
	PFNGLSHADERSOURCEPROC ShaderSource;
	PFNGLCOMPILESHADERPROC CompileShader;
	PFNGLATTACHSHADERPROC AttachShader;
	PFNGLLINKPROGRAMPROC LinkProgram;
	PFNGLUSEPROGRAMPROC UseProgram;
	PFNGLUNIFORM1FPROC Uniform1f;
	PFNGLUNIFORM2FPROC Uniform2f;
	PFNGLUNIFORM3FPROC Uniform3f;
	PFNGLUNIFORM4FVPROC Uniform4fv;
	PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation;
	PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer;
	PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;
	PFNGLDISABLEVERTEXATTRIBARRAYPROC DisableVertexAttribArray;
	PFNGLBINDBUFFERPROC BindBuffer;
	PFNGLBUFFERDATAPROC BufferData;
	PFNGLGENBUFFERSPROC GenBuffers;

#ifdef DEBUG_GL
	PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;
	PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
	PFNGLDEBUGMESSAGECALLBACKPROC DebugMessageCallback;
#endif
};

extern const GL gl;

HWND CreateGLWindow(HDC* pHDC, HGLRC* pHGLRC);
void AppProcessMessages();

unsigned CompileShader(const char* code, GLenum type);
unsigned LinkShaderProgram(unsigned vertexShader, unsigned fragmentShader);
unsigned CompileShaderProgram(const char* vertexCode, const char* fragmentCode);

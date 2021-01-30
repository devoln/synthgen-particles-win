#pragma once

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>
#include <GL/GL.h>

struct GL
{
	void LoadFunctions();

	unsigned(__stdcall *CreateProgram)();
	unsigned(__stdcall *CreateShader)(GLenum type);
	void(__stdcall *ShaderSource)(unsigned shader, int count, const char* const* string, const int* length);
	void(__stdcall *CompileShader)(unsigned shader);
	void(__stdcall *AttachShader)(unsigned program, unsigned shader);
	void(__stdcall *LinkProgram)(unsigned program);
	void(__stdcall *UseProgram)(unsigned program);
	void(__stdcall *Uniform1f)(int location, float v0);
	void(__stdcall *Uniform2f)(int location, float v0, float v1);
	int(__stdcall *GetUniformLocation)(unsigned program, const char* name);
	void(__stdcall *VertexAttribPointer)(unsigned index, int size, GLenum type, GLboolean normalized, int stride, const void* pointer);
	void(__stdcall *EnableVertexAttribArray)(unsigned index);
	void(__stdcall *BindBuffer)(GLenum target, unsigned buffer);
	void(__stdcall *BufferData)(GLenum target, intptr_t size, const void* data, GLenum usage);
	void(__stdcall *GenBuffers)(int n, unsigned* buffers);

#ifdef _DEBUG
	using GLDEBUGPROC = void(__stdcall*)(GLenum source, GLenum type, unsigned id, GLenum severity, int length, const char* message, const void* userParam);
	void(__stdcall* DebugMessageCallback)(GLDEBUGPROC callback, const void* userParam);
#endif

	enum {
		FRAGMENT_SHADER = 0x8B30,
		VERTEX_SHADER,
		ARRAY_BUFFER = 0x8892,
		POINT_SPRITE = 0x8861,
		PROGRAM_POINT_SIZE = 0x8642,
		STATIC_DRAW = 0x88E4,
		DEBUG_OUTPUT_SYNCHRONOUS = 0x8242
	};
};

extern GL gl;

HWND CreateGLWindow(HDC* pHDC, HGLRC* pHGLRC);
bool AppProcessMessages(HWND wnd);

unsigned CompileShader(const char* code, GLenum type);
unsigned LinkShaderProgram(unsigned vertexShader, unsigned fragmentShader);
unsigned CompileShaderProgram(const char* vertexCode, const char* fragmentCode);

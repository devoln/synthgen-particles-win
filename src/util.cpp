#include "util.h"
#include <GL/wglext.h>

void GL::InitExtensions() const
{
#define GLLOAD(name) const_cast<GL*>(this)->name = (decltype(name))wglGetProcAddress("gl" #name)
	GLLOAD(CreateProgram);
	GLLOAD(CreateShader);
	GLLOAD(ShaderSource);
	GLLOAD(CompileShader);
	GLLOAD(AttachShader);
	GLLOAD(LinkProgram);
	GLLOAD(UseProgram);
	GLLOAD(Uniform1f);
	GLLOAD(Uniform2f);
	GLLOAD(Uniform3f);
	GLLOAD(Uniform4fv);
	GLLOAD(GetUniformLocation);
	GLLOAD(VertexAttribPointer);
	GLLOAD(EnableVertexAttribArray);
	GLLOAD(DisableVertexAttribArray);
	GLLOAD(BindBuffer);
	GLLOAD(BufferData);
	GLLOAD(GenBuffers);

#ifdef DEBUG_GL
	GLLOAD(GetShaderInfoLog);
	GLLOAD(GetProgramInfoLog);
	GLLOAD(DebugMessageCallback);
#endif

#undef GLLOAD
}




void AppProcessMessages()
{
	for(;;)
	{
		MSG msg;
		if(!PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) return;
		DispatchMessageA(&msg);
	}
}

#ifdef DEBUG_GL

void LogMessage(const char* message)
{
	OutputDebugStringA(message);
	OutputDebugStringA("\r\n");
}

static void __stdcall GL_debug_output_callback(GLenum /*source*/, GLenum /*type*/, unsigned /*id*/,
	GLenum /*severity*/, int /*length*/, const char* message, void* /*userParam*/)
{
	LogMessage(message);
}

#endif

unsigned CompileShader(const char* code, GLenum type)
{
	const unsigned sh = gl.CreateShader(type);
	gl.ShaderSource(sh, 1, &code, 0);
	gl.CompileShader(sh);
#ifdef DEBUG_GL
	char log[2048];
	gl.GetShaderInfoLog(sh, 2048, nullptr, log);
	LogMessage(log);
#endif
	return sh;
}

unsigned LinkShaderProgram(unsigned vertexShader, unsigned fragmentShader)
{
	const int prog = gl.CreateProgram();
	gl.AttachShader(prog, vertexShader);
	gl.AttachShader(prog, fragmentShader);
	gl.LinkProgram(prog);
	gl.UseProgram(prog);
#ifdef DEBUG_GL
	char log[2048];
	gl.GetProgramInfoLog(prog, 2048, nullptr, log);
	LogMessage(log);
#endif
	return prog;
}

unsigned CompileShaderProgram(const char* vertexCode, const char* fragmentCode)
{
	return LinkShaderProgram(CompileShader(vertexCode, GL_VERTEX_SHADER), CompileShader(fragmentCode, GL_FRAGMENT_SHADER));
}

HWND CreateGLWindow(HDC* pHDC, HGLRC* pHGLRC)
{
	ShowCursor(false);

	/*WNDCLASSA wc = {CS_HREDRAW|CS_VREDRAW, DefWindowProcA, 0, 0, GetModuleHandleA(nullptr), nullptr,
	nullptr, nullptr, 0, "demo_window_class"};
	RegisterClassA(&wc);*/

	HWND hWnd = CreateWindowExA(0, "static", nullptr, WS_POPUP|WS_VISIBLE
#ifndef FULL_SCREEN
		|WS_OVERLAPPEDWINDOW
#endif
		,
		0, 0, 1, 1, nullptr, nullptr, nullptr, nullptr);
	ShowWindow(hWnd, SW_MAXIMIZE);
	HDC hDC = GetDC(hWnd);

	const static PIXELFORMATDESCRIPTOR pfd = {0, 0, PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER};

	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	HGLRC context = wglCreateContext(hDC);
	wglMakeCurrent(hDC, context);

#ifdef DEBUG_GL
	typedef HGLRC(__stdcall *WGLCCA)(HDC, HGLRC, const int*);
	auto wglCreateContextAttribsARB = (WGLCCA)wglGetProcAddress("wglCreateContextAttribsARB");
	if(wglCreateContextAttribsARB)
	{
		wglMakeCurrent(hDC, nullptr);
		wglDeleteContext(context);

		const int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 0,
			WGL_CONTEXT_PROFILE_MASK_ARB, /*coreProfile? WGL_CONTEXT_CORE_PROFILE_BIT_ARB: */WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB|/*WGL_CONTEXT_FORWARD_COMPATIBLE_BIT*/0, 0
		};

		context = wglCreateContextAttribsARB(hDC, 0, attribs);
		wglMakeCurrent(hDC, context);
	}
#endif

	//PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	//if(wglSwapIntervalEXT) wglSwapIntervalEXT(1);

	gl.InitExtensions();

#ifdef DEBUG_GL
	if(gl.DebugMessageCallback)
	{
		gl.DebugMessageCallback(GL_debug_output_callback, nullptr);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
#endif

	if(pHDC) *pHDC = hDC;
	if(pHGLRC) *pHGLRC = context;
	return hWnd;
}

#include "util.h"
#include <cstdio>

void GL::LoadFunctions()
{
#define GLLOAD(name) name = reinterpret_cast<decltype(name)>(wglGetProcAddress("gl" #name))
	GLLOAD(CreateProgram);
	GLLOAD(CreateShader);
	GLLOAD(ShaderSource);
	GLLOAD(CompileShader);
	GLLOAD(AttachShader);
	GLLOAD(LinkProgram);
	GLLOAD(UseProgram);
	GLLOAD(Uniform1f);
	GLLOAD(Uniform2f);
	GLLOAD(GetUniformLocation);
	GLLOAD(VertexAttribPointer);
	GLLOAD(EnableVertexAttribArray);
	GLLOAD(BindBuffer);
	GLLOAD(BufferData);
	GLLOAD(GenBuffers);
#ifdef _DEBUG
	GLLOAD(DebugMessageCallback);
#endif
#undef GLLOAD
}


bool AppProcessMessages(HWND wnd)
{
	for(;;)
	{
		MSG msg;
		if(!PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) return false;
		DispatchMessageA(&msg);
		if(msg.message == WM_QUIT) return true;
	}
}

static void __stdcall onGLDebugMessage(GLenum /*source*/, GLenum /*type*/, unsigned /*id*/,
	GLenum /*severity*/, int /*length*/, const char* message, const void* /*userParam*/)
{
	puts(message);
}

unsigned CompileShader(const char* code, GLenum type)
{
	const unsigned sh = gl.CreateShader(type);
	gl.ShaderSource(sh, 1, &code, 0);
	gl.CompileShader(sh);
	return sh;
}

unsigned LinkShaderProgram(unsigned vertexShader, unsigned fragmentShader)
{
	const int prog = gl.CreateProgram();
	gl.AttachShader(prog, vertexShader);
	gl.AttachShader(prog, fragmentShader);
	gl.LinkProgram(prog);
	gl.UseProgram(prog);
	return prog;
}

unsigned CompileShaderProgram(const char* vertexCode, const char* fragmentCode)
{
	return LinkShaderProgram(
		CompileShader(vertexCode, GL::VERTEX_SHADER),
		CompileShader(fragmentCode, GL::FRAGMENT_SHADER));
}

LRESULT __stdcall WinProc(HWND hWnd, unsigned msg, size_t wParam, LPARAM lParam)
{
	if(msg == WM_DESTROY) PostQuitMessage(0);
	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

HWND CreateGLWindow(HDC* pHDC, HGLRC* pHGLRC)
{
	ShowCursor(false);

	WNDCLASSA wc = {
		CS_HREDRAW|CS_VREDRAW, WinProc, 0, 0, GetModuleHandleA(nullptr), nullptr,
		nullptr, nullptr, 0, "synthgen"
	};
	RegisterClassA(&wc);

	HWND hWnd = CreateWindowExA(0, "synthgen", nullptr, WS_POPUP|WS_VISIBLE
	#ifdef _DEBUG
		|WS_OVERLAPPEDWINDOW
	#endif
		,
		0, 0, 1, 1, nullptr, nullptr, nullptr, nullptr);
	ShowWindow(hWnd, SW_MAXIMIZE);
	HDC hDC = GetDC(hWnd);

	const static PIXELFORMATDESCRIPTOR pfd = {
		0, 0, PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER
	};

	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	HGLRC context = wglCreateContext(hDC);
	wglMakeCurrent(hDC, context);

#ifdef _DEBUG
	if(auto wglCreateContextAttribsARB = reinterpret_cast<
		HGLRC(__stdcall*)(HDC, HGLRC, const int*)>(
			wglGetProcAddress("wglCreateContextAttribsARB")))
	{
		wglMakeCurrent(hDC, nullptr);
		wglDeleteContext(context);

		enum {
			WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091,
			WGL_CONTEXT_MINOR_VERSION_ARB,
			WGL_CONTEXT_FLAGS_ARB = 0x2094,
			WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126,
			WGL_CONTEXT_DEBUG_BIT_ARB = 0x00000001,
			WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB = 0x00000002
		};

		const int attribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 0,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
			0
		};

		context = wglCreateContextAttribsARB(hDC, 0, attribs);
		wglMakeCurrent(hDC, context);
	}
#endif

	if(auto wglSwapIntervalEXT = reinterpret_cast<
		int(__stdcall*)(int interval)>(
			wglGetProcAddress("wglSwapIntervalEXT")))
		wglSwapIntervalEXT(1);

	gl.LoadFunctions();

#ifdef _DEBUG
	if(gl.DebugMessageCallback)
	{
		gl.DebugMessageCallback(onGLDebugMessage, nullptr);
		glEnable(GL::DEBUG_OUTPUT_SYNCHRONOUS);
	}
#endif

	if(pHDC) *pHDC = hDC;
	if(pHGLRC) *pHGLRC = context;
	return hWnd;
}

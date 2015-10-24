#include <windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <cstdio>
#include <cassert>
#include "Line.h"
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "opengl32.lib")

//Global
HDC hDC;
HGLRC openGLRC;
RECT clientRect;

//OpenGL
GLuint vertexBuf = 0;
GLuint indexBuf = 0;
GLuint vShader = 0;
GLuint pShader = 0;
GLuint program = 0;
GLint attributePos = 0;
GLint uniformViewport = 0;
GLenum err = GL_NO_ERROR;

BOOL CreateOpenGLRenderContext(HWND hWnd);
BOOL InitGLEW();
BOOL InitOpenGL();
void DestroyOpenGL(HWND hWnd);
BOOL Update(HWND hWnd);
void Render(HWND hWnd);
void fnCheckGLError(const char* szFile, int nLine);
#define _CheckGLError_ fnCheckGLError(__FILE__,__LINE__);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int __stdcall WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	BOOL bResult = FALSE;

	MSG msg = { 0 };
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = L"Opengl4TemplateWindowClass";
	wc.style = CS_OWNDC;
	if (!RegisterClass(&wc))
		return 1;
	HWND hWnd = CreateWindowW(wc.lpszClassName, L"Opengl 4 Template", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, hInstance, 0);

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	bResult = CreateOpenGLRenderContext(hWnd);
	if (bResult == FALSE)
	{
		OutputDebugStringA("CreateOpenGLRenderContext failed!\n");
		return 1;
	}
	InitGLEW();
	InitOpenGL();

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update(hWnd);
			Render(hWnd);
		}
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		DestroyOpenGL(hWnd);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL InitGLEW()
{
	char buffer[128];
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		sprintf(buffer, "Error: %s\n", (char*)glewGetErrorString(err));
		OutputDebugStringA(buffer);
		return FALSE;
	}
	sprintf(buffer, "Status: Using GLEW %s\n", (char*)glewGetString(GLEW_VERSION));
	OutputDebugStringA(buffer);
	return TRUE;
}


BOOL CreateOpenGLRenderContext(HWND hWnd)
{
	BOOL bResult;
	char buffer[128];
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd  
		1,                     // version number  
		PFD_DRAW_TO_WINDOW |   // support window  
		PFD_SUPPORT_OPENGL |   // support OpenGL  
		PFD_DOUBLEBUFFER,      // double buffered  
		PFD_TYPE_RGBA,         // RGBA type  
		32,                    // 24-bit color depth  
		0, 0, 0, 0, 0, 0,      // color bits ignored  
		0,                     // no alpha buffer  
		0,                     // shift bit ignored  
		0,                     // no accumulation buffer  
		0, 0, 0, 0,            // accum bits ignored  
		24,                    // 32-bit z-buffer      
		8,                     // no stencil buffer  
		0,                     // no auxiliary buffer  
		PFD_MAIN_PLANE,        // main layer  
		0,                     // reserved  
		0, 0, 0                // layer masks ignored  
	};

	hDC = GetDC(hWnd);
	if (hDC == NULL)
	{
		OutputDebugStringA("Error: GetDC Failed!\n");
		return FALSE;
	}

	int pixelFormatIndex;
	pixelFormatIndex = ChoosePixelFormat(hDC, &pfd);
	if (pixelFormatIndex == 0)
	{
		sprintf(buffer, "Error %d: ChoosePixelFormat Failed!\n", GetLastError());
		OutputDebugStringA(buffer);
		return FALSE;
	}
	bResult = SetPixelFormat(hDC, pixelFormatIndex, &pfd);
	if (pixelFormatIndex == FALSE)
	{
		OutputDebugStringA("SetPixelFormat Failed!\n");
		return FALSE;
	}

	openGLRC = wglCreateContext(hDC);
	if (openGLRC == NULL)
	{
		sprintf(buffer, "Error %d: wglCreateContext Failed!\n", GetLastError());
		OutputDebugStringA(buffer);
		return FALSE;
	}
	bResult = wglMakeCurrent(hDC, openGLRC);
	if (bResult == FALSE)
	{
		sprintf(buffer, "Error %d: wglMakeCurrent Failed!\n", GetLastError());
		OutputDebugStringA(buffer);
		return FALSE;
	}

	sprintf(buffer, "OpenGL version info: %s\n", (char*)glGetString(GL_VERSION));
	OutputDebugStringA(buffer);
	return TRUE;
}

void fnCheckGLError(const char* szFile, int nLine)
{
	GLenum ErrCode = glGetError();
	if (GL_NO_ERROR != ErrCode)
	{
		const char* szErr = "GL_UNKNOWN ERROR";
		switch (ErrCode)
		{
		case GL_INVALID_ENUM:		szErr = "GL_INVALID_ENUM		";		break;
		case GL_INVALID_VALUE:		szErr = "GL_INVALID_VALUE		";		break;
		case GL_INVALID_OPERATION:	szErr = "GL_INVALID_OPERATION	";		break;
		case GL_OUT_OF_MEMORY:		szErr = "GL_OUT_OF_MEMORY		";		break;
		}
		char buffer[512];
		sprintf(buffer, "%s(%d):glError %s\n", szFile, nLine, szErr);
		OutputDebugStringA(buffer);
	}
}

BOOL InitOpenGL()
{
	GLint compiled;
	//Vertex shader
	const char* vShaderStr = R"(
uniform vec2 Viewport;
attribute vec4 vPosition;
void main()
{
	gl_Position = vec4(
					2.0*vPosition.x/Viewport.x - 1.0,
					2.0*vPosition.y/Viewport.y - 1.0,
					0.0, 1.0);
}
)";
	vShader = glCreateShader(GL_VERTEX_SHADER);
	if (vShader == 0)
	{
		OutputDebugString(L"glCreateShader Failed!\n");
		return -1;
	}
	glShaderSource(vShader, 1, &vShaderStr, nullptr);
	glCompileShader(vShader);
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		GLint infoLength = 0;
		glGetShaderiv(vShader, GL_INFO_LOG_LENGTH, &infoLength);
		if (infoLength > 1)
		{
			char* infoLog = (char*)malloc(sizeof(char)*infoLength);
			glGetShaderInfoLog(vShader, infoLength, nullptr, infoLog);
			OutputDebugString(L"Error compiling shader: \n");
			OutputDebugStringA(infoLog);
			OutputDebugStringA("\n");
			free(infoLog);
		}
		glDeleteShader(vShader);
		return -1;
	}

	//Fragment shader
	const char* pShaderStr = R"(
void main()
{
gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";
	pShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (vShader == 0)
	{
		OutputDebugString(L"glCreateShader Failed!\n");
		return FALSE;
	}
	glShaderSource(pShader, 1, &pShaderStr, nullptr);
	glCompileShader(pShader);
	glGetShaderiv(pShader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		GLint infoLength = 0;
		glGetShaderiv(pShader, GL_INFO_LOG_LENGTH, &infoLength);
		if (infoLength > 1)
		{
			char* infoLog = (char*)malloc(sizeof(char)*infoLength);
			glGetShaderInfoLog(pShader, infoLength, NULL, infoLog);
			OutputDebugString(L"Error compiling shader: \n");
			OutputDebugStringA(infoLog);
			OutputDebugStringA("\n");
			free(infoLog);
		}
		glDeleteShader(pShader);
		return FALSE;
	}

	//Program
	GLint linked;
	program = glCreateProgram();
	if (program == 0)
	{
		OutputDebugString(L"glCreateProgram Failed!\n");
		return -1;
	}
	glAttachShader(program, vShader);
	glAttachShader(program, pShader);
	glLinkProgram(program);
	attributePos = glGetAttribLocation(program, "vPosition");//get location of attribute <vPosition>
	uniformViewport = glGetUniformLocation(program, "Viewport");//get location of uniform <Viewport>
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		GLint infoLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength);
		if (infoLength > 1)
		{
			char* infoLog = (char*)malloc(sizeof(char) * infoLength);
			glGetProgramInfoLog(program, infoLength, nullptr, infoLog);
			OutputDebugString(L"Error Linking program: \n");
			OutputDebugStringA(infoLog);
			OutputDebugStringA("\n");
			free(infoLog);
		}
		glDeleteProgram(program);
		return FALSE;
	}
	glUseProgram(program);

	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

	//Create Line
	Line line1;
	line1.Set(10.0f, 10.f, 20.0f, 20.0f);

	GLfloat vertex[] =
	{
		100.0f, 100.0f, 0.0f,
		100.0f, 300.0f, 0.0f,
		400.0f, 300.0f, 0.0f,
		400.0f, 100.0f, 0.0f,
	};
	glGenBuffers(1, &vertexBuf);
	assert(vertexBuf != 0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), (GLvoid*)vertex, GL_STATIC_DRAW);

	GLushort index[] = { 0, 1, 2, 2, 3, 0 };
	glGenBuffers(1, &indexBuf);
	assert(indexBuf != 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), (GLvoid*)index, GL_STATIC_DRAW);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	return TRUE;
}

void DestroyOpenGL(HWND hWnd)
{
	//OpenGL Destroy
	glDeleteShader(vShader);
	glDeleteShader(pShader);
	glDeleteProgram(program);

	//OpenGL Context Destroy
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(openGLRC);
	ReleaseDC(hWnd, hDC);
}

BOOL Update(HWND hWnd)
{
	RECT newClientRect;
	// Set viewport according to the client rect
	if (!GetClientRect(hWnd, &newClientRect))
	{
		OutputDebugString(L"GetClientRect Failed!\n");
		return FALSE;
	}
	if (newClientRect.left != clientRect.left
		|| newClientRect.top != clientRect.top
		|| newClientRect.right != clientRect.right
		|| newClientRect.bottom != clientRect.bottom)
	{
		clientRect = newClientRect;
		glViewport(0, 0, GLsizei(clientRect.right - clientRect.left), GLsizei(clientRect.bottom - clientRect.top));
	}
	return TRUE;
}

void Render(HWND hWnd)
{
	glClear(GL_COLOR_BUFFER_BIT);
	_CheckGLError_

	//Clear
	glClear(GL_COLOR_BUFFER_BIT);

	//Set Uniform "Viewport"
	glUniform2f(uniformViewport, GLfloat(clientRect.right - clientRect.left), GLfloat(clientRect.bottom - clientRect.top));

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

	glVertexAttribPointer(attributePos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attributePos);

	//Draw two trangles
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
	err = glGetError();

	SwapBuffers(hDC);
}
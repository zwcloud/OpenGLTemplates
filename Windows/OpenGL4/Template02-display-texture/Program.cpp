#include <windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <cstdio>
#include <cassert>
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "opengl32.lib")
#include "../../LodePNG/lodepng.h"

//Global
HDC hDC;
HGLRC openGLRC;
RECT clientRect;

//OpenGL
GLuint vertexBuf = 0;
GLuint indexBuf = 0;
GLuint texture = 0;
GLuint vShader = 0;
GLuint pShader = 0;
GLuint program = 0;
GLint attributePos = 0;
GLint attributeTexCoord = 0;
GLint uniformViewport = 0;
GLenum err = GL_NO_ERROR;

//image
unsigned char* image;
unsigned width, height;

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
	wc.lpszClassName = L"Opengl330TemplateWindowClass";
	wc.style = CS_OWNDC;
	if (!RegisterClass(&wc))
		return 1;
	HWND hWnd = CreateWindowW(wc.lpszClassName, L"Opengl 3.3 Template", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, hInstance, 0);

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
		32,                    // 32-bit color depth  
		0, 0, 0, 0, 0, 0,      // color bits ignored  
		0,                     // no alpha buffer  
		0,                     // shift bit ignored  
		0,                     // no accumulation buffer  
		0, 0, 0, 0,            // accum bits ignored  
		24,                    // 24-bit z-buffer      
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
	if (bResult == FALSE)
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
#version 330 core
uniform vec2 Viewport;
in vec4 vPosition;
in vec2 vTexCoord;
out vec2 out_TexCoord;
void main()
{
	gl_Position = vec4(
					2.0*vPosition.x/Viewport.x - 1.0,
					2.0*vPosition.y/Viewport.y - 1.0,
					0.0, 1.0);
	out_TexCoord = vTexCoord;
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
			OutputDebugString(L"Error compiling vertex shader: \n");
			OutputDebugStringA(infoLog);
			OutputDebugStringA("\n");
			free(infoLog);
		}
		glDeleteShader(vShader);
		return -1;
	}

	//Fragment shader
	const char* pShaderStr = R"(
#version 330 core
uniform sampler2D mysampler;
in vec2 out_TexCoord;
out vec4 out_Color;
void main()
{
	vec2 st = out_TexCoord.st;
	out_Color = texture2D(mysampler,vec2(st.s, 1- st.t));
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
			OutputDebugString(L"Error compiling fragment shader: \n");
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

	//get attribute and uniform location by name
	attributePos = glGetAttribLocation(program, "vPosition");//get location of attribute <vPosition>
	attributeTexCoord = glGetAttribLocation(program, "vTexCoord");//get location of attribute <vTexCoord>
	uniformViewport = glGetUniformLocation(program, "Viewport");//get location of uniform <Viewport>
	
	//vertex buffer
	GLfloat vertex[] =
	{
		100.0f, 100.0f, 0.0f, 0,0,
		100.0f, 300.0f, 0.0f, 0,1,
		400.0f, 300.0f, 0.0f, 1,1,
		400.0f, 100.0f, 0.0f, 1,0
	};
	glGenBuffers(1, &vertexBuf);
	assert(vertexBuf != 0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), (GLvoid*)vertex, GL_STATIC_DRAW);

	//index buffer
	GLushort index[] = { 0, 1, 2, 2, 3, 0 };
	glGenBuffers(1, &indexBuf);
	assert(indexBuf != 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), (GLvoid*)index, GL_STATIC_DRAW);

	//set attribute of positon and texcoord
	glVertexAttribPointer(attributePos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glVertexAttribPointer(attributeTexCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//enable attribute array
	glEnableVertexAttribArray(attributePos);
	glEnableVertexAttribArray(attributeTexCoord);

	//texture
	unsigned error;

	error = lodepng_decode32_file(&image, &width, &height, "CheckerMap.png");
	if (error)
	{
		char buf[128];
		sprintf(buf, "error %u: %s\n", error, lodepng_error_text(error));
		OutputDebugStringA(buf);
	}

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	free(image);

	//sampler settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//other settings
	glClearColor(0.0f, 0.0f, 0.9f, 1.0f);
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
	_CheckGLError_

	//Clear
	glClear(GL_COLOR_BUFFER_BIT);

	//Set Uniform "Viewport"
	glUniform2f(uniformViewport, GLfloat(clientRect.right - clientRect.left), GLfloat(clientRect.bottom - clientRect.top));

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

	glBindTexture(GL_TEXTURE_2D, texture);

	//Draw two trangles
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
	err = glGetError();

	SwapBuffers(hDC);
}
#include <cstdio>
#include <cstdarg>
#include <string>

#ifdef __APPLE__
#include <GLFW/glfw3.h>
#define vsprintf_s vsnprintf
#elif _WIN32
#include "glfw3.h"
#include <windows.h>
#endif

#ifndef _WIN32
#define APIENTRY
#endif

#include "SystemInfo.hpp"

void _update_fps_counter(GLFWwindow* window){

	static double previous_seconds = glfwGetTime();
	static int frame_count;
	double current_seconds = glfwGetTime();
	double elapsed_seconds = current_seconds - previous_seconds;
	if(elapsed_seconds > 0.25){
		previous_seconds = current_seconds;
		double fps = (double)frame_count / elapsed_seconds;
		char tmp[128];
		sprintf(tmp, "opengl @ fps: %.2f", fps);
		glfwSetWindowTitle(window, tmp);
		frame_count = 0;
	}
	frame_count++;
}

//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------
void dprintf(const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );
	vsprintf_s( buffer, fmt, args );
	va_end( args );

	printf( "%s", buffer );

	OutputDebugStringA( buffer );
}

//-----------------------------------------------------------------------------
// Purpose: Outputs the string in message to debugging output.
//          All other parameters are ignored.
//          Does not return any meaningful value or reference.
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	dprintf( "GL Error: %s\n", message );
}

float FConvertUint32ToFloat(uint32_t val)
{
	static_assert(sizeof(float) == sizeof val, "Error: FConvertUint32ToFloat - passed argument is not compatible");
	float returnVal;
	std::memcpy(&returnVal, &val, sizeof(float));
	return returnVal;
}
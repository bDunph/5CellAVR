#include <iostream>
#include <vector>
#include <windows.h>
#include <cmath>
#include <stdlib.h>

#include "lodepng.h"
#include "Graphics.hpp"
#include "ShaderManager.hpp"
#include "Log.hpp"
#include "SystemInfo.hpp"

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

//-----------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------
Graphics::Graphics(std::unique_ptr<ExecutionFlags>& flagPtr) : 
	m_pGLContext(nullptr),
	m_nCompanionWindowWidth(640),
	m_nCompanionWindowHeight(320),
	m_iTrackedControllerCount(0),
	m_iTrackedControllerCount_Last(-1),
	m_iValidPoseCount_Last(-1),
	m_unControllerVAO(0),
	m_glControllerVertBuffer(0),
	m_unSceneVAO(0),
	m_glMainShaderProgramID(0),
	m_unCompanionWindowProgramID(0),
	m_unControllerTransformProgramID(0),
	m_unRenderModelProgramID(0),	
	m_nControllerMatrixLocation(-1),
	m_nRenderModelMatrixLocation(-1),
	m_bVblank(true),
	m_bGLFinishHack(true),
	m_bDebugOpenGL(false),
	m_unImageCount(0),
	m_uiFrameNumber(0)
{
	m_bDebugOpenGL = flagPtr->flagDebugOpenGL;
	m_bVblank = flagPtr->flagVBlank;
	m_bGLFinishHack = flagPtr->flagGLFinishHack;
	m_bDebugPrintMessages = flagPtr->flagDPrint;	

	m_pRotationVal = std::make_unique<int>();
	*m_pRotationVal = 0;

	m_bRecordScreen = true;

	//m_tStartTime = time(0);

}



//----------------------------------------------------------------------
// Initialise OpenGL context, companion window, glew and vsync.
// ---------------------------------------------------------------------	
bool Graphics::BInitGL(std::unique_ptr<VR_Manager>& vrm, bool fullscreen){
	
	//start gl context and O/S window using the glfw helper library
	if(!glfwInit()){
		std::cerr << "ERROR: could not start GLFW3\n";
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	
	if(m_bDebugOpenGL){
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glDebugMessageCallback( (GLDEBUGPROC)DebugCallback, nullptr);
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	if(!fullscreen){
		windowWidth = m_nCompanionWindowWidth;
		windowHeight = m_nCompanionWindowHeight;
	} else {
		GLFWmonitor* mon = glfwGetPrimaryMonitor();
		const GLFWvidmode* vmode = glfwGetVideoMode(mon);

		windowWidth = vmode->width;
		windowHeight = vmode->height;
	}

	m_pGLContext = glfwCreateWindow(windowWidth, windowHeight, "AVR", NULL, NULL);	

	if(!m_pGLContext){
		std::cerr << "ERROR: could not open window with GLFW3\n";
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(m_pGLContext);

	//start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	if(m_bVblank){
#ifdef _WIN32
	//turn on vsync on windows
	//it uses the WGL_EXT_swap_control extension
	typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int interval);
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if(!wglSwapIntervalEXT){
		std::cout << "Warning: VSync not enabled" << std::endl;
		m_bVblank = false;
	}
	wglSwapIntervalEXT(1);
	std::cout << "Graphics::BInitGL - VSync Enabled" << std::endl;
#endif
	}

	const GLubyte* renderer = glGetString(GL_RENDERER); //get renderer string
	const GLubyte* version = glGetString(GL_VERSION); //version as a string
	std::cout << "Graphics: " << renderer << std::endl;
	std::cout << "OpenGL version supported " << version << std::endl;

	//tell GL only to draw onto a pixel if a shape is closer to the viewer
	glEnable(GL_DEPTH_TEST);//enable depth testing
	glDepthFunc(GL_LESS);//depth testing interprets a smaller value as 'closer'

	// setup scene geometry
	skyboxShaderProg = BCreateSceneShaders("skybox");
	soundObjShaderProg = BCreateSceneShaders("soundObj");
	groundPlaneShaderProg = BCreateSceneShaders("groundPlane");
	fiveCellShaderProg = BCreateSceneShaders("rasterPolychoron");
	fiveCell.setup("mode5cell.csd", skyboxShaderProg, soundObjShaderProg, groundPlaneShaderProg, fiveCellShaderProg);	
	return true;
}

//-----------------------------------------------------------------------------
// Create shaders for scene geometry
//----------------------------------------------------------------------------
GLuint Graphics::BCreateSceneShaders(std::string shaderName){

	std::string vertName = shaderName;
	std::string fragName = shaderName;
	std::string vertShaderName = vertName.append(".vert");
	std::string fragShaderName = fragName.append(".frag");

	//load shaders
	const char* vertShader;
	bool isVertLoaded = load_shader(vertShaderName.c_str(), vertShader);
	if(!isVertLoaded) return NULL;
	
	const char* fragShader;
	bool isFragLoaded = load_shader(fragShaderName.c_str(), fragShader);
	if(!isFragLoaded) return NULL;
	
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertShader, NULL);
	glCompileShader(vs);
	delete[] vertShader;
	//check for compile errors
	bool isVertCompiled = shader_compile_check(vs);
	if(!isVertCompiled) return NULL;
	
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragShader, NULL);
	glCompileShader(fs);
	delete[] fragShader;
	//check for compile errors
	bool isFragCompiled = shader_compile_check(fs);
	if(!isFragCompiled) return NULL;
	
	GLuint shaderProg = glCreateProgram();
	glAttachShader(shaderProg, fs);
	glAttachShader(shaderProg, vs);
	glLinkProgram(shaderProg);
	bool didShadersLink = shader_link_check(shaderProg);
	if(!didShadersLink) return NULL;

	//only use during development as computationally expensive
	bool validProgram = is_valid(shaderProg);
	if(!validProgram){
		fprintf(stderr, "ERROR: shaderProg not valid\n");
		return NULL;
	}

	return shaderProg;
}

//-----------------------------------------------------------------------------
// Creates shaders used for the controllers 
//-----------------------------------------------------------------------------
bool Graphics::BCreateDefaultShaders()
{
	m_unControllerTransformProgramID = CompileGLShader(
		"Controller",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3ColorIn;\n"
		"out vec4 v4Color;\n"
		"void main()\n"
		"{\n"
		"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// fragment shader
		"#version 410\n"
		"in vec4 v4Color;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = v4Color;\n"
		"}\n"
		);
	m_nControllerMatrixLocation = glGetUniformLocation(m_unControllerTransformProgramID, "matrix");
	if(m_nControllerMatrixLocation == -1)
	{
		std::cout << "Unable to find matrix uniform in controller shader\n" << std::endl;;
		return false;
	}

	m_unRenderModelProgramID = CompileGLShader( 
		"render model",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3NormalIn;\n"
		"layout(location = 2) in vec2 v2TexCoordsIn;\n"
		"out vec2 v2TexCoord;\n"
		"void main()\n"
		"{\n"
		"	v2TexCoord = v2TexCoordsIn;\n"
		"	gl_Position = matrix * vec4(position.xyz, 1);\n"
		"}\n",

		//fragment shader
		"#version 410 core\n"
		"uniform sampler2D diffuse;\n"
		"in vec2 v2TexCoord;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = texture( diffuse, v2TexCoord);\n"
		"}\n"

		);
	m_nRenderModelMatrixLocation = glGetUniformLocation( m_unRenderModelProgramID, "matrix" );
	if(m_nRenderModelMatrixLocation == -1)
	{
		std::cout << "Unable to find matrix uniform in render model shader\n" << std::endl;
		return false;
	}

	m_unCompanionWindowProgramID = CompileGLShader(
		"CompanionWindow",

		// vertex shader
		"#version 410 core\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVIn;\n"
		"noperspective out vec2 v2UV;\n"
		"void main()\n"
		"{\n"
		"	v2UV = v2UVIn;\n"
		"	gl_Position = position;\n"
		"}\n",

		// fragment shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"noperspective in vec2 v2UV;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"	
		"		outputColor = texture(mytexture, v2UV);\n"
		"}\n"
		);

	return m_unControllerTransformProgramID != 0
		&& m_unRenderModelProgramID != 0
		&& m_unCompanionWindowProgramID != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Compiles a GL shader program and returns the handle. Returns 0 if
//			the shader couldn't be compiled for some reason. ***********************************************************************Need to converge this code with BMainShaderSetup*************************
//-----------------------------------------------------------------------------
GLuint Graphics::CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader )
{
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource( nSceneVertexShader, 1, &pchVertexShader, NULL);
	glCompileShader( nSceneVertexShader );

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if ( vShaderCompiled != GL_TRUE)
	{
		dprintf("%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneVertexShader );
		return 0;
	}
	glAttachShader( unProgramID, nSceneVertexShader);
	glDeleteShader( nSceneVertexShader ); // the program hangs onto this once it's attached

	GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource( nSceneFragmentShader, 1, &pchFragmentShader, NULL);
	glCompileShader( nSceneFragmentShader );

	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		dprintf("%s - Unable to compile fragment shader %d!\n", pchShaderName, nSceneFragmentShader );
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneFragmentShader );
		return 0;	
	}

	glAttachShader( unProgramID, nSceneFragmentShader );
	glDeleteShader( nSceneFragmentShader ); // the program hangs onto this once it's attached

	glLinkProgram( unProgramID );

	GLint programSuccess = GL_TRUE;
	glGetProgramiv( unProgramID, GL_LINK_STATUS, &programSuccess);
	if ( programSuccess != GL_TRUE )
	{
		dprintf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram( unProgramID );
		return 0;
	}

	glUseProgram( unProgramID );
	glUseProgram( 0 );

	return unProgramID;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Graphics::BSetupStereoRenderTargets(std::unique_ptr<VR_Manager>& vrm)
{
	if ( !vrm->m_pHMD )
		return false;

	vrm->m_pHMD->GetRecommendedRenderTargetSize( &m_nRenderWidth, &m_nRenderHeight );

	bool fboL = BCreateFrameBuffer(leftEyeDesc);
	bool fboR = BCreateFrameBuffer(rightEyeDesc);
	if(!fboL || !fboR) return false;

	return true;
}

//-----------------------------------------------------------------------------
// Creates two frame buffers. One with a depth and colour attachments. Another with just a colour attachment. 
// Returns true if the buffers were set up.
// Returns false if the setup failed.
//-----------------------------------------------------------------------------
bool Graphics::BCreateFrameBuffer(FramebufferDesc& framebufferDesc)
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, m_nRenderWidth, m_nRenderHeight );
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,	framebufferDesc.m_nDepthBufferId );

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId );
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId );
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, m_nRenderWidth, m_nRenderHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId );
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_nRenderWidth, m_nRenderHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error: Frame buffer not created : Graphics::BCreateFrameBuffer" << std::endl;
		return false;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Graphics::BSetupCompanionWindow(std::unique_ptr<VR_Manager>& vrm)
{
	if ( !vrm->m_pHMD )
		return false;

	std::vector<VertexDataWindow> vVerts;

	// left eye verts
	vVerts.push_back( VertexDataWindow( glm::vec2(-1, -1), glm::vec2(0, 1)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(1, -1), glm::vec2(1, 1)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(-1, 1), glm::vec2(0, 0)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(1, 1), glm::vec2(1, 0)) );

	// right eye verts
	//vVerts.push_back( VertexDataWindow( Vector2(0, -1), Vector2(0, 1)) );
	//vVerts.push_back( VertexDataWindow( Vector2(1, -1), Vector2(1, 1)) );
	//vVerts.push_back( VertexDataWindow( Vector2(0, 1), Vector2(0, 0)) );
	//vVerts.push_back( VertexDataWindow( Vector2(1, 1), Vector2(1, 0)) );

	//GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6};
	GLushort vIndices[] = { 0, 1, 3,  0, 3, 2};
	m_uiCompanionWindowIndexSize = _countof(vIndices);

	glGenVertexArrays( 1, &m_unCompanionWindowVAO );
	glBindVertexArray( m_unCompanionWindowVAO );

	glGenBuffers( 1, &m_glCompanionWindowIDVertBuffer );
	glBindBuffer( GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer );
	glBufferData( GL_ARRAY_BUFFER, vVerts.size()*sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW );

	glGenBuffers( 1, &m_glCompanionWindowIDIndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize*sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, position ) );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, texCoord ) );

	glBindVertexArray( 0 );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

//-----------------------------------------------------------------------------
// Main function that renders textures to hmd.
//-----------------------------------------------------------------------------
void Graphics::RenderFrame(std::unique_ptr<VR_Manager>& vrm)
{

	
	//update values from controller actions
	if(vrm->BGetRotate3DTrigger()) IncreaseRotationValue(m_pRotationVal);

	// for now as fast as possible
	if ( vrm->m_pHMD )
	{
		RenderControllerAxes(vrm);
		RenderStereoTargets(vrm);
		RenderCompanionWindow();

		vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = {(void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	}

	if (m_bVblank && m_bGLFinishHack)
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}

	// SwapWindow
	{
		glfwSwapBuffers(m_pGLContext);
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor( 0, 0, 0, 1 );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Flush and wait for swap.
	if (m_bVblank)
	{
		glFlush();
		glFinish();
	}

	// Spew out the controller and pose count whenever they change.
	if (m_iTrackedControllerCount != m_iTrackedControllerCount_Last || vrm->m_iValidPoseCount != m_iValidPoseCount_Last )
	{
		m_iValidPoseCount_Last = vrm->m_iValidPoseCount;
		m_iTrackedControllerCount_Last = m_iTrackedControllerCount;
		
		std::printf( "PoseCount:%d(%s) Controllers:%d\n", vrm->m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount );
	}

	vrm->UpdateHMDMatrixPose();

	m_uiFrameNumber++;
	//std::cout << *m_pRotationVal << std::endl;
}

//-----------------------------------------------------------------------------
// This function increases the angle for the 3D rotation of the structure
// ----------------------------------------------------------------------------
void Graphics::IncreaseRotationValue(std::unique_ptr<int>& pVal){

	//std::cout << "HEY!" << std::endl;
	int count;
	count = *pVal;
	count++;
	int angle = count % 360;
	*pVal = angle;
}
		
//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void Graphics::RenderControllerAxes(std::unique_ptr<VR_Manager>& vrm)
{
	// Don't attempt to update controllers if input is not available
	if( !vrm->m_pHMD->IsInputAvailable() )
		return;

	std::vector<float> vertdataarray;

	m_uiControllerVertCount = 0;
	m_iTrackedControllerCount = 0;

	// this for loop shuold use eHand iterators from VR::Manager	
	for (int i = 0; i <= 1; i++)
	{
		if ( !vrm->m_rHand[i].m_bShowController )
			continue;

		const glm::mat4& mat = vrm->m_rHand[i].m_rmat4Pose;

		glm::vec4 center = mat * glm::vec4( 0, 0, 0, 1 );

		for ( int i = 0; i < 3; ++i )
		{
			glm::vec3 color( 0, 0, 0 );
			glm::vec4 point( 0, 0, 0, 1 );
			point[i] += 0.05f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = mat * point;
			vertdataarray.push_back( center.x );
			vertdataarray.push_back( center.y );
			vertdataarray.push_back( center.z );

			vertdataarray.push_back( color.x );
			vertdataarray.push_back( color.y );
			vertdataarray.push_back( color.z );
		
			vertdataarray.push_back( point.x );
			vertdataarray.push_back( point.y );
			vertdataarray.push_back( point.z );
		
			vertdataarray.push_back( color.x );
			vertdataarray.push_back( color.y );
			vertdataarray.push_back( color.z );
		
			m_uiControllerVertCount += 2;
		}

		glm::vec4 start = mat * glm::vec4( 0, 0, -0.02f, 1 );
		glm::vec4 end = mat * glm::vec4( 0, 0, -39.f, 1 );
		glm::vec3 color( .92f, .92f, .71f );

		vertdataarray.push_back( start.x );vertdataarray.push_back( start.y );vertdataarray.push_back( start.z );
		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );

		vertdataarray.push_back( end.x );vertdataarray.push_back( end.y );vertdataarray.push_back( end.z );
		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );
		m_uiControllerVertCount += 2;
	}

	// Setup the VAO the first time through.
	if ( m_unControllerVAO == 0 )
	{
		glGenVertexArrays( 1, &m_unControllerVAO );
		glBindVertexArray( m_unControllerVAO );

		glGenBuffers( 1, &m_glControllerVertBuffer );
		glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

		GLuint stride = 2 * 3 * sizeof( float );
		uintptr_t offset = 0;

		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof( Vector3 );
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray( 0 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

	// set vertex data if we have some
	if( vertdataarray.size() > 0 )
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData( GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW );
	}
}

//-----------------------------------------------------------------------------
// Draws textures to each eye of hmd. 
//-----------------------------------------------------------------------------
void Graphics::RenderStereoTargets(std::unique_ptr<VR_Manager>& vrm)
{
	glClearColor(0.87, 0.85, 0.75, 0.95);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_MULTISAMPLE);

	// Left Eye
	glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
 	RenderScene(vr::Eye_Left, vrm);
 	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glDisable(GL_MULTISAMPLE);
	 	
 	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId);

   	glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glEnable(GL_MULTISAMPLE);
	
	// Right Eye
	glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
	RenderScene(vr::Eye_Right, vrm);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId);

	glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

//-----------------------------------------------------------------------------
// Renders a scene with respect to nEye.
//-----------------------------------------------------------------------------
void Graphics::RenderScene(vr::Hmd_Eye nEye, std::unique_ptr<VR_Manager>& vrm)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);

	glm::mat4 currentProjMatrix = vrm->GetCurrentProjectionMatrix(nEye);
	glm::mat4 currentViewMatrix = vrm->GetCurrentViewMatrix(nEye);
	glm::mat4 currentEyeMatrix = vrm->GetCurrentEyeMatrix(nEye);
	glm::mat4 viewEyeMatrix = currentEyeMatrix * currentViewMatrix;
	glm::mat4 viewEyeProjMatrix = currentProjMatrix * currentEyeMatrix * currentViewMatrix;

	//update variables for fiveCell
	fiveCell.update(currentProjMatrix, viewEyeMatrix);

	//draw fiveCell scene
	fiveCell.draw(skyboxShaderProg, groundPlaneShaderProg, soundObjShaderProg, fiveCellShaderProg, currentProjMatrix, viewEyeMatrix);

	bool bIsInputAvailable = vrm->m_pHMD->IsInputAvailable();

	if (bIsInputAvailable)
	{
		// draw the controller axis lines
		glUseProgram(m_unControllerTransformProgramID);
		glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, &vrm->GetCurrentViewProjectionMatrix(nEye)[0][0]);
		glBindVertexArray(m_unControllerVAO);
		glDrawArrays(GL_LINES, 0, m_uiControllerVertCount);
		glBindVertexArray(0);
	}

	// ----- Render Model rendering -----
	glUseProgram(m_unRenderModelProgramID);

	// this for loop should use eHand iterators from VR_Manager
	for (int i = 0; i <= 1; i++)
	{
		if (!vrm->m_rHand[i].m_bShowController || !vrm->m_rHand[i].m_pRenderModel)
			continue;

		const glm::mat4& matDeviceToTracking = vrm->m_rHand[i].m_rmat4Pose;
		glm::mat4 matMVP = vrm->GetCurrentViewProjectionMatrix(nEye) * matDeviceToTracking;
		glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, &matMVP[0][0]);

		vrm->m_rHand[i].m_pRenderModel->Draw();
	}

	glUseProgram(0);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void Graphics::RenderCompanionWindow()
{
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight);

	glBindVertexArray(m_unCompanionWindowVAO);
	glUseProgram(m_unCompanionWindowProgramID);

	// render left eye (first half of index array )
	glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize, GL_UNSIGNED_SHORT, 0);
	// render right eye (second half of index array )
	//glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize));

	// Read pixels to memory to save as png **to do - change to C++14 style**
	if(m_bRecordScreen){
		GLubyte *pixels = (GLubyte*) malloc(4 * m_nCompanionWindowWidth * m_nCompanionWindowHeight);

		if(pixels){
			glReadPixels(0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		}
	
		//WriteToPNG(pixels);

		free(pixels);
		pixels = nullptr;
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

//----------------------------------------------------------------------
// Writes data to PNG file. 
//----------------------------------------------------------------------
void Graphics::WriteToPNG(GLubyte* &data){

	char* filename = (char*) malloc(32);
	snprintf(filename, sizeof(char) * 32, "stills3/image%04d.png", m_unImageCount);

	//FILE* imageFile;
	//imageFile = fopen(filename, "wb");
	//if(imageFile == NULL){
	//	std::cout << "ERROR: No image file: Graphics::WriteToPNG" << std::endl;
	//}

	// Encode png image
	unsigned error = lodepng_encode32_file(filename, data, m_nCompanionWindowWidth, m_nCompanionWindowHeight);
	if(error){
		std::cout << "ERROR: Image file not encoded by lodepng: Graphics::WriteToPNG" << std::endl;
	}

	m_unImageCount++;
	free(filename);
	filename = nullptr;
}

//----------------------------------------------------------------------
// Deletes render buffers and textures for each eye of the hmd.
// ---------------------------------------------------------------------
void Graphics::CleanUpGL(std::unique_ptr<VR_Manager>& vrm){

	if(m_pGLContext){

		if( m_bDebugOpenGL )
		{
			glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE );
			glDebugMessageCallback(nullptr, nullptr);
		}

		glDeleteBuffers(1, &m_glSceneVBO);

		if (m_glMainShaderProgramID)
		{
			glDeleteProgram(m_glMainShaderProgramID);
		}
		if ( m_unControllerTransformProgramID )
		{
			glDeleteProgram( m_unControllerTransformProgramID );
		}
		if ( m_unRenderModelProgramID )
		{
			glDeleteProgram( m_unRenderModelProgramID );
		}
		if ( m_unCompanionWindowProgramID )
		{
			glDeleteProgram( m_unCompanionWindowProgramID );
		}	

		glDeleteRenderbuffers( 1, &leftEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nResolveFramebufferId );

		glDeleteRenderbuffers( 1, &rightEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &rightEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &rightEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &rightEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &rightEyeDesc.m_nResolveFramebufferId );

		if( m_unCompanionWindowVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unCompanionWindowVAO );
		}
		if( m_unSceneVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unSceneVAO );
		}
		if( m_unControllerVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unControllerVAO );
		}

		glfwTerminate();

		fiveCell.exit();
	}
}

//-------------------------------------------------
// Temporary function to escape the main loop
// ------------------------------------------------
bool Graphics::TempEsc(){

	glfwPollEvents();

	if(GLFW_PRESS == glfwGetKey(m_pGLContext, GLFW_KEY_ESCAPE)){
		glfwSetWindowShouldClose(m_pGLContext, 1);
			return true;
	}

	return false;
}

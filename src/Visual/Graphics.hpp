#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <memory>
#include <string>
//#include <ctime>

#include "FiveCell.hpp"
#include "VR_Manager.hpp"
#include "Matrices.h"
#include "Vectors.h"
#include "openvr.h"

#ifdef __APPLE__ 
#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#elif _WIN32 
#include "GL/glew.h"
#include "glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

class Graphics{

public:

	Graphics(std::unique_ptr<ExecutionFlags>& flagPtr);
	bool BInitGL(std::unique_ptr<VR_Manager>& vrm, bool fullscreen = false);
	bool BCreateDefaultShaders();
	GLuint BCreateSceneShaders(std::string shaderName);
	GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader );
	bool BSetupStereoRenderTargets(std::unique_ptr<VR_Manager>& vrm);
	void CleanUpGL(std::unique_ptr<VR_Manager>& vrm);
	bool BSetupCompanionWindow(std::unique_ptr<VR_Manager>& vrm);
	void RenderFrame(std::unique_ptr<VR_Manager>& vrm);
	void RenderControllerAxes(std::unique_ptr<VR_Manager>& vrm);
	void RenderStereoTargets(std::unique_ptr<VR_Manager>& vrm);
	void RenderScene(vr::Hmd_Eye nEye, std::unique_ptr<VR_Manager>& vrm);
	void RenderCompanionWindow();
	void WriteToPNG(GLubyte* &data);
	bool TempEsc();
	void IncreaseRotationValue(std::unique_ptr<int>& pVal);

private:

	glm::vec4 m_vFarPlaneDimensions;

	struct FramebufferDesc
	{
		GLuint m_nDepthBufferId;
		GLuint m_nRenderTextureId;
		GLuint m_nRenderFramebufferId;
		GLuint m_nResolveTextureId;
		GLuint m_nResolveFramebufferId;
	};
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;

	bool BCreateFrameBuffer(FramebufferDesc& framebufferDesc);

	GLFWwindow* m_pGLContext; // TODO: convert this to unique_ptr<>	

	uint32_t m_nCompanionWindowWidth;
	uint32_t m_nCompanionWindowHeight;
	uint32_t windowWidth;
	uint32_t windowHeight;
	GLuint m_glMainShaderProgramID;
	GLuint m_gluiTetraShaderProgramID;
	
	struct VertexDataWindow
	{
		glm::vec2 position;
		glm::vec2 texCoord;

		VertexDataWindow( const glm::vec2& pos, const glm::vec2 tex ) :  position(pos), texCoord(tex) {	}
	};
	
	GLuint m_unCompanionWindowVAO;
	GLuint m_glCompanionWindowIDVertBuffer;
	GLuint m_glCompanionWindowIDIndexBuffer;
	unsigned int m_uiCompanionWindowIndexSize;

	unsigned int m_uiControllerVertCount;

	GLuint m_unControllerVAO;
	GLuint m_glControllerVertBuffer;
	//GLint m_nSceneMatrixLocation;
	glm::mat4 m_mat4HMDPose;
	GLuint m_unSceneVAO;
	GLuint m_glSceneVBO;
	GLuint m_gluiTetraVAO;
	GLuint m_gluiTetraVBO;
	GLuint m_gluiTetraIndexBuffer;
	unsigned int m_uiNumSceneVerts;
	unsigned int m_uiNumTetraVerts;
	unsigned int m_uiNumTetraIndices;
	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;
	
	GLuint m_unCompanionWindowProgramID;
	GLuint m_unControllerTransformProgramID;
	GLuint m_unRenderModelProgramID;

	GLint m_nControllerMatrixLocation;
	GLint m_nRenderModelMatrixLocation;

	bool m_bVblank;
	bool m_bGLFinishHack;

	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount_Last;
	std::string m_strPoseClasses;                            // what classes we saw poses for this frame

	bool m_bDebugOpenGL;
	bool m_bDebugPrintMessages;

	//GLint resolution; 
	GLint m_gliViewEyeProjLocation;
	GLint m_nViewMatrixLocation;
	GLint m_nViewEyeMatrixLocation;
	GLint m_nRenderWidthLocation;
	GLint m_nRenderHeightLocation;
	//GLint m_nZNearLocation;
	//GLint m_nZFarLocation;
	GLint m_nAspectLocation;
	GLint m_nTanFovLocation;
	GLint m_nProjectionMatrixLocation;
	GLint m_nEyeMatLocation;
	GLint m_nRotation3DLocation;
	GLint m_gliTimerLocation;

	unsigned int m_uiNumSceneIndices;
	GLuint m_glIndexBuffer;

	std::unique_ptr<int> m_pRotationVal;

	unsigned int m_unImageCount;	
	bool m_bRecordScreen;

	//time_t m_tStartTime;
	unsigned int m_uiFrameNumber;

	//shader handles
	GLuint skyboxShaderProg;
	GLuint soundObjShaderProg;
	GLuint groundPlaneShaderProg;
	GLuint fiveCellShaderProg;

	FiveCell fiveCell;
};
#endif

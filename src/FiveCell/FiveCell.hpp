#ifndef FIVE_CELL_HPP
#define FIVE_CELL_HPP

#include "Skybox.hpp"
#include "SoundObject.hpp"
#include "CsoundSession.hpp"

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

class FiveCell {

public:
	bool setup(std::string csd, GLuint skyboxProg, GLuint soundObjProg, GLuint groundPlaneProg, GLuint fiveCellProg);
	void update(glm::mat4 projMat, glm::mat4 viewMat);
	void draw(GLuint skyboxProg, GLuint groundPlaneProg, GLuint soundObjProg, GLuint fiveCellProg, glm::mat4 projMat, glm::mat4 viewEyeMat);
	void exit();

private:

	glm::vec3 cameraPos;
	glm::vec3 camPosPerEye;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	float deltaTime;
	float lastFrame;
	float currentFrame;
	bool needDraw;
	//float radius;

	MYFLT* hrtfVals[15];

	//ground plane
	GLuint groundVAO;
	GLuint groundIndexBuffer;

	GLint ground_projMatLoc;
	GLint ground_viewMatLoc;
	GLint ground_modelMatLoc;
	GLint ground_lightPosLoc;
	GLint ground_light2PosLoc;
	GLint ground_cameraPosLoc;
	
	//fivecell 
	glm::vec4 vertArray[5];
	GLuint vao;
	GLuint index;
	GLuint lineIndex;

	GLint projMatLoc;
	GLint viewMatLoc;
	GLint fiveCellModelMatLoc;
	GLint rotationZWLoc;
	GLint rotationXWLoc;
	GLint lightPosLoc;
	GLint light2PosLoc;
	GLint alphaLoc;	
	GLint cameraPosLoc;

	glm::mat4 rotationZW;
	glm::mat4 rotationXW;

	//matrices
	glm::mat4 modelMatrix;
	glm::mat4 scale5CellMatrix;
	glm::mat4 fiveCellModelMatrix;
	glm::mat4 groundModelMatrix;

	//lights
	glm::vec3 lightPos;
	glm::vec3 light2Pos;

	//SoundObjects
	SoundObject soundObjects [5];

	//Skybox
	//Skybox skybox;
	GLuint skyboxShaderProg;
	GLuint skyboxVAO;	
	GLuint skyboxTexID;
	GLuint skyboxIndexBuffer;

	GLint skybox_projMatLoc;
	GLint skybox_viewMatLoc;

	//Csound
	CsoundSession *session;
};
#endif

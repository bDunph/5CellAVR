#ifndef FIVE_CELL_HPP
#define FIVE_CELL_HPP

//#include "Skybox.hpp"

#include <string>

#include "SoundObject.hpp"
#include "CsoundSession.hpp"

class FiveCell {

public:
	bool setup(std::string csd, GLuint skyboxProg, GLuint soundObjProg, GLuint groundPlaneProg, GLuint fiveCellProg, GLuint quadShaderProg);
	void update(glm::mat4 projMat, glm::mat4 viewMat, glm::vec3 camFront);
	void draw(GLuint skyboxProg, GLuint groundPlaneProg, GLuint soundObjProg, GLuint fiveCellProg, GLuint quadShaderProg, glm::mat4 projMat, glm::mat4 viewMat, glm::mat4 eyeMat);
	void exit();

private:

	glm::vec4 cameraPos;
	glm::vec3 camPosPerEye;
	//glm::vec3 cameraFront;
	//glm::vec3 cameraUp;
	float deltaTime;
	float lastFrame;
	float currentFrame;
	//bool needDraw;
	//float radius;

	//ground plane
	//GLuint groundVAO;
	//GLuint groundIndexBuffer;

	//GLint ground_projMatLoc;
	//GLint ground_viewMatLoc;
	//GLint ground_modelMatLoc;
	//GLint ground_lightPosLoc;
	//GLint ground_light2PosLoc;
	//GLint ground_cameraPosLoc;
	
	//fivecell 
	glm::vec4 vertArray5Cell[5];
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
	//glm::mat4 groundModelMatrix;
	//glm::mat4 quadModelMatrix;
	glm::mat4 skyboxModelMatrix;

	//lights
	glm::vec3 lightPos;
	glm::vec3 light2Pos;

	//SoundObjects
	SoundObject soundObjects [5];

	//Skybox
	//Skybox skybox;
	//GLuint skyboxShaderProg;
	GLuint skyboxVAO;	
	unsigned int skyboxTexID;
	GLuint skyboxIndexBuffer;

	GLint skybox_projMatLoc;
	GLint skybox_viewMatLoc;
	GLint skybox_modelMatLoc;

	GLint skybox_texUniformLoc;

	//quad
	//GLuint quadVAO;
	//GLuint quadIndexBuffer;
	//unsigned int quadTexID;

	//GLint quad_projMatLoc;
	//GLint quad_viewMatLoc;
	//GLint quad_modelMatLoc;
	//GLint quad_lightPosLoc;
	//GLint quad_light2PosLoc;
	//GLint quad_cameraPosLoc;

	//Csound
	CsoundSession *session;
	MYFLT* hrtfVals[15];
};
#endif

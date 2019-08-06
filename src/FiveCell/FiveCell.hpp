#ifndef FIVE_CELL_HPP
#define FIVE_CELL_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class FiveCell {

public:
	bool setup();
	void update();
	void draw(GLuint skyboxProg, GLuint groundPlaneProg, GLuint soundObjProg, GLuint fiveCellProg);
	void exit();

private:

	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	float deltaTime;
	float lastFrame;
	bool needDraw;
	float radius;

	MYFLT* hrtfVals[15];

	//ground plane
	GLuint groundVAO;
	GLuint groundIndexBuffer;

	GLint ground_projMatLoc;
	GLint ground_viewMatLoc;
	GLint ground_modelLoc;
	GLint ground_lightPosLoc;
	GLint ground_light2PosLoc;
	
	//fivecell 
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

	//matrices
	glm::mat4 modelMatrix;
	glm::mat4 scale5CellMatrix;
	glm::mat4 fiveCellModelMatrix;

	//lights
	glm::vec3 lightPos;
	glm::vec3 light2Pos;
	
};
#endif

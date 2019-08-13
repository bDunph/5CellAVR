#include "FiveCell.hpp"

#include <vector>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <assert.h>
#include <math.h>
#include <cmath>
#include <iostream>

//#include "log.h"
//#include "shader_manager.h"
//#include "utils.h"

#define PI 3.14159265359

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool FiveCell::setup(std::string csd, GLuint skyboxProg, GLuint soundObjProg, GLuint groundPlaneProg, GLuint fiveCellProg){

//************************************************************
//Csound performance thread
//************************************************************
	std::string csdName = "";
	if(!csd.empty()) csdName = csd;
	session = new CsoundSession(csdName);
	for(int i = 0; i < 5; i++){
		std::string val1 = "azimuth" + std::to_string(i);
		const char* azimuth = val1.c_str();	
		if(session->GetChannelPtr(hrtfVals[3 * i], azimuth, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
			std::cout << "GetChannelPtr could not get the azimuth input" << std::endl;
			return false;
		}
		std::string val2 = "elevation" + std::to_string(i);
		const char* elevation = val2 .c_str();
		if(session->GetChannelPtr(hrtfVals[(3 * i) + 1], elevation, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
			std::cout << "GetChannelPtr could not get the elevation input" << std::endl;
			return false;
		}	
		std::string val3 = "distance" + std::to_string(i);
		const char* distance = val3.c_str();
		if(session->GetChannelPtr(hrtfVals[(3 * i) + 2], distance, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
			std::cout << "GetChannelPtr could not get the distance input" << std::endl;
			return false;
		}
	}
//************************************************************

	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//******************************************************************************************
// Matrices & Light Positions
//*******************************************************************************************
	
	//glm::mat4 projectionMatrix;
	//glm::mat4 viewMatrix;

	// projection matrix setup	
	//projectionMatrix = glm::perspective(45.0f, (float)g_gl_width / (float)g_gl_height, 0.1f, 1000.0f);

	// variables for view matrix
	//cameraPos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	//cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	//cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	//model matrix
	modelMatrix = glm::mat4(1.0f);

	lightPos = glm::vec3(-2.0f, -1.0f, -1.5f); 
	light2Pos = glm::vec3(1.0f, 40.0f, 1.5f);
//****************************************************************************************************

//***************************************************************************************************
// Skybox
//**************************************************************************************************

	//bool result = skybox.setup();
	//if(!result){
	//	std::cout << "ERROR: Skybox init failed" << std::endl;
	//	return false;
	//}
	
	//Skybox vertices
	float skyboxVerts [24] = {
		//top left front	
		-1.0f, 1.0f, 1.0f,
		//bottom left front
		-1.0f, -1.0f, 1.0f,
		//bottom right front
		1.0f, -1.0f, 1.0f,
		//top right front
		1.0f, 1.0f, 1.0f,
		//top left back
		-1.0f, 1.0f, -1.0f,
		//bottom left back
		-1.0f, -1.0f, -1.0f,
		//bottom right back
		1.0f, -1.0f, -1.0f,
		//top right back
		1.0f, 1.0f, -1.0f
	};

	unsigned int skyboxIndices [36] = {
		//front face
		0, 3, 2,
		0, 2, 1,
		//right face
		3, 7, 6,
		3, 6, 2,
		//back face
		7, 4, 5,
		7, 5, 6,
		//left face
		4, 0, 1,
		4, 1, 5,
		//bottom face
		1, 2, 6,
		1, 6, 5,
		//top face
		4, 7, 3,
		4, 3, 0
	};

	float skyboxNormals [24] = {
		//top front left
		-1.0f, 1.0f, 1.0f,
		//bottom front left
		-1.0f, -1.0f, 1.0f,
		//bottom front right
		1.0f, -1.0f, 1.0f,
		//top front right
		1.0f, 1.0f, 1.0f,
		//top left back
		-1.0f, 1.0f, -1.0f,
		//bottom left back
		-1.0f, -1.0f, -1.0f,
		//bottom right back
		1.0f, -1.0f, -1.0f,
		//top right back
		1.0f, 1.0f, -1.0f
	};	

	//Set up skybox buffers
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	GLuint skyboxVBO;
	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), skyboxVerts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint skyboxNormalVBO;
	glGenBuffers(1, &skyboxNormalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), skyboxNormals, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//setup texture buffer
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &skyboxTexID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexID);

	//load textures
	std::vector<std::string> textureNames;
	textureNames.push_back("misty_ft.jpg");
	textureNames.push_back("misty_bk.jpg");
	textureNames.push_back("misty_up.jpg");
	textureNames.push_back("misty_dn.jpg");
	textureNames.push_back("misty_rt.jpg");
	textureNames.push_back("misty_lf.jpg");

	//std::string name1 = texName.append("_rt.tga");
	//textureNames.push_back(name1);
	//std::string name2 = texName.append("_lf.tga");
	//textureNames.push_back(name2);
	//std::string name3 = texName.append("_up.tga");
	//textureNames.push_back(name3);
	//std::string name4 = texName.append("_dn.tga");
	//textureNames.push_back(name4);
	//std::string name5 = texName.append("_ft.tga");
	//textureNames.push_back(name5);
	//std::string name6 = texName.append("_bk.tga");
	//textureNames.push_back(name6);	

	int width, height, numChannels;
	unsigned char* data;
	for(GLuint i = 0; i < textureNames.size(); i++){
		std::cout << textureNames[i] << std::endl;
		data = stbi_load(textureNames[i].c_str(), &width, &height, &numChannels, 0);
		if(data){
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			
			stbi_image_free(data);
		} else {
			std::cout << "ERROR: Cubemap not loaded" << std::endl;
			stbi_image_free(data);	
			return false;	
		}
	}			
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  	

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glGenBuffers(1, &skyboxIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), skyboxIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//uniform setup
	skybox_projMatLoc = glGetUniformLocation(skyboxShaderProg, "projMat");
	skybox_viewMatLoc = glGetUniformLocation(skyboxShaderProg, "viewMat");
	skybox_texUniformLoc = glGetUniformLocation(skyboxShaderProg, "skybox");

	glBindVertexArray(0);

//*************************************************************************************************

//***************************************************************************************************
// SoundObject 
//**************************************************************************************************
	for(int i = 0; i < _countof(soundObjects); i++){

		if(!soundObjects[i].setup()){
			std::cout << "ERROR: SoundObject " << std::to_string(i) << " init failed" << std::endl;
			return false;
		}
	}
	
//*************************************************************************************************
			
//**************************************************************************************************
//	Ground Plane Setup
//*********************************************************************************************
	// Ground plane vertices
	float groundVerts [12] = {
		1.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, -1.0f
	};

	unsigned int groundIndices [6] = {
		0, 1, 3,
		3, 1, 2
	};

	//Set up ground plane buffers
	glGenVertexArrays(1, &groundVAO);
	glBindVertexArray(groundVAO);

	GLuint groundVBO;
	glGenBuffers(1, &groundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), groundVerts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &groundIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), groundIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	//uniform setup
	ground_projMatLoc = glGetUniformLocation(groundPlaneProg, "projMat");
	ground_viewMatLoc = glGetUniformLocation(groundPlaneProg, "viewMat");
	ground_modelMatLoc = glGetUniformLocation(groundPlaneProg, "groundModelMat");

	ground_lightPosLoc = glGetUniformLocation(groundPlaneProg, "lightPos");
	ground_light2PosLoc = glGetUniformLocation(groundPlaneProg, "light2Pos");

	ground_cameraPosLoc = glGetUniformLocation(groundPlaneProg, "camPos");
	
	glBindVertexArray(0);

	groundModelMatrix = modelMatrix;
//***************************************************************************************************		
		

//*************************************************************************************************
// 5cell Polytope Setup
//*************************************************************************************************
	/* specify 4D coordinates of 5-cell from https://en.wikipedia.org/wiki/5-cell */
	/*float vertices [20] = {
		1.0f/sqrt(10.0f), 1.0f/sqrt(6.0f), 1.0f/sqrt(3.0f), 1.0f,
		1.0f/sqrt(10.0f), 1.0f/sqrt(6.0f), 1.0f/sqrt(3.0f), -1.0f,
		1.0f/sqrt(10.0f), 1.0f/sqrt(6.0f), -2.0f/sqrt(3.0f), 0.0f,
		1.0f/sqrt(10.0f), -sqrt(3.0f/2.0f), 0.0f, 0.0f,
		-2.0f * sqrt(2.0f/5.0f), 0.0f, 0.0f, 0.0f
	};*/
	float vertices [20] = {
		0.3162f, 0.4082f, 0.5774f, 1.0f,
		0.3162f, 0.4082f, 0.5774f, -1.0f,
		0.3162f, 0.4082f, -1.1547, 0.0f,
		0.3162f, -1.2247f, 0.0f, 0.0f,
		-1.2649f, 0.0f, 0.0f, 0.0f
	};

	/* indices specifying 10 faces */
	unsigned int indices [30] = {
		4, 2, 3,
		3, 0, 2,
		2, 0, 4,
		4, 0, 3,
		3, 1, 0,
		1, 4, 0,
		0, 1, 2,
		2, 3, 1,
		1, 3, 4,
		4, 2, 1	
	};

	//alt indices
	//unsigned int indices [30] = {
	//	0, 2, 1,
	//	0, 3, 2,
	//	0, 4, 3,
	//	0, 2, 4,
	//	2, 3, 4,
	//	3, 1, 4,
	//	4, 1, 2,
	//	2, 1, 3,
	//	3, 1, 0,
	//	0, 1, 4
	//};
	
	unsigned int lineIndices [20] = {
		4, 2,
		2, 3,
		3, 4,
		4, 0,
		0, 2, 
		0, 3,
		3, 1,
		1, 2,
		4, 1,
		1, 0
	};

	//array of verts
	glm::vec4 vertArray [5] = {
		glm::vec4(vertices[0], vertices[1], vertices[2], vertices[3]),
		glm::vec4(vertices[4], vertices[5], vertices[6], vertices[7]),
		glm::vec4(vertices[8], vertices[9], vertices[10], vertices[11]),
		glm::vec4(vertices[12], vertices[13], vertices[14], vertices[15]),
		glm::vec4(vertices[16], vertices[17], vertices[18], vertices[19])
	};

	//array of faces
	glm::vec3 faceArray [10] = {
		glm::vec3(indices[0], indices[1], indices[2]),
		glm::vec3(indices[3], indices[4], indices[5]),
		glm::vec3(indices[6], indices[7], indices[8]),
		glm::vec3(indices[9], indices[10], indices[11]),
		glm::vec3(indices[12], indices[13], indices[14]),
		glm::vec3(indices[15], indices[16], indices[17]),
		glm::vec3(indices[18], indices[19], indices[20]),
		glm::vec3(indices[21], indices[22], indices[23]),
		glm::vec3(indices[24], indices[25], indices[26]),
		glm::vec3(indices[27], indices[28], indices[29]),
	};

	glm::vec4 faceNormalArray [10];
	
	//calculate vertex normals in 4D to send to shaders for lighting
	for(int i = 0; i < _countof(faceArray); i++){
		//calculate three linearly independent vectors for each face
		unsigned int indexA = faceArray[i].x;
		unsigned int indexB = faceArray[i].y;
		unsigned int indexC = faceArray[i].z;

		glm::vec4 vertA = vertArray[indexA];
		glm::vec4 vertB = vertArray[indexB];
		glm::vec4 vertC = vertArray[indexC];

		glm::vec4 vectorA = glm::vec4(vertB.x - vertA.x, vertB.y - vertA.y, vertB.z - vertA.z, vertB.w - vertA.w);
		glm::vec4 vectorB = glm::vec4(vertC.x - vertB.x, vertC.y - vertB.y, vertC.z - vertB.z, vertC.w - vertB.w);
		glm::vec4 vectorC = glm::vec4(vertA.x - vertC.x, vertA.y - vertC.y, vertA.z - vertC.z, vertA.w - vertC.w);

		//calculate orthonormal basis for vectorA, B and C using Gram-Schmidt. We can then calculte
		//the 4D normal
		glm::vec4 u1 = glm::normalize(vectorA);
		
		glm::vec4 y2 = vectorB - ((glm::dot(vectorB, u1)) * u1);
		glm::vec4 u2 = glm::normalize(y2);

		glm::vec4 y3 = vectorC - ((glm::dot(vectorC, u2)) * u2);
		glm::vec4 u3 = glm::normalize(y3);
		
		//calculate the  normal for each face
	 	//using matrices and  Laplace expansion we can find the normal 
		//vector in 4D given three input vectors	
		//this procedure is following the article at https://ef.gy/linear-algebra:normal-vectors-in-higher-dimensional-spaces 
		/* a x b x c = 	| a0 b0 c0 right|
				| a1 b1 c1 up	|
				| a2 b2 c2 back	|	
				| a3 b3 c3 charm|*/
		glm::vec4 right = glm::vec4(1.0, 0.0, 0.0, 0.0);	
		glm::vec4 up = glm::vec4(0.0, 1.0, 0.0, 0.0);	
		glm::vec4 back = glm::vec4(0.0, 0.0, 1.0, 0.0);	
		glm::vec4 charm = glm::vec4(0.0, 0.0, 0.0, 1.0);	

		glm::mat3 matA = glm::mat3(	u1.y, u2.y, u3.y,
						u1.z, u2.z, u3.z,
						u1.w, u2.w, u3.w);

		glm::mat3 matB = glm::mat3(	u1.x, u2.x, u3.x,
						u1.z, u2.z, u3.z,
						u1.w, u2.w, u3.w);

		glm::mat3 matC = glm::mat3(	u1.x, u2.x, u3.x,
						u1.y, u2.y, u3.y,
						u1.w, u2.w, u3.w);
	
		glm::mat3 matD = glm::mat3(	u1.x, u2.x, u3.x,
						u1.y, u2.y, u3.y,
						u1.z, u2.z, u3.z);	

		float determinantA = glm::determinant(matA);	
		float determinantB = glm::determinant(matB);	
		float determinantC = glm::determinant(matC);	
		float determinantD = glm::determinant(matD);	

		glm::vec4 termA = (determinantA * right) * -1.0f;
		glm::vec4 termB = determinantB * up;
		glm::vec4 termC = (determinantC * back) * -1.0f;
		glm::vec4 termD = determinantD * charm;

		glm::vec4 faceNormal = termA + termB + termC + termD;
		faceNormalArray[i] += faceNormal;
	}
	
	float vertexNormalArray [20];

	//calculate the normal for each vertex by taking the average of the normals of each adjacent face
	for(int i = 0; i < _countof(vertArray); i++){
		glm::vec4 cumulativeNormals = glm::vec4(0.0);
		for(int j = 0; j < _countof(faceArray); j++){
				
			//does this face [j] contain vert [i]?
			unsigned int vertA = faceArray[j].x;				 
			unsigned int vertB = faceArray[j].y;
			unsigned int vertC = faceArray[j].z;
			if(vertA == i || vertB == i || vertC == i){
				cumulativeNormals += faceNormalArray[j];
			}	
		}
		glm::vec4 vertexNormal = glm::normalize(cumulativeNormals);
		vertexNormalArray[i*4] += vertexNormal.x;
		vertexNormalArray[i*4+1] += vertexNormal.y;
		vertexNormalArray[i*4+2] += vertexNormal.z;
		vertexNormalArray[i*4+3] += vertexNormal.w;
	}
		
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	GLuint vertNormals;
	glGenBuffers(1, &vertNormals);
	glBindBuffer(GL_ARRAY_BUFFER, vertNormals);
	glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), vertexNormalArray, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 30 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//glGenBuffers(1, &lineIndex);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineIndex);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 20 * sizeof(unsigned int), lineIndices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//uniforms for 4D shape
	projMatLoc = glGetUniformLocation(fiveCellProg, "projMat");
	viewMatLoc = glGetUniformLocation(fiveCellProg, "viewMat");
	fiveCellModelMatLoc = glGetUniformLocation(fiveCellProg, "fiveCellModelMat");
	rotationZWLoc = glGetUniformLocation(fiveCellProg, "rotZW");
	rotationXWLoc = glGetUniformLocation(fiveCellProg, "rotXW");

	lightPosLoc = glGetUniformLocation(fiveCellProg, "lightPos");
	light2PosLoc = glGetUniformLocation(fiveCellProg, "light2Pos");

	cameraPosLoc = glGetUniformLocation(fiveCellProg, "camPos");

	alphaLoc = glGetUniformLocation(fiveCellProg, "alpha");

	glBindVertexArray(0);

	fiveCellModelMatrix = glm::mat4(1.0);

	glm::vec3 scale5Cell = glm::vec3(20.0f, 20.0f, 20.0f);
	scale5CellMatrix = glm::scale(modelMatrix, scale5Cell);
	
//***********************************************************************************************************

	//workaround for macOS Mojave bug
	//needDraw = true;

	//radius = 0.75f;
	
	return true;
}

void FiveCell::update(glm::mat4 projMat, glm::mat4 viewMat){

//***********************************************************************************************************
// Update Stuff Here
//*********************************************************************************************************
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		
		//_update_fps_counter(window);
		
		//glClearColor(0.87, 0.85, 0.75, 0.95);
		//wipe the drawing surface clear
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//update camera position
		cameraPos = glm::vec3(viewMat[3][0], viewMat[3][1], viewMat[3][2]);

		//float rotVal = glm::radians(45.0f);
		//rotation around W axis
		rotationZW = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, cos(glfwGetTime() * 0.2f), -sin(glfwGetTime() * 0.2f),
			0.0f, 0.0f, sin(glfwGetTime() * 0.2f), cos(glfwGetTime() * 0.2f)
		);
	
		rotationXW = glm::mat4(	
			cos(glfwGetTime() * 0.2f), 0.0f, 0.0f, sin(glfwGetTime() * 0.2f),
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 
			-sin(glfwGetTime() * 0.2f), 0.0f, 0.0f, cos(glfwGetTime() * 0.2f) 
		);

		//coords of verts to use for hrtf calculations 
		glm::vec3 projectedVerts [5];
		float projectionDistance = 2.0f;
		for(int i = 0; i < _countof(vertArray); i++){

			glm::vec4 rotatedVert = rotationZW * vertArray[i];

			float projectedPointX = (rotatedVert.x * projectionDistance) / (projectionDistance - rotatedVert.w); 	
			float projectedPointY = (rotatedVert.y * projectionDistance) / (projectionDistance - rotatedVert.w); 	
			float projectedPointZ = (rotatedVert.z * projectionDistance) / (projectionDistance - rotatedVert.w); 	
 	
			projectedVerts[i] = glm::vec3(projectedPointX, projectedPointY, projectedPointZ);
			
			glm::vec4 pos = projMat * viewMat * fiveCellModelMatrix * glm::vec4(projectedVerts[i], 1.0);

			//calculate azimuth and elevation values for hrtf
		
			glm::vec3 viewerPos = cameraPos;
			glm::vec3 soundPos = glm::vec3(pos); 
	
			//distance
			float r = sqrt((pow((soundPos.x - viewerPos.x), 2)) + (pow((soundPos.y - viewerPos.y), 2)) + (pow((soundPos.z - viewerPos.z), 2)));
			//std::cout << r << std::endl;	
	
			//azimuth
			float valX = soundPos.x - viewerPos.x;
			float valZ = soundPos.z - viewerPos.z;
			float azimuth = atan2(valX, valZ);
			azimuth *= (180.0f/PI); 	
			//std::cout << azimuth << std::endl;
	
			//elevation
			float cosVal = (soundPos.y - viewerPos.y) / r;
			float elevation = asin(cosVal);
			elevation *= (180.0f/PI);		
			//std::cout << elevation<< std::endl;

			*hrtfVals[3 * i] = (MYFLT)azimuth;
			*hrtfVals[(3 * i) + 1] = (MYFLT)elevation;
			*hrtfVals[(3 * i) + 2] = (MYFLT)r;
			
			//update sound object position
			soundObjects[i].update(projectedVerts[i]);	
		}

		float rotAngle = glfwGetTime() * 0.2f;
		glm::mat4 fiveCellRotationMatrix3D = glm::rotate(modelMatrix, rotAngle, glm::vec3(0, 1, 0)) ;
		fiveCellModelMatrix = scale5CellMatrix;
		
}

void FiveCell::draw(GLuint skyboxProg, GLuint groundPlaneProg, GLuint soundObjProg, GLuint fiveCellProg, glm::mat4 projMat, glm::mat4 viewEyeMat){
		
//**********************************************************************************************************
// Draw Stuff Here
//*********************************************************************************************************

		//draw skybox
		//skybox.draw(projMat, viewEyeMat, skyboxProg);
		glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(viewEyeMat));

		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndexBuffer); 
		glUseProgram(skyboxProg);
		glUniform1i(skybox_texUniformLoc, 0);
		glUniformMatrix4fv(skybox_projMatLoc, 1, GL_FALSE, &projMat[0][0]);
		glUniformMatrix4fv(skybox_viewMatLoc, 1, GL_FALSE, &viewNoTranslation[0][0]);
		glDrawElements(GL_TRIANGLES, 36 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);

		glDepthFunc(GL_LESS);
		
		//draw 4D polytope	
		//float a = 0.0f;
		//camPosPerEye = glm::vec3(viewEyeMat[3][0], viewEyeMat[3][1], viewEyeMat[3][2]);

		//glBindVertexArray(vao);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
		//glUseProgram(fiveCellProg);

		//glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, &projMat[0][0]);
		//glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, &viewEyeMat[0][0]);
		//glUniformMatrix4fv(fiveCellModelMatLoc, 1, GL_FALSE, &fiveCellModelMatrix[0][0]);
      		//glUniformMatrix4fv(rotationZWLoc, 1, GL_FALSE, &rotationZW[0][0]);
		//glUniformMatrix4fv(rotationXWLoc, 1, GL_FALSE, &rotationXW[0][0]);
		//glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		//glUniform3f(light2PosLoc, light2Pos.x, light2Pos.y, light2Pos.z);
		//glUniform3f(cameraPosLoc, camPosPerEye.x, camPosPerEye.y, camPosPerEye.z);
		//glUniform1f(alphaLoc, a);

		////single draw call for refractive rendering
		////glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
      		////glDrawElements(GL_LINES, 20 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

      		////draw 5-cell using index buffer and 5 pass transparency technique from http://www.alecjacobson.com/weblog/?p=2750
		////1st pass
		//glDisable(GL_CULL_FACE);
		//glDepthFunc(GL_LESS);
		//float f = 0.75f;
		//float origAlpha = 0.4f;	
		//a = 0.0f;
		//glUniform1f(alphaLoc, a);
		//glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

		////2nd pass
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		//glDepthFunc(GL_ALWAYS);
		//a = origAlpha * f;
		//glUniform1f(alphaLoc, a);
		//glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
		//
		////3rd pass
		//glDepthFunc(GL_LEQUAL);
		//a = (origAlpha - (origAlpha * f)) / (1.0f - (origAlpha * f));
		//glUniform1f(alphaLoc, a);
		//glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

		////4th pass
		//glCullFace(GL_BACK);
		//glDepthFunc(GL_ALWAYS);
		//a = origAlpha * f;
		//glUniform1f(alphaLoc, a);
		//glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

		////5th pass
		//glDisable(GL_CULL_FACE);
		//glDepthFunc(GL_LEQUAL);
		//a = (origAlpha - (origAlpha * f)) / (1.0f - (origAlpha * f));
		//glUniform1f(alphaLoc, a);
		//glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		//glBindVertexArray(0);

		// draw ground plane second 
		//glDepthFunc(GL_LESS);

		//glBindVertexArray(groundVAO);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundIndexBuffer); 
		//glUseProgram(groundPlaneProg);

		//glUniformMatrix4fv(ground_projMatLoc, 1, GL_FALSE, &projMat[0][0]);
		//glUniformMatrix4fv(ground_viewMatLoc, 1, GL_FALSE, &viewEyeMat[0][0]);
		//glUniformMatrix4fv(ground_modelMatLoc, 1, GL_FALSE, &groundModelMatrix[0][0]);
		//glUniform3f(ground_lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		//glUniform3f(ground_light2PosLoc, light2Pos.x, light2Pos.y, light2Pos.z);
		//glUniform3f(ground_cameraPosLoc, camPosPerEye.x, camPosPerEye.y, camPosPerEye.z);

		//glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		//glBindVertexArray(0);

		//draw sound test objects
		//for(int i = 0; i < _countof(soundObjects); i++){
		//	soundObjects[i].draw(projMat, viewEyeMat, lightPos, light2Pos, camPosPerEye, soundObjProg);
		//}	

		//update other events like input handling
		//glfwPollEvents();

		// workaround for macOS Mojave bug
		//if(needDraw){
		//	glfwShowWindow(window);
		//	glfwHideWindow(window);
		//	glfwShowWindow(window);
		//	needDraw = false;
		//}

		//put the stuff we've been drawing onto the display
		//glfwSwapBuffers(window);

		//glfwSetCursorPosCallback(window, mouse_callback);
		//glfwSetWindowSizeCallback(window, glfw_window_size_callback);
		//glfwSetErrorCallback(glfw_error_callback);
		
		lastFrame = currentFrame;
}

void FiveCell::exit(){
	//stop csound
	session->StopPerformance();
	//close GL context and any other GL resources
	glfwTerminate();
}

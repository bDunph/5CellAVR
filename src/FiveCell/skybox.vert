#version 410

layout (location = 0) in vec3 position;

uniform mat4 projMat;
uniform mat4 viewMat;

out vec3 TexCoords;

void main(){

	TexCoords = position;
	vec4 projectedPos = projMat * viewMat * vec4(position, 1.0);
	//optimisation: setting vert z value to w so depth will always be 1.0
	//therefore making sure skybox will always be behind objects
	gl_Position = projectedPos.xyww;
}

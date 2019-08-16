#version 410


uniform samplerCube skybox;

in vec3 texCoords;
out vec4 fragColour;

void main(){

	fragColour = texture(skybox, texCoords);
}

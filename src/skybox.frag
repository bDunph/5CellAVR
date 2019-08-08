#version 410

in vec3 TexCoords;

uniform samplerCube skybox;

out vec4 fragColour;

void main(){

	fragColour = texture(skybox, TexCoords);
}

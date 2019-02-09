// Dynamic Enviroment Map fragment shader with additional Multi-Texturing functionality dynamicMultiTexEnvMap.frag matched with dynamicEnvMap.vert
#version 330

// Some drivers require the following
precision highp float;

uniform sampler2D textureUnit0;
uniform samplerCube textureUnit1;

in vec3 ex_WorldNorm;
in vec3 ex_WorldView;

in vec2 ex_TexCoord;
smooth in vec3 ex_CubeTexCoord;

layout(location = 0) out vec4 outColour;

void main(void) 
{
	vec3 reflectTexCoord = reflect(ex_WorldView, normalize(ex_WorldNorm)); //defines the coordinates of our reflection.
	
	outColour = texture(textureUnit0, ex_TexCoord) * texture(textureUnit1,  -reflectTexCoord); //Multitextured reflective output.
	outColour.a = 1.0;

	//outColour = vec4(reflectTexCoord.rgb, 1.0); //Debug functionality.
}
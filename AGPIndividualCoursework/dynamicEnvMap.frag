// Dynamic Enviroment Map fragment shader dynamicEnvMap.frag matched with dynamicEnvMap.vert
#version 330

// Some drivers require the following
precision highp float;

uniform sampler2D textureUnit0;		//Allows for multitextured output.
uniform samplerCube textureUnit1;	//Our dynamic cubemap.

in vec3 ex_WorldNorm;
in vec3 ex_WorldView;

in vec2 ex_TexCoord;
smooth in vec3 ex_CubeTexCoord;

layout(location = 0) out vec4 outColour;

void main(void) 
{
	vec3 reflectTexCoord = reflect(ex_WorldView, normalize(ex_WorldNorm)); //defines the coordinates of our reflection.
	
	outColour = texture(textureUnit1,  -reflectTexCoord); //Reflective output.
	outColour.a = 1.0;

	//outColour = vec4(reflectTexCoord.rgb, 1.0); //Debug functionality.
}
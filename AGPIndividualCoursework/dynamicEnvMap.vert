// dynamicEnvMap.vert
// Vertex shader for use in dynamic enviroment mapping...
#version 330

uniform mat4 modelview;
uniform mat4 projection;
uniform mat4 modelMatrix;
uniform vec3 cameraPos;

in  vec3 in_Position;
in  vec3 in_Normal;
out vec3 ex_WorldView;
out vec3 ex_WorldNorm;

in vec2 in_TexCoord;
out vec2 ex_TexCoord;
smooth out vec3 ex_CubeTexCoord;

void main(void) 
{
	vec4 vertexPosition = modelview * vec4(in_Position,1.0);
	gl_Position = projection * vertexPosition;

	vec3 worldPos = (modelMatrix * vec4(in_Position,1.0)).xyz;
	ex_WorldView = (cameraPos - worldPos);
	
	mat3 normalWorldMatrix=transpose(inverse(mat3(modelMatrix)));
	ex_WorldNorm = normalWorldMatrix * in_Normal;
	
	ex_TexCoord = in_TexCoord;
	ex_CubeTexCoord = normalize(in_Position); //shows the fbos
}
// cellShader.vert
// modified AGP toon shader for use in drawing our non-env mapped objects.
#version 330

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 lightPosition;

in  vec3 in_Position;
in  vec3 in_Normal;
out vec3 ex_N;
out vec3 ex_V;
out vec3 ex_L;

out float ex_D;

void main(void) 
{
	// vertex in eye coordinates...
	vec4 vertexPosition = modelview * vec4(in_Position,1.0);
	ex_D = distance(vertexPosition,lightPosition);
	ex_V = normalize(-vertexPosition).xyz;

	// surface normal in eye coordinates...
	mat3 normalmatrix = transpose(inverse(mat3(modelview)));
	ex_N = normalize(normalmatrix * in_Normal);

	//To light source from vertex...
	ex_L = normalize(lightPosition.xyz - vertexPosition.xyz);

    gl_Position = projection * vertexPosition;
}
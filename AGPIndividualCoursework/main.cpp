//========================================================================//
// AGP Individual Coursework Assignment - Implementation                  //
// "Dynamic Enviroment Mapping Using A FBO Generated Dynamic Cubemap"     //
// Adam Boyd Stanton - B00266256                                          //
//========================================================================//

#include "window.h"
#include "bitmap.h"
#include "texture.h"
#include "cubemap.h"
#include "move.h"
#include "rt3d.h"
#include "rt3dObjLoader.h"
#include "meshObject.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stack>
#include <vector>
#include <math.h>
using namespace std;

#define DEG_TO_RADIAN 0.017453293


//========================================================================//
//Globals:

//Shader Programs...
GLuint shaderProgram;
GLuint skyboxProgram;
GLuint envmapProgram;

//FBO Globals...
GLuint framebuffer;
GLuint depthRenderbuffer;
GLuint screenWidth = 800;
GLuint screenHeight = 600;
static const GLenum fboAttachments[] = { GL_COLOR_ATTACHMENT0 };
static const GLenum frameBuff[] = { GL_BACK_LEFT };

//Dynamic Cubemap...
GLuint dynamicCubeMapID;
GLuint skyboxCubeMapID;
const int cubeMapSize = 1024;
GLenum sides[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
					GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X,
					GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
					GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
					GL_TEXTURE_CUBE_MAP_NEGATIVE_Y };

//Camera Globals...
glm::vec3 eye(0.0f, 0.0f, 10.0f);
glm::vec3 at(0.0f, 4.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

//Textures...
Texture *concreteTex;
Texture *metalTex;
Texture *fabricTex;
Texture *currentTex;

//Meshes...
MeshObject *cube;
MeshObject *rabbit;
MeshObject *statue;
MeshObject *currentMesh;

//Scene Globals...
glm::vec3 envMapCubePos = glm::vec3(0.0f, 0.0f, 0.0f);
GLfloat r = 0.0f;
stack<glm::mat4> mvStack;

GLfloat sceneRadius = 8.0f; //The radius at which the cubes orbit the center of the scene.
GLfloat sceneRotation = 0.0f; //The position on the circle on which the cubes lie in the scene.

//Light Globals...
rt3d::lightStruct light0 = {
	{ 0.4f, 0.4f, 0.4f, 1.0f }, // ambient
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // diffuse
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // specular
	{ 0.0f, 0.0f, 0.0f, 1.0f }  // position
};
glm::vec4 lightPos(10.0f, 0.0f, 0.0f, 1.0f); //light position

//Material Globals...
//Different materials for different colours on each scene cube. (Will allow us to be sure our reflection is working)
//Cyan:
rt3d::materialStruct material0 = {
	{ 1.0f, 1.0f, 0.0f, 1.0f }, // ambient
	{ 0.4f, 0.4f, 0.4f, 1.0f }, // diffuse
	{ 0.2f, 0.2f, 0.2f, 1.0f }, // specular
	2.0f  // shininess
};
//Magenta:
rt3d::materialStruct material1 = {
	{ 1.0f, 0.0f, 1.0f, 1.0f }, // ambient
	{ 0.4f, 0.4f, 0.4f, 1.0f }, // diffuse
	{ 0.2f, 0.2f, 0.2f, 1.0f }, // specular
	2.0f  // shininess
};
//Yellow:
rt3d::materialStruct material2 = {
	{ 0.0f, 1.0f, 1.0f, 1.0f }, // ambient
	{ 0.4f, 0.4f, 0.4f, 1.0f }, // diffuse
	{ 0.2f, 0.2f, 0.2f, 1.0f }, // specular
	2.0f  // shininess
};

//Other Values...
float scaleMod = 1.0f;			//Used to properly rescale different meshes when switching in-scene.
float translateOffset = 0.0f;	//Used to properly position different meshes when switching in-scene.
float theta = 0.0f;				//Used in rotating the scene-cubes.

//========================================================================//
//Methods:

//Used to provide user interaction...
void update(void) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	//Camera Movement & Rotation...
	if (keys[SDL_SCANCODE_W]) eye = move::forward(eye, r, 0.1f);	//Forwards.
	if (keys[SDL_SCANCODE_S]) eye = move::forward(eye, r, -0.1f);	//Backwards.
	if (keys[SDL_SCANCODE_A]) eye = move::right(eye, r, -0.1f);		//Left.
	if (keys[SDL_SCANCODE_D]) eye = move::right(eye, r, 0.1f);		//Right.
	if (keys[SDL_SCANCODE_R]) eye.y += 0.1;							//Up.
	if (keys[SDL_SCANCODE_F]) eye.y -= 0.1;							//Down.
	if (keys[SDL_SCANCODE_COMMA]) r -= 1.0f;	//Rotate Left.
	if (keys[SDL_SCANCODE_PERIOD]) r += 1.0f;	//Rotate Right.

	//Enviroment Mapped Mesh Movement...
	if (keys[SDL_SCANCODE_KP_8]) envMapCubePos = move::forward(envMapCubePos, r, 0.1f);		//Forwards.
	if (keys[SDL_SCANCODE_KP_5]) envMapCubePos = move::forward(envMapCubePos, r, -0.1f);	//Backwards.
	if (keys[SDL_SCANCODE_KP_4]) envMapCubePos = move::right(envMapCubePos, r, -0.1f);		//Left.
	if (keys[SDL_SCANCODE_KP_6]) envMapCubePos = move::right(envMapCubePos, r, 0.1f);		//Right
	if (keys[SDL_SCANCODE_KP_7]) envMapCubePos.y += 0.1;									//Up.
	if (keys[SDL_SCANCODE_KP_9]) envMapCubePos.y -= 0.1;									//Down.

	//Light Movement...
	if (keys[SDL_SCANCODE_I]) lightPos = glm::vec4(move::forward(glm::vec3(lightPos), r, +0.1f), lightPos.w);	//Forwards.
	if (keys[SDL_SCANCODE_K]) lightPos = glm::vec4(move::forward(glm::vec3(lightPos), r, -0.1f), lightPos.w);	//Backwards.
	if (keys[SDL_SCANCODE_J]) lightPos = glm::vec4(move::right(glm::vec3(lightPos), r, -0.1f), lightPos.w);		//Left.
	if (keys[SDL_SCANCODE_L]) lightPos = glm::vec4(move::right(glm::vec3(lightPos), r, 0.1f), lightPos.w);		//Right.
	if (keys[SDL_SCANCODE_U]) lightPos[1] += 0.1;																//Up.
	if (keys[SDL_SCANCODE_H]) lightPos[1] -= 0.1;																//Down.

	//DrawScene Cubes Movement & Rotation...
	if (keys[SDL_SCANCODE_Z]) sceneRadius += 0.1;	//Increase cube orbit range.
	if (keys[SDL_SCANCODE_X]) sceneRadius -= 0.1;	//Decrease cube orbit range.
	if (keys[SDL_SCANCODE_C]) sceneRotation += 0.5;	//Rotate to the right.
	if (keys[SDL_SCANCODE_V]) sceneRotation -= 0.5;	//Rotate to the left.

	//Change The Mesh Of The Env Mapped Object...
	if (keys[SDL_SCANCODE_1]) { currentMesh = cube; scaleMod = 1.0f; translateOffset = 0.0f; }		//Cube Mesh. (Reset Scale/Position Modifiers).
	if (keys[SDL_SCANCODE_2]) { currentMesh = rabbit; scaleMod = 10.0f; translateOffset = -2.0f; }	//Rabbit Mesh. (Update Scale/Position Modifiers).
	if (keys[SDL_SCANCODE_3]) { currentMesh = statue; scaleMod = 1.0f; translateOffset = -5.0f; }	//Statue Mesh. (Update Scale/Position Modifiers).

	//Change The Multitexture Effect Of The Env Map...
	if (keys[SDL_SCANCODE_4]) { currentTex = concreteTex; }		//Concrete.
	if (keys[SDL_SCANCODE_5]) { currentTex = metalTex; }		//Metal.
	if (keys[SDL_SCANCODE_6]) { currentTex = fabricTex; }		//Fabric.

	//Change the draw mode...
	if (keys[SDL_SCANCODE_7]) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); glDisable(GL_CULL_FACE); }	//View the vertices.	(Credit to James Riordan for this particular utility.)
	if (keys[SDL_SCANCODE_8]) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); glEnable(GL_CULL_FACE); }	//Default.				(Credit to James Riordan for this particular utility.)
}

//Initialises all values required before the draw-loop...
void init()
{

	////////////////////
	//Init the shaders//
	////////////////////

	//Default shader for our scene...
	shaderProgram = rt3d::initShaders("cellShader.vert", "cellShader.frag");
	rt3d::setLight(shaderProgram, light0);
	rt3d::setMaterial(shaderProgram, material0);

	//Shader program for our dynamic enviroment map...
	envmapProgram = rt3d::initShaders("dynamicEnvMap.vert", "dynamicMultiTexEnvMap.frag");
	//Set cubemap sampler to texture unit 1...
	GLuint uniformIndex = glGetUniformLocation(envmapProgram, "textureUnit1");
	glUniform1i(uniformIndex, 1);
	//Set tex sampler to texture unit 0...
	uniformIndex = glGetUniformLocation(envmapProgram, "textureUnit0");
	glUniform1i(uniformIndex, 0);

	//Shader program for our cubeMap skybox...
	skyboxProgram = rt3d::initShaders("cubeMap.vert", "cubeMap.frag");


	/////////////////////////
	//Init the mesh objects//
	/////////////////////////

	cube = new MeshObject("cube.obj"); //Creates an object for cube meshes...
	rabbit = new MeshObject("bunny-5000.obj"); //Creates an object for rabbit meshes...
	statue = new MeshObject("statue.obj"); //Creates an object for statue meshes...
	currentMesh = cube; //Stores a pointer to the mesh we want to use, allows easy switching from one object to another.


	/////////////////////
	//Init the textures//
	/////////////////////

	concreteTex = new Texture("concrete.bmp");  
	metalTex = new Texture("studdedmetal.bmp");
	fabricTex = new Texture("fabric.bmp");
	currentTex = concreteTex; //Stores a pointer to the texture we want to use, allows easy switching from one to another.


	///////////////////
	//Init the skybox//
	///////////////////

	const char *cubeTexFiles[6] = {"town-skybox/Town_bk.bmp","town-skybox/Town_ft.bmp","town-skybox/Town_rt.bmp","town-skybox/Town_lf.bmp","town-skybox/Town_up.bmp","town-skybox/Town_dn.bmp"};
	cubemap::load(cubeTexFiles, &skyboxCubeMapID, sides);


	////////////////////////////
	//Init the dynamic cubemap//
	////////////////////////////

	//Generate the dynamic cubemap texture and bind to texture unit 1...
	glGenTextures(1, &dynamicCubeMapID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicCubeMapID);
	//Set the texture parameters...
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//Allocate a different texture for each face of the cubemap and assign to the cubemap texture target...
	for (int i = 0; i < 6; i++) {
		GLenum side = sides[i];
		glTexImage2D(side, 0, GL_RGBA, cubeMapSize, cubeMapSize, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	//Map COLOR_ATTACHMENT0 to texture...
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, dynamicCubeMapID, 0);

	//////////////////
	//Initialise FBO//
	//////////////////

	// Generate FBO, RBO & Texture handles...
	glGenFramebuffers(1, &framebuffer);
	glGenRenderbuffers(1, &depthRenderbuffer);

	// Bind FBO, RBO & Texture & init storage and params...
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, cubeMapSize, cubeMapSize); 

	//Bind DEPTH_ATTACHMENT to depth buffer RBO..
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

	// Check for errors...
	GLenum valid = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (valid != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer Object not complete" << std::endl;
	if (valid == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		std::cout << "Framebuffer incomplete attachment" << std::endl;
	if (valid == GL_FRAMEBUFFER_UNSUPPORTED)
		std::cout << "FBO attachments unsupported" << std::endl;

	// Set default framebuffer...
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	cout << "Init Successful!" << endl;
}

//Used to draw the scene within the confines of the given modelview/projection... (Does not include the env-mapped mesh)...
void drawScene(const glm::mat4& modelview, const glm::mat4& projection, bool drawskybox = true)
{
	mvStack.push(modelview); //Defines the confines of the scene. (modelview may be a reflection in our envmap, or it could be the realspace).

	/////////////////////////////////////
	//Draw A Cube At The Light Position//
	/////////////////////////////////////

	if (drawskybox) {
		glUseProgram(skyboxProgram);
		rt3d::setUniformMatrix4fv(skyboxProgram, "projection", glm::value_ptr(projection));
		glDepthMask(GL_FALSE); // make sure writing to update depth test is off
		glm::mat3 mvRotOnlyMat3 = glm::mat3(mvStack.top());
		mvStack.push(glm::mat4(mvRotOnlyMat3));

		glCullFace(GL_FRONT); //Drawing inside of the cube.

		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubeMapID);
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(1.5f, 1.5f, 1.5f));
		rt3d::setUniformMatrix4fv(skyboxProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::drawIndexedMesh(cube->mesh, cube->indexCount, GL_TRIANGLES);
		mvStack.pop();
	}
	glCullFace(GL_BACK); // return to rendering rest of scene.
	glDepthMask(GL_TRUE); // make sure depth test is on.


	/////////////////////////////////////
	//Draw A Cube At The Light Position//
	/////////////////////////////////////

	//Sets up light position within the confines of the scene...
	glm::vec4 tmp = modelview*lightPos;
	//Updates the light position...
	light0.position[0] = tmp.x;
	light0.position[1] = tmp.y;
	light0.position[2] = tmp.z;

	//Draw the cube at the light position...
	glUseProgram(shaderProgram);
	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));
	rt3d::setLightPos(shaderProgram, glm::value_ptr(tmp));
	mvStack.push(modelview);
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(lightPos[0], lightPos[1], lightPos[2]));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.25f, 0.25f, 0.25f));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(shaderProgram, material0);
	rt3d::setLightPos(shaderProgram, glm::value_ptr(tmp));
	rt3d::drawIndexedMesh(cube->mesh, cube->indexCount, GL_TRIANGLES);
	mvStack.pop();


	/////////////////////////////////////////////////
	//Draw A Collection Of Different Coloured Cubes//
	/////////////////////////////////////////////////

	rt3d::materialStruct matStructArray[9] = { material0, material1, material2, material0, material1, material2, material0, material1, material2 }; //List of materials for different cube colours.
	
	//Draw nine cubes at equidistant locations around the circumference of a circle whose center is the origin...
	for (int i = 0; i < 9; i++)
	{
		rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));
		mvStack.push(mvStack.top());
			mvStack.top() = glm::translate(mvStack.top(), glm::vec3(sceneRadius * cos((sceneRotation + (i * 40)) * DEG_TO_RADIAN), 0.0f, sceneRadius * sin((sceneRotation + (i * 40))* DEG_TO_RADIAN)));
			mvStack.top() = glm::rotate(mvStack.top(), float(theta*DEG_TO_RADIAN), glm::vec3(-2.0f, 3.0f, 0.0f));
			mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.3f, 0.3f, 0.3f));
			rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
			rt3d::setLight(shaderProgram, light0);
			rt3d::setMaterial(shaderProgram, matStructArray[i]); //Determines the cube's colour based on the pass and the materalStructArray.
			rt3d::setLightPos(shaderProgram, glm::value_ptr(tmp));
			rt3d::setUniformMatrix3fv(shaderProgram, "normalmatrix", glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(mvStack.top())))));
			rt3d::drawIndexedMesh(cube->mesh, cube->indexCount, GL_TRIANGLES);
		mvStack.pop();
	}

	mvStack.pop(); //Pops to the initial matrix.
	theta += 0.1f; //Increments the theta value used in cube rotations.
}

//Main draw function...
void draw(SDL_Window * window)
{
	//Clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Defines the MV and Projection of the scene...
	glm::mat4 projection(1.0);
	glm::mat4 modelview(1.0);

	//Updates the camera...
	at = move::forward(eye, r, 1.0f);
	mvStack.push(modelview);
	mvStack.top() = glm::lookAt(eye, at, up);

	//Sets up light position within the confines of the scene...
	glm::vec4 tmp = modelview*lightPos;
	//Updates the light position...
	light0.position[0] = tmp.x;
	light0.position[1] = tmp.y;
	light0.position[2] = tmp.z;

	
	///////////////////////////////////////////////////////
	//Setup The FBO "Reflections" For The Dynamic Cubemap//
	///////////////////////////////////////////////////////

	glm::mat4 cubeMapProjection = glm::perspective(float(90.0f*DEG_TO_RADIAN), 1.0f, 1.0f, 150.0f); //Setup the projection matrix for the cubemap.
	glViewport(0, 0, cubeMapSize, cubeMapSize); //set the viewport to the size of the cube map texture.

	//Bind the FBO...
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

	//Array of vectors storing the target direction and up vectors of each cubemap face: (Credit to James Riordan for identifying the need for these.)
	glm::vec3 targetVectors[6] = {
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f)
	};
	glm::vec3 upVectors[6] = {
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f)
	};
	glm::mat4 MV; // Modelview of the cubemap reflection.

	//Loop once for each face of the cubemap...
	for (int i = 0; i < 6; i++) {
		GLenum side = sides[i];
		//clear the colour and depth buffers
		glClear(GL_DEPTH_BUFFER_BIT);
		//set the FBO view at the center of the reflective object and render the scene with respect to that by using the cube map projection matrix and appropriate viewing settings...
		switch (side) {
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, dynamicCubeMapID, 0);
			MV = glm::lookAt(envMapCubePos, envMapCubePos + targetVectors[0], upVectors[0]);
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, dynamicCubeMapID, 0);
			MV = glm::lookAt(envMapCubePos, envMapCubePos + targetVectors[1], upVectors[1]);
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, dynamicCubeMapID, 0);
			MV = glm::lookAt(envMapCubePos, envMapCubePos + targetVectors[2], upVectors[2]);
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, dynamicCubeMapID, 0);
			MV = glm::lookAt(envMapCubePos, envMapCubePos + targetVectors[3], upVectors[3]);
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, dynamicCubeMapID, 0);
			MV = glm::lookAt(envMapCubePos, envMapCubePos + targetVectors[4], upVectors[4]);
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, dynamicCubeMapID, 0);
			MV = glm::lookAt(envMapCubePos, envMapCubePos + targetVectors[5], upVectors[5]);
			break;
		}
		drawScene(MV, cubeMapProjection); //Calls the method to draw the scene, using the appropriate viewing settings to create the "mirror" effect.
	}


	////////////////////////////////////////////////////
	//Draw The Scene In Realspace - Not The Reflection//
	////////////////////////////////////////////////////

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);		//The value zero is reserved to represent the default framebuffer provided by the windowing system.										   
	glViewport(0, 0, screenWidth, screenHeight);												//Reset the viewport to the size of the screen.
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 150.0f);	//Setup the projection matrix.
	//Draw the scene again...
	drawScene(mvStack.top(), projection);

	/////////////////////////////////////////////
	//Draw The Dynamic Enviroment Mapped Object//
	/////////////////////////////////////////////

	glUseProgram(envmapProgram);
	rt3d::setUniformMatrix4fv(envmapProgram, "projection", glm::value_ptr(projection));
	//Bind textures to the texture units...
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicCubeMapID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currentTex->texture);
	//Draw the mesh...
	glm::mat4 modelMatrix(1.0);
	mvStack.push(mvStack.top());
		modelMatrix = glm::translate(modelMatrix, glm::vec3(envMapCubePos.x, envMapCubePos.y + translateOffset, envMapCubePos.z));	//Remember to take into account the different mesh offsets!
		modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 2.0f) * scaleMod);												//And the difference in size of the different meshes!	
		mvStack.top() = mvStack.top() * modelMatrix;
		rt3d::setUniformMatrix4fv(envmapProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setUniformMatrix4fv(envmapProgram, "modelMatrix", glm::value_ptr(modelMatrix));
		GLuint uniformIndex = glGetUniformLocation(envmapProgram, "cameraPos");
		glUniform3fv(uniformIndex, 1, glm::value_ptr(eye));
		//Draw the mesh...
		rt3d::drawIndexedMesh(currentMesh->mesh, currentMesh->indexCount, GL_TRIANGLES);
	mvStack.pop();

	mvStack.pop(); //Pop to the initial matrix.
	SDL_GL_SwapWindow(window); //Swap buffers.
}

//Entry Point...
int main(int argc, char* argv[])
{
	SDL_Window * hWindow; // window handle
	SDL_GLContext glContext; // OpenGL context handle
	hWindow = SDLWindow::setupRC(glContext); // Create window and render context 

	// Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << std::endl;
		exit(1);
	}
	std::cout << glGetString(GL_VERSION) << std::endl;

	//Initialise the required values...
	init();	
	bool running = true;	//Set running to true.
	SDL_Event sdlEvent;		//Variable to detect SDL events.

	//Event loop:
	while (running) 
	{		
		update();		//Poll user-input and update.
		draw(hWindow);	//Call the main draw function.
		
		while (SDL_PollEvent(&sdlEvent))
		{
			if (sdlEvent.type == SDL_QUIT)	//If the user quits:
				running = false;			//End the loop.
		}
	}

	//Cleanup SDL and quit the program...
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(hWindow);
	SDL_Quit();
	return 0;
}
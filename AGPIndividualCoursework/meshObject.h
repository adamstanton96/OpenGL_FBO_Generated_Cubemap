#pragma once
#include "rt3d.h"
#include "rt3dObjLoader.h"
#include <vector>

class MeshObject
{
public:
	MeshObject();
	MeshObject(const char* filename);
	~MeshObject() {}
	GLuint mesh;
	GLuint indexCount;

protected:
	void loadObject(const char* filename);
	void createMesh();
	std::vector<GLfloat> verts;
	std::vector<GLfloat> norms;
	std::vector<GLfloat> texCoords;
	std::vector<GLuint> indices;
};
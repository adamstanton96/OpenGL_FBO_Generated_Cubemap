#include "meshObject.h"

MeshObject::MeshObject()
{
	MeshObject("cube.obj");
}

MeshObject::MeshObject(const char * filename)
{
	loadObject(filename);
	createMesh();
}

void MeshObject::loadObject(const char * filename)
{
	rt3d::loadObj(filename, verts, norms, texCoords, indices);
	indexCount = indices.size();
}

void MeshObject::createMesh()
{
	mesh = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), texCoords.data(), indexCount, indices.data());
}

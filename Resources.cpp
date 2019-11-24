#include "Resources.h"

std::mutex mesh_mutex;
void loadMesh(std::map<std::string, Mesh>* meshes, const std::string& file)
{
	std::lock_guard<std::mutex> lock(mesh_mutex);
	meshes->insert({ file, Mesh() });
	meshes->at(file).setVertices(loadOBJ(readTextFile(file)));
}

void Resources::loadMeshes(const std::vector<std::string>& files)
{
	for (const std::string& mesh : files) // TODO: Run each iteration as a seperate task using std::async
	{
		std::async(std::launch::async, loadMesh, &meshes, mesh);
		//meshes.insert({ mesh, Mesh() });
		//meshes.at(mesh).setVertices(loadOBJ(readTextFile(mesh)));
	}
}

void Resources::loadTextures(const std::vector<std::string>& files)
{
	for (const std::string& texture : files)
	{
		textures.insert({ texture, Texture() });
		textures.at(texture).load(texture);
	}
}

Mesh& Resources::mesh(const std::string& file) { return meshes.at(file); }
Texture& Resources::texture(const std::string& file) { return textures.at(file); }
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Window.h"
#include "Transform.h"
#include "Tools.h"
#include "Entities.h"
#include "Resources.h"
#include "CubeMap.h"
#include "FrameBuffer.h"
#include <glm/gtx/string_cast.hpp>

namespace CS
{
	struct Transform : ENG::Transform, ENG::ECSComponent<Transform> {};

	struct Model : ENG::ECSComponent<Model>
	{
		ENG::Mesh* mesh;
		ENG::Texture* texture;
		ENG::Shader* shader;
	};

	struct Light : ENG::ECSComponent<Light>
	{
		Light() : colour(1.0f, 1.0f, 0.0f), radius(5.0f) {}

		glm::vec3 colour;
		float radius;
	};
}

void drawModels(ENG::Entities& entities)
{
	auto& ts = entities.getPool<CS::Transform>();
	auto& ms = entities.getPool<CS::Model>();

	for (ENG::EntityID entity : entities.entitiesWith<CS::Transform, CS::Model>())
	{
		CS::Model& m = ms[entity];

		m.shader->setUniform("transform", ts[entity].get());
		m.shader->bind();
		m.mesh->bind();
		m.texture->bind();
		glDrawArrays(GL_TRIANGLES, 0, m.mesh->vertexCount());
	}
}

void setLights(ENG::Entities& entities, ENG::Shader& shader)
{
	auto& ts = entities.getPool<CS::Transform>();
	auto& ls = entities.getPool<CS::Light>();

	std::vector<ENG::EntityID> ents = entities.entitiesWith<CS::Transform, CS::Light>();
	for (std::size_t i = 0; i < ents.size(); i++)
	{
		shader.setUniform("lights[" + std::to_string(i) + "].position", ts[ents[i]].position);
		shader.setUniform("lights[" + std::to_string(i) + "].colour", ls[ents[i]].colour);
		shader.setUniform("lights[" + std::to_string(i) + "].radius", ls[ents[i]].radius);
	}
}

int main()
{
	glm::ivec2 window_size(1920, 1080);
	glm::mat4 projection = glm::perspective(90.0f, static_cast<float>(window_size.x) / window_size.y, 0.1f, 500.0f);

	ENG::Window window(window_size, "ENG");
	ENG::Entities entities;
	ENG::Resources resources;
	
	ENG::Shader def_shader(
		ENG::readTextFile("Resources/Shaders/simple.vert"),
		ENG::readTextFile("Resources/Shaders/simple.frag")
	);

	ENG::Shader skybox_shader(
		ENG::readTextFile("Resources/Shaders/skybox.vert"),
		ENG::readTextFile("Resources/Shaders/skybox.frag")
	);

	ENG::Shader fb_shader(
		ENG::readTextFile("Resources/Shaders/framebuffer.vert"),
		ENG::readTextFile("Resources/Shaders/framebuffer.frag")
	);

	ENG::CubeMap cubemap;
	cubemap.create({
		"Resources/Textures/right.png",
		"Resources/Textures/left.png",
		"Resources/Textures/up.png",
		"Resources/Textures/down.png",
		"Resources/Textures/back.png",
		"Resources/Textures/front.png"
	});

	// FRAMEBUFFER =======================================================================

	ENG::FrameBuffer buf;
	buf.create(window_size);

	std::vector<GLfloat> positions = {
		-1, 1,
		1, -1,
		-1, -1,
		-1, 1,
		1, 1,
		1, -1
	};

	GLuint position_buffer;
	glGenBuffers(1, &position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), &positions.at(0), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);

	GLuint fb_vao;
	glGenVertexArrays(1, &fb_vao);
	glBindVertexArray(fb_vao);

	// Bind position buffer to vertex array.
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);

	//buf.unbind();

	// FRAMEBUFFER ========================================================================

	resources.loadMeshes({
		"Resources/Meshes/cube.obj",
		"Resources/Meshes/lorry.obj",
		"Resources/Meshes/car.obj"
		//"Resources/Meshes/skull.obj"
	});
	resources.loadTextures({
		"Resources/Textures/lorry.jpg",
		"Resources/Textures/rock.png",
		"Resources/Textures/skull.jpg"
	});

	def_shader.setUniform("projection", projection);
	def_shader.setUniform("ambient", { 0.2f, 0.2f, 0.2f });

	skybox_shader.setUniform("projection", projection);

	entities.addComponentPools<CS::Transform, CS::Model, CS::Light>();

	for (int i = -5; i < 5; i++)
	{
		auto e = entities.addEntity();
		auto& m = entities.addComponent<CS::Model>(e);
		m.mesh = &resources.mesh("lorry.obj");
		m.texture = &resources.texture("lorry.jpg");
		m.shader = &def_shader;

		auto& t = entities.addComponent<CS::Transform>(e);
		t.position = { i * 5.0f, 0.0f, -5.0f };
		t.rotation = { -90.0f, 0.0f, -90.0f };
		t.scale *= 0.1f;
	}

	auto p = entities.addEntity<CS::Transform, CS::Light>();
	auto& t2 = entities.getComponent<CS::Transform>(p);
	def_shader.setUniform("view_pos", t2.position);
	def_shader.setUniform("view", glm::inverse(t2.get()));

	skybox_shader.setUniform("view", glm::mat4(glm::mat3(glm::inverse(t2.get()))));

	while (!window.shouldClose())
	{
		if (window.isKeyPressed(GLFW_KEY_ESCAPE))
			window.close();

		setLights(entities, def_shader);

		def_shader.setUniform("view", glm::inverse(t2.get()));
		skybox_shader.setUniform("view", glm::mat4(glm::mat3(glm::inverse(t2.get()))));

		window.clear({ 0.0f, 0.0f, 0.0f, 0.0f });

		glDepthMask(GL_FALSE);
		cubemap.bind();
		skybox_shader.bind();
		resources.mesh("cube.obj").bind();
		glDrawArrays(GL_TRIANGLES, 0, resources.mesh("cube.obj").vertexCount());
		glDepthMask(GL_TRUE);

		drawModels(entities);

		buf.bind();
		fb_shader.bind();
		glBindVertexArray(fb_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(NULL);
		fb_shader.unbind();
		buf.unbind();

		window.display();

		glfwPollEvents();
	}

	return 0;
}
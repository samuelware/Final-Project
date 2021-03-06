#pragma once

#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Collision.h"
#include "Resources.h"
#include "FrameBuffer.h"

namespace ENG
{
	constexpr auto MAX_LAYERS = 8;

	struct Core;
	struct Renderer
	{
		EntityID view_id = 0;
		glm::vec3 ambient;
		bool draw_colliders = false;
	};

	namespace CS
	{
		/**
		* Model component used for drawing a mesh with a texture to the screen.
		*/
		struct Model : ECSComponent<Model>
		{
			Model() { layers[0] = true; }

			std::string mesh = "cube.obj";
			std::string texture = "notexture.png";
			std::string shader = "default.shdr";

			EntityID camera_output = 0;
			bool hud = false;
			bool backface_culling = true;
			std::bitset<MAX_LAYERS> layers;
		};

		/**
		* Sprite component can be used for 2D or 3D sprites. Also supports simple
		* sprite animation.
		*/
		struct Sprite : ECSComponent<Sprite>
		{
			Sprite() : frame(1), frames(1) {}

			std::string texture = "notexture.png";
			bool billboard = true;

			bool animated = false;
			glm::ivec2 frame;
			glm::ivec2 frames;
			float frame_time = 0.0f;
			float timer = 0.0f;
		};

		/**
		* Light component is used as a point light, which has a radius which it illuminates and a colour.
		*/
		struct Light : ECSComponent<Light>
		{
			Light() : colour(1.0f) {}

			glm::vec3 colour;
			float radius = 5.0f;
		};

		/**
		* Camera component represents a 3D camera in the world. Draws to a framebuffer from its perspective.
		*/
		struct Camera : ECSComponent<Camera>
		{
			Camera() : size(0) { layers[0] = true; }
			Camera(const glm::vec2& size, const float fov, const float near, const float far);
			void create(const glm::vec2& size, const float fov, const float near, const float far);
			glm::mat4 get();

			FrameBuffer frame;
			glm::ivec2 size;
			float aspect;
			float fov_y;
			float fov_x;
			float near;
			float far;

			int order = 0;
			std::bitset<MAX_LAYERS> layers;
		};

		struct Text : ECSComponent<Text>
		{
			void setText(const std::string& new_text, Core& core);

			std::string text = "This is a sentence";
			Mesh2D mesh;
		};
	}

	namespace
	{
		Mesh2D quad_2d;
		Mesh quad_3d;
	}

	void drawToCameras(Core& core);
	void drawToScreen(Core& core);

	void startRenderer(Core& core);
	void updateRenderer(Core& core, glm::mat4 view, glm::mat4 projection);

	void drawModels(Core& core, std::bitset<MAX_LAYERS> layers);
	void drawModelsToHUD(Core& core);
	void drawSkybox(Resources& resources);
	void updateSprites(Core& core);
	void drawSprites(Core& core);
	void drawSprites3D(Core& core);
	void drawColliders(Core& core);
	void renderText(Core& core);
	void renderText3D(Core& core);

	/**
	* Uploads lighting information to shader.
	*/
	void setLights(Core& core, Shader& shader);
}
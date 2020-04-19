#include "Portal.h"
#include "Core.h"
#include <glm/gtx/string_cast.hpp>

namespace ENG
{
	void startPortals(Entities& entities, const glm::ivec2& size)
	{
		ComponentMap<CS::Transform>& transforms = entities.getPool<CS::Transform>();
		ComponentMap<CS::Portal>& portals = entities.getPool<CS::Portal>();

		for (EntityID id : entities.entitiesWith<CS::Transform, CS::Portal>())
		{
			portals[id].prev_side = static_cast<int>(glm::sign(glm::dot(transforms[id].forward(), transforms[portals[id].player].position - transforms[id].position)));
			portals[id].framebuffer.create(size);
		}
	}

	void updatePortals(Entities& entities)
	{
		ComponentMap<CS::Transform>& transforms = entities.getPool<CS::Transform>();
		ComponentMap<CS::Portal>& portals = entities.getPool<CS::Portal>();
		ComponentMap<CS::BoxCollider>& boxes = entities.getPool<CS::BoxCollider>();

		EntityID player;
		EntityID other;

		for (EntityID portal : entities.entitiesWith<CS::Transform, CS::Portal>())
		{
			player = portals[portal].player;
			other = portals[portal].other;

			// Check which side of portal player is on
			int side = static_cast<int>(glm::sign(glm::dot(transforms[portal].forward(), transforms[portal].position - transforms[player].position)));

			glm::vec3 p_size = glm::vec3(2.0f) * transforms[portal].scale;
			glm::vec3 pl_size = boxes[player].size * transforms[player].scale;

			// Is the player colliding with the portal? Basically check if the player could travel through the portal.
			if (intersectAABBvAABB(transforms[portal].position, p_size, transforms[player].position, pl_size).intersects)
			{
				portals[portal].active = true;

				// If the player moves from one side of the portal to the other, teleport them.
				if (side != portals[portal].prev_side)
				{
					glm::mat4 m = transforms[other].get() * glm::inverse(transforms[portal].get()) * transforms[player].get();
					transforms[player] = ENG::decompose(m);

					// Never rotate on Z, only X and Y.
					transforms[player].rotation.z = 0.0f;

					// Prevents double teleporting
					portals[other].prev_side = side;
				}
			}
			else
				portals[portal].active = false;

			// Transform camera match players transform relative to the portal.
			portals[portal].camera = transforms[portal].get() * glm::inverse(transforms[other].get()) * transforms[player].get();
			portals[portal].prev_side = side;
		}
	}

	void drawToPortals(Core& core)
	{
		ComponentMap<CS::Transform>& transforms = core.entities.getPool<CS::Transform>();
		ComponentMap<CS::Portal>& portals = core.entities.getPool<CS::Portal>();

		CS::Transform view_t;
		CS::Transform* view_d = core.renderer.view;

		for (EntityID id : core.entities.entitiesWith<CS::Transform, CS::Portal>())
		{
			// Don't render to portal if its not in view.
			if (!inView(*view_d, transforms[id].position, { 2.0f, 2.0f, 2.0f })) continue;

			portals[id].framebuffer.bind();
			view_t = decompose(portals[portals[id].other].camera);
			glm::mat4 view = glm::inverse(portals[portals[id].other].camera);

			core.renderer.view = &view_t;
			updateSprites(core);

			core.resources.shader("default.shdr").setUniform("view", view);
			core.resources.shader("skybox.shdr").setUniform("view", glm::mat4(glm::mat3(view)));

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			drawSkybox(core.resources);
			drawModels(core);
			drawSprites3D(core);
			portals[id].framebuffer.unbind();
		}

		core.renderer.view = view_d;
	}

	void drawPortals(Core& core)
	{
		ComponentMap<CS::Transform>& transforms = core.entities.getPool<CS::Transform>();
		ComponentMap<CS::Portal>& portals = core.entities.getPool<CS::Portal>();

		core.resources.shader("portals.shdr").setUniform("view", glm::inverse(core.renderer.view->get()));
		core.resources.shader("portals.shdr").setUniform("projection", core.perspective);

		// Disable backface culling for drawing portals, so that portals become 2-way.
		glDisable(GL_CULL_FACE);
		
		Mesh& cube = core.resources.mesh("cube.obj");
		cube.bind();

		for (EntityID id : core.entities.entitiesWith<CS::Transform, CS::Portal>())
		{
			if (!inView(*core.renderer.view, transforms[id].position, glm::vec3(1.0f))) continue;

			CS::Transform t = transforms[id];
			if (portals[id].active)
				t = preventNearClipping(core.settings, transforms[id], transforms[portals[id].player]);
			else
				t.scale.z = 0.0f;

			core.resources.shader("portals.shdr").setUniform("transform", t.get());
			core.resources.shader("portals.shdr").bind();
			portals[id].framebuffer.getTexture().bind();

			glDrawArrays(GL_TRIANGLES, 0, cube.vertexCount());
		}
		
		glEnable(GL_CULL_FACE);
	}

	/**
	* Move screen position back and scale wall along Z.
	*/
	CS::Transform preventNearClipping(Settings& settings, CS::Transform screen, CS::Transform player)
	{
		// Calculate distance from camera to corner of near plane.
		float fov = settings.getf("fov");
		float aspect = settings.getf("width") / settings.getf("height");
		float near_dist = 0.1f;

		float half_height = near_dist * glm::tan(fov);
		float half_width = half_height * aspect;
		float corner_dist = glm::length(glm::vec3(half_width, half_height, near_dist));

		// Set scale of screen, and move back from whichever way player is facing.
		bool facing = glm::dot(screen.forward(), screen.position - player.position) > 0;
		screen.scale.z = corner_dist;
		screen.position += (screen.forward() * (facing ? 1.0f : -1.0f));

		return screen;
	}
}
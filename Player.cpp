#include "Player.h"
#include "Core.h"

namespace Game
{
	ENG::EntityID createPlayer(ENG::Core& core)
	{
		ENG::EntityID player = core.entities.addEntity<
			ENG::CS::Transform,
			ENG::CS::Script,
			ENG::CS::BoxCollider,
			ENG::CS::Controller,
			ENG::CS::Camera
		>();

		// player camera sees both layers
		core.entities.getComponent<ENG::CS::Camera>(player).layers[1] = true;

		ENG::EntityID gun = core.entities.addEntity<ENG::CS::Transform, ENG::CS::Model>();
		ENG::CS::Transform& t = core.entities.getComponent<ENG::CS::Transform>(gun);
		t.parent = player;
		t.position = { 0.0f, -0.3f, -0.2f };

		ENG::CS::Model& model = core.entities.getComponent<ENG::CS::Model>(gun);
		model.mesh = "gun.obj";
		model.texture = "gun.png";
		model.hud = true;

		ENG::EntityID crosshair = core.entities.addEntity<ENG::CS::Transform2D, ENG::CS::Sprite>();
		ENG::CS::Transform2D& t2d = core.entities.getComponent<ENG::CS::Transform2D>(crosshair);
		t2d.origin = glm::vec2(core.resources.texture("crosshair.png").getSize()) / 2.0f;
		t2d.position = glm::vec2(core.window.getSize()) / 2.0f;

		ENG::CS::Sprite& s = core.entities.getComponent<ENG::CS::Sprite>(crosshair);
		s.texture = "crosshair.png";

		ENG::CS::Script& scr = core.entities.getComponent<ENG::CS::Script>(player);
		scr.script = std::make_shared<Game::Player>();

		core.entities.getComponent<ENG::CS::Camera>(player).order = 1;
		core.renderer.view_id = player;

		return player;
	}

	void Player::start(ENG::Core& core)
	{
		transform = &core.entities.getComponent<ENG::CS::Transform>(id);
		transform->position.y = 5.0f;
		
		controller = &core.entities.getComponent<ENG::CS::Controller>(id);

		box = &core.entities.getComponent<ENG::CS::BoxCollider>(id);
		box->size = { 0.25f, 0.8f, 0.25f };

		pickup_position = core.entities.addEntity<ENG::CS::Transform>();
		core.entities.getComponent<ENG::CS::Transform>(pickup_position).position.z = -3.0f;
		core.entities.getComponent<ENG::CS::Transform>(pickup_position).parent = id;

		pos_text = core.entities.addEntity<ENG::CS::Transform2D, ENG::CS::Text>();
		core.entities.getComponent<ENG::CS::Transform2D>(pos_text).position = glm::vec2(50.0f);
		core.entities.getComponent<ENG::CS::Text>(pos_text).setText(glm::to_string(glm::ivec3(transform->position)), core);
	}

	void Player::mouselook(ENG::Core& core)
	{
		mouse_offset = last_mouse - core.window.getMousePos();
		last_mouse = core.window.getMousePos();
		transform->rotation += glm::vec3(mouse_offset.y, mouse_offset.x, 0.0f) * sensitivity;

		if (transform->rotation.x > 89.0f) transform->rotation.x = 89.0f;
		else if (transform->rotation.x < -89.0f) transform->rotation.x = -89.0f;
	}

	void Player::movement(ENG::Core& core)
	{
		// Movement
		direction = glm::vec3(0.0f);
		if (core.window.isKeyPressed(GLFW_KEY_W)) direction -= transform->forward();
		else if (core.window.isKeyPressed(GLFW_KEY_S)) direction += transform->forward();
		if (core.window.isKeyPressed(GLFW_KEY_A)) direction -= transform->right();
		else if (core.window.isKeyPressed(GLFW_KEY_D)) direction += transform->right();

		if (direction != glm::vec3(0.0f))
			direction = glm::normalize(direction);

		velocity.x = direction.x * speed;
		velocity.z = direction.z * speed;

		// Apply gravity and jumping
		if (!controller->on_floor)
			velocity.y -= 9.8f * core.clock.deltaTime();
		else if (core.window.isKeyPressed(GLFW_KEY_SPACE))
			velocity.y = 10.0f;
		else
			velocity.y = 0.0f;

		controller->velocity = velocity * transform->scale;
	}

	void Player::actions(ENG::Core& core)
	{
		if (core.window.isMouseButtonPressedOnce(GLFW_MOUSE_BUTTON_LEFT))
		{
			ENG::IntersectData place = ENG::castRay(core.entities, transform->position, -transform->forward(), { id });

			ENG::EntityID thing = core.entities.addEntity<ENG::CS::Transform, ENG::CS::Model>();
			core.entities.getComponent<ENG::CS::Model>(thing).texture = "Space3.jpg";
			core.entities.getComponent<ENG::CS::Model>(thing).mesh = "lamp.obj";
			core.entities.getComponent<ENG::CS::Transform>(thing).scale *= 0.2f;
			core.entities.getComponent<ENG::CS::Transform>(thing).position = transform->position + place.distance * -transform->forward();
			core.entities.getComponent<ENG::CS::Transform>(thing).rotation = place.normal;
		}

		if (ray.id == 0 && core.window.isKeyPressedOnce(GLFW_KEY_E))
		{
			ray = ENG::castRay(core.entities, transform->position, -transform->forward(), { id, ray.id });
			if (ray.id != 0 && ray.distance < 5.0f)
			{
				//core.entities.getComponent<Game::Pickup>(ray.id).active = true;
				//core.entities.getComponent<Game::Pickup>(ray.id).holder = pickup_position;
			}
			else ray.id = 0;
		}
		else if (ray.id != 0 && core.window.isKeyPressedOnce(GLFW_KEY_E))
		{
			//core.entities.getComponent<Game::Pickup>(ray.id).active = false;
			ray.id = 0;
		}
	}

	void Player::update(ENG::Core& core)
	{
		if (core.window.isKeyPressedOnce(GLFW_KEY_ESCAPE))
			core.window.close();

		if (core.window.isKeyPressedOnce(GLFW_KEY_Q))
			core.renderer.draw_colliders = !core.renderer.draw_colliders;

		if (core.window.isKeyPressedOnce(GLFW_KEY_F))
			transform->position = glm::vec3(0.0f, 10.0f, 0.0f);

		mouselook(core);
		movement(core);
		actions(core);

		core.entities.getComponent<ENG::CS::Text>(pos_text).setText(glm::to_string(glm::ivec3(transform->position)), core);
	}
}
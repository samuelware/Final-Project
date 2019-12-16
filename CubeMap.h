#pragma once

#include <string>
#include <array>
#include <GL/glew.h>
#include "Image.h"

namespace ENG
{
	class CubeMap
	{
	public:
		void create(const std::array<std::string, 6>& files);
		void bind();
		void unbind();
		void cleanup();

	private:
		GLuint id;
	};
}
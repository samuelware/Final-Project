#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Output.h"
#include "Tools.h"

namespace ENG
{
	/**
	* Simple shader class, allows compiling of shaders and uniform setting.
	*/
	class Shader
	{
	public:
		Shader();
		Shader(const std::string& vertex, const std::string& fragment);
		void create(const std::string& vertex, const std::string& fragment);
		void setUniform(const std::string& name, const glm::mat4& value);
		void setUniform(const std::string& name, const glm::vec3& value);
		void setUniform(const std::string& name, const float value);
		void setUniform(const std::string& name, const int value);
		void bind();
		void unbind();
		void cleanUp();

	private:
		void compileErrorCheck(const GLuint shader_id);
		void linkErrorCheck();

	private:
		GLuint id;
	};
}
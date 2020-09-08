#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

Shader::Shader(const char *vs_path, const char *fs_path)
{
		// Load the file
	std::string vs_code;
	std::string fs_code;

	std::ifstream vs_file;
	std::ifstream fs_file;

	vs_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	fs_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	
	try {
		vs_file.open(vs_path);
		std::stringstream vs_stream;
		vs_stream << vs_file.rdbuf();
		vs_file.close();
		vs_code = vs_stream.str();

		std::stringstream fs_stream;
		fs_file.open(fs_path);
		fs_stream << fs_file.rdbuf();
		fs_file.close();
		fs_code = fs_stream.str();
	} catch (std::ifstream::failure e) {
		std::cout << "ERROR::SHADER: File not successfully read\n";
	}

	// Compile the shaders
	const char *vs_code_c = vs_code.c_str();
	const char *fs_code_c = fs_code.c_str();

	unsigned int vs, fs;
	int success;
	char info_log[512];
	
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_code_c, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(vs, 512, NULL, info_log);
		std::cout << "ERROR::SHADER: Vertex compilation failed.\n";
		std::cout << info_log << "\n";
	}

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_code_c, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	
	if (!success) {
		glGetShaderInfoLog(fs, 512, NULL, info_log);
		std::cout << "ERROR::SHADER: Fragment compilation failed.\n";
		std::cout << info_log << "\n";
	}
	
	this->id = glCreateProgram();
	glAttachShader(this->id, vs);
	glAttachShader(this->id, fs);
	glLinkProgram(this->id);

	glGetProgramiv(this->id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(this->id, 512, NULL, info_log);
		std::cout << "ERROR::SHADER: Linking failed\n";
	}

	glDeleteShader(vs);
	glDeleteShader(fs);
}

Shader::~Shader()
{
	glDeleteProgram(id);
}

void Shader::use()
{
	glUseProgram(id);
}


void Shader::set_bool(const std::string &name, bool value) const
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::set_int(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::set_float(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::set_mat4(const std::string &name, const glm::mat4 &value) const
{
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()),
					   1,
					   GL_FALSE,
					   &value[0][0]);
}

void Shader::set_mat3(const std::string &name, const glm::mat3 &value) const
{
	glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()),
					   1,
					   GL_FALSE,
					   &value[0][0]);
}

void Shader::set_mat2(const std::string &name, const glm::mat2 &value) const
{
	glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()),
					   1,
					   GL_FALSE,
					   &value[0][0]);
}


void Shader::set_vec2(const std::string &name, const glm::vec2 &value) const
{
	glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::set_vec2(const std::string &name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
}

void Shader::set_vec3(const std::string &name, const glm::vec3 &value) const
{
	glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::set_vec3(const std::string &name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
}

void Shader::set_vec4(const std::string &name, const glm::vec4 &value) const
{
	glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::set_vec4(const std::string &name, float x, float y, float z, float w) const
{
	glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
}

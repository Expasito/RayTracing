#include "Utils.h"

void shaderBuildStatus(unsigned int shader) {
	int result = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	std::cout << "Result: " << result << "\n";
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)(alloca(length * (sizeof(char))));
		glGetShaderInfoLog(shader, length, &length, message);
		std::cout << "Failed to compile shader--\n";
		std::cout << message << "\n";
		glDeleteShader(shader);
		// error so exit
		exit(1);
		return;

	}
}

uint32_t genShader(GLenum shaderType, char** code) {
	uint32_t shader;
	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, code, NULL);
	//printf("Shader code:\n %s\n", *code);
	glCompileShader(shader);
	
	shaderBuildStatus(shader);

	return shader;

}

uint32_t genProgram(std::vector<uint32_t> shaders) {
	uint32_t program;
	program = glCreateProgram();
	for (uint32_t const &shader : shaders) {
		glAttachShader(program, shader);
	}
	glLinkProgram(program);
	
	glUseProgram(program);
	
	//exit(1);

	//for (uint32_t const& shader : shaders) {
	//	glDeleteShader(shader);
	//}

	return program;
}
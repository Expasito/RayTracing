#pragma once
#include <cstdint>
#include <iostream>
#include <glad/glad.h>
#include <vector>


uint32_t genShader(GLenum shaderType, char** code);

uint32_t genProgram(std::vector<uint32_t> shaders);
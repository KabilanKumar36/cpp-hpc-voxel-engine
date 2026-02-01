#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../core/Matrix.h"

namespace Renderer {
	class Shader {
	public:
		unsigned int ID;

		//Constructor
		Shader(const char* cVertexPath, const char* cFragmentPath) {
			std::string strVertexCode;
			std::string strFragmentCode;

			std::ifstream istrmVertexFile;
			std::ifstream istrmFragmentFile;
			istrmVertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			istrmFragmentFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			
			try {
				istrmVertexFile.open(cVertexPath);
				istrmFragmentFile.open(cFragmentPath);

				std::stringstream strstrmVertex, strstrmFragment;
				strstrmVertex << istrmVertexFile.rdbuf();
				strstrmFragment << istrmFragmentFile.rdbuf();
				
				istrmVertexFile.close();
				istrmFragmentFile.close();

				strVertexCode = strstrmVertex.str();
				strFragmentCode = strstrmFragment.str();
			}
			catch (std::ifstream::failure &e) {
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n" << e.what() << std::endl;
				assert(false && "Shader File Not Found!");
				exit(-1);
			}

			const char* cVertexShaderCode = strVertexCode.c_str();
			const char* cFragmentShaderCode = strFragmentCode.c_str();

			unsigned int uiVertex, uiFragment;
			uiVertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(uiVertex, 1, &cVertexShaderCode, nullptr);
			glCompileShader(uiVertex);
			checkCompileErrors(uiVertex, "VERTEX");

			uiFragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(uiFragment, 1, &cFragmentShaderCode, nullptr);
			glCompileShader(uiFragment);
			checkCompileErrors(uiFragment, "FRAGMENT");

			ID = glCreateProgram();
			glAttachShader(ID, uiVertex);
			glAttachShader(ID, uiFragment);
			glLinkProgram(ID);
			checkCompileErrors(ID, "PROGRAM");

			glDeleteShader(uiVertex);
			glDeleteShader(uiFragment);
		}

		void use() {
			glUseProgram(ID);
		}

		void setMat4(const std::string &name, const Core::Mat4 &mat) const {
			glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, mat.elements);
		}

		void setVec3(const std::string &name, const Core::Vec3 &vec) const {
			glUniform3f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y, vec.z);
		}
	private:
		void checkCompileErrors(unsigned int uiShader, std::string strType) {
			int iSuccess;
			char cInfoLog[1024];
			if (strType != "PROGRAM") {
				glGetShaderiv(uiShader, GL_COMPILE_STATUS, &iSuccess);
				if (!iSuccess) {
					glGetShaderInfoLog(uiShader, 1024, nullptr, cInfoLog);
					std::cout << "ERROR::SHADER::COMPILATION_ERROR\n" << cInfoLog;
					std::cout << "-----------------------------------------------------------------------";
				}
			}
			else {
				glGetProgramiv(uiShader, GL_LINK_STATUS, &iSuccess);
				if (!iSuccess) {
					glGetShaderInfoLog(uiShader, 1024, nullptr, cInfoLog);
					std::cout << "ERROR::PROGRAM::LINKING_ERROR\n" << cInfoLog;
					std::cout << "-----------------------------------------------------------------------";
				}
			}
		}
	};
}
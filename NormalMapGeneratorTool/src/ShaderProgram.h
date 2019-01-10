#pragma once
#include <string>
#include <GLM\glm.hpp>
class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();
	//Compiler vertex shader and fragment shader
	void compileShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	//Link shaders and create program
	void linkShaders();
	void addAttribute(const std::string& attributeName);
	void use();
	void unuse();
	int getUniformLocation(const std::string& uniformName)const;
	static void applyShaderUniformMatrix(int uniformId, const glm::mat4& matrixValue);
	static void applyShaderVector3(int uniformId, const glm::vec3& value);
	static void applyShaderFloat(int uniformId, float value);
	static void applyShaderInt(int uniformId, int value);
	static void applyShaderBool(int uniformId, bool value);
private:
	unsigned int programID, vertexShaderID, fragmentShaderID;
	void compileShader(const std::string& filePath, unsigned int ID);
	int attributeCount;
};


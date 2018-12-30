#pragma once
#include <GLM\common.hpp>
class UndoRedoSystem
{
private:
	unsigned unsigned char* data = nullptr;
	unsigned int maxAllocatedMemoryInBytes = 512 * 512 * 4 * 20; //Can store 20 512 x 512 images of 4 component count (RGBA)
	unsigned int bytesPerSection = 512 * 512 * 4;
	unsigned int sectionsFilled = 0;
public:
	UndoRedoSystem(unsigned int maxMemoryToAllocate, unsigned int bytesPerSection);
	UndoRedoSystem(const glm::vec2& sampleImageRes, unsigned int componentCount, unsigned int numberOfUndoSteps);
	const unsigned int getMaxUndoSteps(const glm::vec2& sampleImageRes);
	void record(unsigned char* data);
	unsigned char* retrieve();
	~UndoRedoSystem();
};


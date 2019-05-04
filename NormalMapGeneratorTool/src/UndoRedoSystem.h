#pragma once
#include <GLM\common.hpp>
class UndoRedoSystem
{
private:
	unsigned char* data = nullptr;
	unsigned int maxAllocatedMemoryInBytes = 512 * 512 * 4 * 20; //Ex: Can store 20 512 x 512 images of 4 component count (RGBA)
	unsigned int bytesPerSection = 512 * 512 * 4;
	int currentSection = 0;
	int maxSectionsFilled = 0;
public:
	UndoRedoSystem();
	UndoRedoSystem(unsigned int maxMemoryToAllocate, unsigned int bytesPerSection);
	UndoRedoSystem(const glm::ivec2& sampleImageRes, unsigned int componentCount, unsigned int numberOfUndoSteps);
	UndoRedoSystem(const UndoRedoSystem& undoRedo);
	void updateAllocation(const glm::ivec2& sampleImageRes, unsigned int componentCount, unsigned int numberOfUndoSteps);
	const unsigned int getMaxUndoSteps();
	const unsigned int getCurrentSectionPosition();
	unsigned int getMaxSectionsFilled();
	void record(unsigned char* data);
	unsigned char* retrieve(bool grabPrevious = true);
	void clear();
	~UndoRedoSystem();
};
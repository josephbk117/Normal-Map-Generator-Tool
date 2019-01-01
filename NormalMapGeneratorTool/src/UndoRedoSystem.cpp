#include "UndoRedoSystem.h"
#include <iostream>

UndoRedoSystem::UndoRedoSystem(unsigned int maxMemoryToAllocate, unsigned int bytesPerSection)
{
	this->maxAllocatedMemoryInBytes = maxMemoryToAllocate;
	this->bytesPerSection = bytesPerSection;
	data = new unsigned char[maxMemoryToAllocate];
}

UndoRedoSystem::UndoRedoSystem(const glm::vec2 & sampleImageRes, unsigned int componentCount, unsigned int numberOfUndoSteps)
{
	this->bytesPerSection = sampleImageRes.x * sampleImageRes.y * componentCount;
	this->maxAllocatedMemoryInBytes = this->bytesPerSection * numberOfUndoSteps;
	data = new unsigned char[maxAllocatedMemoryInBytes];
}

const unsigned int UndoRedoSystem::getMaxUndoSteps(const glm::vec2& sampleImageRes)
{
	return maxAllocatedMemoryInBytes / bytesPerSection;
}

const unsigned int UndoRedoSystem::getCurrentSectionPosition()
{
	return sectionsFilled;
}

void UndoRedoSystem::record(unsigned char * data)
{
	if (sectionsFilled + 1 > maxAllocatedMemoryInBytes / bytesPerSection)
	{
		std::cout << "\nOut of UNDO/REDO Memory bounds (Upper bound)";
		return;
	}
	std::memcpy(this->data + sectionsFilled * bytesPerSection, data, bytesPerSection);
	++sectionsFilled;
}

unsigned char * UndoRedoSystem::retrieve()
{
	--sectionsFilled;
	if (sectionsFilled < 0)
	{
		std::cout << "\nOut of UNDO/REDO Memory bounds (Lower bound)";
		sectionsFilled = 0;
	}
	return data + sectionsFilled * bytesPerSection;
}

UndoRedoSystem::~UndoRedoSystem()
{
	delete[] data;
}

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

void UndoRedoSystem::record(unsigned char * data)
{
	std::memcpy(this->data, data, bytesPerSection);
}

unsigned char * UndoRedoSystem::retrieve()
{
	return data;
}

UndoRedoSystem::~UndoRedoSystem()
{
	delete[] data;
}

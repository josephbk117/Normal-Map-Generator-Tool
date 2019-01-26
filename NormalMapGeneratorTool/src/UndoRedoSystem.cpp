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

const unsigned int UndoRedoSystem::getMaxUndoSteps()
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
		std::memcpy(this->data, this->data + bytesPerSection, maxAllocatedMemoryInBytes - bytesPerSection);
		std::memcpy(this->data + (maxAllocatedMemoryInBytes - bytesPerSection), data, bytesPerSection);
		std::cout << "\nOut of UNDO/REDO Memory bounds (Upper bound)";
	}
	else
	{
		//std::memcpy(this->data + sectionsFilled * bytesPerSection, data, bytesPerSection);
	}
	sectionsFilled = glm::min(sectionsFilled + 1, (int)(maxAllocatedMemoryInBytes / bytesPerSection));
	if (maxSectionsFilled < sectionsFilled)
		maxSectionsFilled = sectionsFilled;
}

unsigned char * UndoRedoSystem::retrieve(bool grabPrevious)
{
	if (grabPrevious)
	{
		--sectionsFilled;
		if (sectionsFilled - 1 < 0)
		{
			std::cout << "\nOut of UNDO/REDO Memory bounds (Lower bound)";
			sectionsFilled = 1;
		}
	}
	else
	{
		sectionsFilled = glm::min(sectionsFilled + 1, (int)(maxAllocatedMemoryInBytes / bytesPerSection));
		if (sectionsFilled > maxSectionsFilled)
			sectionsFilled = maxSectionsFilled;
	}
	return data + (sectionsFilled - 1) * bytesPerSection;
}

UndoRedoSystem::~UndoRedoSystem()
{
	delete[] data;
}

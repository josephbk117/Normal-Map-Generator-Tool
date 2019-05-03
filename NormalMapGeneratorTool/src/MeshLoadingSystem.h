#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "ModelObject.h"

namespace MeshLoadingSystem
{
	class MeshLoader
	{
	public:
		MeshLoader() {}
		~MeshLoader() {}

		ModelObject* createModelFromFile(const std::string& Path)
		{
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(Path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
			// check for errors
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
				std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;

			aiMesh* mesh = scene->mMeshes[0];
			float dist = maxVertexDistance(mesh);
			float scaleMultiplier = (dist < 3.0f) ? 1.0f : 3.0f / dist;

			unsigned int numVertices = mesh->mNumVertices;
			float *vertexData = new float[numVertices * 14];

			int count = 0;
			for (int i = 0; i < numVertices; i++)
			{

				vertexData[count] = mesh->mVertices[i].x * scaleMultiplier;
				vertexData[count + 1] = mesh->mVertices[i].y * scaleMultiplier;
				vertexData[count + 2] = mesh->mVertices[i].z * scaleMultiplier;

				vertexData[count + 3] = mesh->mNormals[i].x;
				vertexData[count + 4] = mesh->mNormals[i].y;
				vertexData[count + 5] = mesh->mNormals[i].z;

				vertexData[count + 6] = mesh->mTextureCoords[0][i].x;
				vertexData[count + 7] = mesh->mTextureCoords[0][i].y;

				vertexData[count + 8] = mesh->mTangents[i].x;
				vertexData[count + 9] = mesh->mTangents[i].y;
				vertexData[count + 10] = mesh->mTangents[i].z;

				vertexData[count + 11] = mesh->mBitangents[i].x;
				vertexData[count + 12] = mesh->mBitangents[i].y;
				vertexData[count + 13] = mesh->mBitangents[i].z;

				count += 14;
			}
			std::vector<unsigned int> indices;
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}
			ModelObject* modelObj = new ModelObject();
			modelObj->updateMeshData(vertexData, numVertices * 14 * sizeof(float), &indices[0], indices.size());
			delete[] vertexData;
			return modelObj;
		}
	private:
		//The maximum distance a vertex is from origin, Useful for Bounding spheres
		float maxVertexDistance(aiMesh* mesh)const
		{
			unsigned int numVertices = mesh->mNumVertices;

			float maxDist = 0;
			aiVector3D* vecArray = mesh->mVertices;
			for (unsigned int i = 0; i < numVertices; i++)
			{
				glm::vec3 vec(vecArray[i].x, vecArray[i].y, vecArray[i].z);
				float dist = glm::length(vec);
				if (dist > maxDist)
					maxDist = dist;
			}
			return maxDist;
		}
	};
}

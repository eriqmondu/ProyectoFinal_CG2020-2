#ifndef MODEL_HM
#define MODEL_HM

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh_m.h"
#include "shader_m.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <stdlib.h>
using namespace std;

#include <glm/gtx/string_cast.hpp>

unsigned int TextureFromFileAnim(const char *path, const string &directory, bool gamma = false);

struct BoneInfo
{
	aiMatrix4x4 BoneOffset;
	aiMatrix4x4 FinalTransformation;

	BoneInfo()
	{
		aiMatrix4x4::Scaling(aiVector3D(0.0),BoneOffset);
		aiMatrix4x4::Scaling(aiVector3D(0.0), FinalTransformation);
	}
};

struct VertexBoneData
{
	unsigned int IDs[2*MAX_NUM_BONES];
	float Weights[2*MAX_NUM_BONES];
	int numBones;

	VertexBoneData()
	{
		Reset();
	};

	void Reset()
	{
		numBones = 0;
		for (int i = 0; i < 2 * MAX_NUM_BONES; i++) // ...initialize it
		{
			IDs[i] = 0;
			Weights[i] = 0.0f;
		}
	}

	void AddBoneData(unsigned int BoneID, float Weight) {
		if (numBones < 2 * MAX_NUM_BONES) {
			IDs[numBones] = BoneID;
			Weights[numBones] = Weight;
			numBones++;
		}
	}
};

struct Bone {
	aiString name;
	vector<unsigned int>    IDs;
	vector<float>           weights;
	glm::mat4               transformation;
	glm::mat4               offsetMatrix;

	Bone() {
		name = "";
		transformation = glm::mat4(1.0f);
		offsetMatrix   = glm::mat4(1.0f);
	};

	void push(unsigned int ID, float w) {
		IDs.push_back(ID);
		weights.push_back(w);
	}

};

class ModelAnim 
{
public:
    /*  ModelAnim Data */
    vector<TextureAnim> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<MeshAnim> meshes;
    string directory;
    bool gammaCorrection;

	string filename;

	/* Bones data */
	vector<Bone> bones;

	/* Scene data */
	Assimp::Importer importer;
	const aiScene* scene;
	
	map<string, unsigned int> m_BoneMapping; // maps a bone name to its index
	unsigned int m_NumBones;
	vector<BoneInfo> m_BoneInfo;
	glm::mat4 m_GlobalInverseTransform;

    /*  Functions   */
    // constructor, expects a filepath to a 3D model.
    ModelAnim(string const &path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(ShaderAnim shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

	// update transformations in time 
	void SetPose(float time, glm::mat4 *gBones) {
		
		processNode(scene->mRootNode, time);

		for (int b = 0; b < 100; b++) {
			gBones[b] = glm::mat4(1.0f);
		}

		for (unsigned int i = 0; i < bones.size(); i++) {
			if (i < 100) {
				gBones[i] = bones[i].transformation;
				//cout << "bone: " << i << " : " << bones[i].name.data << " T= " << glm::to_string(bones[i].transformation) << endl;
			}
		}
	}

	// Return the duration of the animation in ticks (frames)
	double getNumFrames() {

		const aiAnimation* pAnimation = scene->mAnimations[0];
		if (pAnimation == nullptr) return -1.0;

		cout << "Animation total frames:" << pAnimation->mDuration << endl;

		return pAnimation->mDuration;

	}

	// return the number of ticks per second
	double getFramerate() {
		const aiAnimation* pAnimation = scene->mAnimations[0];
		if (pAnimation == nullptr) return -1.0;

		cout << "Animation framerate:" << pAnimation->mTicksPerSecond << " fps" << endl;

		return  pAnimation->mTicksPerSecond;
	}

private:

    /*  Functions   */

	inline glm::mat4 aiMatrix4x4ToGlm(aiMatrix4x4 from)
	{
		glm::mat4 to;

		to[0][0] = (GLfloat)from.a1; to[0][1] = (GLfloat)from.b1;  to[0][2] = (GLfloat)from.c1; to[0][3] = (GLfloat)from.d1;
		to[1][0] = (GLfloat)from.a2; to[1][1] = (GLfloat)from.b2;  to[1][2] = (GLfloat)from.c2; to[1][3] = (GLfloat)from.d2;
		to[2][0] = (GLfloat)from.a3; to[2][1] = (GLfloat)from.b3;  to[2][2] = (GLfloat)from.c3; to[2][3] = (GLfloat)from.d3;
		to[3][0] = (GLfloat)from.a4; to[3][1] = (GLfloat)from.b4;  to[3][2] = (GLfloat)from.c4; to[3][3] = (GLfloat)from.d4;

		return to;
	}

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
        // read file via ASSIMP
		scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
		filename = path;
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

		aiMatrix4x4 inverseTransform = scene->mRootNode->mTransformation;
		inverseTransform.Inverse();
		m_GlobalInverseTransform = aiMatrix4x4ToGlm(inverseTransform);

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

	void processNode(aiNode *node, float time){
		
		if (node == nullptr) return;

		glm::mat4 n_matrix(1.0f);
		ReadNodeHierarchy(time, node, n_matrix);

		//for (unsigned int i = 0; i < node->mNumChildren; i++)
		//{
		//	processNode(node->mChildren[i], time);
		//}
	}

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
		// cout << "Meshes: " << node->mNumMeshes << endl;
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			// cout << "Mesh: " << mesh->mName.data << endl;
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    MeshAnim processMesh(aiMesh *mesh, const aiScene *scene)
    {
        // data to fill
        vector<VertexAnim> vertices;
        vector<unsigned int> indices;
        vector<TextureAnim> textures;

		cout << "NumVertices: " << mesh->mNumVertices << endl;

        // Walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            VertexAnim vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
            
			// Bones
			int bcount = 0;
			for (unsigned int pb = 0; pb < 4; pb++) {
				vertex.IDs1[pb] = 0;
				vertex.IDs2[pb] = 0;
				vertex.IDs3[pb] = 0;
				vertex.Weights1[pb] = 0.0f; //Sin asociación a los huesos
				vertex.Weights2[pb] = 0.0f;
				vertex.Weights3[pb] = 0.0f;
			}
			m_NumBones = mesh->mNumBones;
			for (unsigned int j = 0; j < mesh->mNumBones; j++) {

				for (unsigned int k = 0; k < mesh->mBones[j]->mNumWeights; k++) {
					unsigned int VertexID = mesh->mBones[j]->mWeights[k].mVertexId;
					float Weight = (float)(mesh->mBones[j]->mWeights[k].mWeight);
					if ( VertexID == i && bcount < MAX_NUM_BONES ) {
						vertex.IDs1[bcount] = j;
						vertex.Weights1[bcount] = Weight;
						bcount++;
						/*cout << "Vertex " << VertexID << ": Bone: " << j
						<< ": W = " << Weight << endl;*/
					}
					if (VertexID == i && bcount >= MAX_NUM_BONES && bcount < 2*MAX_NUM_BONES) {
						vertex.IDs2[bcount - MAX_NUM_BONES] = j;
						vertex.Weights2[bcount - MAX_NUM_BONES] = Weight;
						bcount++;
						/*cout << "Vertex " << VertexID << ": Bone: " << j
							<< ": W = " << Weight << endl;*/
					}
					if (VertexID == i && bcount >= 2 * MAX_NUM_BONES && bcount < 3 * MAX_NUM_BONES) {
						vertex.IDs3[bcount - 2*MAX_NUM_BONES] = j;
						vertex.Weights3[bcount - 2*MAX_NUM_BONES] = Weight;
						bcount++;
						/*cout << "Vertex " << VertexID << ": Bone: " << j
							<< ": W = " << Weight << endl;*/
					}
				}
			}
			vertices.push_back(vertex);
			// cout << "Vertex " << i << ": " << glm::to_string(vertex.IDs1) << glm::to_string(vertex.IDs2) << glm::to_string(vertex.IDs3)  << endl;
			// cout << "Vertex " << i << ": " << glm::to_string(vertex.Weights1) << glm::to_string(vertex.Weights2) << glm::to_string(vertex.Weights3) << endl;
        }
		cout << "Vertex readed: " << vertices.size() << endl;

		// Process Bones
		cout << "Num bones readed: " << mesh->mNumBones << endl;
		bones.clear();
		for (unsigned int i = 0; i < mesh->mNumBones; i++) {

			Bone  newBone;
			newBone.name = mesh->mBones[i]->mName;
			newBone.offsetMatrix = aiMatrix4x4ToGlm(mesh->mBones[i]->mOffsetMatrix);
			newBone.transformation = glm::mat4(1.0f);

			for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
				unsigned int VertexID = mesh->mBones[i]->mWeights[j].mVertexId;
				float Weight = mesh->mBones[i]->mWeights[j].mWeight;
				newBone.push( VertexID, Weight );
			}

			bones.push_back(newBone);
		}
		cout << "NumFaces: " << mesh->mNumFaces << endl;

        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<TextureAnim> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<TextureAnim> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<TextureAnim> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<TextureAnim> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
        // return a mesh object created from the extracted mesh data
        return MeshAnim(vertices, indices, textures);
    }

	void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, glm::mat4& ParentTransform)
	{
		if (pNode == nullptr || scene == nullptr) return;

		string NodeName(pNode->mName.data);

		const aiAnimation* pAnimation = scene->mAnimations[0];
		
		if (pAnimation == nullptr) return;

		aiMatrix4x4 NodeTransformation(pNode->mTransformation);

		const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

		if (pNodeAnim != nullptr) {
			aiVector3D Scaling;
			CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
			aiMatrix4x4 ScalingM;
			aiMatrix4x4::Scaling(Scaling, ScalingM);

			aiQuaternion RotationQ;
			CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
			aiMatrix4x4 RotationM(RotationQ.GetMatrix());

			aiVector3D Translation;
			CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
			aiMatrix4x4 TranslationM;
			aiMatrix4x4::Translation(Translation, TranslationM);

			// Combine the above transformations
			NodeTransformation = TranslationM * RotationM * ScalingM;

			// cout << "Compute Node Transformation: " << pNodeAnim->mNodeName.data << endl;
		}

		glm::mat4 GlobalTransformation = ParentTransform * aiMatrix4x4ToGlm(NodeTransformation);

		//cout << "GT=" << glm::to_string(GlobalTransformation) << endl;

		// cout << "Parent: " << NodeName << endl;

		// Modify bones transformation
		for (unsigned int b = 0; b < bones.size(); b++) {
			if (bones[b].name == pNode->mName) {
				bones[b].transformation =m_GlobalInverseTransform * GlobalTransformation * bones[b].offsetMatrix;
				// cout << "bone: " << b << " : " << bones[b].name.data << " T= " << glm::to_string(bones[b].transformation) << endl;
			}
		}

		for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
			// cout << "Child: " << pNode->mChildren[i]->mName.data << ": " << i << endl;
			ReadNodeHierarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
		}
	}

	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName)
	{
		for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
			const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

			if (string(pNodeAnim->mNodeName.data) == NodeName) {
				return pNodeAnim;
			}
		}

		return nullptr;
	}


	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		if (pNodeAnim->mNumScalingKeys == 1) {
			Out = pNodeAnim->mScalingKeys[0].mValue;
			return;
		}

		unsigned int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
		unsigned int NextScalingIndex = (ScalingIndex + 1);
		assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
		float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
		const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
		aiVector3D Delta = End - Start;
		Out = Start + Factor * Delta;
	}

	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumScalingKeys > 0);

		for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);

		return 0;
	}

	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumRotationKeys == 1) {
			Out = pNodeAnim->mRotationKeys[0].mValue;
			return;
		}

		unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
		unsigned int NextRotationIndex = (RotationIndex + 1);
		assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
		float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
		const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
		aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
		Out = Out.Normalize();
	}

	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumRotationKeys > 0);

		for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);

		return 0;
	}

	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);

		return 0;
	}

	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		if (pNodeAnim->mNumPositionKeys == 1) {
			Out = pNodeAnim->mPositionKeys[0].mValue;
			return;
		}

		unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
		unsigned int NextPositionIndex = (PositionIndex + 1);
		assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
		float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
		const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
		aiVector3D Delta = End - Start;
		Out = Start + Factor * Delta;
	}

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a TextureAnim struct.
    vector<TextureAnim> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
    {
        vector<TextureAnim> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                TextureAnim texture;
                texture.id = TextureFromFileAnim(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }
};


unsigned int TextureFromFileAnim(const char *path, const string &directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "TextureAnim failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
#endif
//
//  Model.cpp
//  ogl4
//
//  Created by Philipp Lensing on 21.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "Model.h"
#include "phongshader.h"
#include <list>
#include <fstream>

Model::Model()
    : pMeshes(NULL),
      MeshCount(0),
      pMaterials(NULL),
      MaterialCount(0),
      m_diffuseTextureOverride(NULL),
      m_normalTextureOverride(NULL)
{
    
}

Model::Model(const char* ModelFile, bool FitSize)
    : pMeshes(NULL),
      MeshCount(0),
      pMaterials(NULL),
      MaterialCount(0),
      m_diffuseTextureOverride(NULL),
      m_normalTextureOverride(NULL)
{   
    bool ret = load(ModelFile, FitSize);
    if(!ret)
        throw std::exception();
}
Model::~Model()
{
	delete[] pMeshes;
	delete[] pMaterials;
    deleteNodes(&RootNode);
}

void Model::diffuseTextureOverride(const Texture* texture)
{
    m_diffuseTextureOverride = texture;
}

void Model::normalTextureOverride(const Texture* texture)
{
    m_normalTextureOverride = texture;
}

void Model::deleteNodes(Node* pNode)
{
    if(!pNode)
        return;
    for(unsigned int i=0; i<pNode->ChildCount; ++i)
        deleteNodes(&(pNode->Children[i]));
    if(pNode->ChildCount>0)
        delete [] pNode->Children;
    if(pNode->MeshCount>0)
        delete [] pNode->Meshes;
}

bool Model::load(const char* ModelFile, bool FitSize)
{
    unsigned int importFlags = aiProcessPreset_TargetRealtime_Fast | aiProcess_TransformUVCoords;

    std::string modelPath = ModelFile;

    std::transform(modelPath.begin(),
                   modelPath.end(),
                   modelPath.begin(),
                   [](unsigned char character)
                   {
                       return static_cast<char>(std::tolower(character));
                   });

    bool isGltf =
        (modelPath.size() >= 5 && modelPath.compare(modelPath.size() - 5, 5, ".gltf") == 0)
        || (modelPath.size() >= 4 && modelPath.compare(modelPath.size() - 4, 4, ".glb") == 0);

    if (isGltf)
    {
        importFlags |= aiProcess_FlipUVs | aiProcess_PreTransformVertices;
    }

    const aiScene* pScene = aiImportFile(ModelFile, importFlags);    
    if(pScene==NULL || pScene->mNumMeshes<=0)
        return false;
    
    Filepath = ModelFile;
    Path = Filepath;
    size_t pos = Filepath.rfind('/');
    if(pos == std::string::npos)
        pos = Filepath.rfind('\\');
    if(pos !=std::string::npos)
        Path.resize(pos+1);
    
    loadMeshes(pScene, FitSize);
    loadMaterials(pScene);
    loadNodes(pScene);
    
    return true;
}

void Model::loadMeshes(const aiScene* pScene, bool FitSize)
{
    this->m_localVertices.clear();

    BoundingBox = AABB();
    calcBoundingBox(pScene, BoundingBox);

    float scale = 1.0f;
    if (FitSize)
    {
        Vector size = BoundingBox.Max - BoundingBox.Min;
        float maxDim = std::max(size.X, std::max(size.Y, size.Z));
        if (maxDim > 0.0f) {
            scale = 5.0f / maxDim;
        }

        BoundingBox.Min.X *= scale;
        BoundingBox.Min.Y *= scale;
        BoundingBox.Min.Z *= scale;

        BoundingBox.Max.X *= scale;
        BoundingBox.Max.Y *= scale;
        BoundingBox.Max.Z *= scale;
    }

    MeshCount = pScene->mNumMeshes;
    pMeshes = new Mesh[MeshCount];

    for (unsigned int i = 0; i < MeshCount; ++i)
    {
        aiMesh* pMesh = pScene->mMeshes[i];
        Mesh& mesh = pMeshes[i];
        mesh.MaterialIdx = pMesh->mMaterialIndex;
        mesh.VB.begin();

        for (unsigned int v = 0; v < pMesh->mNumVertices; ++v)
        {
            if (pMesh->mNormals)
                mesh.VB.addNormal(pMesh->mNormals[v].x, pMesh->mNormals[v].y, pMesh->mNormals[v].z);

            if (pMesh->mTextureCoords[0])
                mesh.VB.addTexcoord0(pMesh->mTextureCoords[0][v].x, pMesh->mTextureCoords[0][v].y);

            if (pMesh->HasTangentsAndBitangents())
            {
                mesh.VB.addTexcoord1(pMesh->mTangents[v].x, pMesh->mTangents[v].y, pMesh->mTangents[v].z);
                mesh.VB.addTexcoord2(pMesh->mBitangents[v].x, pMesh->mBitangents[v].y, pMesh->mBitangents[v].z);
			}

            if (pMesh->mVertices)
            {
                Vector position(pMesh->mVertices[v].x * scale,
                                pMesh->mVertices[v].y * scale,
                                pMesh->mVertices[v].z * scale);

                mesh.VB.addVertex(position.X, position.Y, position.Z);

                this->m_localVertices.push_back(position);
            }
        }
        mesh.VB.end();

        mesh.IB.begin();
        for (unsigned int f = 0; f < pMesh->mNumFaces; ++f)
        {
            aiFace& Face = pMesh->mFaces[f];
            mesh.IB.addIndex(Face.mIndices[0]);
            mesh.IB.addIndex(Face.mIndices[1]);
            mesh.IB.addIndex(Face.mIndices[2]);
        }
        mesh.IB.end();
    }
}

void Model::loadMaterials(const aiScene* pScene)
{
    MaterialCount = pScene->mNumMaterials;
    pMaterials = new Material[pScene->mNumMaterials];

    for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
    {
        aiMaterial* pMat = pScene->mMaterials[i];
        Material& Mat = pMaterials[i];

        Mat.DiffTex = NULL;
        Mat.NormalTex = NULL;

       aiColor3D Color3D;

        if (pMat->Get(AI_MATKEY_COLOR_DIFFUSE, Color3D) == AI_SUCCESS)
        {
            Mat.DiffColor = Color(Color3D.r, Color3D.g, Color3D.b);
        }

        if (pMat->Get(AI_MATKEY_COLOR_SPECULAR, Color3D) == AI_SUCCESS)
        {
            Mat.SpecColor = Color(Color3D.r, Color3D.g, Color3D.b);
        }

        if (pMat->Get(AI_MATKEY_COLOR_AMBIENT, Color3D) == AI_SUCCESS)
        {
            Mat.AmbColor = Color(Color3D.r, Color3D.g, Color3D.b);
        }
        else
        {
            Mat.AmbColor = Mat.DiffColor * 0.15f;
        }

        float SpecExp;

        if (pMat->Get(AI_MATKEY_SHININESS, SpecExp) == AI_SUCCESS)
        {
            Mat.SpecExp = SpecExp;
        }

        aiString diffusePath;

        if (pMat->GetTexture(aiTextureType_DIFFUSE, 0, &diffusePath) == AI_SUCCESS)
        {
            if (diffusePath.length > 0 && diffusePath.C_Str()[0] != '*')
            {
                std::string texturePath = Path + diffusePath.C_Str();

                Mat.DiffTex = Texture::LoadShared(texturePath.c_str());
            }
        }

        aiString normalPath;

        if (pMat->GetTexture(aiTextureType_NORMALS, 0, &normalPath) == AI_SUCCESS)
        {
            if (normalPath.length > 0 && normalPath.C_Str()[0] != '*')
            {
                std::string texturePath = Path + normalPath.C_Str();

                Mat.NormalTex = Texture::LoadShared(texturePath.c_str());
            }
        }

        if (Mat.NormalTex == NULL && diffusePath.length > 0 && diffusePath.C_Str()[0] != '*')
        {
            std::string normalPath = Path + diffusePath.C_Str();

            size_t dot = normalPath.rfind('.');

            if (dot != std::string::npos)
            {
                normalPath.insert(dot, "_n");

                std::ifstream file(normalPath.c_str());

                if (file.good())
                {
                    Mat.NormalTex = Texture::LoadShared(normalPath.c_str());
                }
            }
        }
    }
}

void Model::calcBoundingBox(const aiScene* pScene, AABB& Box)
{
    Box.Min = Vector(1e7f, 1e7f, 1e7f);
    Box.Max = Vector(-1e7f, -1e7f, -1e7f);

    for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
    {
        aiMesh* pMesh = pScene->mMeshes[i];
        for (unsigned int v = 0; v < pMesh->mNumVertices; ++v)
        {
            if (pMesh->mVertices)
            {
                Vector Vertex(pMesh->mVertices[v].x, pMesh->mVertices[v].y, pMesh->mVertices[v].z);
                Box.Min.X = std::min(Box.Min.X, Vertex.X);
                Box.Min.Y = std::min(Box.Min.Y, Vertex.Y);
                Box.Min.Z = std::min(Box.Min.Z, Vertex.Z);
                Box.Max.X = std::max(Box.Max.X, Vertex.X);
                Box.Max.Y = std::max(Box.Max.Y, Vertex.Y);
                Box.Max.Z = std::max(Box.Max.Z, Vertex.Z);
            }
        }
    }
}

void Model::loadNodes(const aiScene* pScene)
{
    deleteNodes(&RootNode);
    copyNodesRecursive(pScene->mRootNode, &RootNode);
}

void Model::copyNodesRecursive(const aiNode* paiNode, Node* pNode)
{
    pNode->Name = paiNode->mName.C_Str();
    pNode->Trans = convert(paiNode->mTransformation);
    
    if(paiNode->mNumMeshes > 0)
    {
        pNode->MeshCount = paiNode->mNumMeshes;
        pNode->Meshes = new int[pNode->MeshCount];
        for(unsigned int i=0; i<pNode->MeshCount; ++i)
            pNode->Meshes[i] = (int)paiNode->mMeshes[i];
    }
    
    if(paiNode->mNumChildren <=0)
        return;
    
    pNode->ChildCount = paiNode->mNumChildren;
    pNode->Children = new Node[pNode->ChildCount];
    for(unsigned int i=0; i<paiNode->mNumChildren; ++i)
    {
        copyNodesRecursive(paiNode->mChildren[i], &(pNode->Children[i]));
        pNode->Children[i].Parent = pNode;
    }
}

void Model::applyMaterial( unsigned int index)
{
    if(index>=MaterialCount)
        return;
    
    PhongShader* pPhong = dynamic_cast<PhongShader*>(shader());
    if(!pPhong) {
        return;
    }
    
    Material* pMat = &pMaterials[index];
    pPhong->ambientColor(pMat->AmbColor);
    pPhong->diffuseColor(pMat->DiffColor);
    pPhong->specularExp(pMat->SpecExp);
    pPhong->specularColor(pMat->SpecColor);

    auto diffuseOverride = m_diffuseTextureOverrides.find(index);

    if (diffuseOverride != m_diffuseTextureOverrides.end())
    {
        pPhong->diffuseTexture(diffuseOverride->second);
    }
    else if (m_diffuseTextureOverride != NULL)
    {
        pPhong->diffuseTexture(m_diffuseTextureOverride);
    }
    else
    {
        pPhong->diffuseTexture(pMat->DiffTex);
    }

    if (m_normalTextureOverride != NULL)
    {
        pPhong->normalTexture(m_normalTextureOverride);
    }
    else
    {
        pPhong->normalTexture(pMat->NormalTex);
    }
}

void Model::draw(const BaseCamera& Cam)
{
    if(!pShader) {
        std::cout << "BaseModel::draw() no shader found" << std::endl;
        return;
    }
    pShader->modelTransform(transform());
    
    std::list<Node*> DrawNodes;
    DrawNodes.push_back(&RootNode);
    
    while(!DrawNodes.empty())
    {
        Node* pNode = DrawNodes.front();
        Matrix GlobalTransform;
        
        if(pNode->Parent != NULL)
            pNode->GlobalTrans = pNode->Parent->GlobalTrans * pNode->Trans;
        else
            pNode->GlobalTrans = transform() * pNode->Trans;
        
        pShader->modelTransform(pNode->GlobalTrans);
    
        for(unsigned int i = 0; i<pNode->MeshCount; ++i )
        {
            Mesh& mesh = pMeshes[pNode->Meshes[i]];
            mesh.VB.activate();
            mesh.IB.activate();
            applyMaterial(mesh.MaterialIdx);
            pShader->activate(Cam);
            glDrawElements(GL_TRIANGLES, mesh.IB.indexCount(), mesh.IB.indexFormat(), 0);
            mesh.IB.deactivate();
            mesh.VB.deactivate();
        }
        for(unsigned int i = 0; i<pNode->ChildCount; ++i )
            DrawNodes.push_back(&(pNode->Children[i]));
        
        DrawNodes.pop_front();
    }
}

Matrix Model::convert(const aiMatrix4x4& m)
{
    return Matrix(m.a1, m.a2, m.a3, m.a4,
                  m.b1, m.b2, m.b3, m.b4,
                  m.c1, m.c2, m.c3, m.c4,
                  m.d1, m.d2, m.d3, m.d4);
}

void Model::diffuseTextureOverride(unsigned int materialIndex, const Texture* texture)
{
    m_diffuseTextureOverrides[materialIndex] = texture;
}

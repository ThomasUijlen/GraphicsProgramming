#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "stb_image.h"
#include <map>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 uv;
};


class Model {
private:
    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
        GLuint abledoTexture;
        GLuint normalTexture;
        GLuint roughnessTexture;

        Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, GLuint abledoTexture, GLuint normalTexture, GLuint roughnessTexture)
            : vertices(vertices), indices(indices), abledoTexture(abledoTexture), normalTexture(normalTexture), roughnessTexture(roughnessTexture) {
            // Create buffers/arrays
            glGenVertexArrays(1, &this->vao);
            glGenBuffers(1, &this->vbo);
            glGenBuffers(1, &this->ebo);

            glBindVertexArray(this->vao);

            // Load data into vertex buffers
            glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
            glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), &this->indices[0], GL_STATIC_DRAW);

            // Set the vertex attribute pointers
            // Vertex Positions
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            // Vertex Normals
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            // Vertex Tangents
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
            // Vertex Bitangents
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
            // Vertex Texture Coords
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));


            glBindVertexArray(0);
        }
    };

    std::vector<Model::Mesh> meshes;
    GLuint program;
    std::string directory;
    std::map<std::string, GLuint> textureCache;


    unsigned int loadTexture(const char* filename);
    Model::Mesh processMesh(aiMesh* mesh, const aiScene* scene);

public:
    Model(const std::string& path, GLuint program);
    void render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, glm::vec3& ambientLightColor, glm::vec3& lightDirection);
};

unsigned int Model::loadTexture(const char* filename) {
    auto it = textureCache.find(filename);
    if (it != textureCache.end()) {
        return it->second;
    }

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture wrapping and filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image file
    int width, height, numChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename, &width, &height, &numChannels, 0);
    if (data)
    {
        // Determine the image format based on the number of channels
        GLenum format;
        if (numChannels == 1)
            format = GL_RED;
        else if (numChannels == 3)
            format = GL_RGB;
        else if (numChannels == 4)
            format = GL_RGBA;

        // Generate the texture and bind it
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Free the image data
        stbi_image_free(data);
    }
    else
    {
        // If loading the image failed, print an error message and return 0
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);

    textureCache[filename] = textureID;

    return textureID;
}

Model::Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    GLuint abledoTexture = 0;
    GLuint normalTexture = 0;
    GLuint roughnessTexture = 0;

    // Process vertices
    vertices.resize(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        // position
        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        // normal
        vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        // tangent
        vertex.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        // bitangent
        vertex.bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        // texture coordinates
        if (mesh->mTextureCoords[0])
            vertex.uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        else
            vertex.uv = glm::vec2(0.0f, 0.0f);

        vertices[i] = vertex;
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // Process material
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiString str;

        // Check for textures
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &str) == AI_SUCCESS)
            abledoTexture = loadTexture((directory + "/" + str.C_Str()).c_str());
        if (material->GetTexture(aiTextureType_HEIGHT, 0, &str) == AI_SUCCESS)
            normalTexture = loadTexture((directory + "/" + str.C_Str()).c_str());
        if (material->GetTexture(aiTextureType_SHININESS, 0, &str) == AI_SUCCESS)
            roughnessTexture = loadTexture((directory + "/" + str.C_Str()).c_str());
    }

    return Model::Mesh(vertices, indices, abledoTexture, normalTexture, roughnessTexture);
}

Model::Model(const std::string& path, GLuint program) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));

    // Process all the meshes in the scene
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
        meshes.push_back(processMesh(scene->mMeshes[i], scene));

    // save the shader program
    this->program = program;
}

void Model::render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, glm::vec3& ambientLightColor, glm::vec3& lightDirection) {
    for (auto& mesh : meshes) {
        // Use the shader program
        glUseProgram(program);

        // Pass the transformation matrices to the shader
        GLint modelLoc = glGetUniformLocation(program, "model");
        GLint viewLoc = glGetUniformLocation(program, "view");
        GLint projLoc = glGetUniformLocation(program, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Bind textures and pass them to the shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.abledoTexture);
        glUniform1i(glGetUniformLocation(program, "albedoTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mesh.normalTexture);
        glUniform1i(glGetUniformLocation(program, "normalTexture"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mesh.roughnessTexture);
        glUniform1i(glGetUniformLocation(program, "specularTexture"), 2);

        glUniform3fv(glGetUniformLocation(program, "ambientLightColor"), 1, glm::value_ptr(ambientLightColor));
        glUniform3fv(glGetUniformLocation(program, "lightDirection"), 1, glm::value_ptr(lightDirection));

        glm::vec3 cameraPosition = glm::vec3(view[3][0], view[3][1], view[3][2]);
        glUniform3fv(glGetUniformLocation(program, "viewPos"), 1, glm::value_ptr(cameraPosition));

        // Bind vertex array object
        glBindVertexArray(mesh.vao);

        // Render the mesh
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

        // Unbind to cleanup
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}


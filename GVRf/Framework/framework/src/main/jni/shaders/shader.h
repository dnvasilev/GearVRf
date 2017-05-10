/* Copyright 2015 Samsung Electronics Co., LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/***************************************************************************
 * A shader which an user can add in run-time.
 ***************************************************************************/

#ifndef SHADER_H_
#define SHADER_H_

#include <map>
#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include "unordered_map"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "objects/uniform_block.h"

namespace gvr {
    struct uniformDefination
    {
        std::string type;
        int size;
    };
class Mesh;
/**
 * Contains information about the vertex attributes, textures and
 * uniforms used by the shader source and the sources for the
 * vertex and fragment shaders.
 *
 * Shaders are only created by the ShaderManager when addShader is called.
 */
class Shader
{
public:
    static const bool LOG_SHADER;

/*
 * Facilitates visiting the individual elements of a descriptor string.
 * Used to visit the textures and vertex attributes
 */
    class ShaderVisitor
    {
    protected:
        Shader* shader_;

    public:
        ShaderVisitor(Shader* shader) : shader_(shader) { };
        virtual void visit(const std::string& key, const std::string& type, int size) = 0;
    };

/*
 * Creates a native shader description.
 * The actual GL program is not made until the first call to render()
 * @param id                ShaderManager ID for the shader
 * @param signature         Unique signature string
 * @param uniformDescriptor String giving the names and types of shader material uniforms
 *                          This does NOT include uniforms used by light sources
 * @param textureDescriptor String giving the names and types of texture samplers
 * @param vertexDescriptor  String giving the names and types of vertex attributes
 * @param vertexShader      String with GLSL source for vertex shader
 * @param fragmentShader    String with GLSL source for fragment shader
 * @see ShaderManager::addShader
 */
    Shader(int id, const std::string& signature,
            const std::string& uniformDescriptor,
            const std::string& textureDescriptor,
            const std::string& vertexDescriptor,
            const std::string& vertexShader,
            const std::string& fragmentShader);
    virtual ~Shader() { };

    /*
     * Returns the unique signature for this shader (provided
     * to ShaderManager::addShader when this Shader was created).
     */
    const std::string& signature() const { return signature_; }

    /*
     * Returns the ShaderManager ID (generated by addShader)
     */
    int getShaderID() const { return id_; }

    /*
     *  returns the vertex attributes string
     */
    const std::string& getVertexDescriptor() const
    {
        return vertexDescriptor_;
    }
    /*
    *  returns the vertex attributes string
    */
    const std::string& getTextureDescriptor() const
    {
        return textureDescriptor_;
    }
    /*
 *  returns the vertex attributes string
 */
    const std::string& getUniformDescriptor() const
    {
        return uniformDescriptor_;
    }
    bool isTransformUboPresent(){
        return transformUboPresent;
    }
    bool isShaderDirty(){
        return  shaderDirty;
    }
    void setShaderDirty(bool flag){
        shaderDirty = flag;
    }
    virtual bool useShader(Mesh*) = 0;
    static int calcSize(std::string type);
    void forEach(const std::string& descriptor, ShaderVisitor& visitor);
    const std::unordered_map<std::string, std::string>& getTextures()
    {
        return textures_;
    }
    void parseDescriptor(const std::string& descriptor);
    int getNumberOfTextures()
    {
        return textures_.size();
    }
private:
    Shader(const Shader& shader);
    Shader(Shader&& shader);
    Shader& operator=(const Shader& shader);
    Shader& operator=(Shader&& shader);

protected:
    std::unordered_map<std::string,uniformDefination> nameTypeMap;
    std::string signature_;
    std::string vertexShader_;
    std::string fragmentShader_;
    std::string vertexDescriptor_;
    std::string textureDescriptor_;
    std::string uniformDescriptor_;
    std::unordered_map<std::string, std::string> textures_;
    int id_;
    bool shaderDirty = true;
    bool transformUboPresent = true;
};

}
#endif

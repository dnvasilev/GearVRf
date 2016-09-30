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
#include <set>
#include <memory>
#include <string>
#include <mutex>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "objects/eye_type.h"
#include "objects/hybrid_object.h"
#include "objects/light.h"
#include "objects/mesh.h"

namespace gvr {

struct ShaderUniformsPerObject;

typedef std::function<void(Mesh&, GLuint)> AttributeVariableBind;
typedef std::function<void(ShaderData&, GLuint)> UniformVariableBind;

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
 * Used to visit the textures, vertex attributes and uniforms
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
    explicit Shader(long id, const std::string& signature,
            const std::string& uniformDescriptor,
            const std::string& textureDescriptor,
            const std::string& vertexDescriptor,
            const std::string& vertexShader,
            const std::string& fragmentShader);
    virtual ~Shader();

    /*
     * Updates the state of the shader per frame.
     * Copies uniforms from the material into the GPU.
     * Binds textures and vertex buffers to the GPU.
     */
    virtual void render(RenderState* rstate, RenderData* render_data, ShaderData* material);

    /*
     * Returns the ShaderManager ID (generated by addShader)
     */
    long getShaderID() const { return id_; }

/*
 * Returns the unique signature for this shader (provided
 * to ShaderManager::addShader when this Shader was created).
 */
    const std::string& signature() const { return signature_; }

    /*
     * Returns the GL program ID for the native shader
     */
    GLuint getProgramId()
    {
        if (program_)
        {
            return program_->id();
        }
        else
        {
            return -1;
        }
    }

    /*
     * Returns the GL location for a vertex attribute, texture or uniform given its name.
     */
    GLuint getLocation(const std::string& key) {
        auto it = locations_.find(key);
        if (it != locations_.end()) {
            return it->second;
        }
        return -1;
    }
    /*
     * Sets the GL location for a named uniform, texture or vertex attribute.
     */
    void setLocation(const std::string& key, int loc) {
        locations_[key] = loc;
    }


    /*
     * TODO: This is for batching. Roshan should implement it.
     */
    void programInit(RenderState* rstate, RenderData* render_data, ShaderData* material,
                     const std::vector<glm::mat4>& model_matrix, int drawcount, bool batching) { }

private:
    Shader(const Shader& shader);
    Shader(Shader&& shader);
    Shader& operator=(const Shader& shader);
    Shader& operator=(Shader&& shader);

    void initializeOnDemand(RenderState* rstate, Mesh* mesh);
    void forEach(const std::string& descriptor, ShaderVisitor& visitor);
    static int calcSize(std::string type);
    bool hasUniform(const std::string& name) { return uniformDescriptor_.find(name) != std::string::npos; }
    bool hasTexture(const std::string& name) { return textureDescriptor_.find(name) != std::string::npos; }
    bool hasAttribute(const std::string& name) { return vertexDescriptor_.find(name) != std::string::npos; }

private:
    GLProgram* program_;
    std::string signature_;
    long id_;
    GLuint u_mvp_;
    GLuint u_mv_;
    GLuint u_view_;
    GLuint u_mv_it_;
    GLuint u_right_;
    GLuint u_model_;
    std::mutex textureVariablesLock_;
    std::mutex attributeVariablesLock_;
    std::mutex uniformVariablesLock_;
    std::string vertexShader_;
    std::string fragmentShader_;
    std::string vertexDescriptor_;
    std::string textureDescriptor_;
    std::string uniformDescriptor_;
    std::map<std::string, int> locations_;
};

}
#endif

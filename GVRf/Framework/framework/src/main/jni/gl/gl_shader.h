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

#ifndef GL_SHADER_H_
#define GL_SHADER_H_

#include "shaders/shader.h"
#include "gl/gl_program.h"

namespace gvr {
    class Mesh;
    class GLMaterial;

/**
 * Contains information about the vertex attributes, textures and
 * uniforms used by the shader source and the sources for the
 * vertex and fragment shaders.
 *
 * Shaders are only created by the ShaderManager when addShader is called.
 */
class GLShader : public Shader
{

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
public:
    explicit GLShader(int id, const std::string& signature,
            const std::string& uniformDescriptor,
            const std::string& textureDescriptor,
            const std::string& vertexDescriptor,
            const std::string& vertexShader,
            const std::string& fragmentShader);
    virtual ~GLShader();

    virtual bool useShader(Mesh*);

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
     * Returns the GL location for a vertex attribute or texture given its name.
     */
    GLuint getLocation(const std::string& key)
    {
        auto it = locations_.find(key);
        if (it != locations_.end())
        {
            return it->second;
        }
        return -1;
    }
    /*
     * Sets the GL location for a named texture or vertex attribute.
     */
    void setLocation(const std::string& key, int loc)
    {
        locations_[key] = loc;
    }
    void convertToGLShaders();
    int bindTextures(GLMaterial* material);

protected:
    void initialize(Mesh*);

private:
    GLShader(const GLShader& shader);
    GLShader(GLShader&& shader);
    GLShader& operator=(const GLShader& shader);
    GLShader& operator=(GLShader&& shader);

    GLProgram* program_;
    std::mutex attributeVariablesLock_;
    std::map<std::string, int> locations_;
    std::vector<int> mTextureLocs;
};

}
#endif

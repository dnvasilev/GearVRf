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

#include "gl/gl_shader.h"
#include "objects/mesh.h"
#include "engine/renderer/renderer.h"

namespace gvr {
    class AttributeLocation : public Shader::ShaderVisitor
    {
    private:
        Mesh*   mesh_;
    public:
        AttributeLocation(Shader* shader, Mesh* mesh) : ShaderVisitor(shader), mesh_(mesh) { }
        void visit(const std::string& key, const std::string& type, int size);
    };

    void AttributeLocation::visit(const std::string& key, const std::string& type, int size)
    {
        GLShader* shader = reinterpret_cast<GLShader*>(shader_);
        int loc = shader->getLocation(key);
        if (loc < 0)
        {
            loc = glGetAttribLocation(shader->getProgramId(), key.c_str());
            if (loc >= 0)
            {
                shader->setLocation(key, loc);
              //  if (Shader::LOG_SHADER) LOGE("SHADER::attribute:location: %s location: %d", key.c_str(), loc);
                switch (size)
                {
                    case 1:
                    mesh_->setVertexAttribLocF(loc, key);
                    break;

                    case 2:
                    mesh_->setVertexAttribLocV2(loc, key);
                    break;

                    case 3:
                    mesh_->setVertexAttribLocV3(loc, key);
                    break;

                    case 4:
                    mesh_->setVertexAttribLocV4(loc, key);
                    break;
                }
            }
        }
    }


GLShader::GLShader(int id,
               const std::string& signature,
               const std::string& uniformDescriptor,
               const std::string& textureDescriptor,
               const std::string& vertexDescriptor,
               const std::string& vertexShader,
               const std::string& fragmentShader)
    : Shader(id, signature, uniformDescriptor, textureDescriptor, vertexDescriptor, vertexShader, fragmentShader),
      program_(NULL)
{ }


GLShader::~GLShader()
{
    if (program_)
    {
        delete program_;
    }
}

void GLShader::initialize()
{
    std::string modified_frag_shader;
    if (fragmentShader_.find("samplerExternalOES")!= std::string::npos)
    {
        std::istringstream iss(fragmentShader_.c_str());
        const char* extensions = (const char*) glGetString(GL_EXTENSIONS);
        std::string extension_string;
        if(strstr(extensions, "GL_OES_EGL_image_external_essl3"))
        {
            extension_string = "#extension GL_OES_EGL_image_external_essl3 : require \n";
        }
        else
        {
            extension_string = "#extension GL_OES_EGL_image_external : require\n";
        }
        std::string line;
        while (std::getline(iss, line))
        {
            if(line.find("GL_OES_EGL_image_external") != std::string::npos)
            {
                modified_frag_shader = modified_frag_shader + extension_string + "\n";
            }
            else
            {
                modified_frag_shader = modified_frag_shader + line + "\n";
            }
        }
    }
    else
    {
        modified_frag_shader = fragmentShader_;
    }

    program_ = new GLProgram(vertexShader_.c_str(), modified_frag_shader.c_str());
    if (use_multiview && !(strstr(vertexShader_.c_str(), "gl_ViewID_OVR")
                           && strstr(vertexShader_.c_str(), "GL_OVR_multiview2")
                           && strstr(vertexShader_.c_str(), "GL_OVR_multiview2")))
    {
        std::string error = "Your shaders are not multiview";
        LOGE("Your shaders are not multiview");
        throw error;
    }
    vertexShader_.clear();
    fragmentShader_.clear();
}

void GLShader::bindMesh(Mesh* mesh)
{
    if (LOG_SHADER) LOGD("SHADER: getting attribute locations");
    {
        std::lock_guard<std::mutex> lock(attributeVariablesLock_);
        AttributeLocation avisit(this, mesh);
        forEach(vertexDescriptor_, avisit);
    }
}

bool GLShader::useShader()
{
    if (nullptr == program_)
    {
        initialize();
    }
    GLint programID = getProgramId();
    if (programID <= 0)
    {
        LOGE("SHADER: shader could not be generated %s", signature_.c_str());
        return false;
    }
    if (LOG_SHADER) LOGV("SHADER: rendering with program %d", programID);
    glUseProgram(programID);
    return true;
}

} /* namespace gvr */

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
 * Manages instances of shaders.
 ***************************************************************************/

#ifndef SHADER_MANAGER_H_
#define SHADER_MANAGER_H_

#include <mutex>
#include <map>

#include "objects/hybrid_object.h"
#include "shaders/material/custom_shader.h"

namespace gvr {
class ShaderManager: public HybridObject {
public:
    ShaderManager() :
            HybridObject(),
            latest_shader_id_(0),
            name_("MaterialShaderManager")
    { }

    ~ShaderManager();

    long addShader(const std::string& signature,
                   const std::string& uniformDescriptor,
                   const std::string& textureDescriptor,
                   const std::string& vertexDescriptor,
                   const std::string& vertex_shader,
                   const std::string& fragment_shader);
    Shader* findShader(const std::string& signature);
    Shader* getShader(long id);
    const std::string& name() { return name_; }
    void    dump();

private:
    ShaderManager(const ShaderManager& shader_manager);
    ShaderManager(ShaderManager&& shader_manager);
    ShaderManager& operator=(const ShaderManager& shader_manager);
    ShaderManager& operator=(ShaderManager&& shader_manager);

protected:
    std::string name_;

private:
    long latest_shader_id_ = 0;
    std::map<std::string, Shader*> shadersBySignature;
    std::map<long, Shader*> shadersByID;
    std::mutex lock_;
};

}
#endif

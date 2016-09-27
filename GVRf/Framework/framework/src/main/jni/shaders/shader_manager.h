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

#include "objects/hybrid_object.h"
#include "shaders/material/custom_shader.h"
#include "util/gvr_log.h"

namespace gvr {
class ShaderManager: public HybridObject {
public:
    ShaderManager() :
            HybridObject(),
            latest_shader_id_(INITIAL_SHADER_INDEX)  {
    }

    long addShader(const std::string& signature,
            const std::string& vertex_shader,
            const std::string& fragment_shader) {
        long id = latest_shader_id_++;
        Shader* shader = new Shader(signature, vertex_shader, fragment_shader);
        shadersBySignature[signature] = shader;
        shadersByID[id] = shader;
        LOGE("SHADER: end added custom shader %ld %s", id, signature.c_str());
        return id;
    }

    Shader* findShader(const std::string& signature)
    {
        auto it = shadersBySignature.find(signature);
        if (it != shadersBySignature.end())
        {
            Shader* shader = it->second;
            LOGE("SHADER: findShader %s -> %d", signature.c_str(), shader->getShaderID());
            return shader;
        }
        else
        {
            return NULL;
        }
    }

    Shader* getShader(long id)
    {
        auto it = shadersByID.find(id);
        if (it != shadersByID.end())
        {
            Shader* shader = it->second;
            LOGE("SHADER: getShader %ld -> %s", id, shader->signature().c_str());
            return shader;
        }
        else
        {
            return NULL;
        }
    }

private:
    ShaderManager(const ShaderManager& shader_manager);
    ShaderManager(ShaderManager&& shader_manager);
    ShaderManager& operator=(const ShaderManager& shader_manager);
    ShaderManager& operator=(ShaderManager&& shader_manager);

private:
    static const int INITIAL_SHADER_INDEX = 1;
    int latest_shader_id_ = 0;
    std::map<std::string, Shader*> shadersBySignature;
    std::map<int, Shader*> shadersByID;
};

}
#endif

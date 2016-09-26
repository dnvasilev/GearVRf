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

    int addShader(std::string signature,
            std::string vertex_shader,
            std::string fragment_shader) {
        int id = latest_shader_id_++;
        Shader* shader = new Shader(signature, vertex_shader, fragment_shader);
        shadersBySignature[signature] = shader;
        shadersByID[id] = shader;
        return id;
    }

    Shader* getShader(const std::string& signature) {
        auto it = shadersBySignature.find(signature);
        if (it != shadersBySignature.end()) {
            return it->second;
        } else {
            LOGE("ShaderManager::getShader() %s not found", signature.c_str());
            return NULL;
        }
    }

    Shader* getShader(int id) {
        auto it = shadersByID.find(id);
        if (it != shadersByID.end()) {
            return it->second;
        } else {
            LOGE("ShaderManager::getShader() %d not found", id);
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
    int latest_shader_id_;
    std::map<std::string, Shader*> shadersBySignature;
    std::map<int, Shader*> shadersByID;
};

}
#endif

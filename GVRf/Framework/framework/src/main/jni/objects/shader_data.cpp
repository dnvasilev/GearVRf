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
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "shaders/shader.h"
#include "shader_data.h"

namespace gvr {

    Texture* ShaderData::getTexture(const std::string& key) const
    {
        auto it = textures_.find(key);
        if (it != textures_.end())
        {
            return it->second;
        }
        return NULL;
    }

    void ShaderData::setTexture(const std::string& key, Texture* texture)
    {
        std::lock_guard<std::mutex> lock(lock_);
        if(textures_.find(key) != textures_.end()){
            dirty(MOD_TEXTURE);
        }
        else {
            dirty(NEW_TEXTURE);
        }
        textures_[key] = texture;
        //By the time the texture is being set to its attaching material, it is ready
        //This is guaranteed by upper java layer scheduling
    }

    /**
     * Visits each texture in the material and calls the given function.
     */
    void ShaderData::forEachTexture(std::function< void(const std::string& texname, Texture* tex) > func)
    {
        for (auto it = textures_.begin(); it != textures_.end(); ++it)
        {
            func(it->first, it->second);
        }
    }

    int ShaderData::getByteSize(const std::string& name) const
    {
        return uniforms().getByteSize(name);
    }

    const std::string& ShaderData::getDescriptor() const
    {
        return uniforms().getDescriptor();
    }

    bool ShaderData::getFloat(const std::string& name, float& v) const
    {
        return uniforms().getFloat(name, v);
    }

    bool   ShaderData::getInt(const std::string& name, int& v) const
    {
        return uniforms().getInt(name, v);
    }

    bool  ShaderData::setInt(const std::string& name, int val)
    {
        std::lock_guard<std::mutex> lock(lock_);
        dirty(MAT_DATA);
        return uniforms().setInt(name, val);
    }

    bool  ShaderData::setFloat(const std::string& name, float val)
    {
        std::lock_guard<std::mutex> lock(lock_);
        dirty(MAT_DATA);
        return uniforms().setFloat(name, val);
    }

    bool  ShaderData::setIntVec(const std::string& name, const int* val, int n)
    {
        std::lock_guard<std::mutex> lock(lock_);
        dirty(MAT_DATA);
        return uniforms().setIntVec(name, val, n);
    }

    bool  ShaderData::setFloatVec(const std::string& name, const float* val, int n)
    {
        std::lock_guard<std::mutex> lock(lock_);
        dirty(MAT_DATA);
        return uniforms().setFloatVec(name, val, n);
    }

    bool  ShaderData::getFloatVec(const std::string& name, float* val, int n)
    {
        return uniforms().getFloatVec(name, val, n);
    }

    bool  ShaderData::getIntVec(const std::string& name, int* val, int n)
    {
        return uniforms().getIntVec(name, val, n);
    }

    bool  ShaderData::setVec2(const std::string& name, const glm::vec2& v)
    {
        std::lock_guard<std::mutex> lock(lock_);
        return uniforms().setVec2(name, v);
    }

    bool  ShaderData::setVec3(const std::string& name, const glm::vec3& v)
    {
        std::lock_guard<std::mutex> lock(lock_);
        return uniforms().setVec3(name, v);
    }

    bool  ShaderData::setVec4(const std::string& name, const glm::vec4& v)
    {
        std::lock_guard<std::mutex> lock(lock_);
        return uniforms().setVec4(name, v);
    }

    bool  ShaderData::setMat4(const std::string& name, const glm::mat4& m)
    {
        std::lock_guard<std::mutex> lock(lock_);
        return uniforms().setMat4(name, m);
    }

    void ShaderData::add_dirty_flag(const std::shared_ptr<u_short>& dirty_flag)
    {
        dirty_flags_.insert(dirty_flag);
    }

    void ShaderData::add_dirty_flags(const std::unordered_set<std::shared_ptr<u_short>>& dirty_flags)
    {
        dirty_flags_.insert(dirty_flags.begin(), dirty_flags.end());
    }

    void ShaderData::dirty(DIRTY_BITS bit)
    {
        dirtyImpl(dirty_flags_,bit);
    }

    bool ShaderData::hasTexture(const std::string& key) const
    {
        auto it = textures_.find(key);
        return (it != textures_.end());
    }

    bool ShaderData::hasUniform(const std::string& key) const
    {
        return (uniforms().getByteSize(key) > 0);
    }

    int ShaderData::updateGPU(Renderer* renderer)
    {
        std::lock_guard<std::mutex> lock(lock_);
        for (auto it = textures_.begin(); it != textures_.end(); ++it)
        {
            Texture *tex = it->second;
            if (tex == NULL)
            {
                LOGV("ShaderData::updateGPU %s is null", it->first.c_str());
                return -1;
            }
            bool ready = tex->isReady();
            if (Shader::LOG_SHADER)
            {
                LOGV("ShaderData::updateGPU %s is %s", it->first.c_str(),
                     ready ? "ready" : "not ready");
            }
            if ((tex == NULL) || !ready)
            {
                return -1;
            }
        }
        if (uniforms().updateGPU(renderer))
        {
            return 1;
        }
        return 0;
    }

}


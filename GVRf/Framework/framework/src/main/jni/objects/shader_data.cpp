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

    ShaderData::ShaderData(const char* descriptor) :
            mNativeShader(0),
            mLock()
    {

    }

    Texture* ShaderData::getTexture(const char* key) const
    {
        for (auto it = mNames.begin(); it < mNames.end(); ++it)
        {
            if (*it == key)
            {
                return mTextures[it - mNames.begin()];
            }
        }
        return NULL;
    }

    void ShaderData::setTexture(const char* key, Texture* texture)
    {
        std::lock_guard<std::mutex> lock(mLock);
        for (auto it = mNames.begin(); it < mNames.end(); ++it)
        {
            if (*it == key)
            {
                dirty(MOD_TEXTURE);
                mTextures[it - mNames.begin()] = texture;
                return;
            }
        }
        dirty(NEW_TEXTURE);
        mNames.push_back(key);
        mTextures.push_back(texture);
        //By the time the texture is being set to its attaching material, it is ready
        //This is guaranteed by upper java layer scheduling
    }

    /**
     * Visits each texture in the material and calls the given function.
     */
    void ShaderData::forEachTexture(std::function< void(const char* texname, Texture* tex) > func)
    {
        std::lock_guard<std::mutex> lock(mLock);
        for (auto it = mTextures.begin(); it != mTextures.end(); ++it)
        {
            const std::string& name = mNames[it - mTextures.begin()];
            func(name.c_str(), *it);
        }
    }

    std::string ShaderData::makeShaderLayout()
    {
        return uniforms().makeShaderLayout();
    }

    int ShaderData::getByteSize(const char* name) const
    {
        return uniforms().getByteSize(name);
    }

    const char* ShaderData::getDescriptor() const
    {
        return uniforms().getDescriptor();
    }

    bool ShaderData::getFloat(const char* name, float& v) const
    {
        return uniforms().getFloat(name, v);
    }

    bool   ShaderData::getInt(const char* name, int& v) const
    {
        return uniforms().getInt(name, v);
    }

    bool  ShaderData::setInt(const char* name, int val)
    {
        std::lock_guard<std::mutex> lock(mLock);
        dirty(MAT_DATA);
        return uniforms().setInt(name, val);
    }

    bool  ShaderData::setFloat(const char* name, float val)
    {
        std::lock_guard<std::mutex> lock(mLock);
        dirty(MAT_DATA);
        return uniforms().setFloat(name, val);
    }

    bool  ShaderData::setIntVec(const char* name, const int* val, int n)
    {
        std::lock_guard<std::mutex> lock(mLock);
        dirty(MAT_DATA);
        return uniforms().setIntVec(name, val, n);
    }

    bool  ShaderData::setFloatVec(const char* name, const float* val, int n)
    {
        std::lock_guard<std::mutex> lock(mLock);
        dirty(MAT_DATA);
        return uniforms().setFloatVec(name, val, n);
    }

    bool  ShaderData::getFloatVec(const char* name, float* val, int n)
    {
        return uniforms().getFloatVec(name, val, n);
    }

    bool  ShaderData::getIntVec(const char* name, int* val, int n)
    {
        return uniforms().getIntVec(name, val, n);
    }

    bool  ShaderData::setVec2(const char* name, const glm::vec2& v)
    {
        std::lock_guard<std::mutex> lock(mLock);
        return uniforms().setVec2(name, v);
    }

    bool  ShaderData::setVec3(const char* name, const glm::vec3& v)
    {
        std::lock_guard<std::mutex> lock(mLock);
        return uniforms().setVec3(name, v);
    }

    bool  ShaderData::setVec4(const char* name, const glm::vec4& v)
    {
        std::lock_guard<std::mutex> lock(mLock);
        return uniforms().setVec4(name, v);
    }

    bool  ShaderData::setMat4(const char* name, const glm::mat4& m)
    {
        std::lock_guard<std::mutex> lock(mLock);
        return uniforms().setMat4(name, m);
    }

    void ShaderData::add_dirty_flag(const std::shared_ptr<u_short>& dirty_flag)
    {
        mDirtyFlags.insert(dirty_flag);
    }

    void ShaderData::add_dirty_flags(const std::unordered_set<std::shared_ptr<u_short>>& dirty_flags)
    {
        mDirtyFlags.insert(dirty_flags.begin(), dirty_flags.end());
    }

    void ShaderData::dirty(DIRTY_BITS bit)
    {
        dirtyImpl(mDirtyFlags,bit);
    }

    bool ShaderData::hasTexture(const char* key) const
    {
        for (auto it = mNames.begin(); it < mNames.end(); ++it)
        {
            if (*it == key)
            {
                return true;
            }
        }
        return false;
    }

    bool ShaderData::hasUniform(const char* key) const
    {
        return (uniforms().getByteSize(key) > 0);
    }

    int ShaderData::updateGPU(Renderer* renderer)
    {
        std::lock_guard<std::mutex> lock(mLock);
        for (auto it = mTextures.begin(); it != mTextures.end(); ++it)
        {
            Texture *tex = *it;
            const std::string& name = mNames[it - mTextures.begin()];
            if (tex == NULL)
            {
                LOGV("ShaderData::updateGPU %s is null", name.c_str());
                return -1;
            }
            bool ready = tex->isReady();
            if (Shader::LOG_SHADER)
            {
                LOGV("ShaderData::updateGPU %s is %s", name.c_str(),
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


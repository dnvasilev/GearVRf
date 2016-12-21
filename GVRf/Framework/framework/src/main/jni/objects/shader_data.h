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
 * Data for doing a post effect on the scene.
 ***************************************************************************/

#ifndef SHADER_DATA_H_
#define SHADER_DATA_H_

#include <map>
#include <memory>
#include <string>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "helpers.h"

#include "objects/hybrid_object.h"
#include "objects/textures/texture.h"
#include "objects/uniform_block.h"

namespace gvr {

class Texture;

class ShaderData : public HybridObject
{
public:
    ShaderData(const std::string& descriptor) :
            native_shader_(0), textures_() { }

    virtual ~ShaderData() { }

    int get_shader()
    {
        return native_shader_;
    }

    void set_shader(int shader)
    {
        native_shader_ = shader;
    }

    Texture* getTexture(const std::string& key) const
    {
        auto it = textures_.find(key);
        if (it != textures_.end())
        {
            return it->second;
        }
        return NULL;
    }

    void setTexture(const std::string& key, Texture* texture)
    {
        textures_[key] = texture;
        //By the time the texture is being set to its attaching material, it is ready
        //This is guaranteed by upper java layer scheduling
        if (texture != NULL)
        {
            LOGE("SHADER: texture %s is ready", key.c_str());
            texture->setReady(true);
            dirty();
        }
    }

    int getByteSize(const std::string& name) const
    {
        return uniforms().getByteSize(name);
    }

    const std::string& getDescriptor() const
    {
        return uniforms().getDescriptor();
    }

    bool getFloat(const std::string& name, float& v) const
    {
        return uniforms().getFloat(name, v);
    }

    bool   getInt(const std::string& name, int& v) const
    {
        return uniforms().getInt(name, v);
    }

    bool  setInt(const std::string& name, int val)
    {
        dirty();
        return uniforms().setInt(name, val);
    }

    bool  setFloat(const std::string& name, float val)
    {
        dirty();
        return uniforms().setFloat(name, val);
    }

    bool  setIntVec(const std::string& name, const int* val, int n)
    {
        dirty();
        return uniforms().setIntVec(name, val, n);
    }

    bool  setFloatVec(const std::string& name, const float* val, int n)
    {
        dirty();
        return uniforms().setFloatVec(name, val, n);
    }

    bool  getFloatVec(const std::string& name, float* val, int n)
    {
        return uniforms().getFloatVec(name, val, n);
    }

    bool  getIntVec(const std::string& name, int* val, int n)
    {
        return uniforms().getIntVec(name, val, n);
    }

    bool  setVec2(const std::string& name, const glm::vec2& v)
    {
        return uniforms().setVec2(name, v);
    }

    bool  setVec3(const std::string& name, const glm::vec3& v)
    {
        return uniforms().setVec3(name, v);
    }

    bool  setVec4(const std::string& name, const glm::vec4& v)
    {
        return uniforms().setVec4(name, v);
    }

    bool  setMat4(const std::string& name, const glm::mat4& m)
    {
        return uniforms().setMat4(name, m);
    }

    const std::map<std::string, Texture*>& getAllTextures()
    {
        return textures_;
    }

    void add_dirty_flag(const std::shared_ptr<bool>& dirty_flag)
    {
        dirty_flags_.insert(dirty_flag);
    }

    void add_dirty_flags(const std::unordered_set<std::shared_ptr<bool>>& dirty_flags)
    {
        dirty_flags_.insert(dirty_flags.begin(), dirty_flags.end());
    }

    void dirty()
    {
        dirtyImpl(dirty_flags_);
    }

    bool hasTexture(const std::string& key) const
    {
        auto it = textures_.find(key);
        return (it != textures_.end());
    }

    bool hasUniform(const std::string& key) const
    {
        return uniforms().hasUniform(key);
    }

    bool areTexturesReady()
    {
        for (auto it = textures_.begin(); it != textures_.end(); ++it)
        {
            Texture* tex = it->second;
            if ((tex == NULL) || !tex->isReady())
            {
                return false;
            }
        }
        return true;
    }

    virtual UniformBlock&   uniforms() = 0;
    virtual const UniformBlock& uniforms() const = 0;

private:
    ShaderData(const ShaderData&);
    ShaderData(ShaderData&&);
    ShaderData& operator=(const ShaderData&);
    ShaderData& operator=(ShaderData&&);

protected:
    int native_shader_;
    std::map<std::string, Texture*> textures_;
    std::unordered_set<std::shared_ptr<bool>> dirty_flags_;
};

}
#endif

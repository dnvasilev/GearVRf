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
#include <functional>

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
            native_shader_(0),
            textures_(),
            lock_()
    {

    }

    virtual ~ShaderData() { }

    int getNativeShader()
    {
        return native_shader_;
    }

    void setNativeShader(int shader)
    {
        native_shader_ = shader;
        dirty(NATIVE_SHADER);
    }

    const std::map<std::string, Texture*>& getAllTextures()
    {
        return textures_;
    }

    Texture* getTexture(const std::string& key) const;
    void setTexture(const std::string& key, Texture* texture);
    void forEachTexture(std::function< void(const std::string& texname, Texture* tex) > func);
    int getByteSize(const std::string& name) const;
    const std::string& getDescriptor() const;
    bool getFloat(const std::string& name, float& v) const;
    bool getInt(const std::string& name, int& v) const;
    bool  setInt(const std::string& name, int val);
    bool  setFloat(const std::string& name, float val);
    bool  setIntVec(const std::string& name, const int* val, int n);
    bool  setFloatVec(const std::string& name, const float* val, int n);
    bool  getFloatVec(const std::string& name, float* val, int n);
    bool  getIntVec(const std::string& name, int* val, int n);
    bool  setVec2(const std::string& name, const glm::vec2& v);
    bool  setVec3(const std::string& name, const glm::vec3& v);
    bool  setVec4(const std::string& name, const glm::vec4& v);
    bool  setMat4(const std::string& name, const glm::mat4& m);
    void add_dirty_flag(const std::shared_ptr<u_short>& dirty_flag);
    void add_dirty_flags(const std::unordered_set<std::shared_ptr<u_short>>& dirty_flags);
    void dirty(DIRTY_BITS bit);
    bool hasTexture(const std::string& key) const;
    bool hasUniform(const std::string& key) const;
    virtual int updateGPU(Renderer* renderer);
    u_int32_t getNumTextures() { return textures_.size(); }
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
    std::mutex lock_;
    std::unordered_set<std::shared_ptr<u_short>> dirty_flags_;
};

}
#endif

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
#include "shaderbase.h"
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

class Shader: public ShaderBase {
public:

    class ShaderVisitor
    {
    protected:
        Shader* shader_;

    public:
        ShaderVisitor(Shader* shader) : shader_(shader) { };
        virtual void visit(const std::string& key, const std::string& type, int size) = 0;
    };

    explicit Shader(long id, const std::string& signature,
            const std::string& uniformDescriptor,
            const std::string& textureDescriptor,
            const std::string& vertexDescriptor,
            const std::string& vertex_shader,
            const std::string& fragment_shader);
    virtual ~Shader();

    virtual void render(RenderState* rstate, RenderData* render_data, ShaderData* material);
    virtual void programInit(RenderState* rstate, RenderData* render_data, ShaderData* material,
                        const std::vector<glm::mat4>& model_matrix, int drawcount, bool batching) { }

    static int getGLTexture(int n);
    GLuint getProgramId();

    GLuint getLocation(const std::string& key) {
        auto it = locations_.find(key);
        if (it != locations_.end()) {
            return it->second;
        }
        return -1;
    }

    void setLocation(const std::string& key, int loc) {
        locations_[key] = loc;
    }

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

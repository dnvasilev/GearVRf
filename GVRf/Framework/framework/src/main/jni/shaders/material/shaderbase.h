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
 * Renders a GL_TEXTURE_EXTERNAL_OES texture.
 ***************************************************************************/

#ifndef SHADER_BASE_H_
#define SHADER_BASE_H_

#include "gl/gl_headers.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "gl/gl_program.h"

#include "objects/hybrid_object.h"

namespace gvr {
struct RenderState;
class RenderData;
class ShaderData;
class GLProgram;

class ShaderBase: public HybridObject {
public:
    ShaderBase(long id, const std::string& signature) : program_(nullptr), signature_(signature), id_(id) {  };

    const std::string& signature() const { return signature_; }
    virtual void render(RenderState* rstate, RenderData* render_data, ShaderData* material)=0;

    long getShaderID() const { return id_; }

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

protected:
    GLProgram* program_;
    std::string signature_;
    long id_;
};

}

#endif

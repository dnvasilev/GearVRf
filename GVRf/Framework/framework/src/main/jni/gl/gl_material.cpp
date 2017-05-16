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

#include "gl/gl_material.h"
#include "gl/gl_shader.h"
#include "gl/gl_imagetex.h"

namespace gvr
{

    int GLMaterial::bindToShader(Shader *shader, Renderer* renderer)
    {
        GLShader* glshader = static_cast<GLShader*>(shader);
        int texIndex = glshader->bindTextures(this);
        if (texIndex >= 0)
        {
           // std::string s = uniforms_.toString();
            uniforms_.bindBuffer(shader, renderer);
        }
        return texIndex;
    }

    bool GLMaterial::bindTexture(Texture* tex, int texIndex, int loc)
    {
        if (tex && tex->getImage())
        {
            GLImageTex* image = static_cast<GLImageTex*>(tex->getImage());
            int texid = image->getId();

            LOGV("GLMaterial::bindTexture index=%d loc=%d id=%d", texIndex, loc, texid);
            glActiveTexture(GL_TEXTURE0 + texIndex);
            glBindTexture(image->getTarget(), texid);
            glUniform1i(loc, texIndex);
            checkGLError("Material::bindTexture");
            return true;
        }
        return false;
    }
}


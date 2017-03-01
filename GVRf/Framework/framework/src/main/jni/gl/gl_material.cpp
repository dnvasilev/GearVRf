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

namespace gvr
{

    int GLMaterial::bindToShader(Shader *shader)
    {
        int texIndex = bindTextures(shader);
        if (texIndex >= 0)
        {
           // std::string s = uniforms_.toString();
            uniforms_.bindBuffer(shader);
        }
        return texIndex;
    }

    int GLMaterial::bindTextures(Shader *shader)
    {
        int texIndex = 0;
        bool allOK = true;
        GLShader *glshader = reinterpret_cast<GLShader *>(shader);
        GLuint programID = glshader->getProgramId();

        for (auto it = textures_.begin(); it != textures_.end(); ++it)
        {
            int loc = glshader->getLocation(it->first);
            if (loc < 0)
            {
                loc = glGetUniformLocation(programID, it->first.c_str());
                if (loc < 0)
                {
                    allOK = false;
                    continue;
                }
            }
            Texture* tex = it->second;
            if (tex && tex->getImage())
            {
                GLImageTex* image = static_cast<GLImageTex*>(tex->getImage());

                glActiveTexture(GL_TEXTURE0 + texIndex);
                glBindTexture(image->getTarget(), image->getId());
                glUniform1i(loc, texIndex);
                checkGLError("Material::bindTextures");
                ++texIndex;
            }
            else
            {
                allOK = false;
            }
        }
        return allOK ? texIndex : -1;
    }
}


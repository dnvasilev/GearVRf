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

#include "objects/scene.h"
#include "gl/gl_render_data.h"
#include "gl/gl_material.h"

namespace gvr {

void GLRenderData::renderBones(int programId)
{
    GLUniformBlock* bones_ubo = reinterpret_cast<GLUniformBlock*>(bones_ubo_);
    if (bones_ubo != nullptr)
    {
        if (bones_ubo->getBuffer() == 0)
        {
            bones_ubo->setBlockName("Bones_ubo");
        }
        bones_ubo->bindBuffer(programId);
        bones_ubo->render(programId);
    }
}
}

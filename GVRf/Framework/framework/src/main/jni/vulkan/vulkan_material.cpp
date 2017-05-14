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

#include "vulkan/vulkan_material.h"

namespace gvr
{
    int VulkanMaterial::render(Shader *shader, Renderer *unused)
    {
        if (uniforms_.isDirty())
        {
            std::lock_guard<std::mutex> lock(lock_);
            uniforms_.bindBuffer(shader, unused);
        }
        return 0;
    }

}
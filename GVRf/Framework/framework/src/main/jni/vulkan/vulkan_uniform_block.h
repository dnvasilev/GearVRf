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

#ifndef FRAMEWORK_VULKAN_UNIFORM_BLOCK_H
#define FRAMEWORK_VULKAN_UNIFORM_BLOCK_H

#include "../objects/uniform_block.h"
#include "vulkan/vulkanCore.h"
#include "util/gvr_log.h"
#include "vulkan/vulkanInfoWrapper.h"


namespace gvr {

    class VulkanUniformBlock: public UniformBlock
    {
    public:
        GVR_Uniform m_bufferInfo;
        VulkanUniformBlock();
        VulkanUniformBlock(const std::string& descriptor);
        void createBuffer(VkDevice &,VulkanCore*);
        GVR_Uniform& getBuffer() { return m_bufferInfo; }
        void updateBuffer(VkDevice &device,VulkanCore* vk);
    };

}

#endif //FRAMEWORK_VULKAN_UNIFORM_BLOCK_H

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
#include "vulkan/vulkan_uniform_block.h"
#include "util/gvr_gl.h"
#include "vulkan/vulkanCore.h"
#include "../objects/uniform_block.h"

namespace gvr {

    VulkanUniformBlock::VulkanUniformBlock(const std::string& descriptor) : UniformBlock(descriptor)
    {

        LOGE("pararameter");
    }

    VulkanUniformBlock::VulkanUniformBlock() : UniformBlock()
    {
        LOGE("default");
    }
    void VulkanUniformBlock::updateBuffer(VkDevice &device,VulkanCore* vk){

        //  if(!buffer_init_)
        //      createBuffer(device, vk);

        VkResult ret = VK_SUCCESS;
        uint8_t *pData;

        ret = vkMapMemory(device, m_bufferInfo.mem, 0, m_bufferInfo.allocSize, 0, (void **) &pData);
        assert(!ret);

        memcpy(pData, UniformData, TotalSize);

        vkUnmapMemory(device, m_bufferInfo.mem);

    }
    void VulkanUniformBlock::createBuffer(VkDevice &device,VulkanCore* vk){

        VkResult err = VK_SUCCESS;
        memset(&m_bufferInfo, 0, sizeof(m_bufferInfo));
        //err = vkCreateBuffer(m_device, &bufferCreateInfo, NULL, &m_modelViewMatrixUniform.buf);
        err = vkCreateBuffer(device, gvr::BufferCreateInfo(TotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), NULL, &m_bufferInfo.buf);
        assert(!err);
        // Obtain the requirements on memory for this buffer
        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(device, m_bufferInfo.buf, &mem_reqs);
        assert(!err);

        // And allocate memory according to those requirements
        VkMemoryAllocateInfo memoryAllocateInfo;
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.pNext = NULL;
        memoryAllocateInfo.allocationSize = 0;
        memoryAllocateInfo.memoryTypeIndex = 0;
        memoryAllocateInfo.allocationSize  = mem_reqs.size;
        bool pass = vk->GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
        assert(pass);

        // We keep the size of the allocation for remapping it later when we update contents
        m_bufferInfo.allocSize = memoryAllocateInfo.allocationSize;

        err = vkAllocateMemory(device, &memoryAllocateInfo, NULL, &m_bufferInfo.mem);
        assert(!err);

        // Bind our buffer to the memory
        err = vkBindBufferMemory(device, m_bufferInfo.buf, m_bufferInfo.mem, 0);
        assert(!err);

        m_bufferInfo.bufferInfo.buffer = m_bufferInfo.buf;
        m_bufferInfo.bufferInfo.offset = 0;
        m_bufferInfo.bufferInfo.range = TotalSize;

    }
}

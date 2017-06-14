/****
 *
 * VertexBuffer maintains a vertex data array with locations, normals,
 * colors and texcoords.
 *
 ****/
#include <engine/renderer/vulkan_renderer.h>
#include "vulkan_vertex_buffer.h"
#include "vulkan_shader.h"

namespace gvr {
    VulkanVertexBuffer::VulkanVertexBuffer(const char* layout_desc, int vertexCount)
    : VertexBuffer(layout_desc, vertexCount)
    {
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        freeGPUResources();
    }

    void VulkanVertexBuffer::freeGPUResources()
    {
    }

    bool VulkanVertexBuffer::updateGPU(Renderer* renderer, IndexBuffer* ibuf, Shader* shader)
    {
        VulkanRenderer* vkrender = reinterpret_cast<VulkanRenderer*>(renderer);
        generateVKBuffers(vkrender->getCore(),shader);
        if (ibuf)
        {
            ibuf->updateGPU(renderer);
        }
    }

    void VulkanVertexBuffer::generateVKBuffers(VulkanCore* vulkanCore, Shader* shader)
    {
        if (!isDirty())
            return;

        VkResult   err;
        bool   pass;

        // Our m_vertices member contains the types required for storing
        // and defining our vertex buffer within the graphics pipeline

        // Create our buffer object.
        VkDevice& device = vulkanCore->getDevice();
        VkBufferCreateInfo bufferCreateInfo = {};
        int bufferByteSize = getVertexCount() * mTotalSize;

        VkCommandBuffer trnCmdBuf;
        vulkanCore->createTransientCmdBuffer(trnCmdBuf);
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.pNext = nullptr;
        bufferCreateInfo.size = bufferByteSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferCreateInfo.flags = 0;
        //err = vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &m_vertices.buf);
        err = vkCreateBuffer(device, gvr::BufferCreateInfo(bufferByteSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), nullptr, &m_vertices.buf);
        GVR_VK_CHECK(!err);

        // Obtain the memory requirements for this buffer.
        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(device, m_vertices.buf, &mem_reqs);
        GVR_VK_CHECK(!err);

        // And allocate memory according to those requirements.
        VkMemoryAllocateInfo memoryAllocateInfo = {};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.pNext = nullptr;
        memoryAllocateInfo.allocationSize = 0;
        memoryAllocateInfo.memoryTypeIndex = 0;
        memoryAllocateInfo.allocationSize  = mem_reqs.size;
        pass = vulkanCore->GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
        GVR_VK_CHECK(pass);

        VkDeviceMemory mem_staging_vert;
        VkBuffer buf_staging_vert;
        err = vkCreateBuffer(device, gvr::BufferCreateInfo(bufferByteSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), nullptr, &buf_staging_vert);
        GVR_VK_CHECK(!err);

        err = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &mem_staging_vert);
        GVR_VK_CHECK(!err);

        // Now we need to map the memory of this new allocation so the CPU can edit it.
        void *data;

        err = vkMapMemory(device, mem_staging_vert, 0, memoryAllocateInfo.allocationSize, 0, &data);
        GVR_VK_CHECK(!err);

        // Copy our triangle verticies and colors into the mapped memory area.
        memcpy(data, mVertexData, bufferByteSize);

        // Unmap the memory back from the CPU.
        vkUnmapMemory(device, mem_staging_vert);
        err = vkBindBufferMemory(device, buf_staging_vert, mem_staging_vert, 0);
        GVR_VK_CHECK(!err);

        // Create Device memory optimal
        pass = vulkanCore->GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);
        GVR_VK_CHECK(pass);
        err = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &m_vertices.mem);
        GVR_VK_CHECK(!err);
        // Bind our buffer to the memory.
        err = vkBindBufferMemory(device, m_vertices.buf, m_vertices.mem, 0);
        GVR_VK_CHECK(!err);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(trnCmdBuf, &beginInfo);
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = bufferCreateInfo.size;
        vkCmdCopyBuffer(trnCmdBuf, buf_staging_vert, m_vertices.buf, 1, &copyRegion);
        vkEndCommandBuffer(trnCmdBuf);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &trnCmdBuf;

        vkQueueSubmit(vulkanCore->getVkQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(vulkanCore->getVkQueue());
        vkFreeCommandBuffers(device, vulkanCore->getTransientCmdPool(), 1, &trnCmdBuf);

      /*  m_vertices.vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        m_vertices.vi.pNext = nullptr;
        // check this
        m_vertices.vi.vertexBindingDescriptionCount = 1;
        m_vertices.vi.pVertexBindingDescriptions = &m_vertices.vi_bindings;

        m_vertices.vi_bindings.binding = 0;

        m_vertices.vi_bindings.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        LOGE("attrMapping.size() %d, total_size= %d", mLayout.size(), bufferByteSize);
        int i = 0;
        int stride = 0;
        forEachEntry([this, i, &stride](const DataEntry& e) mutable {
            if (e.IsSet){
                stride += e.Size;
            }
        });

        m_vertices.vi_bindings.stride = 32;


        forEachEntry([this, i](const DataEntry& e) mutable
                     {
                         if (e.IsSet && std::strstr(e.Name, "normal") == NULL)
                         {
                             VkVertexInputAttributeDescription binding;
                             binding.binding = GVR_VK_VERTEX_BUFFER_BIND_ID;
                             binding.location = e.Index;
                             LOGE("location %d attrMapping[i].offset %d , name %s", e.Index, e.Offset, e.Name);
                             binding.format = getDataType(e.Type); //float3
                             binding.offset = e.Offset;
                             m_vertices.vi_attrs.push_back(binding);
                             i++;
                         }
                     });
        m_vertices.vi.vertexAttributeDescriptionCount = mLayout.size() - 1;
        m_vertices.vi.pVertexAttributeDescriptions = m_vertices.vi_attrs.data();
*/
        // The vertices need to be defined so that the pipeline understands how the
        // data is laid out. This is done by providing a VkPipelineVertexInputStateCreateInfo
        // structure with the correct information.
        m_vertices.vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        m_vertices.vi.pNext = nullptr;
        int i = 0;

        shader->getVertexDescriptor().forEachEntry([this, &i](const DataDescriptor::DataEntry &e)
        {
            LOGV("VertexBuffer::bindToShader find %s", e.Name);
            const DataDescriptor::DataEntry* entry = find(e.Name);

            if (!e.NotUsed)                             // shader uses this vertex attribute?
            {
                if ((entry != nullptr) && entry->IsSet) // mesh uses this vertex attribute?
                {
                    VkVertexInputAttributeDescription binding;
                    binding.binding = GVR_VK_VERTEX_BUFFER_BIND_ID;
                    binding.location = e.Index;
                    LOGE("location %d attrMapping[i].offset %d , name %s", e.Index, e.Offset, e.Name);
                    binding.format = getDataType(e.Type); //float3
                    binding.offset = e.Offset;
                    m_vertices.vi_attrs.push_back(binding);
                    i++;
                }
                else                                // mesh uses attribute but shader does not
                {
                    LOGE("entry is not present");

                }

            }
        });
        m_vertices.vi_bindings.stride = mTotalSize;
        m_vertices.vi.vertexAttributeDescriptionCount = i;
        m_vertices.vi.pVertexAttributeDescriptions = m_vertices.vi_attrs.data();
        m_vertices.vi.vertexBindingDescriptionCount = 1;
        m_vertices.vi.pVertexBindingDescriptions = &m_vertices.vi_bindings;
        m_vertices.vi_bindings.binding = 0;
        m_vertices.vi_bindings.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


        mIsDirty = false;
    }

    VkFormat VulkanVertexBuffer::getDataType(const std::string& type)
    {
        if(type.compare("float")==0)
            return VK_FORMAT_R32_SFLOAT;

        if(type.compare("vec2")==0 || type.compare("float2")==0)
            return VK_FORMAT_R32G32_SFLOAT;

        if(type.compare("float3")==0 || type.compare("vec3")==0)
            return VK_FORMAT_R32G32B32_SFLOAT;

        if(type.compare("float4")==0 || type.compare("vec4")==0)
            return VK_FORMAT_R32G32B32A32_SFLOAT;

    }
} // end gvrf

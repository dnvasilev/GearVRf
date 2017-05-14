#pragma once

#include "objects/vertex_buffer.h"
#include "vulkan/vulkan_headers.h"
#include <vector>
#include <map>

namespace gvr {
    class Shader;
    class Renderer;

 /**
  * Interleaved vertex storage for OpenGL
  *
  * @see VertexBuffer
  */
    class VulkanVertexBuffer : public VertexBuffer
    {
    public:
        VulkanVertexBuffer(const std::string& layout_desc, int vertexCount);
        virtual ~VulkanVertexBuffer();

        virtual bool    updateGPU(Renderer*);
        virtual bool    bindBuffer(Shader*, Renderer*) { }
        GVR_VK_Vertices* getVKVertices() const  { return vkVertices_; }
        void    generateVKBuffers(VulkanCore* vulkanCore);

    protected:
        void    freeGPUResources();
        VkFormat getDataType(const std::string& type);

        GVR_VK_Vertices m_vertices;
        GVR_VK_Vertices* vkVertices_;
    };

} // end gvrf


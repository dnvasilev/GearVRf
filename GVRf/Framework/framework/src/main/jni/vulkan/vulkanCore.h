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


#ifndef FRAMEWORK_VULKANCORE_H
#define FRAMEWORK_VULKANCORE_H

#define VK_USE_PLATFORM_ANDROID_KHR

#include <android/native_window_jni.h>	// for native window JNI
#include "vulkan/vulkan_wrapper.h"
#include <vector>
#include "glm/glm.hpp"
//#include "vulkanThreadPool.h"

#define GVR_VK_CHECK(X) if (!(X)) { LOGD("VK_CHECK Failure"); assert((X));}
#define GVR_VK_VERTEX_BUFFER_BIND_ID 0
#define GVR_VK_SAMPLE_NAME "GVR Vulkan"
#define VK_KHR_ANDROID_SURFACE_EXTENSION_NAME "VK_KHR_android_surface"


namespace gvr {
class Scene;
class RenderData;
class Camera;
extern  uint8_t *oculusTexData;

struct GVR_VK_SwapchainBuffer
{
    VkImage image;
    VkCommandBuffer cmdBuffer;
    VkImageView view;
    VkDeviceSize size;
    VkDeviceMemory mem;
    VkBuffer buf;
};

struct GVR_VK_DepthBuffer {
    VkFormat format;
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
};

struct GVR_VK_Vertices {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkPipelineVertexInputStateCreateInfo vi;
    VkVertexInputBindingDescription      vi_bindings[1];
    VkVertexInputAttributeDescription    vi_attrs[2];
};

struct Uniform {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo bufferInfo;
    VkDeviceSize allocSize;
};

struct OutputBuffer
{
    VkBuffer imageOutputBuffer;
    VkDeviceMemory memory;
    VkDeviceSize size;
};

// Index buffer
struct GVR_VK_Indices {
    VkDeviceMemory memory;
    VkBuffer buffer;
    uint32_t count;
};


class VulkanCore {
public:
    // Return NULL if Vulkan inititialisation failed. NULL denotes no Vulkan support for this device.
    static VulkanCore* getInstance(ANativeWindow * newNativeWindow = nullptr) {
        if (!theInstance) {
            theInstance = new VulkanCore(newNativeWindow);
        }
        if (theInstance->m_Vulkan_Initialised)
            return theInstance;
        return NULL;
    }
    void UpdateUniforms(Scene* scene, Camera* camera, RenderData* render_data);
     void InitUniformBuffersForRenderData(Uniform &m_modelViewMatrixUniform);
     void InitUniformBuffersForRenderDataLights(Uniform &m_modelViewMatrixUniform);
     void InitDescriptorSetForRenderData(Uniform &m_modelViewMatrixUniform, Uniform &m_lightsUniform, VkDescriptorSet &m_descriptorSet);
     void BuildCmdBufferForRenderData(std::vector <VkDescriptorSet> &allDescriptors, int &swapChainIndex, std::vector<RenderData*>& render_data_vector);
     void DrawFrameForRenderData(int &swapChainIndex);
      int AcquireNextImage();
      void InitVertexBuffersFromRenderData(const std::vector<glm::vec3>& vertices, GVR_VK_Vertices &m_vertices, GVR_VK_Indices &m_indices, const std::vector<unsigned short> & indices);
     //void InitVertexBuffersFromRenderData(GVR_VK_Vertices &m_vertices, GVR_VK_Indices &m_indices);
      void InitPipelineForRenderData(GVR_VK_Vertices &m_vertices, VkPipeline &m_pipeline);
      VkShaderModule CreateShaderModuleAscii(const uint32_t* code, uint32_t size);
      void BuildSecondaryCmdBuffer(VkCommandBuffer secondaryCmdBuff, VkCommandBufferBeginInfo secondaryBeginInfo, RenderData* renderData, VkDescriptorSet allDescriptors);
private:
    std::vector<VkFence> waitFences;
    static VulkanCore* theInstance;
    VulkanCore(ANativeWindow * newNativeWindow) : m_pPhysicalDevices(NULL){
        m_Vulkan_Initialised = false;
        initVulkanCore(newNativeWindow);
    }
    bool CreateInstance();
    VkShaderModule CreateShaderModule(std::vector<uint32_t> code, uint32_t size);
    bool GetPhysicalDevices();
    void initVulkanCore(ANativeWindow * newNativeWindow);
    bool InitDevice();
    void InitSurface();
    void InitSwapchain(uint32_t width, uint32_t height);
    bool GetMemoryTypeFromProperties( uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);
    void InitCommandbuffers();
    void InitTransientCmdPool();
    VkCommandBuffer GetTransientCmdBuffer();
    void InitVertexBuffers();
    void InitLayouts();
    void InitRenderPass();
    void InitFrameBuffers();
    void InitSync();
    void BuildCmdBuffer();

    void InitUniformBuffers();


    bool m_Vulkan_Initialised;
    ANativeWindow * m_androidWindow;

    VkInstance m_instance;
    VkPhysicalDevice* m_pPhysicalDevices;
    VkPhysicalDevice m_physicalDevice;
    VkPhysicalDeviceProperties m_physicalDeviceProperties;
    VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
    VkDevice m_device;
    uint32_t m_physicalDeviceCount;
    uint32_t m_queueFamilyIndex;
    VkQueue m_queue;
    VkSurfaceKHR m_surface;
    VkSurfaceFormatKHR m_surfaceFormat;

    VkSwapchainKHR m_swapchain;
    GVR_VK_SwapchainBuffer* m_swapchainBuffers;
    GVR_VK_SwapchainBuffer* outputImage;

    uint32_t m_swapchainCurrentIdx;
    uint32_t m_height;
    uint32_t m_width;
    uint32_t m_swapchainImageCount;
    VkSemaphore m_backBufferSemaphore;
    VkSemaphore m_renderCompleteSemaphore;
    VkFramebuffer* m_frameBuffers;

    VkCommandPool m_commandPool;
    VkCommandPool m_commandPoolTrans;
    GVR_VK_DepthBuffer* m_depthBuffers;
    GVR_VK_Vertices m_vertices;

    VkDescriptorSetLayout m_descriptorLayout;
    VkPipelineLayout  m_pipelineLayout;
    VkRenderPass m_renderPass;
    VkPipeline m_pipeline;
    OutputBuffer* m_outputBuffers;
    uint8_t * texDataVulkan;
    int imageIndex = 0;
    uint8_t *finaloutput;
    Uniform m_modelViewMatrixUniform;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSet m_descriptorSet;
    GVR_VK_Indices m_indices;

    //uint m_threadCount;
    //ThreadPool m_threadPool;
};


extern VulkanCore gvrVulkanCore;
}
#endif //FRAMEWORK_VULKANCORE_H

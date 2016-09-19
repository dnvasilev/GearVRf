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
};

struct GVR_VK_DepthBuffer {
    VkFormat format;
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
};

struct GVR_VK_Vertices {
    ~GVR_VK_Vertices(){
        delete vi_attrs;
    }
    VkBuffer buf;
    VkDeviceMemory mem;
    VkPipelineVertexInputStateCreateInfo vi;
    VkVertexInputBindingDescription      vi_bindings[1];
    VkVertexInputAttributeDescription    *vi_attrs;
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
    void DrawFrame(int m_swapchainCurrentIdx);
    void updatePipelineInfo(VkGraphicsPipelineCreateInfo& pipelineCreateInfo,VkPipelineVertexInputStateCreateInfo& vi);
    void createGraphicsPipeline(VkPipeline& pipeline,VkGraphicsPipelineCreateInfo& pipelineCreateInfo );
    void bindCommandBuffer(int swapChainIndex);
     void unBindCommandBuffer(int swapChainIndex);
    void bindRenderData(RenderData* render_data,int);
    void initPipelineMetaData(int swapChainIndex);
    VkDevice& getDevice(){
        return m_device;
    }
    void bindPipeline(RenderData* render_data);
    VkGraphicsPipelineCreateInfo& getPipelineCreateInfo(){
        return m_pipelineCreateInfo_;
    }
    bool GetMemoryTypeFromProperties( uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);
    int AcquireNextImage();
    GVR_VK_Vertices& getVKVertices(){
        return m_vertices;
    }
private:
    VkGraphicsPipelineCreateInfo m_pipelineCreateInfo_;
    std::vector<VkFence> waitFences;
    static VulkanCore* theInstance;
    VulkanCore(ANativeWindow * newNativeWindow) : m_pPhysicalDevices(NULL){
        m_Vulkan_Initialised = false;
        initVulkanCore(newNativeWindow);
        m_pipelineCreateInfo_ = {};
        m_prePresentBarrier_ = {};
        m_rp_begin_ = {};
        m_preRenderBarrier_ = {};
        cmd_buf_hinfo = {};
        cmd_buf_info = {};
        clear_values[0] = {};
        clear_values[1] = {};
       vi = {};
	  ia = {};
	   rs = {};
	    att_state[1] = {};
	      cb = {};
	        vp = {};
	   viewport = {};
	    scissor = {};
	      ds = {};
	     ms = {};
	      shaderStages[0] = {};
   shaderStages[1] = {};

    }
    bool CreateInstance();
    VkShaderModule CreateShaderModule(std::vector<uint32_t> code, uint32_t size);
    bool GetPhysicalDevices();
    void initVulkanCore(ANativeWindow * newNativeWindow);
    bool InitDevice();
    void InitSurface();
    void InitSwapchain(uint32_t width, uint32_t height);

    void InitCommandbuffers();
    void InitVertexBuffers();
    void InitLayouts();
    void InitRenderPass();
    void InitPipeline(VkGraphicsPipelineCreateInfo& pipelineCreateInfo);
    void InitFrameBuffers();
    void InitSync();
    void BuildCmdBuffer();

    void InitDescriptorSet();
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

    uint32_t m_swapchainCurrentIdx;
    uint32_t m_height;
    uint32_t m_width;
    uint32_t m_swapchainImageCount;
    VkSemaphore m_backBufferSemaphore;
    VkSemaphore m_renderCompleteSemaphore;
    VkFramebuffer* m_frameBuffers;

    VkCommandPool m_commandPool;
    GVR_VK_DepthBuffer* m_depthBuffers;
    GVR_VK_Vertices m_vertices;
    VkImageMemoryBarrier m_prePresentBarrier_;
    VkRenderPassBeginInfo m_rp_begin_;
    VkImageMemoryBarrier m_preRenderBarrier_;

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

    VkCommandBufferInheritanceInfo cmd_buf_hinfo;
 VkCommandBufferBeginInfo cmd_buf_info;
  VkClearValue clear_values[2] ;


      VkPipelineVertexInputStateCreateInfo   vi;
  	 VkPipelineInputAssemblyStateCreateInfo ia ;
  	  VkPipelineRasterizationStateCreateInfo rs;
  	   VkPipelineColorBlendAttachmentState att_state[1];
  	  VkPipelineColorBlendStateCreateInfo    cb;
  	  VkPipelineViewportStateCreateInfo      vp ;
  	  VkViewport viewport ;
  	   VkRect2D scissor ;
  	    VkPipelineDepthStencilStateCreateInfo  ds;
  	  VkPipelineMultisampleStateCreateInfo   ms ;
  	     VkPipelineShaderStageCreateInfo shaderStages[2];


};


extern VulkanCore gvrVulkanCore;
}
#endif //FRAMEWORK_VULKANCORE_H

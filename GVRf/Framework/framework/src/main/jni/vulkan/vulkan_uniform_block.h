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

#ifndef VULKAN_UNIFORMBLOCK_H_
#define VULKAN_UNIFORMBLOCK_H_

#include "objects/uniform_block.h"

namespace gvr {
    class VulkanUniformBlock;

    class VulkanDescriptor
    {
    public:
        VulkanDescriptor();
       // VulkanDescriptor(const std::string& ubo_descriptor);
        ~VulkanDescriptor();
        void createDescriptor(VulkanCore*, int, VkShaderStageFlagBits);

        void createLayoutBinding(int binding_index,int stageFlags, bool sampler=false);
        //void createDescriptorWriteInfo(int binding_index,int stageFlags, VkDescriptorSet& descriptor, bool sampler=false);
       // VulkanUniformBlock* getUBO();
        VkDescriptorSetLayoutBinding& getLayoutBinding();
      //  VkWriteDescriptorSet& getDescriptorSet();

    private:
      //  void createBuffer(VkDevice &device, VulkanCore* vk,VulkanUniformBlock*);
        //VulkanUniformBlock* ubo;
        VkDescriptorSetLayoutBinding layout_binding;
    //    VkWriteDescriptorSet writeDescriptorSet;
    };

    /**
     * Manages a Uniform Block containing data parameters to pass to
     * Vulkan vertex and fragment shaders.
     *
     * The UniformBlock may be updated by the application. If it has changed,
     * GearVRf resends the entire data block to Vulkan.
     */
    class VulkanUniformBlock : public UniformBlock
    {
    public:
        VulkanUniformBlock(const std::string& descriptor, int bindingPoint, const std::string& blockName);

        virtual bool updateGPU(Renderer*);
        virtual bool bindBuffer(Shader*, Renderer*) { };
        virtual std::string makeShaderLayout();

        VulkanDescriptor* getDescriptor();
        void createDescriptorWriteInfo(int binding_index,int stageFlags, bool sampler=false);
        GVR_Uniform& getBuffer() { return m_bufferInfo; }

        GVR_Uniform m_bufferInfo;
        VkWriteDescriptorSet& getDescriptorSet();

    protected:
        void createBuffer(VulkanCore*);
        void updateBuffer(VulkanCore* vk);

        bool buffer_init_ = false;
        VkWriteDescriptorSet writeDescriptorSet;
        VulkanDescriptor* vk_descriptor;
    };


    inline VulkanDescriptor::VulkanDescriptor() {  }
   // inline VulkanDescriptor::VulkanDescriptor(const std::string& ubo_descriptor){ }
  //  inline VulkanDescriptor::VulkanDescriptor( VulkanUniformBlock* ubo) { }
    inline VulkanDescriptor::~VulkanDescriptor() { }

    inline VulkanDescriptor* VulkanUniformBlock::getDescriptor() { return vk_descriptor; }
}
#endif

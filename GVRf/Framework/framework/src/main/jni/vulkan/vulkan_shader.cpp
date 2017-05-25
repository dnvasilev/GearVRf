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

/***************************************************************************
 * A shader which an user can add in run-time.
 ***************************************************************************/

#include "vulkan/vulkan_shader.h"
#include "vulkan/vulkan_material.h"
#include "engine/renderer/vulkan_renderer.h"
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <glslang/Include/Common.h>

namespace gvr {

VulkanShader::VulkanShader(int id,
               const char* signature,
               const char* uniformDescriptor,
               const char* textureDescriptor,
               const char* vertexDescriptor,
               const char* vertexShader,
               const char* fragmentShader)
    : Shader(id, signature, uniformDescriptor, textureDescriptor, vertexDescriptor, vertexShader, fragmentShader) { }

void VulkanShader::initialize()
{
}

int VulkanShader::makeLayout(VulkanMaterial& vkMtl, std::vector<VkDescriptorSetLayoutBinding>& samplerBinding, int index)
{
    if (getUniformDescriptor().getNumEntries() > 0)
    {
        VkDescriptorSetLayoutBinding &material_uniformBinding = vkMtl.getVulkanUniforms().getDescriptor()->getLayoutBinding();
        samplerBinding.push_back(material_uniformBinding);
        index++;
    }
    vkMtl.forEachTexture([this, samplerBinding, index](const char* texname, Texture* t) mutable
    {
        if (mTextures.find(texname) <= 0)
        {
            return;
        }
        VkDescriptorSetLayoutBinding layoutBinding;
        layoutBinding.binding = index++;
        layoutBinding.descriptorCount = 1;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBinding.pImmutableSamplers = nullptr;
        samplerBinding.push_back(layoutBinding);
    });
    return index;
}

int VulkanShader::bindTextures(VulkanMaterial& material, std::vector<VkWriteDescriptorSet>& writes, VkDescriptorSet& descriptorSet)
{
    int texIndex = 0;
    bool fail = false;
    std::vector<VkDescriptorImageInfo> info;
    u_int32_t index = 0;
    std::vector<VkDescriptorImageInfo>& descriptorImageInfo = info;

    material.forEachTexture([this, index, writes, descriptorImageInfo, descriptorSet](const std::string& texname, Texture* t) mutable
    {
        VkTexture *tex = static_cast<VkTexture *>(t);
        if (mTextures.find(texname) <= 0)
        {
            return;
        }
        descriptorImageInfo[index].sampler = tex->getVkSampler();
        descriptorImageInfo[index].imageView = tex->getVkImageView();
        descriptorImageInfo[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet write;
        memset(&write, 0, sizeof(write));

        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstBinding = index;
        write.dstSet = descriptorSet;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &descriptorImageInfo[index];
        writes.push_back(write);
        index++;
    });
    if (!fail)
    {
        return texIndex;
    }
    return -1;
}

VulkanShader::~VulkanShader() { }

    std::vector<uint32_t> VulkanShader::CompileVulkanShader(const std::string& shaderName, ShaderType shaderTypeID, std::string& shaderContents)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        shaderc_shader_kind shaderType;

        switch (shaderTypeID)
        {
            case VERTEX_SHADER:
                shaderType = shaderc_glsl_default_vertex_shader;
                break;
            case FRAGMENT_SHADER:
                shaderType = shaderc_glsl_default_fragment_shader;
                break;
        }

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderContents.c_str(),
                                                                         shaderContents.size(),
                                                                         shaderType,
                                                                         shaderName.c_str(),
                                                                         options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            LOGI("Vulkan shader unable to compile : %s", module.GetErrorMessage().c_str());
        }

        std::vector<uint32_t> result(module.cbegin(), module.cend());
        return result;
    }
} /* namespace gvr */

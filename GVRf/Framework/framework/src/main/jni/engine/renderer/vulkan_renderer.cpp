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
 * Renders a scene, a screen.
 ***************************************************************************/

#include "renderer.h"
#include "gl/gl_program.h"
#include "glm/gtc/matrix_inverse.hpp"

#include "eglextension/tiledrendering/tiled_rendering_enhancer.h"
#include "objects/material.h"
#include "objects/post_effect_data.h"
#include "objects/scene.h"
#include "objects/scene_object.h"
#include "objects/components/camera.h"
#include "objects/components/render_data.h"
#include "objects/textures/render_texture.h"
#include "objects/mesh.h"
#include "shaders/shader_manager.h"
#include "shaders/post_effect_shader_manager.h"
#include "util/gvr_gl.h"
#include "util/gvr_log.h"
#include "batch_manager.h"
#include "renderer.h"
#include "vulkan_renderer.h"
#include <unordered_map>
#include <unordered_set>

namespace gvr {
     void VulkanRenderer::renderCamera(Scene* scene, Camera* camera,
             ShaderManager* shader_manager,
             PostEffectShaderManager* post_effect_shader_manager,
             RenderTexture* post_effect_render_texture_a,
             RenderTexture* post_effect_render_texture_b) {

/*
        int swapChainIndex =  vulkanCore_->AcquireNextImage();
         vulkanCore_->initPipelineMetaData(swapChainIndex);
        vulkanCore_->bindCommandBuffer(swapChainIndex);
      //  LOGI("VK calling draw %d", render_data_vector.size());
        for(auto &render_data : render_data_vector) {
         //   render_data->mesh()->generateVAO(vulkanCore_->getDevice(), vulkanCore_);
            //GVR_VK_Vertices* vkVertices_ = render_data->mesh()->getVKVertices();
            GVR_VK_Vertices& vkVertices_ = vulkanCore_->getVKVertices();
            VkPipeline& m_pipeline = render_data->getVKPipeline();

            VkGraphicsPipelineCreateInfo& m_pipelineCreateInfo_ = vulkanCore_->getPipelineCreateInfo();
        //    vulkanCore_->updatePipelineInfo(m_pipelineCreateInfo_,vkVertices_.vi);
            vulkanCore_->createGraphicsPipeline(m_pipeline, m_pipelineCreateInfo_);
            vulkanCore_->UpdateUniforms(scene,camera, render_data);
            vulkanCore_->bindRenderData(render_data, swapChainIndex);
        }
        vulkanCore_->unBindCommandBuffer(swapChainIndex);
        vulkanCore_->DrawFrame(swapChainIndex);*/

            std::vector <VkDescriptorSet> allDescriptors;
              //  LOGI("VK calling draw %d", render_data_vector.size());
              int swapChainIndex = vulkanCore_->AcquireNextImage();

                for(auto &rdata : render_data_vector) {
               //     LOGI("VK calling rdata");
                // Creating and initializing Uniform Buffer for Each Render Data
                if(rdata->uniform_dirty){
                const std::vector<glm::vec3>& vertices=  rdata->mesh()->vertices();
                 const std::vector<unsigned short> & indices =  rdata->mesh()->triangles();
                vulkanCore_->InitVertexBuffersFromRenderData(vertices, rdata->m_vertices, rdata->m_indices, indices);
                //vulkanCore_->InitVertexBuffersFromRenderData(rdata->m_vertices, rdata->m_indices);
                vulkanCore_->InitUniformBuffersForRenderData(rdata->m_modelViewMatrixUniform);
                vulkanCore_->InitUniformBuffersForRenderDataLights(rdata->m_lightUniform);

                vulkanCore_->InitDescriptorSetForRenderData(rdata->m_modelViewMatrixUniform, rdata->m_lightUniform, rdata->m_descriptorSet);
                vulkanCore_->InitPipelineForRenderData(rdata->m_vertices, rdata->m_pipeline);

                rdata->uniform_dirty = false;
                }

                    allDescriptors.push_back(rdata->m_descriptorSet);
                    vulkanCore_->UpdateUniforms(scene,camera, rdata);

                    //vulkanCore_->DrawFrame();
                    //break;
                }
                //int swapChainIndex = vulkanCore_->AcquireNextImage();
                //vulkanCore_->BuildCmdBufferForRenderData(allDescriptors, swapChainIndex, rdata->m_pipeline, rdata->m_vertices, rdata->m_indices);
                vulkanCore_->BuildCmdBufferForRenderData(allDescriptors, swapChainIndex, render_data_vector);//rdata->m_pipeline, rdata->m_vertices, rdata->m_indices);
                vulkanCore_->DrawFrameForRenderData(swapChainIndex);

     }

}
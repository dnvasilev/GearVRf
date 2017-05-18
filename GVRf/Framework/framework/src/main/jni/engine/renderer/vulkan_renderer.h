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

#ifndef FRAMEWORK_VULKANRENDERER_H
#define FRAMEWORK_VULKANRENDERER_H

#include <vector>
#include <memory>

#define __gl2_h_
#include "EGL/egl.h"
#include "EGL/eglext.h"
#ifndef GL_ES_VERSION_3_0
#include "GLES3/gl3.h"
#include <GLES2/gl2ext.h>
#include "GLES3/gl3ext.h"
#endif

#include "glm/glm.hpp"
#include "batch.h"
#include "objects/eye_type.h"
#include "objects/mesh.h"
#include "objects/bounding_volume.h"
#include <unordered_map>
#include "batch_manager.h"
#include "renderer.h"
#include "vulkan/vulkan_headers.h"

namespace gvr {

class Camera;
class Scene;
class SceneObject;
class ShaderData;
class RenderData;
class RenderTexture;
class Light;
class BitmapImage;
class CubemapImage;
class CompressedImage;

class VulkanRenderer: public Renderer {
    friend class Renderer;

protected:
    virtual ~VulkanRenderer(){}

public:
    Texture* createSharedTexture( int id) {};

    VulkanRenderer() : vulkanCore_(nullptr) {
        vulkanCore_ = VulkanCore::getInstance();
    }
    VulkanCore* getCore() { return vulkanCore_; }
    VkDevice& getDevice(){
        return vulkanCore_->getDevice();
    }
    bool GetMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask,
                                     uint32_t *typeIndex){
        return vulkanCore_->GetMemoryTypeFromProperties(typeBits,requirements_mask,typeIndex);
    }
    void initCmdBuffer(VkCommandBufferLevel level,VkCommandBuffer& cmdBuffer){
        vulkanCore_->initCmdBuffer(level,cmdBuffer);
    }
    VkQueue& getQueue(){
        return vulkanCore_->getVkQueue();
    }
    VkPhysicalDevice& getPhysicalDevice(){
        return vulkanCore_->getPhysicalDevice();
    }
    // pure virtual
     void renderCamera(Scene* scene, Camera* camera,
             ShaderManager* shader_manager,
             PostEffectShaderManager* post_effect_shader_manager,
             RenderTexture* post_effect_render_texture_a,
             RenderTexture* post_effect_render_texture_b);

   void renderCamera(Scene* scene, Camera* camera, int viewportX,
             int viewportY, int viewportWidth, int viewportHeight,
             ShaderManager* shader_manager,
             PostEffectShaderManager* post_effect_shader_manager,
             RenderTexture* post_effect_render_texture_a,
             RenderTexture* post_effect_render_texture_b){}
   void renderCamera(Scene* scene, Camera* camera, int framebufferId,
             int viewportX, int viewportY, int viewportWidth, int viewportHeight,
             ShaderManager* shader_manager,
             PostEffectShaderManager* post_effect_shader_manager,
             RenderTexture* post_effect_render_texture_a,
             RenderTexture* post_effect_render_texture_b){}
   void renderCamera(Scene* scene, Camera* camera,
             RenderTexture* render_texture, ShaderManager* shader_manager,
             PostEffectShaderManager* post_effect_shader_manager,
             RenderTexture* post_effect_render_texture_a,
             RenderTexture* post_effect_render_texture_b){}
    void restoreRenderStates(RenderData* render_data){}
    void setRenderStates(RenderData* render_data, RenderState& rstate){}
    virtual void cullAndRender(RenderTarget* renderTarget, Scene* scene,
                        ShaderManager* shader_manager, PostEffectShaderManager* post_effect_shader_manager,
                        RenderTexture* post_effect_render_texture_a,
                        RenderTexture* post_effect_render_texture_b) {};
    void makeShadowMaps(Scene* scene, ShaderManager* shader_manager){}
    void set_face_culling(int cull_face){}
    virtual ShaderData* createMaterial(const std::string& desc);
    virtual RenderData* createRenderData();
    virtual UniformBlock* createUniformBlock(const std::string& desc, int binding, const std::string& name);
    Image* createImage(int type, int format);
    virtual Texture* createTexture(int target = GL_TEXTURE_2D);
    virtual RenderTexture* createRenderTexture(int width, int height, int sample_count,
                                               int jcolor_format, int jdepth_format, bool resolve_depth,
                                               const TextureParameters* texture_parameters);
    virtual RenderTexture* createRenderTexture(int width, int height, int sample_count, int layers) { }
    virtual VertexBuffer* createVertexBuffer(const std::string& desc, int vcount);
    virtual IndexBuffer* createIndexBuffer(int bytesPerIndex, int icount);
    virtual Shader* createShader(int id, const std::string& signature,
                                 const std::string& uniformDescriptor, const std::string& textureDescriptor,
                                 const std::string& vertexDescriptor, const std::string& vertexShader,
                                 const std::string& fragmentShader);
    virtual void renderWithShader(RenderState& rstate, Shader* shader, RenderData* renderData, ShaderData* shaderData);
private:
    VulkanCore* vulkanCore_;
    void renderMesh(RenderState& rstate, RenderData* render_data){}
    void renderMaterialShader(RenderState& rstate, RenderData* render_data, ShaderData *material, Shader*){}
    void occlusion_cull(RenderState& rstate,
                std::vector<SceneObject*>& scene_objects){
        occlusion_cull_init(rstate.scene, scene_objects);

    }


};
}
#endif //FRAMEWORK_VULKANRENDERER_H

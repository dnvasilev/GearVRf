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

#ifndef RENDERER_H_
#define RENDERER_H_

#include <vector>
#include <memory>
#include <unordered_map>

#include "glm/glm.hpp"
#include "batch.h"
#include "objects/eye_type.h"
#include "objects/mesh.h"
#include "objects/bounding_volume.h"
#include "shaders/shader_manager.h"
#include "batch_manager.h"

typedef unsigned long Long;

namespace gvr {
extern bool use_multiview;
class Camera;
class Scene;
class SceneObject;
class ShaderData;
class RenderData;
class RenderTarget;
class RenderTexture;
class Light;
class BitmapImage;
class CubemapImage;
class CompressedImage;
class FloatImage;
class VertexBuffer;
class IndexBuffer;
class UniformBlock;
class Image;
class Texture;

/*
 * These uniforms are commonly used in shaders.
 * They are calculated by the GearVRF renderer.
 */
struct ShaderUniformsPerObject {
    glm::mat4   u_model;        // Model matrix
    glm::mat4   u_view;         // View matrix
    glm::mat4   u_proj;         // projection matrix
    glm::mat4   u_view_[2];     // for multiview
    glm::mat4   u_view_inv;     // inverse of View matrix
    glm::mat4   u_view_inv_[2]; // inverse of View matrix
    glm::mat4   u_mv;           // ModelView matrix
    glm::mat4   u_mv_[2];       // ModelView matrix
    glm::mat4   u_mvp;          // ModelViewProjection matrix
    glm::mat4   u_mvp_[2];      // ModelViewProjection matrix
    glm::mat4   u_mv_it;        // inverse transpose of ModelView
    glm::mat4   u_mv_it_[2];    // inverse transpose of ModelView
    int         u_right;        // 1 = right eye, 0 = left
};

struct RenderState {
    int                     render_mask;
    int                     viewportX;
    int                     viewportY;
    int                     viewportWidth;
    int                     viewportHeight;
    bool                    invalidateShaders;
    Scene*                  scene;
    ShaderData*             material_override;
    ShaderUniformsPerObject uniforms;
    ShaderManager*          shader_manager;
    bool                    shadow_map;
};

class Renderer {
public:
    void resetStats() {
        numberDrawCalls = 0;
        numberTriangles = 0;
    }
    bool isVulkanInstace(){
        return isVulkan_;
    }
    void freeBatch(Batch* batch){
        batch_manager->freeBatch(batch);
    }
    int getNumberDrawCalls() {
        return numberDrawCalls;
    }

     int getNumberTriangles() {
        return numberTriangles;
     }
     int incrementTriangles(int number=1){
        return numberTriangles += number;
     }
     int incrementDrawCalls(){
        return ++numberDrawCalls;
     }
     static Renderer* getInstance(std::string type =  " ");
     static void resetInstance(){
        delete instance;
         instance = NULL;
     }
     virtual ShaderData* createMaterial(const std::string& desc) = 0;
     virtual RenderData* createRenderData() = 0;
     virtual UniformBlock* createUniformBlock(const std::string& desc, int, const std::string& name) = 0;
     virtual Image* createImage(int type, int format) = 0;
     virtual Texture* createTexture(int target = GL_TEXTURE_2D) = 0;
     virtual RenderTexture* createRenderTexture(int width, int height, int sample_count,
                                                int jcolor_format, int jdepth_format, bool resolve_depth,
                                                const TextureParameters* texture_parameters) = 0;
    virtual RenderTexture* createRenderTexture(int width, int height, int sample_count, int layers) = 0;
    virtual Shader* createShader(int id, const std::string& signature,
                                 const std::string& uniformDescriptor, const std::string& textureDescriptor,
                                 const std::string& vertexDescriptor, const std::string& vertexShader,
                                 const std::string& fragmentShader) = 0;
     virtual VertexBuffer* createVertexBuffer(const std::string& descriptor, int vcount) = 0;
     virtual IndexBuffer* createIndexBuffer(int bytesPerIndex, int icount) = 0;
     void updateTransforms(RenderState& rstate, UniformBlock* block, Transform* model);
     virtual void initializeStats();
     virtual void set_face_culling(int cull_face) = 0;
     virtual void renderRenderDataVector(RenderState &rstate);
     virtual void cull(Scene *scene, Camera *camera,
            ShaderManager* shader_manager);
     virtual void renderRenderData(RenderState& rstate, RenderData* render_data);


     virtual void renderCamera(Scene* scene, Camera* camera,
             ShaderManager* shader_manager,
             PostEffectShaderManager* post_effect_shader_manager,
             RenderTexture* post_effect_render_texture_a,
             RenderTexture* post_effect_render_texture_b) = 0;

     virtual void renderCamera(Scene* scene, Camera* camera, int viewportX,
             int viewportY, int viewportWidth, int viewportHeight,
             ShaderManager* shader_manager,
             PostEffectShaderManager* post_effect_shader_manager,
             RenderTexture* post_effect_render_texture_a,
             RenderTexture* post_effect_render_texture_b)=0;

     virtual void renderCamera(Scene* scene, Camera* camera, int framebufferId,
            int viewportX, int viewportY, int viewportWidth, int viewportHeight,
            ShaderManager* shader_manager,
            PostEffectShaderManager* post_effect_shader_manager,
            RenderTexture* post_effect_render_texture_a,
            RenderTexture* post_effect_render_texture_b) = 0;

     virtual void renderCamera(Scene* scene, Camera* camera,
            RenderTexture* render_texture, ShaderManager* shader_manager,
            PostEffectShaderManager* post_effect_shader_manager,
            RenderTexture* post_effect_render_texture_a,
            RenderTexture* post_effect_render_texture_b) = 0;
    virtual void cullFromCamera(Scene *scene, Camera *camera,
                                ShaderManager* shader_manager);
    virtual void restoreRenderStates(RenderData* render_data) = 0;
    virtual void setRenderStates(RenderData* render_data, RenderState& rstate) = 0;
    virtual Texture* createSharedTexture(int id) = 0;
    virtual int renderWithShader(RenderState& rstate, Shader* shader, RenderData* renderData, ShaderData* shaderData) = 0;
    virtual void cullAndRender(RenderTarget* renderTarget, Scene* scene,
                        ShaderManager* shader_manager, PostEffectShaderManager* post_effect_shader_manager,
                        RenderTexture* post_effect_render_texture_a,
                        RenderTexture* post_effect_render_texture_b) = 0;
    virtual void makeShadowMaps(Scene* scene, ShaderManager* shader_manager) = 0;
    virtual void occlusion_cull(RenderState& rstate, std::vector<SceneObject*>& scene_objects) = 0;
private:
    static bool isVulkan_;
    virtual void build_frustum(float frustum[6][4], const float *vp_matrix);
    virtual void frustum_cull(glm::vec3 camera_position, SceneObject *object,
            float frustum[6][4], std::vector<SceneObject*>& scene_objects,
            bool continue_cull, int planeMask);

    Renderer(const Renderer& render_engine);
    Renderer(Renderer&& render_engine);
    Renderer& operator=(const Renderer& render_engine);
    Renderer& operator=(Renderer&& render_engine);
    BatchManager* batch_manager;
    static Renderer* instance;
    
protected:
    Renderer();
    virtual ~Renderer(){
        if(batch_manager)
            delete batch_manager;
        batch_manager = NULL;
    }
    virtual void state_sort();
    virtual void renderMesh(RenderState& rstate, RenderData* render_data) = 0;
    virtual void renderMaterialShader(RenderState& rstate, RenderData* render_data, ShaderData *material, Shader* shader) = 0;
    void addRenderData(RenderData *render_data, Scene* scene);
    virtual bool occlusion_cull_init(Scene* scene, std::vector<SceneObject*>& scene_objects);

    virtual void renderPostEffectData(RenderState& rstate,
            Texture* render_texture, ShaderData* post_effect_data);
    RenderData* post_effect_render_data();

    std::vector<RenderData*> render_data_vector;
    int numberDrawCalls;
    int numberTriangles;
    bool useStencilBuffer_ = false;

public:
    //to be used only on the rendering thread
    const std::vector<RenderData*>& getRenderDataVector() const { return render_data_vector; }
    int numLights;
    RenderData* post_effect_render_data_;
    void setUseStencilBuffer(bool enable) { useStencilBuffer_ = enable; }
};
extern Renderer* gRenderer;
}
#endif

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

#include "util/jni_utils.h"
#include "objects/hybrid_object.h"
#include "objects/scene.h"
#include "objects/components/render_data.h"
#include "gl/gl_material.h"

namespace gvr {

RenderData::~RenderData() {
}

void RenderData::add_pass(RenderPass* render_pass) {
    render_pass_list_.push_back(render_pass);
    render_pass->add_dirty_flag(dirty_flag_);
    *dirty_flag_ = true;
}

const RenderPass* RenderData::pass(int pass) const {
    if (pass >= 0 && pass < render_pass_list_.size()) {
        return render_pass_list_[pass];
    }
}

void RenderData::set_mesh(Mesh* mesh) {
    mesh_ = mesh;
    mesh->add_dirty_flag(dirty_flag_);
    *dirty_flag_ = true;
}

void RenderData::setDirty(bool dirty){
    *dirty_flag_ = dirty;
}

bool RenderData::cull_face(int pass) const {
    if (pass >= 0 && pass < render_pass_list_.size()) {
        return render_pass_list_[pass]->cull_face();
    }
}

ShaderData* RenderData::material(int pass) const {
    if (pass >= 0 && pass < render_pass_list_.size()) {
        return render_pass_list_[pass]->material();
    }
}

void RenderData::setCameraDistanceLambda(std::function<float()> func)
{
    cameraDistanceLambda_ = func;
}

JNIEnv *RenderData::set_java(jobject javaObj, JavaVM *javaVM)
{
    JNIEnv *env = JavaComponent::set_java(javaObj, javaVM);
    if (env)
    {
        jclass renderDataClass = env->GetObjectClass(javaObj);
        bindShaderMethod_ = env->GetMethodID(renderDataClass, "bindShaderNative",
                                             "(Lorg/gearvrf/GVRScene;)V");
        if (bindShaderMethod_ == 0)
        {
            LOGE("RenderData::bindShader ERROR cannot find 'GVRRenderData.bindShaderNative()' Java method");
        }
    }
}


/**
 * Called when the shader for a RenderData needs to be generated on the Java side.
 * This function spawns a Java task on the Framework thread which generates the shader.
 */
void RenderData::bindShader(Scene *scene)
{
    if ((bindShaderMethod_ == NULL) || (javaObj_ == NULL))
    {
        LOGE("SHADER: RenderData::bindShader could not call bindShaderNative");
    }

    JNIEnv* env = NULL;
    int rc = scene->get_java_env(&env);
    if (env && (rc >= 0))
    {
        LOGD("SHADER: Calling GVRRenderData.bindShaderNative(%p)", this);
        env->CallVoidMethod(javaObj_, bindShaderMethod_, scene->getJavaObj());
        if (rc > 0)
        {
            scene->getJavaVM()->DetachCurrentThread();
        }
    }
}

bool compareRenderDataByShader(RenderData *i, RenderData *j)
{
    // Compare renderData by their material's shader type
    // Note: multi-pass renderData is skipped for now and put to later position,
    // since each of the passes has a separate material (and shader as well).
    // An advanced sorting may be added later to take multi-pass into account
    if (j->pass_count() > 1) {
        return true;
    }

    if (i->pass_count() > 1) {
        return false;
    }

    return i->get_shader(0) < j->get_shader(0);
}

bool compareRenderDataByOrderShaderDistance(RenderData *i, RenderData *j) {
    //1. rendering order needs to be sorted first to guarantee specified correct order
    if (i->rendering_order() == j->rendering_order())
    {
        if (i->get_shader(0) == j->get_shader(0))
        {
            int no_passes1 = i->pass_count();
            int no_passes2 = j->pass_count();

            if (no_passes1 == no_passes2)
            {
                // if it is a transparent object, sort by camera distance from back to front
                if (i->rendering_order() >= RenderData::Transparent
                    && i->rendering_order() < RenderData::Overlay)
                {
                    return i->camera_distance() > j->camera_distance();
                }

                //@todo what about the other passes

                //this is pointer comparison; assumes batching is on; if the materials are not
                //the same then comparing the pointers further is an arbitrary decision; hence
                //falling back to camera distance.
                if (i->material(0) == j->material(0))
                {
                    if (i->cull_face(0) == j->cull_face(0))
                    {
                        if (i->getHashCode().compare(j->getHashCode()) == 0)
                        {
                            // otherwise sort from front to back
                            return i->camera_distance() < j->camera_distance();
                        }
                        return i->getHashCode() < j->getHashCode();
                    }
                    return i->cull_face(0) < j->cull_face(0);
                }
                return i->material(0) < j->material(0);
            }
            return no_passes1 < no_passes2;
        }
        return i->get_shader() < j->get_shader();
    }
    return i->rendering_order() < j->rendering_order();
}

std::string RenderData::getHashCode()
{
    if (hash_code_dirty_)
    {
        std::string render_data_string;
        render_data_string.append(to_string(use_light_));
        render_data_string.append(to_string(light_));
        render_data_string.append(to_string(getComponentType()));
        render_data_string.append(to_string(use_lightmap_));
        render_data_string.append(to_string(render_mask_));
        render_data_string.append(to_string(offset_));
        render_data_string.append(to_string(offset_factor_));
        render_data_string.append(to_string(offset_units_));
        render_data_string.append(to_string(depth_test_));
        render_data_string.append(to_string(alpha_blend_));
        render_data_string.append(to_string(alpha_to_coverage_));
        render_data_string.append(to_string(sample_coverage_));
        render_data_string.append(to_string(invert_coverage_mask_));
        render_data_string.append(to_string(draw_mode_));

        hash_code = render_data_string;
        hash_code_dirty_ = false;

    }
    return hash_code;
}

void RenderData::updateBones(const float* boneData, int numFloats)
{
    if (bones_ubo_ == NULL)
    {
        bones_ubo_ = Renderer::getInstance()->createUniformBlock("mat4 u_bone_matrix[60];");
    }
    bones_ubo_->setFloatVec("u_bone_matrix", boneData, numFloats);
}
}

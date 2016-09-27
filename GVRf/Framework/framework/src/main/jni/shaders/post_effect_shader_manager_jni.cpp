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
 * JNI
 ***************************************************************************/

#include "post_effect_shader_manager.h"

#include "util/gvr_jni.h"

namespace gvr {
extern "C" {
JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativePostEffectShaderManager_ctor(
        JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL
Java_org_gearvrf_NativePostEffectShaderManager_addShader(
        JNIEnv * env, jobject obj, jlong jpost_effect_shader_manager,
        jstring signature, jstring vertex_shader, jstring fragment_shader);
JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativePostEffectShaderManager_getShaderByID(
        JNIEnv * env, jobject obj, jlong jpost_effect_shader_manager, jint id);

JNIEXPORT jint JNICALL
Java_org_gearvrf_NativePostEffectShaderManager_getShader(
        JNIEnv * env, jobject obj, jlong jpost_effect_shader_manager, jstring signature);

JNIEXPORT void JNICALL
Java_org_gearvrf_NativePostEffectShaderManager_delete(
        JNIEnv * env, jobject obj, jlong jpost_effect_shader_manager);
}

JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativePostEffectShaderManager_ctor(
        JNIEnv * env, jobject obj) {
    return reinterpret_cast<jlong>(new PostEffectShaderManager());
}

JNIEXPORT jint JNICALL
Java_org_gearvrf_NativePostEffectShaderManager_addShader(
        JNIEnv * env, jobject obj, jlong jpost_effect_shader_manager,
        jstring signature,
        jstring vertex_shader, jstring fragment_shader) {
    PostEffectShaderManager* post_effect_shader_manager =
            reinterpret_cast<PostEffectShaderManager*>(jpost_effect_shader_manager);

    const char *sig_str = env->GetStringUTFChars(signature, 0);
    const char *vertex_str = env->GetStringUTFChars(vertex_shader, 0);
    const char *fragment_str = env->GetStringUTFChars(fragment_shader, 0);
    std::string native_sig(sig_str);
    std::string native_vertex_shader(vertex_str);
    std::string native_fragment_shader(fragment_str);

    int id = post_effect_shader_manager->addShader(native_sig,
            native_vertex_shader, native_fragment_shader);

    env->ReleaseStringUTFChars(vertex_shader, vertex_str);
    env->ReleaseStringUTFChars(fragment_shader, fragment_str);
    env->ReleaseStringUTFChars(signature, sig_str);
    return id;
}

JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativePostEffectShaderManager_getShaderByID(
        JNIEnv * env, jobject obj, jlong jpost_effect_shader_manager, jint id) {
    PostEffectShaderManager* post_effect_shader_manager =
            reinterpret_cast<PostEffectShaderManager*>(jpost_effect_shader_manager);
    try {
        Shader* custom_post_effect_shader =
                post_effect_shader_manager->getShader(id);
        return reinterpret_cast<jlong>(custom_post_effect_shader);
    } catch (char const *c) {
        return 0;
    }
}

JNIEXPORT jint JNICALL
Java_org_gearvrf_NativePostEffectShaderManager_getShader(
    JNIEnv * env, jobject obj, jlong jshader_manager, jstring signature) {
    PostEffectShaderManager* shader_manager = reinterpret_cast<PostEffectShaderManager*>(jshader_manager);
    const char *sig_str = env->GetStringUTFChars(signature, 0);
    std::string native_sig = std::string(sig_str);
    try {
        return reinterpret_cast<jint>(shader_manager->findShader(native_sig));
    } catch (char const *e) {
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_org_gearvrf_NativePostEffectShaderManager_delete(
        JNIEnv * env, jobject obj, jlong jpost_effect_shader_manager) {
    delete reinterpret_cast<PostEffectShaderManager*>(jpost_effect_shader_manager);
}

}

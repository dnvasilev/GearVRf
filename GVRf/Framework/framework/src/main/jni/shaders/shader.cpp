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

#include <jni_utils.h>
#include "shader.h"

namespace gvr {

const bool Shader::LOG_SHADER = true;

Shader::Shader(int id,
               const char* signature,
               const char* uniformDescriptor,
               const char* textureDescriptor,
               const char* vertexDescriptor,
               const char* vertex_shader,
               const char* fragment_shader)
    : mId(id), mSignature(signature),
      mUniformDesc(uniformDescriptor),
      mTextures(textureDescriptor),
      mVertexDesc(vertexDescriptor),
      mVertexShader(vertex_shader),
      mFragmentShader(fragment_shader), shaderDirty(true),
      mUseTransformBuffer(false), mUseLights(false),
      mJavaShaderClass(0), mJavaVM(nullptr), mCalcMatrixMethod(0)
{
    if (strstr(vertex_shader, "Transform_ubo") ||
        strstr(fragment_shader, "Transform_ubo"))
        mUseTransformBuffer = true;
    if (strstr(signature, "$LIGHTSOURCES"))
        mUseLights = true;
}

void Shader::setJava(jclass shaderClass, JavaVM *javaVM)
{
    JNIEnv *env = getCurrentEnv(javaVM);
    mJavaVM = javaVM;
    if (env)
    {
        mJavaShaderClass = shaderClass;
        mCalcMatrixMethod = env->GetStaticMethodID(shaderClass, "calcMatrix",
                                             "(Ljava/nio/FloatBuffer;Ljava/nio/FloatBuffer;)V");
    }
}

bool Shader::calcMatrix(float* inputMatrices, int inputSize, float* outputMatrices, int outputSize)
{
    if (mJavaVM && mJavaShaderClass && mCalcMatrixMethod)
    {
        JNIEnv *env = getCurrentEnv(mJavaVM);
        jobject inputBuffer = env->NewDirectByteBuffer((void *) inputMatrices, inputSize);
        jobject outputBuffer = env->NewDirectByteBuffer((void *) outputMatrices, outputSize);
        env->CallStaticVoidMethod(mJavaShaderClass, mCalcMatrixMethod, inputBuffer, outputBuffer);
    }
}

int Shader::calcSize(const char* type)
{
    return DataDescriptor::calcSize(type) / sizeof(float);
}



} /* namespace gvr */

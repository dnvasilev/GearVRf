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

#include <engine/renderer/renderer.h>
#include "objects/vertex_buffer.h"

#include "util/gvr_log.h"
#include "util/gvr_jni.h"

namespace gvr {
    extern "C" {
    JNIEXPORT jlong JNICALL
    Java_org_gearvrf_NativeVertexBuffer_ctor(JNIEnv* env, jobject obj,
                                             jstring descriptor, int vertexCount);
    JNIEXPORT bool JNICALL
    Java_org_gearvrf_NativeVertexBuffer_getIntVec(JNIEnv* env, jobject obj,
                                                  jlong jvbuf, jstring attribName,
                                                  jintArray data, jint stride);

    JNIEXPORT bool JNICALL
    Java_org_gearvrf_NativeVertexBuffer_setIntVec(JNIEnv* env, jobject obj,
                                                  jlong jvbuf, jstring attribName,
                                                  jintArray data, jint stride);

    JNIEXPORT bool JNICALL
    Java_org_gearvrf_NativeVertexBuffer_getFloatVec(JNIEnv* env, jobject obj,
                                                    jlong jvbuf, jstring attribName,
                                                    jobject jfloatbuf, jint stride);
    JNIEXPORT jfloatArray JNICALL
    Java_org_gearvrf_NativeVertexBuffer_getFloatArray(JNIEnv * env, jobject obj,
                                                         jlong jvbuf, jstring attribName);

    JNIEXPORT bool JNICALL
    Java_org_gearvrf_NativeVertexBuffer_setFloatArray(JNIEnv* env, jobject obj,
                                                    jlong jvbuf, jstring attribName,
                                                    jfloatArray data, jint stride);
    JNIEXPORT bool JNICALL
            Java_org_gearvrf_NativeVertexBuffer_setFloatVec(JNIEnv* env, jobject obj,
                                                    jlong jvbuf, jstring attribName,
                                                    jobject jfloatbuf, jint stride);

    JNIEXPORT bool JNICALL
    Java_org_gearvrf_NativeVertexBuffer_isSet(JNIEnv* env, jobject obj,
                                              jlong jvbuf, jstring attribName);
    JNIEXPORT int JNICALL
    Java_org_gearvrf_NativeVertexBuffer_getVertexCount(JNIEnv* env, jobject obj,
                                                      jlong jvbuf);

    JNIEXPORT int JNICALL
    Java_org_gearvrf_NativeVertexBuffer_getAttributeSize(JNIEnv* env, jobject obj,
                                                        jlong jvbuf, jstring attribName);

    JNIEXPORT int JNICALL
    Java_org_gearvrf_NativeVertexBuffer_getBoundingVolume(JNIEnv* env, jobject obj,
                                                         jlong jvbuf, jobject floatbuf);
};

JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeVertexBuffer_ctor(JNIEnv* env, jobject obj, jstring descriptor, int vertexCount)
{
    const char* char_desc = env->GetStringUTFChars(descriptor, 0);
    std::string native_desc = std::string(char_desc);
    VertexBuffer* vbuf = Renderer::getInstance()->createVertexBuffer(native_desc, vertexCount);
    env->ReleaseStringUTFChars(descriptor, char_desc);
    return reinterpret_cast<jlong>(vbuf);
}

JNIEXPORT bool JNICALL
Java_org_gearvrf_NativeVertexBuffer_getFloatVec(JNIEnv * env, jobject obj,
                                                jlong jvbuf, jstring attribName, jobject jfloatbuf, jint stride)
{
    VertexBuffer* vbuf = reinterpret_cast<VertexBuffer*>(jvbuf);
    const char* char_key = env->GetStringUTFChars(attribName, 0);
    std::string native_key = std::string(char_key);
    void* bufptr = env->GetDirectBufferAddress(jfloatbuf);
    bool rc = false;
    if (bufptr)
    {
        int capacity = env->GetDirectBufferCapacity(jfloatbuf);
        rc = vbuf->getFloatVec(native_key, (float *) bufptr, capacity, stride);
    }
    env->ReleaseStringUTFChars(attribName, char_key);
    return rc;
}

JNIEXPORT jfloatArray JNICALL
Java_org_gearvrf_NativeVertexBuffer_getFloatArray(JNIEnv * env, jobject obj,
                                                jlong jvbuf, jstring attribName)
{
    VertexBuffer* vbuf = reinterpret_cast<VertexBuffer*>(jvbuf);
    const char* char_key = env->GetStringUTFChars(attribName, 0);
    std::string native_key = std::string(char_key);
    VertexBuffer::DataEntry* entry = vbuf->find(native_key);

    if (entry == NULL)
    {
        return nullptr;
    }
    int n = (vbuf->getVertexCount() * entry->Size) / sizeof(float);
    jfloatArray jdata = env->NewFloatArray(n);
    float* data = env->GetFloatArrayElements(jdata, 0);
    vbuf->getFloatVec(native_key, data, n, 0);
    env->ReleaseFloatArrayElements(jdata, data, 0);
    env->ReleaseStringUTFChars(attribName, char_key);
    return jdata;
}

JNIEXPORT bool JNICALL
Java_org_gearvrf_NativeVertexBuffer_getIntVec(JNIEnv * env, jobject obj,
                                              jlong jvbuf, jstring attribName, jintArray jdata, jint stride)
{
    VertexBuffer* vbuf = reinterpret_cast<VertexBuffer*>(jvbuf);
    const char* char_key = env->GetStringUTFChars(attribName, 0);
    std::string native_key = std::string(char_key);

    jint* attribData = env->GetIntArrayElements(jdata, 0);
    bool rc = vbuf->getIntVec(native_key, attribData, static_cast<int>(env->GetArrayLength(jdata)), stride);
    env->ReleaseStringUTFChars(attribName, char_key);
    return rc;
}

JNIEXPORT bool JNICALL
Java_org_gearvrf_NativeVertexBuffer_setFloatArray(JNIEnv * env, jobject obj,
                                                jlong jvbuf, jstring attribName, jfloatArray jdata, jint stride)
{
    VertexBuffer* vbuf = reinterpret_cast<VertexBuffer*>(jvbuf);
    const char* char_key = env->GetStringUTFChars(attribName, 0);
    std::string native_key = std::string(char_key);

    jfloat* attribData = env->GetFloatArrayElements(jdata, 0);
    bool rc = vbuf->setFloatVec(native_key, attribData, static_cast<int>(env->GetArrayLength(jdata)), stride);
    env->ReleaseStringUTFChars(attribName, char_key);
    return rc;
}

JNIEXPORT bool JNICALL
Java_org_gearvrf_NativeVertexBuffer_setFloatVec(JNIEnv * env, jobject obj,
                                                jlong jvbuf, jstring attribName, jobject jfloatbuf, jint stride)
{
    VertexBuffer* vbuf = reinterpret_cast<VertexBuffer*>(jvbuf);
    const char* char_key = env->GetStringUTFChars(attribName, 0);
    std::string native_key = std::string(char_key);
    void* bufptr = env->GetDirectBufferAddress(jfloatbuf);
    bool rc = false;
    if (bufptr)
    {
        int capacity = env->GetDirectBufferCapacity(jfloatbuf);
        rc = vbuf->setFloatVec(native_key, (float *) bufptr, capacity, stride);
    }
    env->ReleaseStringUTFChars(attribName, char_key);
    return rc;
}

JNIEXPORT bool JNICALL
Java_org_gearvrf_NativeVertexBuffer_setIntVec(JNIEnv * env, jobject obj,
                                              jlong jvbuf, jstring attribName, jintArray jdata, jint stride)
{
    VertexBuffer* vbuf = reinterpret_cast<VertexBuffer*>(jvbuf);
    const char* char_key = env->GetStringUTFChars(attribName, 0);
    std::string native_key = std::string(char_key);

    jint* attribData = env->GetIntArrayElements(jdata, 0);
    bool rc = vbuf->setIntVec(native_key, attribData, static_cast<int>(env->GetArrayLength(jdata)), stride);
    env->ReleaseStringUTFChars(attribName, char_key);
    return rc;
}

JNIEXPORT bool JNICALL
Java_org_gearvrf_NativeVertexBuffer_isSet(JNIEnv* env, jobject obj,
                                          jlong jvbuf, jstring attribName)
{
    VertexBuffer* vbuf = reinterpret_cast<VertexBuffer*>(jvbuf);
    const char* char_key = env->GetStringUTFChars(attribName, 0);
    std::string native_key = std::string(char_key);

    bool rc = vbuf->isSet(native_key);
    env->ReleaseStringUTFChars(attribName, char_key);
    return rc;
}

JNIEXPORT int JNICALL
Java_org_gearvrf_NativeVertexBuffer_getVertexCount(JNIEnv* env, jobject obj, jlong jvbuf)
{
    VertexBuffer* vbuf = reinterpret_cast<VertexBuffer*>(jvbuf);
    return vbuf->getVertexCount();
}

JNIEXPORT int JNICALL
Java_org_gearvrf_NativeVertexBuffer_getAttributeSize(JNIEnv* env, jobject obj,
                                                     jlong jvbuf, jstring attribName)
{
    VertexBuffer* vbuf = reinterpret_cast<VertexBuffer*>(jvbuf);
    const char* char_key = env->GetStringUTFChars(attribName, 0);
    std::string native_key = std::string(char_key);
    int size = vbuf->getByteSize(native_key) / sizeof(float);
    env->ReleaseStringUTFChars(attribName, char_key);
    return size;
}

JNIEXPORT int JNICALL
Java_org_gearvrf_NativeVertexBuffer_getBoundingVolume(JNIEnv* env, jobject obj,
                                                      jlong jvbuf, jobject jfloatbuf)
{
    VertexBuffer* vbuf = reinterpret_cast<VertexBuffer*>(jvbuf);
    void* bufptr = env->GetDirectBufferAddress(jfloatbuf);
    if (bufptr)
    {
        int capacity = env->GetDirectBufferCapacity(jfloatbuf);
        if (capacity < 4)
        {
            LOGE("VertexBuffer::getBoundingVolume destination buffer must hold at least 4 floats");
            return -1;
        }
        BoundingVolume bv;
        float* f = (float*) bufptr;
        vbuf->getBoundingVolume(bv);
        if (capacity == 4)
        {
            f[0] = bv.center().x;
            f[1] = bv.center().y;
            f[2] = bv.center().z;
            f[0] = bv.radius();
        }
        else if (capacity == 6)
        {
            f[0] = bv.min_corner().x;
            f[1] = bv.min_corner().y;
            f[2] = bv.min_corner().z;
            f[3] = bv.max_corner().x;
            f[4] = bv.max_corner().y;
            f[5] = bv.max_corner().z;
        }
        else if (capacity >= 10)
        {
            f[0] = bv.center().x;
            f[1] = bv.center().y;
            f[2] = bv.center().z;
            f[3] = bv.min_corner().x;
            f[4] = bv.min_corner().y;
            f[5] = bv.min_corner().z;
            f[6] = bv.max_corner().x;
            f[7] = bv.max_corner().y;
            f[8] = bv.max_corner().z;
            f[9] = bv.radius();
        }
        return (bv.radius() > 0) ? 1 : 0;
    }
    return -1;
}

}

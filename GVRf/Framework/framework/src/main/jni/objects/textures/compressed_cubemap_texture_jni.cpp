/***************************************************************************
 * JNI
 ***************************************************************************/

#include "cubemap_image.h"
#include "util/gvr_jni.h"
#include "util/gvr_java_stack_trace.h"
#include "android/asset_manager_jni.h"

namespace gvr {
extern "C" {
    JNIEXPORT jlong JNICALL
    Java_org_gearvrf_NativeCompressedCubemapTexture_constructor(JNIEnv * env,
            jobject obj, jint internalFormat, jint width, jint height, jint imageSize,
            jobjectArray textureArray, jintArray joffsetArray);

    JNIEXPORT void JNICALL
    Java_org_gearvrf_NativeCompressedCubemapTexture_update(JNIEnv * env, jobject obj, jobject jTexture,
        jint imageSize, jobjectArray textureArray, jintArray joffsetArray);
}
;

JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeCompressedCubemapTexture_constructor(JNIEnv * env,
    jobject obj, jint internalFormat, jint width, jint height, jint imageSize,
    jobjectArray textureArray, jintArray joffsetArray) {
    if (env->GetArrayLength(textureArray) != 6) {
        std::string error =
        "new CubemapImage() failed! Input texture list's length is not 6.";
        throw error;
    }
    if (env->GetArrayLength(joffsetArray) != 6) {
        std::string error =
        "new CubemapImage() failed! Texture offset list's length is not 6.";
        throw error;
    }
    try {
        jint* texture_offsets = env->GetIntArrayElements(joffsetArray, 0);
        jlong rv = reinterpret_cast<jlong>(new CubemapImage(env,
                internalFormat, width, height, imageSize,
                textureArray, texture_offsets));
        env->ReleaseIntArrayElements(joffsetArray, texture_offsets, 0);
        return rv;
    } catch (const std::string &err) {
        printJavaCallStack(env, err);
        throw err;
    }
}

JNIEXPORT void JNICALL
Java_org_gearvrf_NativeCompressedCubemapTexture_update(JNIEnv* env,
    jobject obj, jobject jTexture, jint imageSize,
    jobjectArray textureArray, jintArray joffsetArray)
{
    if (env->GetArrayLength(joffsetArray) != 6)
    {
        std::string error =
                "new CubemapImage() failed! Texture offset list's length is not 6.";
        throw error;
    }
    jint* offsets = env->GetIntArrayElements(joffsetArray, 0);
    CubemapImage* cubemap = reinterpret_cast<CubemapImage*>(jTexture);
    cubemap->update(env, imageSize, textureArray, offsets);
    env->ReleaseIntArrayElements(joffsetArray, offsets, 0);
}
;
}

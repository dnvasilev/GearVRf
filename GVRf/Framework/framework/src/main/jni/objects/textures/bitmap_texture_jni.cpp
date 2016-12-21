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

#include "bitmap_texture.h"
#include "util/gvr_jni.h"
#include "util/gvr_java_stack_trace.h"
#include "android/asset_manager_jni.h"


namespace gvr {

extern "C" {
    JNIEXPORT jlong JNICALL
    Java_org_gearvrf_NativeBitmapImage_constructor(JNIEnv * env, jobject obj,
            jint width, jint height, jbyteArray data);

    JNIEXPORT void JNICALL
    Java_org_gearvrf_NativeBitmapImage_update(JNIEnv * env, jobject obj,
            jlong jtexture, jint width, jint height, jbyteArray jdata);
}

JNIEXPORT jlong JNICALL
Java_org_gearvrf_NativeBitmapImage_constructor(JNIEnv * env, jobject obj,
                jint width, jint height, jbyteArray data)
{
    jlong result =  reinterpret_cast<jlong>(new BitmapImage(env, width, height, data));
    return result;
}

JNIEXPORT void JNICALL
Java_org_gearvrf_NativeBitmapImage_update(JNIEnv * env, jobject obj,
        jlong jtexture, jint width, jint height, jbyteArray jdata) {
    BitmapImage* texture = reinterpret_cast<BitmapImage*>(jtexture);
    texture->update(env, width, height, jdata);
}

}

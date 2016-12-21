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
 * Texture from a (Java-loaded) byte stream containing a compressed texture
 ***************************************************************************/

#ifndef bitmap_texture_H_
#define bitmap_texture_H_

#include "objects/textures/texture.h"
#include "util/gvr_jni.h"
#include "util/gvr_log.h"
#include "util/jni_utils.h"

namespace gvr {
class BitmapImage : public Image
{
public:

    explicit BitmapImage(JNIEnv* env, int width, int height, jbyteArray data) :
            Image(GL_TEXTURE_2D, width, height, width * height, GL_LUMINANCE, 1)
    {
        if (JNI_OK != env->GetJavaVM(&mJava)) {
            FAIL("GetJavaVM failed");
        }
        update(env, width, height, data);
    }

    explicit BitmapImage(JNIEnv* env, int target, int width, int height, int imagesize, int format, int levels) :
            Image(target, width, height, imagesize, format, levels)
    {
        if (JNI_OK != env->GetJavaVM(&mJava)) {
            FAIL("GetJavaVM failed");
        }
    }

    virtual ~BitmapImage()
    {
        JNIEnv* env = getCurrentEnv(mJava);
        env->DeleteGlobalRef(mData);
    }

    void update(JNIEnv* env, int width, int height, jbyteArray data)
    {
        mWidth = width;
        mHeight = height;
        mData = static_cast<jbyteArray>(env->NewGlobalRef(data));
        mPendingUpdate = true;
    }

    virtual void update(int texid)
    {
        JNIEnv* env = getCurrentEnv(mJava);
        jbyte* data = env->GetByteArrayElements(mData, 0);
        env->ReleaseByteArrayElements(mData, data, 0);
        env->DeleteGlobalRef(mData);
        glBindTexture(GL_TEXTURE_2D, texid);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mWidth, mHeight, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, mData);
        glGenerateMipmap (GL_TEXTURE_2D);
        mPendingUpdate = false;
    }

private:
    BitmapImage(const BitmapImage& texture) = delete;
    BitmapImage(BitmapImage&& texture) = delete;
    BitmapImage& operator=(const BitmapImage& texture) = delete;
    BitmapImage& operator=(BitmapImage&& texture) = delete;


protected:
    JavaVM* mJava;
    jbyteArray mData;
};

}
#endif

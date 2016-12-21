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
 * Cube map texture made by six bitmaps.
 ***************************************************************************/

#ifndef CUBEMAP_TEXTURE_H_
#define CUBEMAP_TEXTURE_H_

#include <string>

#include <android/bitmap.h>

#include "objects/textures/texture.h"
#include "util/gvr_log.h"
#include "util/scope_exit.h"
#include "util/jni_utils.h"

namespace gvr {
class CubemapTexture: public Image
{
public:
    explicit CubemapTexture(JNIEnv* env, jobjectArray bitmapArray) :
            Image(GL_TEXTURE_CUBE_MAP, GL_RGBA)
    {
        if (JNI_OK != env->GetJavaVM(&mJava))
        {
            FAIL("GetJavaVM failed");
        }
        for (int i = 0; i < 6; i++)
        {
            mBitmaps[i] = env->NewGlobalRef(env->GetObjectArrayElement(bitmapArray, i));
        }
        mPendingUpdate = true;
    }

    explicit CubemapTexture(JNIEnv* env, int format, int width, int height, int imageSize,
            jobjectArray textureArray, int* dataOffsets) :
            Image(GL_TEXTURE_CUBE_MAP, width, height, imageSize, format, 6)
    {
        if (JNI_OK != env->GetJavaVM(&mJava))
        {
            FAIL("GetJavaVM failed");
        }
        setDataOffsets(dataOffsets, 6);
        for (int i = 0; i < 6; i++)
        {
            mTextures[i] = env->NewGlobalRef(env->GetObjectArrayElement(textureArray, i));
        }
        mPendingUpdate = true;
    }

    void update(JNIEnv* env, jobjectArray bitmapArray)
    {
        clearData(env);
        for (int i = 0; i < 6; i++)
        {
            mBitmaps[i] = env->NewGlobalRef(env->GetObjectArrayElement(bitmapArray, i));
        }
        mPendingUpdate = true;
    }

    void update(JNIEnv* env, int imageSize,
                jobjectArray textureArray, int* textureOffset)
    {
        mImageSize = imageSize;
        setDataOffsets(textureOffset, 6);
        clearData(env);
        for (int i = 0; i < 6; i++)
        {
            mTextures[i] = env->NewGlobalRef(env->GetObjectArrayElement(textureArray, i));
        }
        mPendingUpdate = true;
    }

    virtual ~CubemapTexture()
    {
        JNIEnv* env = getCurrentEnv(mJava);
        clearData(env);
    }

    virtual void update(int texid)
    {
        if (mBitmaps != NULL)
        {
            updateFromBitmap(texid);
            mPendingUpdate = false;
        }
        else if (mTextures != NULL)
        {
            updateFromMemory(texid);
            mPendingUpdate = false;
        }
    }
private:
    CubemapTexture(const CubemapTexture& base_texture);
    CubemapTexture(CubemapTexture&& base_texture);
    CubemapTexture& operator=(const CubemapTexture& base_texture);
    CubemapTexture& operator=(CubemapTexture&& base_texture);

    void clearData(JNIEnv* env)
    {
        if (mBitmaps != NULL)
        {
            for (int i = 0; i < 6; i++) {
                env->DeleteGlobalRef(mBitmaps[i]);
            }
        }
        if (mTextures != NULL)
        {
            for (int i = 0; i < 6; i++) {
                env->DeleteGlobalRef(mTextures[i]);
            }
        }
    }

    void updateFromBitmap(int texid)
    {
        JNIEnv *env = getCurrentEnv(mJava);
        // Clean up upon scope exit. The SCOPE_EXIT utility is used
        // to avoid duplicated code in the throw case and normal
        // case.
        SCOPE_EXIT( clearData(env); );
        glBindTexture(mType, texid);
        for (int i = 0; i < 6; i++)
        {
            jobject bitmap = mBitmaps[i];

            AndroidBitmapInfo info;
            void *pixels;
            int ret;

            if (bitmap == NULL)
            {
                std::string error = "new BaseTexture() failed! Input bitmap is NULL.";
                throw error;
            }
            if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0)
            {
                std::string error = "AndroidBitmap_getInfo () failed! error = " + ret;
                throw error;
            }
            if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0)
            {
                std::string error = "AndroidBitmap_lockPixels () failed! error = " + ret;
                throw error;
            }
            if (i == 0)
            {
                mWidth = info.width;
                mHeight = info.height;
                mImageSize = info.height * info.stride;
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                         info.width, info.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         pixels);
            AndroidBitmap_unlockPixels(env, bitmap);
        }
    }

    void updateFromMemory(int texid)
    {
        JNIEnv *env = getCurrentEnv(mJava);
        // Clean up upon scope exit
        SCOPE_EXIT( clearData(env); );

        glBindTexture(mType, texid);
        for (int i = 0; i < 6; i++)
        {
            jbyteArray byteArray = static_cast<jbyteArray>(mTextures[i]);
            jbyte *textureData = env->GetByteArrayElements(byteArray, 0);

            if (byteArray == NULL)
            {
                std::string error = "new CubemapTexture() failed! Input texture is NULL.";
                throw error;
            }
            glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                                   mFormat, mWidth, mWidth, 0, mImageSize,
                                   textureData + getDataOffset(i));
            env->ReleaseByteArrayElements(byteArray, textureData, 0);
        }
    }

private:
    JavaVM* mJava;
    jobject mBitmaps[6];
    jobject mTextures[6];
};

}
#endif

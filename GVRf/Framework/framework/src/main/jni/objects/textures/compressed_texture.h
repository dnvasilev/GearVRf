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

#ifndef compressed_texture_H_
#define compressed_texture_H_

#include "util/gvr_jni.h"
#include "util/gvr_log.h"
#include "util/jni_utils.h"
#include "bitmap_image.h"

namespace gvr
{
class CompressedTexture : public BitmapImage
{
public:

    // The constructor to use when loading a single-level texture
    explicit CompressedTexture(JNIEnv *env, int target, int format,
                               int width, int height, jbyteArray bytes,
                               int levels, int *dataOffsets) :
            BitmapImage(env, target, width, height, env->GetArrayLength(bytes), format, levels)
    {
        update(env, width, height, bytes, levels, dataOffsets);
    }

    void update(JNIEnv *env, int width, int height, jbyteArray bytes, int levels,
                int *dataOffsets)
    {
        env->DeleteGlobalRef(mData);
        mWidth = width;
        mHeight = height;
        mLevels = levels;
        mImageSize = env->GetArrayLength(bytes);
        mData = static_cast<jbyteArray>(env->NewGlobalRef(bytes));
        setDataOffsets(dataOffsets, mLevels);
        mPendingUpdate = true;
    }

    virtual void update(int texid)
    {
        JNIEnv *env = getCurrentEnv(mJava);
        jbyte *data = env->GetByteArrayElements(mData, 0);
        glBindTexture(mType, texid);
        if (mLevels > 1) {
            loadCompressedMipMaps(data);
        }
        else {
            glCompressedTexImage2D(mType, 0, mFormat, mWidth, mHeight, 0,
                                   mImageSize, data + getDataOffset(0));
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        env->ReleaseByteArrayElements(mData, data, 0);
        env->DeleteGlobalRef(mData);
        mPendingUpdate = false;
    }

private:
    CompressedTexture(const CompressedTexture &compressed_texture) = delete;

    CompressedTexture(CompressedTexture &&compressed_texture) = delete;

    CompressedTexture &operator=(const CompressedTexture &compressed_texture) = delete;

    CompressedTexture &operator=(CompressedTexture &&compressed_texture) = delete;


    void loadCompressedMipMaps(jbyte *data)
    {
        for (int level = 0; level < mLevels; ++level) {
            int levelOffset = getDataOffset(level);
            int levelSize = getDataOffset(level + 1) - levelOffset;
            int width = mWidth >> level;
            int height = mHeight >> level;
            if (width < 1) width = 1;
            if (height < 1) height = 1;
            glCompressedTexImage2D(mType, level, mFormat, width, height, levelOffset, levelSize,
                                   data);
        }
    }
};
}
#endif

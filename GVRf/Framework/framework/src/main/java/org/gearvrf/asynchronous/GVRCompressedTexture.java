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

package org.gearvrf.asynchronous;

import static android.opengl.GLES20.*;

import org.gearvrf.GVRContext;
import org.gearvrf.GVRImage;
import org.gearvrf.utility.Log;

/**
 * A GL compressed texture; you get it from
 * {@linkplain GVRContext#loadCompressedTexture(org.gearvrf.GVRAndroidResource.CompressedTextureCallback, org.gearvrf.GVRAndroidResource)
 * GVRContext.loadCompressedTexture()}.
 * 
 * This is mostly an internal, implementation class: You <em>may</em> find
 * {@link #mLevels} and/or {@link #mQuality} useful.
 * 
 * @since 1.6.1
 */
public class GVRCompressedTexture extends GVRImage
{

    static final int GL_TARGET = GL_TEXTURE_2D;

    private static final String TAG = Log.tag(GVRCompressedTexture.class);

    /*
     * Texture field(s) and constructors
     */


    /**
     * The speed/quality parameter passed to
     * {@link GVRContext#loadCompressedTexture(org.gearvrf.GVRAndroidResource.CompressedTextureCallback, org.gearvrf.GVRAndroidResource, int)
     * GVRContext.loadCompressedTexture()}.
     * 
     * This copy has been 'clamped' to one of the
     * {@linkplain GVRCompressedTexture#SPEED public constants} in
     * {@link GVRCompressedTexture}.
     */
    protected final int mQuality;
    protected final byte[] mData;
    protected final int mTarget;

    // Texture parameters
    GVRCompressedTexture(GVRContext gvrContext, int width,
            int height, int internalFormat, byte[] data,
            int imageSize, int levels, int quality)
    {
        super(gvrContext, width, height, internalFormat, 1);
        mData = data;
        mTarget = GVRCompressedTexture.GL_TARGET;
        setImageSize(imageSize);
        mQuality = GVRCompressedTexture.clamp(quality);
    }

    GVRCompressedTexture(GVRContext gvrContext, int width, int height, byte[] data, int levels, int quality)
    {
        super(gvrContext, width, height, levels);
        mQuality = GVRCompressedTexture.clamp(quality);
        mData = data;
        mTarget = GVRCompressedTexture.GL_TARGET;
    }

    public void setDataOffsets(int[] offsets)
    {
        assert(mLevels == offsets.length);
        for (int i = 0; i < mLevels; ++i)
        {
            mDataOffsets[i] = offsets[i];
        }
    }

    public int getTarget()          { return mTarget; }
    public byte[] getData()         { return mData; }
    public int getQuality()         { return mQuality; }

     /*
     * Quality tradeoff constants
     */

    /** Optimize for render speed */
    public static final int SPEED = -1;
    /** Strike a balance between speed and quality */
    public static final int BALANCED = 0;
    /** Optimize for render quality */
    public static final int QUALITY = 1;

    protected static final int DEFAULT_QUALITY = SPEED;

    private static int clamp(int quality) {
        if (quality < 0) {
            return SPEED;
        } else if (quality > 0) {
            return QUALITY;
        } else {
            return BALANCED;
        }
    }
}

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

package org.gearvrf;

import java.util.List;


/**
 * Encapsulates the data used to construct a GVRTexture.
 * An image can be a simple bitmap, a compressed bitmap,
 * a cubemap (6 separate bitmaps for each cube face) or
 * a compressed cubemap (6 compressed bitmaps).
 *
 * Images are typically read in from files using the @{link GVRAssetLoader}
 * but it is also possible to create them programatically.
 */
public class GVRImage extends GVRHybridObject
{
    private List<GVRAtlasInformation> mAtlasInformation = null;
    protected int mWidth;
    protected int mHeight;
    protected int mFormat;
    protected int mLevels;
    protected int[] mDataOffsets = null;
    protected int mImageSize;

    protected GVRImage(GVRContext gvrContext, long ptr)
    {
        super(gvrContext, ptr);
        mWidth = 0;
        mHeight = 0;
        mFormat = 0;
        mLevels = 0;
        mImageSize = 0;
        mDataOffsets = null;
    }

    protected GVRImage(GVRContext gvrContext)
    {
        super(gvrContext, 0);
        mWidth = 0;
        mHeight = 0;
        mFormat = 0;
        mLevels = 0;
        mImageSize = 0;
        mDataOffsets = null;
    }

    protected GVRImage(GVRContext gvrContext, int w, int h, int format)
    {
        super(gvrContext, 0);
        mWidth = w;
        mHeight = h;
        mLevels = 0;
        mFormat = format;
        mImageSize = 0;
        mDataOffsets = null;
    }

    protected GVRImage(GVRContext gvrContext, int w, int h, int format, int levels)
    {
        super(gvrContext, 0);
        mWidth = w;
        mHeight = h;
        mLevels = levels;
        mFormat = format;
        mImageSize = 0;
        if (levels > 0)
        {
            mDataOffsets = (levels > 0) ? new int[levels] : null;
        }
    }

    public int getDataOffset(int level)
    {
        if (mDataOffsets == null)
        {
            return 0;
        }
        if (level < mLevels)
        {
            return mDataOffsets[level];
        }
        return mImageSize - mDataOffsets[mLevels - 1];
    }

    protected void setDataOffset(int level, int offset)
    {
        mDataOffsets[level] = offset;
    }

    public int[] getDataOffsets()
    {
        return mDataOffsets;
    }

    public int getImageSize()
    {
        return mImageSize;
    }

    protected void setImageSize(int imageSize)
    {
        mImageSize = imageSize;
    }

    /**
     * Returns the list of atlas information necessary to map
     * the texture atlas to each scene object.
     *
     * @return List of atlas information.
     */
    public List<GVRAtlasInformation> getAtlasInformation() {
        return mAtlasInformation;
    }

    /**
     * Set the list of {@link GVRAtlasInformation} to map the texture atlas
     * to each object of the scene.
     *
     * @param atlasInformation Atlas information to map the texture atlas to each
     *        scene object.
     */
    public void setAtlasInformation(List<GVRAtlasInformation> atlasInformation)
    {
        mAtlasInformation = atlasInformation;
    }

    /**
     * Inform if the texture is a large image containing "atlas" of sub-images
     * with a list of {@link GVRAtlasInformation} necessary to map it to the
     * scene objects.
     *
     * @return True if the texture is a large image containing "atlas",
     *         otherwise it returns false.
     */
    public boolean isAtlasedTexture()
    {
        return mAtlasInformation != null
                && !mAtlasInformation.isEmpty();
    }

    /**
     * Number of texture levels. 1 means a single image, with no mipmap chain;
     * values higher than 1 mean the texture has a mipmap chain.
     */
    public int getLevels()  { return mLevels; }

    public int getWidth()   { return mWidth; }
    public int getHeight()  { return mHeight; }
    public int getFormat()  { return mFormat; }

    protected static final String TAG = "GVRImage";
}

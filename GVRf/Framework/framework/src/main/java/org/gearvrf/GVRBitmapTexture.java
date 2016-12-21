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

import static android.opengl.GLES20.*;

import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.opengl.GLUtils;

import org.gearvrf.utility.Log;

import java.io.IOException;
import java.util.concurrent.Callable;
import java.util.concurrent.Future;
import java.util.concurrent.RunnableFuture;

/** Bitmap-based texture. */
public class GVRBitmapTexture extends GVRImage
{
    /**
     * Constructs a texture using a pre-existing {@link Bitmap}.
     * 
     * @param gvrContext
     *            Current {@link GVRContext}
     * @param bitmap
     *            A non-null {@link Bitmap} instance; do *not* call
     *            recycle on the bitmap
     */
    public GVRBitmapTexture(GVRContext gvrContext, Bitmap bitmap)
    {
        super(gvrContext, bitmap.getWidth(), bitmap.getHeight(), bitmap.hasAlpha() ? GL_RGBA : GL_RGB);
        mBitmap = bitmap;
        mGrayscaleData = null;
    }

    /**
     * Constructs a texture by loading a bitmap from a PNG file in (or under)
     * the {@code assets} directory.
     * 
     * @param gvrContext
     *            Current {@link GVRContext}
     * @param pngAssetFilename
     *            The name of a {@code .png} file, relative to the assets
     *            directory. The assets directory may contain an arbitrarily
     *            complex tree of subdirectories; the file name can specify any
     *            location in or under the assets directory.
     */
    public GVRBitmapTexture(GVRContext gvrContext, String pngAssetFilename)
    {
        this(gvrContext, loadBitmap(gvrContext, pngAssetFilename));
    }

     /**
     * Create a new, grayscale texture, from an array of luminance bytes.
     * 
     * @param gvrContext
     *            Current {@link GVRContext}
     * @param width
     *            Texture width, in pixels
     * @param height
     *            Texture height, in pixels
     * @param grayscaleData
     *            {@code width * height} bytes of gray scale data
     *
     * @throws IllegalArgumentException
     *             If {@code width} or {@code height} is {@literal <= 0,} or if
     *             {@code grayScaleData} is {@code null}, or if
     *             {@code grayscaleData.length < height * width}
     */
    public GVRBitmapTexture(GVRContext gvrContext, int width, int height, byte[] grayscaleData)
            throws IllegalArgumentException
    {
        super(gvrContext, width, height, GL_LUMINANCE);
        mBitmap = null;
        mGrayscaleData = grayscaleData;
    }

    Bitmap  getBitmap()         { return mBitmap; }
    byte[]  getGrayscale()      { return mGrayscaleData; }

    public void setBitmap(Bitmap bmap)
    {
        mBitmap = bmap;
        mWidth = bmap.getWidth();
        mHeight = bmap.getHeight();
        mFormat = GL_RGB;
        if (bmap.hasAlpha())
        {
            mFormat = GL_RGBA;
        }
    }

    private static Bitmap loadBitmap(GVRContext gvrContext, String pngAssetFilename)
    {
        try
        {
            return BitmapFactory.decodeStream(
                    gvrContext.getContext().getAssets().open(pngAssetFilename));
        }
        catch (final IOException exc)
        {
            Log.e(TAG, "asset not found", exc);
        }
        return null;
    }

    private Bitmap mBitmap;
    private byte[] mGrayscaleData;
    private final static String TAG = "GVRBitmapTexture";
}


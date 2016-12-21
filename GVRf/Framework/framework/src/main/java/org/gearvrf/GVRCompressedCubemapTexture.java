package org.gearvrf;

public class GVRCompressedCubemapTexture extends GVRImage
{
    protected byte[][] mData;

  public GVRCompressedCubemapTexture(GVRContext gvrContext, int internalFormat, int width,
          int height, int imageSize, byte[][] data, int[] dataOffsets)
  {
      super(gvrContext, width, height, internalFormat, dataOffsets.length);
      setImageSize(imageSize);
      setDataOffsets(dataOffsets);
      mData = data;
  }

    public void setDataOffsets(int[] offsets)
    {
        assert(mLevels == offsets.length);
        for (int i = 0; i < mLevels; ++i)
        {
            mDataOffsets[i] = offsets[i];
        }
    }

    public byte[][] getData() { return mData; }
}

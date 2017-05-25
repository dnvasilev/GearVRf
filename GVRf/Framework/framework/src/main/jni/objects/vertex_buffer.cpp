/****
 *
 * VertexBuffer maintains a vertex data array with locations, normals,
 * colors and texcoords.
 *
 ****/
#include "vertex_buffer.h"
#include "util/gvr_log.h"
#include <sstream>

namespace gvr {

    VertexBuffer::VertexBuffer(const char* layout_desc, int vertexCount)
    : DataDescriptor(layout_desc),
      mVertexCount(0),
      mBoneFlags(0),
      mVertexData(NULL)
    {
        mVertexData = NULL;
        mBoneFlags == 0;
        setVertexCount(vertexCount);
    }

    VertexBuffer::~VertexBuffer()
    {
        if (mVertexData != NULL)
        {
            delete [] mVertexData;
            mVertexData = NULL;
        }
        mVertexCount = 0;
    }

 /**
  * Establishes the layout of the vertex attributes.
  * Each component has a name, a type, and a size indicating
  * the number of 32 ints or floats it occupies.
  * The layout description is a string that contains the type
  * and name of each vertex attribute.
  *
  * For example, a vertex with locations, normals and one set
  * of 2D texture coordinates might be described as:
  *  float3 position float3 normal float2 texcoord
  */
    void VertexBuffer::parseDescriptor()
    {
        int index = 0;
        mTotalSize = 0;
        DataDescriptor::forEach([this, index] (const char* name, const char* type, int size) mutable
        {
            short byteSize = calcSize(type);

            if (byteSize == 0)
                return;

            DataEntry entry;
            entry.Type = makeShaderType(type, byteSize, 1);
            entry.IsSet = false;
            entry.IsDynamic = false;
            entry.IsInt = type[0] == 'i';
            entry.Size = byteSize;
            entry.Offset = mTotalSize;
            entry.Index = index++;
            mTotalSize += byteSize;
            addName(name, entry);
            mLayout.push_back(entry);
            LOGV("VertexBuffer: %s index=%d offset=%d size=%d %d entries\n",
                 entry.Name, entry.Index, entry.Offset, entry.Size, mLayout.size());
        });
    }

    void VertexBuffer::getBoundingVolume(BoundingVolume& bv) const
    {
        const float* verts = getVertexData();
        int stride = getVertexSize();

        bv.reset();
        for (int i = 0; i < mVertexCount; ++i)
        {
            glm::vec3 v;
            const float* src = verts + i * stride;
            v.x = *src++;
            v.y = *src++;
            v.z = *src;
            bv.expand(v);
        }
        LOGV("VertexBuffer::getBoundingVolume (%f, %f, %f) (%f, %f, %f) %d verts",
            bv.min_corner().x, bv.min_corner().y, bv.min_corner().z,
            bv.max_corner().x, bv.max_corner().y, bv.max_corner().z, mVertexCount);
    }

    const void*  VertexBuffer::getData(int index, int& size) const
    {
        if ((index < 0) || (index > mLayout.size()))
        {
            return NULL;
        }
        std::lock_guard<std::mutex> lock(mLock);
        const DataEntry& e = mLayout[index];
        size = e.Size;
        return getVertexData() + (e.Offset / sizeof(float));
    }

    const void*  VertexBuffer::getData(const char* attributeName, int& size) const
    {
        std::lock_guard<std::mutex> lock(mLock);
        const DataEntry* e = find(attributeName);

        if ((e == NULL) || !e->IsSet)
        {
            return NULL;
        }
        size = e->Size;
        return getVertexData() + (e->Offset / sizeof(float));
    }

    /**
     * Update a float vertex attribute from memory data.
     * @param attributeName name of attribute to update
     * @param src           pointer to source array of float data
     * @param srcSize       total number of floats in source array
     * @param srcStride     number of floats in a single entry of source array.
     *                      this is provided to allow copies from source vertex
     *                      formats that are not closely packed.
     *                      If it is zero, it is assumed the source array is
     *                      closely packed and the stride is the size of the attribute.
     * @return true if attribute was updated, false on error
     */
    bool    VertexBuffer::setFloatVec(const char* attributeName, const float* src, int srcSize, int srcStride)
    {
        std::lock_guard<std::mutex> lock(mLock);
        DataEntry*      attr = find(attributeName);
        const float*    srcend;
        float*          dest;
        int             dstStride;
        int             nverts;
        int             attrStride;

        LOGD("VertexBuffer::setFloatVec %s %d", attributeName, srcSize);
        if (attr == NULL)
        {
            LOGE("VertexBuffer: ERROR attribute %s not found in vertex buffer", attributeName);
            return false;
        }
        if (src == NULL)
        {
            LOGE("VertexBuffer: cannot set attribute %s, source array not found", attributeName);
            return false;
        }
        attrStride = attr->Size / sizeof(float);    // # of floats in vertex attribute
        if (srcStride == 0)
        {
            srcStride = attrStride;
        }
        else if (attrStride > srcStride)            // stride too small for this attribute?
        {
            LOGE("VertexBuffer: cannot copy to vertex array %s, stride is %d should be >= %d", attributeName, srcStride, attrStride);
            return false;
        }
        nverts = srcSize / srcStride;               // # of vertices in input array
        if (!setVertexCount(nverts))
        {
            LOGE("VertexBuffer: cannot enlarge vertex array %s, vertex count mismatch", attributeName);
            return false;
        }
        dest = reinterpret_cast<float*>(mVertexData) + attr->Offset / sizeof(float);
        dstStride = getTotalSize() / sizeof(float);
        srcend = src + srcSize;

        for (int i = 0; i < mVertexCount; ++i)
        {
            for (int j = 0; j < attrStride; ++j)
            {
                dest[j] = src[j];
            }
            dest += dstStride;
            if (src >= srcend)
            {
                LOGE("VertexBuffer: error copying to vertex array %s, not enough vertices in source array", attributeName);
                break;
            }
            src += srcStride;
        }
        markDirty();
        attr->IsSet = true;
        return true;
    }


    bool    VertexBuffer::getFloatVec(const char* attributeName, float* dest, int destSize, int destStride) const
    {
        std::lock_guard<std::mutex> lock(mLock);
        const DataEntry* attr = find(attributeName);
        const float*    dstend;
        const float*    src = reinterpret_cast<float*>(mVertexData);
        int             attrSize = attr->Size / sizeof(float);
        int             srcStride = getVertexSize();

        if ((attr == NULL) || !attr->IsSet)
        {
            LOGE("VertexBuffer: ERROR attribute %s not found in vertex buffer", attributeName);
            return false;
        }
        if (src == NULL)
        {
            LOGD("VertexBuffer: cannot set attribute %s", attributeName);
            return false;
        }
        src += attr->Offset / sizeof(float);
        dstend = dest + destSize;
        if (destStride == 0)
        {
            destStride = attrSize;
        }
        for (int i = 0; i < mVertexCount; ++i)
        {
            for (int j = 0; j < attrSize; ++j)
            {
                dest[j] = src[j];
            }
            src += srcStride;
            dest += destStride;
            if (dest > dstend)
            {
                LOGE("VertexBuffer: error reading from vertex array %s, not enough room in destination array", attributeName);
                return false;
            }
        }
        return true;
    }

/**
 * Update an integer vertex attribute from memory data.
 * @param attributeName name of attribute to update
 * @param src           pointer to source array of int data
 * @param srcSize       total number of ints in source array
 * @param srcStride     number of ints in a single entry of source array.
 *                      this is provided to allow copies from source vertex
 *                      formats that are not closely packed.
 *                      If it is zero, it is assumed the source array is
 *                      closely packed and the stride is the size of the attribute.
 * @return true if attribute was updated, false on error
 */
    bool    VertexBuffer::setIntVec(const char* attributeName, const int* src, int srcSize, int srcStride)
    {
        std::lock_guard<std::mutex> lock(mLock);
        DataEntry*      attr = find(attributeName);
        const int*      srcend;
        int*            dest;
        int             dstStride;
        int             nverts;
        int             attrStride;

        LOGD("VertexBuffer::setIntVec %s %d", attributeName, srcSize);
        if (attr == NULL)
        {
            LOGE("VertexBuffer: ERROR attribute %s not found in vertex buffer", attributeName);
            return false;
        }
        if (src == NULL)
        {
            LOGE("VertexBuffer: cannot set attribute %s, source array not found", attributeName);
            return false;
        }
        if (srcStride == 0)
        {
            srcStride = attrStride;         // # of ints in vertex attribute
        }
        else if (attrStride > srcStride)    // stride too small for this attribute?
        {
            LOGE("VertexBuffer: cannot copy to vertex array %s, stride is %d should be >= %d", attributeName, srcStride, attrStride);
            return false;
        }
        nverts = srcSize / srcStride;       // # of vertices in input array
        if  (!setVertexCount(nverts))
        {
            LOGE("VertexBuffer: cannot enlarge vertex array %s, vertex count mismatch", attributeName);
            return false;
        }
        dest = reinterpret_cast<int*>(mVertexData) + attr->Offset / sizeof(int);
        dstStride = getTotalSize() / sizeof(int);
        attrStride = attr->Size / sizeof(int);
        srcend = src + srcSize;

        for (int i = 0; i < mVertexCount; ++i)
        {
            for (int j = 0; j < attrStride; ++j)
            {
                dest[j] = src[j];
            }
            dest += dstStride;
            if (src >= srcend)
            {
                LOGE("VertexBuffer: error copying to vertex array %s, not enough vertices in source array", attributeName);
                break;
            }
            src += srcStride;
        }
        markDirty();
        attr->IsSet = true;
        return true;
    }

    bool    VertexBuffer::getIntVec(const char* attributeName, int* dest, int destSize, int destStride) const
    {
        std::lock_guard<std::mutex> lock(mLock);
        const DataEntry* attr = find(attributeName);
        const int*      dstend;
        const int*      src;
        int             attrSize = attr->Size / sizeof(int);
        int             srcStride = getVertexSize();

        if ((attr == NULL) || !attr->IsSet)
        {
            LOGE("VertexBuffer: ERROR attribute %s not found in vertex buffer", attributeName);
            return false;
        }
        if (src == NULL)
        {
            LOGE("VertexBuffer: cannot set attribute %s", attributeName);
            return false;
        }
        src += attr->Offset / sizeof(float);
        srcStride = getTotalSize() / sizeof(float);
        dstend = dest + destSize;
        if (destStride == 0)
        {
            destStride = attrSize;
        }
        for (int i = 0; i < mVertexCount; ++i)
        {
            for (int j = 0; j < attrSize; ++j)
            {
                dest[j] = src[j];
            }
            src += srcStride;
            if (dest > dstend)
            {
                LOGE("VertexBuffer: error reading from vertex array %s, not enough room in destination array", attributeName);
                return false;
            }
            dest += destStride;
        }
        return true;
    }

    bool VertexBuffer::getInfo(const char* attributeName, int& index, int& offset, int& size) const
    {
        std::lock_guard<std::mutex> lock(mLock);
        const DataEntry* attr = find(attributeName);

        if ((attr == NULL) || !attr->IsSet)
            return false;
        offset = attr->Offset;
        index = attr->Index;
        size = attr->Size;
        return true;
    }

    bool VertexBuffer::setVertexCount(int count)
    {
        if ((mVertexCount != 0) && (mVertexCount != count))
        {
            LOGE("VertexBuffer: cannot change size of vertex buffer from %d vertices to %d", mVertexCount, count);
            return false;
        }
        if (mVertexCount == count)
        {
            return true;
        }
        mVertexCount = count;
        if (count > 0)
        {
            int vsize = getTotalSize();
            int datasize = vsize * count;
            LOGV("VertexBuffer: allocating vertex buffer of %d bytes with %d vertices\n", datasize, count);
            mVertexData = new char[datasize];
        }
        else
        {
            LOGE("VertexBuffer: ERROR: no vertex buffer allocated\n");
        }
        return true;
    }

    void VertexBuffer::setBoneData(VertexBoneData& bones)
    {
        std::lock_guard<std::mutex> lock(mLock);
        int sizeBoneIndex, sizeBoneWeight;
        float* boneIndex = (float*) getData("a_bone_indices", sizeBoneIndex);
        float* boneWeight = (float*) getData("a_bone_weights", sizeBoneWeight);

        if ((boneIndex == NULL) || (boneWeight == NULL))
        {
            LOGE("VertexBuffer: ERROR cannot set bone data unless vertex array has a_bone_indices and a_bone_weights attributes");
            return;
        }
        int boneSlots = BONES_PER_VERTEX;
        for (int i = 0; i < getVertexCount(); ++i)
        {
            VertexBoneData::BoneData& bd = bones.boneData[i];
            for (int j = 0; j < boneSlots; ++j)
            {
                *boneIndex++ = bd.ids[j];
                *boneWeight++ = bd.weights[j];
            }
        }
    }

    bool VertexBuffer::forAllVertices(std::function<void(int iter, const float* vertex)> func) const
    {
        std::lock_guard<std::mutex> lock(mLock);
        const float*    data = reinterpret_cast<float*>(mVertexData);
        int             stride = getVertexSize();

        if (data == NULL)
        {
            return false;
        }
        for (int i = 0; i < mVertexCount; ++i)
        {
            func(i, data);
            data += stride;
        }
        return true;
    }

    bool VertexBuffer::forAllVertices(const char* attrName, std::function<void (int iter, const float* vertex)> func) const
    {
        std::lock_guard<std::mutex> lock(mLock);
        const DataEntry* attr = find(attrName);
        const float*    data = reinterpret_cast<float*>(mVertexData);
        int             stride = getVertexSize();

        if ((attr == NULL) || !attr->IsSet)
        {
            LOGE("VertexBuffer: ERROR attribute %s not found in vertex buffer", attrName);
            return false;
        }
        if (data == NULL)
        {
            LOGD("VertexBuffer: cannot find attribute %s", attrName);
            return false;
        }
        for (int i = 0; i < mVertexCount; ++i)
        {
            func(i, data + attr->Offset);
            data += stride;
        }
        return true;
    }

    void VertexBuffer::dump() const
    {
        int vsize = getVertexSize();
        forAllVertices([vsize](int iter, const float* vertex)
        {
            const float* v = vertex;
            std::ostringstream os;
            os.precision(3);
            for (int i = 0; i < vsize; ++i)
            {
                float f = *v++;
                os << std::fixed << f << " ";
            }
            LOGV("%s", os.str().c_str());
        });
    }

} // end gvrf


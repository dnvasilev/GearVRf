
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


#ifndef DATA_DESCRIPTOR_H_
#define DATA_DESCRIPTOR_H_

#include<unordered_map>
#include <map>
#include <vector>
#include <functional>

namespace gvr {

/**
 * Data descriptor which defines the layout for uniform blocks
 * and vertex arrays.
 *
 * @see UniformBlock
 */
class DataDescriptor
{
public:
    /*
     * Information kept for each uniform in the block.
     */
    struct DataEntry
    {
        short Index;        // 0-based index in descriptor order
        short Offset;       // offset in bytes from the top of the uniform block
        short Size;         // byte size of uniform entry
        char  IsSet;        // true if the entry has been set, else false
        std::string Type;   // shader type "int", "float", "mat", "vec", "ivec"
        std::string Name;   // name of the entry
    };

public:
    DataDescriptor(const std::string& descriptor);
    virtual ~DataDescriptor() { }

   /**
    * Determine if a named uniform exists in this block.
    * This function will return false for names which are
    * in the descriptor but have not been given a value yet.
    *
    * @param name name of uniform to look for
    * @returns true if uniform is in this block, false if not
    */
    bool isSet(const std::string& name) const
    {
        for (auto it = mLayout.begin(); it != mLayout.end(); ++it)
        {
            if (((*it).Name == name) && (*it).IsSet)
            {
                return true;
            }
        }
        return false;
    }

    /*
     * Get the number of bytes occupied by the vertex or data area.
     * @return number of bytes
     */
    int getTotalSize() const
    {
        return mTotalSize;
    }

    /**
     *   Get the number of entries in the layout descriptor
     */
     int getNumEntries() const { return mLayout.size(); }

    /**
     * Get the layout descriptor.
     * The layout descriptor defines the name, type and size
     * of each uniform or vertex. This descriptor
     * should match the layout used by the shader it
     * is intended to work with.
     * {@code
     *  "float3 color, float opacity"
     *  "float factor float power int2 offset"
     * }
     * @return layout descriptor string
     * @see setDescriptor
     */
    const std::string& getDescriptor() const
    {
        return mDescriptor;
    }

    /**
     * Visits each entry in the descriptor and calls the given function
     * once for each named item.
     */
    void forEach(std::function< void(const std::string& name, const std::string& type, int size) > func);

    /**
     * Visits each entry in the descriptor and calls the given function
     * with the entry.
     */
    void forEachEntry(std::function< void(const DataEntry&) > func);

    /**
     * Look up the named uniform in the mLayout.
     * This function fails if the uniform found does not
     * have the same byte size as the input bytesize.
     * @param name name of uniform to find.
     * @param dataptr pointer to where to store data pointer
     * @return pointer to Uniform structure describing the uniform or NULL on failure
     */
    const DataEntry* find(const std::string& name) const;
    DataEntry* find(const std::string& name);

    /*
     * Get the number of bytes occupied by the named entry.
     * For vertex arrays, it is the number of bytes occupied
     * by that attribute in a single vertex.
     * @param name string name of uniform whose size you want
     */
    int getByteSize(const std::string& name) const;

    /*
     * Determine if data has changed since last render.
     * @returns true if data has been updated, else false.
     */
    bool isDirty() const { return mIsDirty; }
    virtual void markDirty() { mIsDirty = true; }
    virtual std::string makeShaderType(const std::string& type, int byteSize, int arraySize);

protected:

    /**
     * Parse the descriptor string to create the map
     * which contains the name, offset and size of all uniforms.
     */
    virtual void parseDescriptor();

    /**
     * Calculate the byte size of the given type.
     */
    short calcSize(const std::string& type) const;


    mutable bool mIsDirty;       // true if data in block has changed since last render
    std::string mDescriptor;     // descriptor with name, type and size of uniforms
    int         mTotalSize;      // number of bytes in data block or vertex
    std::vector<DataEntry> mLayout;
};

}
#endif

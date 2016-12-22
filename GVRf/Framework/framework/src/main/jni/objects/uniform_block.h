
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


#ifndef UNIFORMBLOCK_H_
#define UNIFORMBLOCK_H_

#include<unordered_map>
#include "glm/glm.hpp"
#include "util/gvr_log.h"
#include <map>
#include <vector>

#define TRANSFORM_UBO_INDEX 0
#define MATERIAL_UBO_INDEX  1
#define SAMPLER_UBO_INDEX   2
#define BONES_UBO_INDEX     3

namespace gvr {
class SceneObject;

/**
 * Manages a Uniform Block containing data parameters to pass to
 * the vertex and fragment shaders.
 *
 * The UniformBlock may be updated by the application. If it has changed,
 * GearVRf resends the entire data block to the GPU. Each block has one or more
 * named entries that refer to floating point or integer vectors.
 * Each entry can be independently accessed by name. All of the entries are
 * packed into a single data block.
 *
 * A uniform block is a renderer-dependent class which is implemented
 * differently depending on which underlying renderer GearVRf is using.
 *
 * @see GLUniformBlock
 * @see VulkanUniformBlock
 */
class UniformBlock
{
protected:
    /*
     * Information kept for each uniform in the block.
     */
    struct Uniform
    {
        short Offset;       // offset in bytes from the top of the uniform block
        short Size;         // byte size of uniform entry
        char IsSet;         // true if the entry has been set, else false
        std::string Type;   // type of the entry "int", "float" or "mat"
        std::string Name;   // name of the entry
    };

public:
    UniformBlock();
    UniformBlock(const std::string& descriptor, int binding_point);
    /**
     * Gets the OpenGL binding point for this uniform block.
     * @return GL binding point or -1 if not set
     */
    int getBindingPoint() const
    {
        return bindingPoint_;
    }
    /**
     * Determine if a named uniform exists in this block.
     * This function will return false for names which are
     * in the descriptor but have not been given a value yet.
     *
     * @param name name of uniform to look for
     * @returns true if uniform is in this block, false if not
     */
    bool hasUniform(std::string name) const
    {
        auto it = UniformMap.find(name);
        if (it == UniformMap.end())
        {
            return false;
        }
        return (it->second).IsSet;
    }

    /*
     * Get the number of bytes occupied by this uniform block.
     * @return byte size of uniform block
     */
    int getTotalSize() const
    {
        return TotalSize;
    }

    /**
     * Get the uniform descriptor.
     * The uniform descriptor defines the name, type and size
     * of each uniform in the block. This descriptor
     * should match the layout used by the shader this
     * block is intended to work with.
     * {@code
     *  "float3 color, float opacity"
     *  "float factor float power int2 offset"
     * }
     * @return uniform descriptor string
     * @see setDescriptor
     */
    const std::string& getDescriptor() const
    {
        return Descriptor;
    }

    /**
     * Set the uniform descriptor.
     * The uniform descriptor defines the name, type and size
     * of each uniform in the block. Each entry has a type,
     * size and name. Entries are separated by spaces but
     * other delimiters (commas, semicolons) are permitted.
     * {@code Sample strings:
     *  "float4 diffuseColor, float specularExponent"
     *  "int2 offset mat4 texMatrix"
     *  }
     * This descriptor should match the layout used by the shader this
     * block is intended to work with. The unforms must have the
     * same names, types and be in the same order as in the shader.
     *
     * @param descriptor string with uniform descriptor.
     * @see getDescriptor
     */
    virtual void setDescriptor(const std::string& descriptor)
    {
        if (!Descriptor.empty())
        {
            LOGE("UniformBlock: ERROR: descriptor cannot be changed once it is set\n");
            return;
        }
        Descriptor = descriptor;
        parseDescriptor();
    }

    /**
     * Set the value of an integer uniform.
     * If the named uniform is not an "int" in the descriptor
     * this function will fail and log an error.
     * @param name name of uniform to set.
     * @param val integer value to set.
     * @returns true if successfully set, false on error.
     * @see getInt
     */
    virtual bool setInt(const std::string&, int val);

    /**
     * Set the value of a floating point uniform.
     * If the named uniform is not a "float" in the descriptor
     * this function will fail and log an error.
     * @param name name of uniform to set.
     * @param val float value to set.
     * @returns true if successfully set, false on error.
     * @see getFloat
     */
    virtual bool setFloat(const std::string&, float val);

    /**
     * Set the value of an integer vector uniform.
     * If the named uniform is not an int vector in the descriptor
     * of the proper size this function will fail and log an error.
     * @param name name of uniform to set.
     * @param val pointer to integer vector.
     * @param n number of integers in the vector.
     * @returns true if successfully set, false on error.
     * @see getIntVec
     */
    virtual bool setIntVec(const std::string& name, const int* val, int n);

    /**
     * Set the value of a floating point vector uniform.
     * If the named uniform is not a float vector in the descriptor
     * of the proper size this function will fail and log an error.
     * @param name name of uniform to set.
     * @param val pointer to float vector.
     * @param n number of floats in the vector.
     * @returns true if successfully set, false on error.
     * @see getVec
     */
    virtual bool setFloatVec(const std::string& name, const float* val, int n);

    /**
     * Set the value of a 2D vector uniform.
     * If the named uniform is not a "float2" in the descriptor
     * this function will fail and log an error.
     * @param name name of uniform to set.
     * @param val 2D vector value to set.
     * @returns true if successfully set, false on error.
     * @see setVec
     * @see getVec
     * @see getVec2
     */
    virtual bool setVec2(const std::string& name, const glm::vec2& val);

    /**
     * Set the value of a 3D vector uniform.
     * If the named uniform is not a "float3" in the descriptor
     * this function will fail and log an error.
     * @param name name of uniform to set.
     * @param val 3D vector value to set.
     * @returns true if successfully set, false on error.
     * @see setVec
     * @see getVec
     * @see getVec3
     */
    virtual bool setVec3(const std::string& name, const glm::vec3& val);

    /**
     * Set the value of a 4D vector uniform.
     * If the named uniform is not a "float4" in the descriptor
     * this function will fail and log an error.
     * @param name name of uniform to set.
     * @param val 4D vector value to set.
     * @returns true if successfully set, false on error.
     * @see getVec
     * @see setVec
     * @see getVec4
     */
    virtual bool setVec4(const std::string& name, const glm::vec4& val);

    /**
     * Set the value of a 4x4 matrix uniform.
     * If the named uniform is not a "mat4" in the descriptor
     * this function will fail and log an error.
     * @param name name of uniform to set.
     * @param val 4x4 matrix value to set.
     * @see setMat4
     * @see setVec
     * @see getVec
     */
    virtual bool setMat4(const std::string& name,  const glm::mat4& val);

    /**
     * Get the value of a 2D vector uniform.
     * If the named uniform is not a 2D vector this function
     * will return null.
     * @param name name of uniform to get.
     * @returns pointer to 2D vector or NULL if uniform not found.
     * @see setVec2
     * @see setVec
     * @see getVec
     */
    virtual const glm::vec2* getVec2(const std::string& name) const;

    /**
     * Get the value of a 3D vector uniform.
     * If the named uniform is not a 3D vector this function
     * will return null.
     * @param name name of uniform to get.
     * @returns pointer to 3D vector or NULL if uniform not found.
     * @see getVec
     * @see setVec3
     * @see setVec
     */
    virtual const glm::vec3* getVec3(const std::string& name) const;

    /**
     * Get the value of a 4D vector uniform.
     * If the named uniform is not a 4D vector this function
     * will return null.
     * @param name name of uniform to get.
     * @returns pointer to 4D vector or NULL if uniform not found.
     * @see getVec
     * @see setVec4
     * @see setVec
     */
    virtual const glm::vec4* getVec4(const std::string& name) const;

    /**
     * Get the value of a 4x4 matrix uniform.
     * If the named uniform is not a 4x4 matrix this function
     * will return false.
     * @param name name of uniform to get.
     * @returns true if matrix found, false if not.
     * @see getVec
     * @see setMat4
     * @see setVec
     */
    virtual bool getMat4(const std::string&, glm::mat4& val) const;

    /**
     * Get the value of a floating po2int uniform.
     * If the named uniform is not a "float" in the descriptor
     * this function returns 0 and logs an error.
     * @param name name of uniform to get.
     * @param v where to store float value.
     * @returns true if value found, else false.
     * @see setVec
     * @see getVec
     * @see setFloat
     */
    virtual bool getFloat(const std::string& name, float& v) const;

    /**
     * Get the value of an integer uniform.
     * If the named uniform is not "inat" in the descriptor
     * this function returns 0 and logs an error.
     * @param name name of uniform to get.
     * @param v where to store integer value.
     * @returns true if value found, else false.
     * @see setVec
     * @see getVec
     * @see setInt
     */
    virtual bool getInt(const std::string& name, int& v) const;

    /**
     * Get the value of a float vector uniform.
     * If the named uniform is not a float vector
     * of the proper size this function will return null.
     * @param name name of uniform to get.
     * @param val pointer to float array to get value.
     * @param n number of floats in the array.
     * @return true if vector retrieved, false if not found or size is wrong.
     * @see setVec
     */
    virtual bool getFloatVec(const std::string& name, float* val, int n) const;

    /**
     * Get the value of an integer vector uniform.
     * If the named uniform is not an int vector
     * of the proper size this function will return null.
     * @param name name of uniform to get.
     * @param val pointer to float array to get value.
     * @param n number of ints in the array.
     * @return true if vector retrieved, false if not found or size is wrong.
     * @see setVec
     */
    virtual bool getIntVec(const std::string& name, int* val, int n) const;

    /*
     * Get the number of bytes occupied by the named uniform.
     * @param name string name of uniform whose size you want
     */
    int getByteSize(const std::string& name) const;

    virtual ~UniformBlock()
    {
        if ((UniformData != NULL) && ownData)
        {
            free(UniformData);
        }
        UniformData = NULL;
    }

    /**
     * Returns a string with the names and offsets
     * of all the uniforms in the block.
     * @return string describing the uniform block.
     */
    std::string toString();

    /**
     * Get a pointer to the entire uniform data area.
     * @returns -> uniform block data if it exists, else NULL
     */
    const void* getData() { return UniformData; }

protected:

    /**
     * Parse the descriptor string to create the UniformMap
     * which contains the name, offset and size of all uniforms.
     */
    void parseDescriptor();

    /**
     * Calculate the byte size of the given type.
     */
    short calcSize(const std::string& type) const;

    /**
     * Constructs the data block containing the values
     * for all the uniform variables in the descriptor.
     */
    void  makeData()
    {
        if (UniformData == NULL)
        {
            UniformData = malloc(TotalSize);
            ownData = true;
        }
    }

    /**
     * Look up the named uniform in the UniformMap.
     * This function fails if the uniform found does not
     * have the same byte size as the input bytesize.
     * @param name name of uniform to find.
     * @param bytesize byte size of uniform.
     * @return pointer to Uniform structure describing the uniform or NULL on failure
     */
    Uniform* getUniform(const std::string& name, int& bytesize);
    const Uniform* getUniform(const std::string& name, int& bytesize) const;


    /**
     * Get a pointer to the value for the named uniform.
     * @param name name of uniform to get.
     * @param bytesize number of bytes uniform occupies
     * @return pointer to start of uniform value or NULL if not found.
     */
    char* getData(const std::string& name, int& bytesize);
    const char* getData(const std::string& name, int& bytesize) const;
    int          bindingPoint_;
    bool        ownData;        // true if this uniform owns its data block
    std::string Descriptor;     // descriptor with name, type and size of uniforms
    void*       UniformData;    // -> data block with uniform values
    int        TotalSize;      // number of bytes in data block
    std::map<std::string, Uniform> UniformMap;
};

}
#endif

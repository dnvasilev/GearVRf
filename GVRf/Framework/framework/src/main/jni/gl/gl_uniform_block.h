
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


#ifndef GL_UNIFORMBLOCK_H_
#define GL_UNIFORMBLOCK_H_

#include "objects/uniform_block.h"

namespace gvr {

/**
 * Manages a Uniform Block containing data parameters to pass to
 * OpenGL vertex and fragment shaders.
 *
 * The UniformBlock may be updated by the application. If it has changed,
 * GearVRf resends the entire data block to OpenGL.
 */
class GLUniformBlock : public UniformBlock
{
public:
    GLUniformBlock(const std::string& descriptor);

    virtual ~GLUniformBlock()
    {
    }

    /**
     * Copy the data from the uniform block into OpenGL.
     */
    virtual void render(GLuint programId);

    /**
     * Create an OpenGL buffer in the GPU for this uniform block.
     */
    virtual void bindBuffer(GLuint programId);

    /**
     * Set the name of this block (the name used by the shader to refer to the block).
     * @param name string with name of uniform block
     * @see #getBlockName
     */
    void setBlockName(const std::string& name) { BlockName = name; }

    /**
     * Get the name of the uniform block.
     * This name should be set by the caller to be the same
     * as the name used for the block in the shader.
     * @returns uniform block name or NULL if not set.
     * @see #setBlockName
     */
    const std::string& getBlockName() const { return BlockName; }

    /**
     * Gets the OpenGL binding point for this uniform block.
     * @return GL binding point or -1 if not set
     */
    int getGLBindingPoint() const
    {
        return GLBindingPoint;
    }

    /**
     * Sets the GL binding point for this uniform block.
     * This is a zero-based index indicating which buffer
     * slot in the shader this uniform block is bound to.
     * @param bufferNum 0 based buffer index
     */
    void setGLBindingPoint(int bufferNum)
    {
        GLBindingPoint = bufferNum;
    }

    /**
     * Dump the contents of the shader uniforms to the log.
     * @param programID OpenGL program ID for shader
     * @param blockIndex OpenGL binding point
     */
    static void dump(GLuint programID, int blockIndex);

protected:
    std::string BlockName;
    GLint       GLBlockIndex;
    GLint       GLBindingPoint;
    GLuint      GLBuffer;
    GLuint      GLOffset;
};

}
#endif

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
#include "gl/gl_material.h"
#include "gl/gl_shader.h"

namespace gvr {
    GLUniformBlock::GLUniformBlock(const std::string& descriptor, int bindingPoint, const std::string& blockName) :
            UniformBlock(descriptor, bindingPoint, blockName),
            GLBlockIndex(-1),
            GLOffset(0),
            GLBuffer(0)
    {
        //parseDescriptor();
    }

    GLUniformBlock::~GLUniformBlock()
    {
        glDeleteBuffers(1,&GLBuffer);
    }

    bool GLUniformBlock::updateGPU(Renderer* unused)
    {
        if (mBindingPoint < 0)
        {
            return false;
        }
        if (GLBuffer == 0)
        {
            glGenBuffers(1, &GLBuffer);
            glBindBuffer(GL_UNIFORM_BUFFER, GLBuffer);
            glBufferData(GL_UNIFORM_BUFFER, getTotalSize(), NULL, GL_DYNAMIC_DRAW);
            mIsDirty = true;
        }
        if (mIsDirty)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, mBindingPoint, GLBuffer);
            glBufferSubData(GL_UNIFORM_BUFFER, GLOffset, getTotalSize(), getData());
            mIsDirty = false;
            if (Shader::LOG_SHADER) LOGV("UniformBlock::updateGPU %s size %d\n", getBlockName().c_str(), getTotalSize());
        }
        checkGLError("GLUniformBlock::updateGPU");
        return true;
    }

    bool GLUniformBlock::bindBuffer(Shader* shader, Renderer* unused)
    {
        GLShader* glshader = static_cast<GLShader*>(shader);
        if (GLBuffer > 0)
        {
            glBindBuffer(GL_UNIFORM_BUFFER, GLBuffer);

            // Do not uncomment below line: UBO is not per shader, so block indices will be different for different shader
       //     if (GLBlockIndex < 0)
            {
                GLBlockIndex = glGetUniformBlockIndex(glshader->getProgramId(), getBlockName().c_str());
                if (GLBlockIndex < 0)
                {
                    LOGE("UniformBlock: ERROR: cannot find block named %s\n", getBlockName().c_str());
                    return false;
                }
            }
            glUniformBlockBinding(glshader->getProgramId(), GLBlockIndex, mBindingPoint);
            glBindBufferBase(GL_UNIFORM_BUFFER, mBindingPoint, GLBuffer);
            if (Shader::LOG_SHADER) LOGV("UniformBlock::bindBuffer %s bind at %d index = %d\n", getBlockName().c_str(), mBindingPoint, GLBlockIndex);
            checkGLError("GLUniformBlock::bindBuffer");
            return true;
        }
        return false;
    }

    std::string GLUniformBlock::makeShaderLayout()
    {
        std::string layout("layout (std140) ");
        return layout + UniformBlock::makeShaderLayout();
    }

    void GLUniformBlock::dump(GLuint programID, int blockIndex)
    {
        // get size of name of the uniform block
        GLint nameLength;
        glGetActiveUniformBlockiv(programID, blockIndex, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);

        // get name of uniform block
        GLchar blockName[nameLength];
        glGetActiveUniformBlockName(programID, blockIndex, nameLength, NULL, blockName);

        // get size of uniform block in bytes
        GLint byteSize;
        glGetActiveUniformBlockiv(programID, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &byteSize);

        // get number of uniform variables in uniform block
        GLint numberOfUniformsInBlock;
        glGetActiveUniformBlockiv(programID, blockIndex,
                                  GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numberOfUniformsInBlock);

        // get indices of uniform variables in uniform block
        GLint uniformsIndices[numberOfUniformsInBlock];
        glGetActiveUniformBlockiv(programID, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniformsIndices);
        LOGV("UniformBlock: %s %d bytes\n", blockName, byteSize);

        // get parameters of all uniform variables in uniform block
        for (int uniformMember=0; uniformMember<numberOfUniformsInBlock; uniformMember++)
        {
            if (uniformsIndices[uniformMember] > 0)
            {
                // index of uniform variable
                GLuint tUniformIndex = uniformsIndices[uniformMember];

                uniformsIndices[uniformMember];
                GLint uniformNameLength, uniformOffset, uniformSize;
                GLint uniformType, arrayStride, matrixStride;

                // get length of name of uniform variable
                glGetActiveUniformsiv(programID, 1, &tUniformIndex,
                                      GL_UNIFORM_NAME_LENGTH, &uniformNameLength);
                // get name of uniform variable
                GLchar uniformName[uniformNameLength];
                glGetActiveUniform(programID, tUniformIndex, uniformNameLength,
                                   NULL, NULL, NULL, uniformName);

                // get offset of uniform variable related to start of uniform block
                glGetActiveUniformsiv(programID, 1, &tUniformIndex,
                                      GL_UNIFORM_OFFSET, &uniformOffset);
                // get size of uniform variable (number of elements)
                glGetActiveUniformsiv(programID, 1, &tUniformIndex,
                                      GL_UNIFORM_SIZE, &uniformSize);
                // get type of uniform variable (size depends on this value)
                glGetActiveUniformsiv(programID, 1, &tUniformIndex,
                                      GL_UNIFORM_TYPE, &uniformType);
                // offset between two elements of the array
                glGetActiveUniformsiv(programID, 1, &tUniformIndex,
                                      GL_UNIFORM_ARRAY_STRIDE, &arrayStride);
                // offset between two vectors in matrix
                glGetActiveUniformsiv(programID, 1, &tUniformIndex,
                                      GL_UNIFORM_MATRIX_STRIDE, &matrixStride);
           }
        }
    }

}
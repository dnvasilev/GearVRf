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
#include "gl/gl_uniform_block.h"
#include "glm/gtc/type_ptr.hpp"
#include "util/gvr_gl.h"


namespace gvr {
    GLUniformBlock::GLUniformBlock(const std::string& descriptor) :
            UniformBlock(descriptor),
            GLBlockIndex(-1),
            GLBindingPoint(-1),
            GLOffset(0),
            GLBuffer(0)
    {
    }

    GLUniformBlock::GLUniformBlock() :
            UniformBlock(),
            GLBlockIndex(-1),
            GLOffset(0),
            GLBuffer(0),
            GLBindingPoint(-1)
    {

    }

    void GLUniformBlock::bindBuffer(GLuint programId)
    {

        if (GLBindingPoint < 0)
            return;
        if (GLBlockIndex < 0)
        {
            GLBlockIndex = glGetUniformBlockIndex(programId, BlockName.c_str());
            if (GLBlockIndex < 0)
            {
                LOGE("UniformBlock: ERROR: cannot find block named %s\n", BlockName.c_str());
                return;
            }

            glGenBuffers(1, &GLBuffer);
            glBindBuffer(GL_UNIFORM_BUFFER, GLBuffer);
            glBufferData(GL_UNIFORM_BUFFER, TotalSize, NULL, GL_DYNAMIC_DRAW);
            glUniformBlockBinding(programId, GLBlockIndex, GLBindingPoint);
            glBindBufferBase(GL_UNIFORM_BUFFER, GLBindingPoint, GLBuffer);
            checkGlError("bindUBO ");
            LOGV("SHADER: UniformBlock: %s bound to #%d at index %d buffer = %d\n", BlockName.c_str(), GLBindingPoint, GLBlockIndex, GLBuffer);
        }
        else {
            glBindBuffer(GL_UNIFORM_BUFFER, GLBuffer);
            glBindBufferBase(GL_UNIFORM_BUFFER, GLBindingPoint, GLBuffer);
        }
    }

    void GLUniformBlock::render(GLuint programId)
    {
        auto it = Dirty.find(programId);

        if (it != Dirty.end() && !it->second)
            return;

        //  LOGE("it should not come hrere");
        Dirty[programId] = false;
        if (GLBuffer == 0 )
            bindBuffer(programId);
        if (GLBuffer >= 0)
        {
            glBindBuffer(GL_UNIFORM_BUFFER, GLBuffer);
            glBindBufferBase(GL_UNIFORM_BUFFER, GLBindingPoint, GLBuffer);
            glBufferSubData(GL_UNIFORM_BUFFER, GLOffset, TotalSize, UniformData);
            LOGV("SHADER: UniformBlock: offset %d : total Size %d\n", GLOffset, TotalSize);
        }
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

                // Size of uniform variable in bytes
                byteSize = uniformSize * sizeFromUniformType(uniformType);
                LOGV("UniformBlock: %s GL offset = %d, byteSize = %d\n", uniformName, uniformOffset, byteSize);
            }
        }
    }

    GLint GLUniformBlock::sizeFromUniformType(GLint type)
    {
        GLint s;

#define UNI_CASE(type, numElementsInType, elementType) \
       case type : s = numElementsInType * sizeof(elementType); break;

        switch (type)
        {
            UNI_CASE(GL_FLOAT, 1, GLfloat);
            UNI_CASE(GL_FLOAT_VEC2, 2, GLfloat);
            UNI_CASE(GL_FLOAT_VEC3, 3, GLfloat);
            UNI_CASE(GL_FLOAT_VEC4, 4, GLfloat);
            UNI_CASE(GL_INT, 1, GLint);
            UNI_CASE(GL_INT_VEC2, 2, GLint);
            UNI_CASE(GL_INT_VEC3, 3, GLint);
            UNI_CASE(GL_INT_VEC4, 4, GLint);
            UNI_CASE(GL_UNSIGNED_INT, 1, GLuint);
            UNI_CASE(GL_UNSIGNED_INT_VEC2, 2, GLuint);
            UNI_CASE(GL_UNSIGNED_INT_VEC3, 3, GLuint);
            UNI_CASE(GL_UNSIGNED_INT_VEC4, 4, GLuint);
            UNI_CASE(GL_BOOL, 1, GLboolean);
            UNI_CASE(GL_BOOL_VEC2, 2, GLboolean);
            UNI_CASE(GL_BOOL_VEC3, 3, GLboolean);
            UNI_CASE(GL_BOOL_VEC4, 4, GLboolean);
            UNI_CASE(GL_FLOAT_MAT2, 4, GLfloat);
            UNI_CASE(GL_FLOAT_MAT3, 9, GLfloat);
            UNI_CASE(GL_FLOAT_MAT4, 16, GLfloat);
            UNI_CASE(GL_FLOAT_MAT2x3, 6, GLfloat);
            UNI_CASE(GL_FLOAT_MAT2x4, 8, GLfloat);
            UNI_CASE(GL_FLOAT_MAT3x2, 6, GLfloat);
            UNI_CASE(GL_FLOAT_MAT3x4, 12, GLfloat);
            UNI_CASE(GL_FLOAT_MAT4x2, 8, GLfloat);
            UNI_CASE(GL_FLOAT_MAT4x3, 12, GLfloat);
            default : s = 0; break;
        }
        return s;
    }

}
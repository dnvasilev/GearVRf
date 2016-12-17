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


#ifndef FRAMEWORK_GL_UNIFORM_BLOCK_H
#define FRAMEWORK_GL_UNIFORM_BLOCK_H

#include "objects/uniform_block.h"
#include "glm/glm.hpp"
#include "util/gvr_log.h"


namespace gvr {
/**
 * Manages a GLSL Uniform Block containing data parameters to pass to
 * the vertex and fragment shaders.
 */
    class GLUniformBlock : public UniformBlock
    {
    public:
        GLUniformBlock();
        GLUniformBlock(const std::string& descriptor);

        virtual void render(GLuint programId);
        virtual void bindBuffer(GLuint programId);


        virtual ~GLUniformBlock()
        {
            if (GLBuffer > 0)
                glDeleteBuffers(1, &GLBuffer);
        }


        void setBuffer(GLuint buffer, GLuint bindingPoint)
        {
            if (GLBuffer != 0)
            {
                LOGE("UniformBlock: ERROR: GL buffer cannot be changed\n");
                return;
            }
            GLBuffer = buffer;
            GLBindingPoint = bindingPoint;
        }

        int getGLBindingPoint() const
        {
            return GLBindingPoint;
        }

        void setGLBindingPoint(int bufferNum)
        {
            GLBindingPoint = bufferNum;
        }


        /*
         * Mark the block as needing update for all shaders using it
         */
        virtual void setDirty()
        {
            for (auto it = Dirty.begin(); it != Dirty.end(); ++it)
            {
                it->second = true;
            }
        }

        static void dump(GLuint programID, int blockIndex);

    protected:
        static GLint sizeFromUniformType(GLint type);

        GLint       GLBlockIndex;
        GLint       GLBindingPoint;
        GLuint      GLBuffer;
        GLuint      GLOffset;
        std::map<int, bool> Dirty;
    };
}

#endif //FRAMEWORK_GL_UNIFORM_BLOCK_H

#pragma once

#include <string>
#include <vector>
#include <map>

#ifndef GL_ES_VERSION_3_0
#include "GLES3/gl3.h"
#endif
#include "glm/glm.hpp"
#include "gl/gl_program.h"
#include "util/gvr_gl.h"

#include "objects/vertex_buffer.h"


namespace gvr {
    class GlDelete;

 /**
  * Interleaved vertex storage for OpenGL
  *
  * @see VertexBuffer
  */
    class GLVertexBuffer : public VertexBuffer
    {
    public:
        GLVertexBuffer(const std::string& layout_desc, int vertexCount);
        virtual ~GLVertexBuffer();

        virtual bool    bindBuffer(Shader*, Renderer*);
        virtual bool    updateGPU(Renderer*);

    protected:
        void            bindToShader(DataDescriptor& desc, GLuint programId);

        GLuint          mVBufferID;
        GLuint          mVArrayID;
        GLuint          mProgramID;
    };

} // end gvrf


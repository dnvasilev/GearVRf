/****
 *
 * VertexBuffer maintains a vertex data array with locations, normals,
 * colors and texcoords.
 *
 ****/
#include "gl_vertex_buffer.h"
#include "gl_shader.h"

namespace gvr {
    GLVertexBuffer::GLVertexBuffer(const std::string& layout_desc, int vertexCount)
    : VertexBuffer(layout_desc, vertexCount),
      mVBufferID(-1), mVArrayID(-1), mProgramID(-1)
    {
    }

    GLVertexBuffer::~GLVertexBuffer()
    {
        if (mVArrayID != -1)
        {
            glDeleteBuffers(1, &mVArrayID);
            mVArrayID = -1;
        }
        if (mVBufferID != -1)
        {
            glDeleteBuffers(1, &mVBufferID);
            mVBufferID = -1;
        }
    }

    bool GLVertexBuffer::bindBuffer(Shader* shader, Renderer* renderer)
    {
        if (mVBufferID != -1)
        {
            GLShader* glshader = reinterpret_cast<GLShader*>(shader);
            LOGV("VertexBuffer::bindBuffer %d %d", mVArrayID, mVBufferID);
            glBindVertexArray(mVArrayID);
            glBindBuffer(GL_ARRAY_BUFFER, mVBufferID);
            bindToShader(glshader->getVertexDescriptor(), glshader->getProgramId());
            checkGLError("VertexBuffer::bindBuffer");
            return true;
        }
        return false;
    }


    void GLVertexBuffer::bindToShader(DataDescriptor& desc, GLuint programId)
    {
        if (mProgramID == programId)
        {
            return;
        }
        mProgramID = programId;
        LOGV("VertexBuffer::bindToShader bind buffer %d to shader %d", mVBufferID, programId);
        int vsize = getTotalSize();
        desc.forEachEntry([this, programId, vsize](const DataDescriptor::DataEntry &e)
        {
            const DataDescriptor::DataEntry* entry = find(e.Name);
            if (entry != nullptr)
            {
                long offset =  entry->Offset;
                GLint loc = glGetAttribLocation(programId, e.Name.c_str());
                if (loc >= 0)
                {
                    glVertexAttribPointer(loc, entry->Size / sizeof(float),
                                       (entry->Type[0] == 'i') ? GL_INT : GL_FLOAT, GL_FALSE,
                                       vsize, (GLvoid*) offset);
                    glEnableVertexAttribArray(loc);
                    LOGV("VertexBuffer: vertex attrib #%d %s loc %d ofs %ld", e.Index, e.Name.c_str(), loc, offset);
                    checkGLError("VertexBuffer::bindToShader");
                }
                else
                {
                    LOGE("SHADER: vertex attribute %s has no location in shader", e.Name.c_str());
                }
            }
        });
    }

    bool GLVertexBuffer::updateGPU(Renderer* renderer)
    {
        std::lock_guard<std::mutex> lock(mLock);
        const float* vertexData = getVertexData();
        if ((getVertexCount() == 0) || (vertexData == NULL))
        {
            LOGE("VertexBuffer::updateGPU no vertex data yet");
            return false;
        }
        if (mVArrayID == -1)
        {
            glGenVertexArrays(1, (GLuint*) &mVArrayID);
            LOGD("VertexBuffer::updateGPU creating vertex array %d", mVArrayID);
        }
        if (mVBufferID == -1)
        {
            glGenBuffers(1, (GLuint*) &mVBufferID);
            mIsDirty = true;
            LOGV("VertexBuffer::updateGPU created vertex buffer %d with %d vertices", mVBufferID, getVertexCount());
        }
        if (mIsDirty)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mVBufferID);
            glBufferData(GL_ARRAY_BUFFER, mTotalSize, mVertexData, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            mIsDirty = false;
            LOGV("VertexBuffer::updateGPU updated vertex buffer %d", mVBufferID);
        }
        return true;
    }


} // end gvrf


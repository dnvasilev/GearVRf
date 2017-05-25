/****
 *
 * VertexBuffer maintains a vertex data array with locations, normals,
 * colors and texcoords.
 *
 ****/
#include "gl_vertex_buffer.h"
#include "gl_index_buffer.h"
#include "gl_shader.h"

namespace gvr {
    GLVertexBuffer::GLVertexBuffer(const char* layout_desc, int vertexCount)
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

    /***
     * Binds a VertexBuffer to a specific shader.
     * The binding occurs if the VertexBuffer was previously used
     * by a different shader. The GL vertex array object is
     * configuration so that its vertex attributes match the
     * associated shader. The IndexBuffer is also associated
     * with the GL vertex array.
     * @param shader shader vertex buffer is rendered with
     * @param ibuf index buffer
     */
    void GLVertexBuffer::bindToShader(Shader* shader, IndexBuffer* ibuf)
    {
        GLuint programId = static_cast<GLShader*>(shader)->getProgramId();

        glBindVertexArray(mVArrayID);
        if (mProgramID == programId)
        {
            return;
        }
        mProgramID = programId;
        if (ibuf)
        {
            ibuf->bindBuffer(shader);
        }
        LOGV("VertexBuffer::bindToShader bind vertex array %d to shader %d", mVBufferID, programId);
        glBindBuffer(GL_ARRAY_BUFFER, mVBufferID);

        shader->getVertexDescriptor().forEachEntry([this, programId](const DataDescriptor::DataEntry &e)
        {
            LOGV("VertexBuffer::bindToShader find %s", e.Name);
            const DataDescriptor::DataEntry* entry = find(e.Name);
            if ((entry != nullptr) && entry->IsSet)
            {
                long offset =  entry->Offset;
                GLint loc = glGetAttribLocation(programId, e.Name);
                if (loc >= 0)
                {
                    glEnableVertexAttribArray(loc);
                    glVertexAttribPointer(loc, entry->Size / sizeof(float),
                                       (entry->Type[0] == 'i') ? GL_INT : GL_FLOAT, GL_FALSE,
                                          getTotalSize(), (GLvoid*) offset);
                    LOGV("VertexBuffer: vertex attrib #%d %s loc %d ofs %d", e.Index, e.Name, loc, entry->Offset);
                    checkGLError("VertexBuffer::bindToShader");
                }
                else
                {
                    LOGE("SHADER: vertex attribute %s has no location in shader", e.Name);
                }
            }
            else
            {
                LOGE("SHADER: shader needs vertex attribute %s but it is not found", e.Name);
            }
        });
    }

    bool GLVertexBuffer::updateGPU(Renderer* renderer, IndexBuffer* ibuf)
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
            glGenVertexArrays(1, &mVArrayID);
            LOGD("VertexBuffer::updateGPU creating vertex array %d", mVArrayID);
        }
        if (mVBufferID == -1)
        {
            glGenBuffers(1, &mVBufferID);
            LOGV("VertexBuffer::updateGPU created vertex buffer %d with %d vertices", mVBufferID, getVertexCount());
            mIsDirty = true;
        }
        if (ibuf)
        {
            ibuf->updateGPU(renderer);
        }
        if (mIsDirty)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mVBufferID);
            glBufferData(GL_ARRAY_BUFFER, getDataSize(), mVertexData, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            mIsDirty = false;
            LOGV("VertexBuffer::updateGPU updated vertex buffer %d", mVBufferID);
        }
        return true;
    }


} // end gvrf


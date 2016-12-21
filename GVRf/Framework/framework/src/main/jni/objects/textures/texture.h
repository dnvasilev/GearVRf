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

/***************************************************************************
 * Textures.
 ***************************************************************************/

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "gl/gl_texture.h"
#include "objects/hybrid_object.h"
#include "objects/gl_pending_task.h"
#include "image.h"

namespace gvr {

class Texture: public HybridObject, GLPendingTask {
public:
    virtual ~Texture() {
        delete gl_texture_;
    }

    // Should be called in GL context.
    virtual GLuint getId() {
        if (gl_texture_ == 0) {
            // must be recycled already. The caller will handle error.
            return 0;
        }
        // Before returning the ID makes sure nothing is pending
        runPendingGL();
        return gl_texture_->id();
    }

    virtual void setImage(Image* image)
    {
        mImage = image;
        if (gl_texture_)
        {
            GLenum target = (GLenum) mImage->getType();
            gl_texture_->set_target(target);
        }
    }

    void updateTextureParameters(int* texture_parameters, int n)
    {
       gl_texture_->set_texture_parameters(texture_parameters);
    }

    virtual GLenum getTarget() const
    {
        if (mImage)
        {
            return mImage->getType();
        }
        return 0;
    }

    virtual void runPendingGL()
    {
        int texid = gl_texture_->id();
        if (mImage && texid)
        {
            if (mImage->checkForUpdate(texid))
            {
                setReady(true);
            }
        }
        gl_texture_->runPendingGL();
    }

    bool isReady() {
        return ready;
    }

    void setReady(bool ready) {
        this->ready = ready;
    }

protected:
    Texture(GLTexture* gl_texture) : HybridObject() {
        gl_texture_ = gl_texture;
        mImage = NULL;
    }

    GLTexture*  gl_texture_;
    Image*      mImage;

private:
    Texture(const Texture& texture);
    Texture(Texture&& texture);
    Texture& operator=(const Texture& texture);
    Texture& operator=(Texture&& texture);

private:
    static const GLenum target = GL_TEXTURE_2D;
    bool ready = false;
};

}

#endif

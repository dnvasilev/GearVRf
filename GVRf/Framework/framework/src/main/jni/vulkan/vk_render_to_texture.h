#ifndef FRAMEWORK_VK_RENDER_TO_TEXTURE_H
#define FRAMEWORK_VK_RENDER_TO_TEXTURE_H



#include "objects/textures/render_texture.h"
#include "vk_framebuffer.h"


namespace gvr{
class VkRenderTexture : public RenderTexture
{
    VKFramebuffer* fbo;
    void createRenderPass();
    int mWidth, mHeight;
    VkClearValue clear_values[2];
public:
    VKFramebuffer* getFBO(){
        return fbo;
    }
    virtual ~VkRenderTexture(){
        delete fbo;
    }
    VkRenderTexture(int width, int height, int sample_count = 1):RenderTexture(sample_count), fbo(nullptr),mWidth(width), mHeight(height){}
    virtual unsigned int getFrameBufferId() const {

    }

    virtual unsigned int getDepthBufferId() const {

    }
    virtual void bind();
    virtual void beginRendering(Renderer* renderer);
    virtual void endRendering(Renderer* renderer);
    // Start to read back texture in the background. It can be optionally called before
    // readRenderResult() to read pixels asynchronously. This function returns immediately.
    virtual void startReadBack() {

    }

    // Copy data in pixel buffer to client memory. This function is synchronous. When
    // it returns, the pixels have been copied to PBO and then to the client memory.
    virtual bool readRenderResult(unsigned int *readback_buffer, long capacity) {
    }
    bool readVkRenderResult(uint8_t **readback_buffer, VkCommandBuffer& cmd_buffer,VkFence& fence);
    const VkRenderPass* getRenderPass(){
        if(fbo == nullptr)
            bind();

        return fbo->getRenderPass();
    }

};

}
#endif //FRAMEWORK_VK_RENDER_TO_TEXTURE_H
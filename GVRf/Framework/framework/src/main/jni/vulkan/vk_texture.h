//
// Created by roshan on 12/28/16.
//

#ifndef FRAMEWORK_VK_TEXTURE_H
#define FRAMEWORK_VK_TEXTURE_H
#include <cstdlib>
#include "vulkan/vulkanCore.h"
#include "vulkan/vulkanInfoWrapper.h"
#include "../objects/textures/image.h"
#include "../objects/textures/texture.h"
#include "vulkan_image.h"
#include "vk_bitmap_image.h"
#include "vk_cubemap_image.h"

namespace gvr {
    class VkTexture : public Texture
    {
    public:
        explicit VkTexture() : Texture() { }

        explicit VkTexture(int texture_type) :
                Texture(texture_type)
        {
        }

        virtual ~VkTexture();
        virtual bool isReady();

        const VkImageView& getVkImageView()
        {
            if (mImage == NULL)
                LOGE("GetImageView : image is NULL");

            VkCubemapImage* cubemapImage;
            VkBitmapImage* bitmapImage;
            switch(mImage->getType()){

                case Image::ImageType::CUBEMAP:
                    cubemapImage = reinterpret_cast<VkCubemapImage*>(mImage);
                    return cubemapImage->getVkImageView();

                case Image::ImageType::BITMAP:
                    bitmapImage = reinterpret_cast<VkBitmapImage*>(mImage);
                    return bitmapImage->getVkImageView();
            }
        }
        VkSampler& getVkSampler(){
            return m_sampler;
        }
    private:
        VkTexture(const VkTexture& gl_texture);
        VkTexture(VkTexture&& gl_texture);
        VkTexture& operator=(const VkTexture& gl_texture);
        VkTexture& operator=(VkTexture&& gl_texture);
        void createSampler(int maxLod);
        void updateSampler();
        bool updateImage();

    protected:
        static VkSamplerAddressMode MapWrap[];
        static VkFilter MapFilter[];
        static VkSamplerMipmapMode mipmapMode[];
        VkSampler m_sampler;
        VkWriteDescriptorSet writeDescriptorSet;
    };

}
#endif //FRAMEWORK_VK_TEXTURE_H

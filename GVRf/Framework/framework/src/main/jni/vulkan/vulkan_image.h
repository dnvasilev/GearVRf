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

#ifndef VULKAN_IMAGE_H
#define VULKAN_IMAGE_H

#include "objects/textures/image.h"
#include "vk_image.h"

namespace gvr {
    class VulkanImage : public vkImage, public Image
    {
    public:
        VulkanImage(int target);
        VulkanImage(ImageType type, int format, short width, short height);

        virtual bool isReady()
        {
            return checkForUpdate(true);
        }

        virtual void texParamsChanged(const TextureParameters& texparams) { }
    };

    inline VulkanImage::VulkanImage(int target) :
            Image(ImageType::BITMAP, target),
            vkImage(VK_IMAGE_VIEW_TYPE_2D)
    { }

    inline VulkanImage::VulkanImage(ImageType type, int format, short width, short height) :
        Image(type, format),
        vkImage(VK_IMAGE_VIEW_TYPE_2D)
    {
        Image::mWidth = width;
        Image::mHeight = height;
    }

}
#endif
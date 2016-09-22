#include "vulkanInfoWrapper.h"

namespace gvr {

ImageCreateInfo::ImageCreateInfo(VkImageType aImageType, VkFormat aFormat,
    int32_t aWidth, int32_t aHeight, int32_t aDepth,
    VkImageTiling aTiling, VkImageUsageFlags aUsage, VkImageLayout aLayout)
    : ImageCreateInfo(aImageType, aFormat, aWidth, aHeight, aDepth, 1, aTiling, aUsage, aLayout)
{
}

ImageCreateInfo::ImageCreateInfo(VkImageType aImageType, VkFormat aFormat, int32_t aWidth,
    int32_t aHeight, int32_t aDepth, uint32_t aArraySize, VkImageTiling aTiling,
    VkImageUsageFlags aUsage, VkImageLayout aLayout)
    : ImageCreateInfo(aImageType, aFormat, aWidth, aHeight, aDepth, 1, 1, aTiling, aUsage, VK_SAMPLE_COUNT_1_BIT, aLayout)
{
}

ImageCreateInfo::ImageCreateInfo(VkImageType aImageType, VkFormat aFormat,
    int32_t aWidth, int32_t aHeight, int32_t aDepth, uint32_t aMipLevels,
    uint32_t aArraySize, VkImageTiling aTiling, VkImageUsageFlags aUsage,
    VkSampleCountFlagBits aSamples,
    VkImageLayout aLayout) :
    mInfo{}
{
    mInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    mInfo.imageType = aImageType;
    mInfo.format = aFormat;
    mInfo.extent.width = aWidth;
    mInfo.extent.height = aHeight;
    mInfo.extent.depth = aDepth;
    mInfo.mipLevels = aMipLevels;
    mInfo.arrayLayers = aArraySize;
    mInfo.samples = aSamples;
    mInfo.tiling = aTiling;
    mInfo.usage = aUsage;
    mInfo.initialLayout = aLayout;
    mInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
}

ImageViewCreateInfo::ImageViewCreateInfo(VkImage aImage, VkImageViewType aType, VkFormat aFormat,
    VkImageAspectFlags aAspectFlags)
    : ImageViewCreateInfo(aImage, aType, aFormat, 1, aAspectFlags)
{
}

ImageViewCreateInfo::ImageViewCreateInfo(VkImage aImage, VkImageViewType aType, VkFormat aFormat,
    uint32_t aArraySize, VkImageAspectFlags aAspectFlags)
    : ImageViewCreateInfo(aImage, aType, aFormat, 1, aArraySize, aAspectFlags)
{
}

ImageViewCreateInfo::ImageViewCreateInfo(VkImage aImage, VkImageViewType aType, VkFormat aFormat, uint32_t aMipLevels,
    uint32_t aArraySize, VkImageAspectFlags aAspectFlags)
    : mInfo{}
{
    mInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    mInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    mInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    mInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    mInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    mInfo.format = aFormat;
    mInfo.image = aImage;
    mInfo.subresourceRange.aspectMask = aAspectFlags;
    mInfo.subresourceRange.baseArrayLayer = 0;
    mInfo.subresourceRange.layerCount = aArraySize;
    mInfo.subresourceRange.baseMipLevel = 0;
    mInfo.subresourceRange.levelCount = aMipLevels;
    mInfo.viewType = aType;
}

CmdPoolCreateInfo::CmdPoolCreateInfo(VkCommandPoolCreateFlags aFlags, uint32_t aFamilyIndex)
    : mInfo()
{
    mInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    mInfo.pNext = nullptr;
    mInfo.flags = aFlags;
    mInfo.queueFamilyIndex = aFamilyIndex;
}

CmdBufferCreateInfo::CmdBufferCreateInfo(VkCommandBufferLevel aLevel, VkCommandPool aCmdPool)
    : mInfo()
{
    mInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    mInfo.level = aLevel;
    mInfo.commandPool = aCmdPool;
    mInfo.commandBufferCount = 1;
}

BufferCreateInfo::BufferCreateInfo(VkDeviceSize aSize,
    VkBufferUsageFlags aUsageFlags, VkBufferCreateFlags aCreateFlags)
    : mInfo()
{
    mInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    mInfo.size = aSize;
    mInfo.usage = aUsageFlags;
    mInfo.flags = aCreateFlags;
}

ShaderModuleCreateInfo::ShaderModuleCreateInfo(const uint32_t* aCode, size_t aCodeSize,
    VkShaderModuleCreateFlags aFlags)
{
    mCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    mCreateInfo.pNext = nullptr;
    mCreateInfo.codeSize = aCodeSize;
    mCreateInfo.pCode = aCode;
    mCreateInfo.flags = aFlags;
}

SemaphoreCreateInfo::SemaphoreCreateInfo(VkSemaphoreCreateFlags aFlags)
    : mInfo { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, aFlags }
{
}

FenceCreateInfo::FenceCreateInfo(VkFenceCreateFlags aFlags)
    : mInfo()
{
    mInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
}

}
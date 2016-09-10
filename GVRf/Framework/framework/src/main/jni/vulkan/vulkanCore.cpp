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

#include "vulkanCore.h"
#include "util/gvr_log.h"
#include <assert.h>
#include <cstring>
#include <iostream>
#include <vector>
#define UINT64_MAX 99999
VulkanCore* VulkanCore::theInstance = NULL;
uint8_t *finaloutput;
#define QUEUE_INDEX_MAX 99999
#define VERTEX_BUFFER_BIND_ID 0

bool VulkanCore::CreateInstance(){
    VkResult ret = VK_SUCCESS;

    // Discover the number of extensions listed in the instance properties in order to allocate
    // a buffer large enough to hold them.
    uint32_t instanceExtensionCount = 0;
    ret = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
    GVR_VK_CHECK(!ret);

    VkBool32 surfaceExtFound = 0;
    VkBool32 platformSurfaceExtFound = 0;
    VkExtensionProperties* instanceExtensions = nullptr;
    instanceExtensions = new VkExtensionProperties[instanceExtensionCount];

    // Now request instanceExtensionCount VkExtensionProperties elements be read into out buffer
    ret = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions);
    GVR_VK_CHECK(!ret);

    // We require two extensions, VK_KHR_surface and VK_KHR_android_surface. If they are found,
    // add them to the extensionNames list that we'll use to initialize our instance with later.
    uint32_t enabledExtensionCount = 0;
    const char* extensionNames[16];
    for (uint32_t i = 0; i < instanceExtensionCount; i++) {
        if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instanceExtensions[i].extensionName)) {
            surfaceExtFound = 1;
            extensionNames[enabledExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }

        if (!strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, instanceExtensions[i].extensionName)) {
            platformSurfaceExtFound = 1;
            extensionNames[enabledExtensionCount++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
        }
        GVR_VK_CHECK(enabledExtensionCount < 16);
    }
    if (!surfaceExtFound) {
        LOGE("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_SURFACE_EXTENSION_NAME" extension.");
        return false;
    }
    if (!platformSurfaceExtFound) {
        LOGE("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_ANDROID_SURFACE_EXTENSION_NAME" extension.");
        return false;
    }

    // We specify the Vulkan version our application was built with,
    // as well as names and versions for our application and engine,
    // if applicable. This allows the driver to gain insight to what
    // is utilizing the vulkan driver, and serve appropriate versions.
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = GVR_VK_SAMPLE_NAME;
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName = "VkSample";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    // Creation information for the instance points to details about
    // the application, and also the list of extensions to enable.
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;
    instanceCreateInfo.enabledExtensionCount = enabledExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = extensionNames;


    // The main Vulkan instance is created with the creation infos above.
    // We do not specify a custom memory allocator for instance creation.
    ret = vkCreateInstance(&instanceCreateInfo, nullptr, &(m_instance));

    // we can delete the list of extensions after calling vkCreateInstance
    delete[] instanceExtensions;

    // Vulkan API return values can expose further information on a failure.
    // For instance, INCOMPATIBLE_DRIVER may be returned if the API level
    // an application is built with, exposed through VkApplicationInfo, is
    // newer than the driver present on a device.
    if (ret == VK_ERROR_INCOMPATIBLE_DRIVER) {
        LOGE("Cannot find a compatible Vulkan installable client driver: vkCreateInstance Failure");
        return false;
    } else if (ret == VK_ERROR_EXTENSION_NOT_PRESENT) {
        LOGE("Cannot find a specified extension library: vkCreateInstance Failure");
        return false;
    } else {
        GVR_VK_CHECK(!ret);
    }

    return true;
}

bool VulkanCore::GetPhysicalDevices(){
    VkResult ret = VK_SUCCESS;

    // Query number of physical devices available
    ret = vkEnumeratePhysicalDevices(m_instance, &(m_physicalDeviceCount), nullptr);
    GVR_VK_CHECK(!ret);

    if (m_physicalDeviceCount == 0)
    {
        LOGE("No physical devices detected.");
        return false;
    }

    // Allocate space the the correct number of devices, before requesting their data
    m_pPhysicalDevices = new VkPhysicalDevice[m_physicalDeviceCount];
    ret = vkEnumeratePhysicalDevices(m_instance, &(m_physicalDeviceCount), m_pPhysicalDevices);
    GVR_VK_CHECK(!ret);


    // For purposes of this sample, we simply use the first device.
    m_physicalDevice = m_pPhysicalDevices[0];

    // By querying the device properties, we learn the device name, amongst
    // other details.
    vkGetPhysicalDeviceProperties(m_physicalDevice, &(m_physicalDeviceProperties));

    LOGI("Vulkan Device: %s", m_physicalDeviceProperties.deviceName);

    // Get Memory information and properties - this is required later, when we begin
    // allocating buffers to store data.
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &(m_physicalDeviceMemoryProperties));

    return true;
}

void VulkanCore::InitSurface()
{
    VkResult ret = VK_SUCCESS;
    // At this point, we create the android surface. This is because we want to
    // ensure our device is capable of working with the created surface object.
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.window = m_androidWindow;
    LOGI("Vulkan Before surface creation");
    if(m_androidWindow == NULL)
        LOGI("Vulkan Before surface null");
    else
        LOGI("Vulkan Before not null surface creation");
    ret = vkCreateAndroidSurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface);
    GVR_VK_CHECK(!ret);
    LOGI("Vulkan After surface creation");
}

bool VulkanCore::InitDevice() {
    VkResult ret = VK_SUCCESS;
    // Akin to when creating the instance, we can query extensions supported by the physical device
    // that we have selected to use.
    uint32_t deviceExtensionCount = 0;
    VkExtensionProperties *device_extensions = nullptr;
    ret = vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &deviceExtensionCount, nullptr);
    GVR_VK_CHECK(!ret);

    VkBool32 swapchainExtFound = 0;
    VkExtensionProperties* deviceExtensions = new VkExtensionProperties[deviceExtensionCount];
    ret = vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &deviceExtensionCount, deviceExtensions);
    GVR_VK_CHECK(!ret);

    // For our example, we require the swapchain extension, which is used to present backbuffers efficiently
    // to the users screen.
    uint32_t enabledExtensionCount = 0;
    const char* extensionNames[16] = {0};
    for (uint32_t i = 0; i < deviceExtensionCount; i++) {
        if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, deviceExtensions[i].extensionName)) {
            swapchainExtFound = 1;
            extensionNames[enabledExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
        GVR_VK_CHECK(enabledExtensionCount < 16);
    }
    if (!swapchainExtFound) {
        LOGE("vkEnumerateDeviceExtensionProperties failed to find the " VK_KHR_SWAPCHAIN_EXTENSION_NAME " extension: vkCreateInstance Failure");

        // Always attempt to enable the swapchain
        extensionNames[enabledExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    }

    InitSurface();

    // Before we create our main Vulkan device, we must ensure our physical device
    // has queue families which can perform the actions we require. For this, we request
    // the number of queue families, and their properties.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

    VkQueueFamilyProperties* queueProperties = new VkQueueFamilyProperties[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueProperties);
    GVR_VK_CHECK(queueFamilyCount >= 1);

    // We query each queue family in turn for the ability to support the android surface
    // that was created earlier. We need the device to be able to present its images to
    // this surface, so it is important to test for this.
    VkBool32* supportsPresent = new VkBool32[queueFamilyCount];
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i,  m_surface, &supportsPresent[i]);
    }


    // Search for a graphics queue, and ensure it also supports our surface. We want a
    // queue which can be used for both, as to simplify operations.
    uint32_t queueIndex = QUEUE_INDEX_MAX;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if ((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (supportsPresent[i] == VK_TRUE) {
                queueIndex = i;
                break;
            }
        }
    }

    delete [] supportsPresent;
    delete [] queueProperties;

    if (queueIndex == QUEUE_INDEX_MAX) {
        GVR_VK_CHECK("Could not obtain a queue family for both graphics and presentation." && 0);
        return false;
    }

    // We have identified a queue family which both supports our android surface,
    // and can be used for graphics operations.
    m_queueFamilyIndex = queueIndex;


    // As we create the device, we state we will be creating a queue of the
    // family type required. 1.0 is the highest priority and we use that.
    float queuePriorities[1] = { 1.0 };
    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = nullptr;
    deviceQueueCreateInfo.queueFamilyIndex = m_queueFamilyIndex;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    // Now we pass the queue create info, as well as our requested extensions,
    // into our DeviceCreateInfo structure.
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = enabledExtensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = extensionNames;

    // Create the device.
    ret = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
    GVR_VK_CHECK(!ret);

    // Obtain the device queue that we requested.
    vkGetDeviceQueue(m_device, m_queueFamilyIndex, 0, &m_queue);
    return true;
}

void VulkanCore::InitSwapchain(uint32_t width, uint32_t height){
    VkResult ret = VK_SUCCESS;
    // By querying the supported formats of our surface, we can ensure that
    // we use one that the device can work with.
    uint32_t formatCount;
    ret = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    GVR_VK_CHECK(!ret);

    VkSurfaceFormatKHR* surfFormats = new VkSurfaceFormatKHR[formatCount];
    ret = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, surfFormats);
    GVR_VK_CHECK(!ret);

    finaloutput = (uint8_t*)malloc(width * height *4 * sizeof(uint8_t));
    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned. For the purposes of this sample,
    // we use the first format returned.
    if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        m_surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        m_surfaceFormat.colorSpace = surfFormats[0].colorSpace;
    }
    else
    {
        m_surfaceFormat = surfFormats[0];
    }

    //m_surfaceFormat.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    delete [] surfFormats;

        LOGI("Vulkan Surface format %d %d", m_surfaceFormat.format, m_surfaceFormat.colorSpace);

    // Now we obtain the surface capabilities, which contains details such as width and height.
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    ret = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCapabilities);
    GVR_VK_CHECK(!ret);

    m_width = surfaceCapabilities.currentExtent.width;
    m_height = surfaceCapabilities.currentExtent.height;
    m_width = width;
    m_height = height;

    // Now that we have selected formats and obtained ideal surface dimensions,
    // we create the swapchain. We use FIFO mode, which is always present. This
    // mode has a queue of images internally, that will be presented to the screen.
    // The swapchain will be created and expose the number of images created
    // in the queue, which will be at least the number specified in minImageCount.
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface               = m_surface;
    swapchainCreateInfo.minImageCount         = surfaceCapabilities.minImageCount;
    swapchainCreateInfo.imageFormat           = m_surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace       = m_surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent.width     = m_width;
    swapchainCreateInfo.imageExtent.height    = m_height;
    swapchainCreateInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;//surfaceCapabilities.supportedUsageFlags;
    swapchainCreateInfo.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.imageArrayLayers      = 1;
    swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode           = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.clipped               = VK_TRUE;


    ret = vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain);
    GVR_VK_CHECK(!ret);


    // Query the number of swapchain images. This is the number of images in the internal
    // queue.
    ret = vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainImageCount, nullptr);
    GVR_VK_CHECK(!ret);

    LOGI("Swapchain Image Count: %d\n", m_swapchainImageCount);
    //m_swapchainImageCount = 1;

    // Now we can retrieve these images, as to use them in rendering as our framebuffers.
    VkImage* pSwapchainImages = new VkImage[m_swapchainImageCount];
    ret = vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainImageCount, pSwapchainImages);
    GVR_VK_CHECK(!ret);

    // We prepare our own representation of the swapchain buffers, for keeping track
    // of resources during rendering.
    m_swapchainBuffers = new GVR_VK_SwapchainBuffer[m_swapchainImageCount];
    GVR_VK_CHECK(m_swapchainBuffers);

    m_outputBuffers = new OutputBuffer[m_swapchainImageCount];


    // From the images obtained from the swapchain, we create image views.
    // This gives us context into the image.
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.format = m_surfaceFormat.format;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.flags = 0;


    VkMemoryRequirements memReqs;

    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
        VkResult err;
        // We create an Imageview for each swapchain image, and track
        // the view and image in our swapchainBuffers object.
        m_swapchainBuffers[i].image = pSwapchainImages[i];
        imageViewCreateInfo.image = pSwapchainImages[i];

        err = vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_swapchainBuffers[i].view);
        GVR_VK_CHECK(!err);


        // Create Output VkBuffer for each image

        // Get Image Requirements from Swap Chain Images
        vkGetBufferMemoryRequirements(m_device, m_swapchainBuffers[i].image, &memReqs);

        VkBufferCreateInfo outputinfo = {};
        outputinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        outputinfo.size = m_width*m_height*4* sizeof(uint8_t);//memReqs.size;
        outputinfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        outputinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;



        err = vkCreateBuffer(m_device, &outputinfo, nullptr, &m_outputBuffers[i].imageOutputBuffer);
        GVR_VK_CHECK(!err);

        vkGetBufferMemoryRequirements(m_device, m_outputBuffers[i].imageOutputBuffer, &memReqs);

        m_outputBuffers[i].size =  memReqs.size;
        LOGE("Vulkan buffer size %u \t %u",  memReqs.size);

        // Allocate Memory
        VkMemoryAllocateInfo memAlloc = {};
        memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = 0;

        uint32_t memoryTypeIndex;
        if(GetMemoryTypeFromProperties(memReqs.memoryTypeBits,  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryTypeIndex)!=true)
            LOGE("Vulkan failed got memory requirements");

        memAlloc.memoryTypeIndex = memoryTypeIndex;
        err = vkAllocateMemory(m_device, &memAlloc, nullptr, &m_outputBuffers[i].memory);
        GVR_VK_CHECK(!err);

        // Bind our buffer to the memory.
        err = vkBindBufferMemory(m_device, m_outputBuffers[i].imageOutputBuffer, m_outputBuffers[i].memory, 0);
        GVR_VK_CHECK(!err);


    }

    // At this point, we have the references now in our swapchainBuffer object
    delete [] pSwapchainImages;


    // Now we create depth buffers for our swapchain images, which form part of
    // our framebuffers later.
    m_depthBuffers = new GVR_VK_DepthBuffer[m_swapchainImageCount];
    for (int i = 0; i < m_swapchainImageCount; i++) {
        const VkFormat depthFormat = VK_FORMAT_D16_UNORM;

        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.pNext = nullptr;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = depthFormat;
        imageCreateInfo.extent = {m_width, m_height, 1};
        imageCreateInfo .mipLevels = 1;
        imageCreateInfo .arrayLayers = 1;
        imageCreateInfo .samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageCreateInfo .flags = 0;

        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo .pNext = nullptr;
        imageViewCreateInfo .image = VK_NULL_HANDLE;
        imageViewCreateInfo.format = depthFormat;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

        VkMemoryRequirements mem_reqs;
        VkResult  err;
        bool  pass;

        m_depthBuffers[i].format = depthFormat;

        // Create the image with details as imageCreateInfo
        err = vkCreateImage(m_device, &imageCreateInfo, nullptr, &m_depthBuffers[i].image);
        GVR_VK_CHECK(!err);

        // discover what memory requirements are for this image.
        vkGetImageMemoryRequirements(m_device, m_depthBuffers[i].image, &mem_reqs);

        // Allocate memory according to requirements
        VkMemoryAllocateInfo memoryAllocateInfo = {};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.pNext = nullptr;
        memoryAllocateInfo.allocationSize = 0;
        memoryAllocateInfo.memoryTypeIndex = 0;
        memoryAllocateInfo.allocationSize = mem_reqs.size;
        pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, 0, &memoryAllocateInfo.memoryTypeIndex);
        GVR_VK_CHECK(pass);

        err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_depthBuffers[i].mem);
        GVR_VK_CHECK(!err);

        // Bind memory to the image
        err = vkBindImageMemory(m_device, m_depthBuffers[i].image, m_depthBuffers[i].mem, 0);
        GVR_VK_CHECK(!err);

        // Create the view for this image
        imageViewCreateInfo.image = m_depthBuffers[i].image;
        err = vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_depthBuffers[i].view);
        GVR_VK_CHECK(!err);
    }

 }

bool VulkanCore::GetMemoryTypeFromProperties( uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex)
{
    GVR_VK_CHECK(typeIndex != nullptr);
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < 32; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((m_physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags &
                 requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

void VulkanCore::InitCommandbuffers(){
    VkResult ret = VK_SUCCESS;
    // Command buffers are allocated from a pool; we define that pool here and create it.
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.queueFamilyIndex = m_queueFamilyIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    ret = vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool);
    GVR_VK_CHECK(!ret);


    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandPool = m_commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    // Create render command buffers, one per swapchain image
    for (int i=0; i < m_swapchainImageCount; i++)
    {
        ret = vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_swapchainBuffers[i].cmdBuffer);
        GVR_VK_CHECK(!ret);
    }
}

void VulkanCore::InitVertexBuffers(){
    // Our vertex buffer data is a simple triangle, with associated vertex colors.
    const float vb[3][7] = {
            //      position                   color
            { -0.9f, -0.9f,  0.9f,     1.0f, 0.0f, 0.0f, 1.0f },
            {  0.9f, -0.9f,  0.9f,     1.0f, 0.0f, 0.0f, 1.0f },
            {  0.0f,  0.9f,  0.9f,     1.0f, 0.0f, 0.0f, 1.0f },
    };

    VkResult   err;
    bool   pass;

    // Our m_vertices member contains the types required for storing
    // and defining our vertex buffer within the graphics pipeline.
    memset(&m_vertices, 0, sizeof(m_vertices));

    // Create our buffer object.
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.size = sizeof(vb);
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCreateInfo.flags = 0;
    err = vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &m_vertices.buf);
    GVR_VK_CHECK(!err);

    // Obtain the memory requirements for this buffer.
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_device, m_vertices.buf, &mem_reqs);
    GVR_VK_CHECK(!err);

    // And allocate memory according to those requirements.
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.allocationSize = 0;
    memoryAllocateInfo.memoryTypeIndex = 0;
    memoryAllocateInfo.allocationSize  = mem_reqs.size;
    pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
    GVR_VK_CHECK(pass);

    err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_vertices.mem);
    GVR_VK_CHECK(!err);

    // Now we need to map the memory of this new allocation so the CPU can edit it.
    void *data;
    err = vkMapMemory(m_device, m_vertices.mem, 0, memoryAllocateInfo.allocationSize, 0, &data);
    GVR_VK_CHECK(!err);

    // Copy our triangle verticies and colors into the mapped memory area.
    memcpy(data, vb, sizeof(vb));


    // Unmap the memory back from the CPU.
    vkUnmapMemory(m_device, m_vertices.mem);

    // Bind our buffer to the memory.
    err = vkBindBufferMemory(m_device, m_vertices.buf, m_vertices.mem, 0);
    GVR_VK_CHECK(!err);

    // The vertices need to be defined so that the pipeline understands how the
    // data is laid out. This is done by providing a VkPipelineVertexInputStateCreateInfo
    // structure with the correct information.
    m_vertices.vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_vertices.vi.pNext = nullptr;
    m_vertices.vi.vertexBindingDescriptionCount = 1;
    m_vertices.vi.pVertexBindingDescriptions = m_vertices.vi_bindings;
    m_vertices.vi.vertexAttributeDescriptionCount = 2;
    m_vertices.vi.pVertexAttributeDescriptions = m_vertices.vi_attrs;

    // We bind the buffer as a whole, using the correct buffer ID.
    // This defines the stride for each element of the vertex array.
    m_vertices.vi_bindings[0].binding = GVR_VK_VERTEX_BUFFER_BIND_ID;
    m_vertices.vi_bindings[0].stride = sizeof(vb[0]);
    m_vertices.vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Within each element, we define the attributes. At location 0,
    // the vertex positions, in float3 format, with offset 0 as they are
    // first in the array structure.
    m_vertices.vi_attrs[0].binding = GVR_VK_VERTEX_BUFFER_BIND_ID;
    m_vertices.vi_attrs[0].location = 0;
    m_vertices.vi_attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT; //float3
    m_vertices.vi_attrs[0].offset = 0;

    // The second location is the vertex colors, in RGBA float4 format.
    // These appear in each element in memory after the float3 vertex
    // positions, so the offset is set accordingly.
    m_vertices.vi_attrs[1].binding = GVR_VK_VERTEX_BUFFER_BIND_ID;
    m_vertices.vi_attrs[1].location = 1;
    m_vertices.vi_attrs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; //float4
    m_vertices.vi_attrs[1].offset = sizeof(float) * 3;
}

void VulkanCore::InitLayouts(){
    VkResult ret = VK_SUCCESS;
    // This sample has no bindings, so the layout is empty.
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.pNext = nullptr;
    descriptorSetLayoutCreateInfo.bindingCount = 0;
    descriptorSetLayoutCreateInfo.pBindings = nullptr;

    ret = vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorLayout);
    GVR_VK_CHECK(!ret);

    // Our pipeline layout simply points to the empty descriptor layout.
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext              = nullptr;
    pipelineLayoutCreateInfo.setLayoutCount     = 1;
    pipelineLayoutCreateInfo.pSetLayouts        = &m_descriptorLayout;
    ret = vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
    GVR_VK_CHECK(!ret);
}

void VulkanCore::InitRenderPass(){
// The renderpass defines the attachments to the framebuffer object that gets
    // used in the pipeline. We have two attachments, the colour buffer, and the
    // depth buffer. The operations and layouts are set to defaults for this type
    // of attachment.
    VkAttachmentDescription attachmentDescriptions[2] = {};
    attachmentDescriptions[0].flags = 0;
    attachmentDescriptions[0].format = m_surfaceFormat.format;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    attachmentDescriptions[1].flags = 0;
    attachmentDescriptions[1].format = m_depthBuffers[0].format;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // We have references to the attachment offsets, stating the layout type.
    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // There can be multiple subpasses in a renderpass, but this example has only one.
    // We set the color and depth references at the grahics bind point in the pipeline.
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.flags = 0;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pResolveAttachments = nullptr;
    subpassDescription.pDepthStencilAttachment = nullptr;//&depthReference;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;

    // The renderpass itself is created with the number of subpasses, and the
    // list of attachments which those subpasses can reference.
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 0;
    renderPassCreateInfo.pDependencies = nullptr;

    VkResult ret;
    ret = vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &m_renderPass);
    GVR_VK_CHECK(!ret);
}

void VulkanCore::InitPipeline(){

    VkResult   err;

    // The pipeline contains all major state for rendering.

    // Our vertex input is a single vertex buffer, and its layout is defined
    // in our m_vertices object already. Use this when creating the pipeline.
    VkPipelineVertexInputStateCreateInfo   vi = {};
    vi = m_vertices.vi;

    // Our vertex buffer describes a triangle list.
    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // State for rasterization, such as polygon fill mode is defined.
    VkPipelineRasterizationStateCreateInfo rs = {};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;

    // For this example we do not do blending, so it is disabled.
    VkPipelineColorBlendAttachmentState att_state[1] = {};
    att_state[0].colorWriteMask = 0xf;
    att_state[0].blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo    cb = {};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &att_state[0];


    // We define a simple viewport and scissor. It does not change during rendering
    // in this sample.
    VkPipelineViewportStateCreateInfo      vp = {};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    VkViewport viewport = {};
    viewport.height = (float) m_height;
    viewport.width = (float) m_width;
    viewport.minDepth = (float) 0.0f;
    viewport.maxDepth = (float) 1.0f;
    vp.pViewports = &viewport;

    VkRect2D scissor = {};
    scissor.extent.width = m_width;
    scissor.extent.height = m_height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vp.pScissors = &scissor;

    // Standard depth and stencil state is defined
    VkPipelineDepthStencilStateCreateInfo  ds = {};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    // We do not use multisample
    VkPipelineMultisampleStateCreateInfo   ms = {};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pSampleMask = nullptr;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // We define two shader stages: our vertex and fragment shader.
    // they are embedded as SPIR-V into a header file for ease of deployment.
    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    shaderStages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = CreateShaderModule( (const uint32_t*)&shader_tri_vert[0], shader_tri_vert_size);
    shaderStages[0].pName  = "main";
    shaderStages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = CreateShaderModule( (const uint32_t*)&shader_tri_frag[0], shader_tri_frag_size);
    shaderStages[1].pName  = "main";

    // Pipelines are allocated from pipeline caches.
    VkPipelineCacheCreateInfo pipelineCache = {};
    pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCache.pNext = nullptr;
    pipelineCache.flags = 0;

    VkPipelineCache piplineCache;
    err = vkCreatePipelineCache(m_device, &pipelineCache, nullptr, &piplineCache);
    GVR_VK_CHECK(!err);

    // Out graphics pipeline records all state information, including our renderpass
    // and pipeline layout. We do not have any dynamic state in this example.
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout              = m_pipelineLayout;
    pipelineCreateInfo.pVertexInputState   = &vi;
    pipelineCreateInfo.pInputAssemblyState = &ia;
    pipelineCreateInfo.pRasterizationState = &rs;
    pipelineCreateInfo.pColorBlendState    = &cb;
    pipelineCreateInfo.pMultisampleState   = &ms;
    pipelineCreateInfo.pViewportState      = &vp;
    pipelineCreateInfo.pDepthStencilState  = nullptr;//&ds;
    pipelineCreateInfo.pStages             = &shaderStages[0];
    pipelineCreateInfo.renderPass          = m_renderPass;
    pipelineCreateInfo.pDynamicState       = nullptr;
    pipelineCreateInfo.stageCount          = 2; //vertex and fragment

    err = vkCreateGraphicsPipelines(m_device, piplineCache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
    GVR_VK_CHECK(!err);

    // We can destroy the cache now as we do not need it. The shader modules also
    // can be destroyed after the pipeline is created.
    vkDestroyPipelineCache(m_device, piplineCache, nullptr);

    vkDestroyShaderModule(m_device, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(m_device, shaderStages[1].module, nullptr);

}

void VulkanCore::InitFrameBuffers(){
//The framebuffer objects reference the renderpass, and allow
    // the references defined in that renderpass to now attach to views.
    // The views in this example are the colour view, which is our swapchain image,
    // and the depth buffer created manually earlier.
    VkImageView attachments [2] = {};
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = m_renderPass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = attachments;
    framebufferCreateInfo.width  = m_width;
    framebufferCreateInfo.height = m_height;
    framebufferCreateInfo.layers = 1;

    VkResult ret;

    m_frameBuffers = new VkFramebuffer[m_swapchainImageCount];
    // Reusing the framebufferCreateInfo to create m_swapchainImageCount framebuffers,
    // only the attachments to the relevent image views change each time.
    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
        attachments[0] = m_swapchainBuffers[i].view;
        //framebufferCreateInfo.pAttachments = &m_swapchainBuffers[i].view;
        attachments[1] = m_depthBuffers[i].view;

        LOGE("Vulkan view %d created", i);
        if((m_swapchainBuffers[i].view == VK_NULL_HANDLE) || (m_renderPass == VK_NULL_HANDLE)){
            LOGE("Vulkan image view null");
        }
        else
            LOGE("Vulkan image view not null");
        ret = vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &m_frameBuffers[i]);
        GVR_VK_CHECK(!ret);
    }
}

void VulkanCore::InitSync(){
    VkResult ret = VK_SUCCESS;
    // For synchronization, we have semaphores for rendering and backbuffer signalling.
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;
    ret = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_backBufferSemaphore);
    GVR_VK_CHECK(!ret);

    ret = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderCompleteSemaphore);
    GVR_VK_CHECK(!ret);


    	// Fences (Used to check draw command buffer completion)
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // Create in signaled state so we don't wait on first render of each command buffer
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    waitFences.resize(m_outputBuffers.size());
    for (auto& fence : waitFences)
    {
        GVR_VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
    }

}
void VulkanCore::BuildCmdBuffer()
{
    // For the triangle sample, we pre-record our command buffer, as it is static.
    // We have a buffer per swap chain image, so loop over the creation process.
    for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
        VkCommandBuffer &cmdBuffer = m_swapchainBuffers[i].cmdBuffer;

        // vkBeginCommandBuffer should reset the command buffer, but Reset can be called
        // to make it more explicit.
        VkResult err;
        err = vkResetCommandBuffer(cmdBuffer, 0);
        GVR_VK_CHECK(!err);

        VkCommandBufferInheritanceInfo cmd_buf_hinfo = {};
        cmd_buf_hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        cmd_buf_hinfo.pNext = nullptr;
        cmd_buf_hinfo.renderPass = VK_NULL_HANDLE;
        cmd_buf_hinfo.subpass = 0;
        cmd_buf_hinfo.framebuffer = VK_NULL_HANDLE;
        cmd_buf_hinfo.occlusionQueryEnable = VK_FALSE;
        cmd_buf_hinfo.queryFlags = 0;
        cmd_buf_hinfo.pipelineStatistics = 0;

        VkCommandBufferBeginInfo cmd_buf_info = {};
        cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_buf_info.pNext = nullptr;
        cmd_buf_info.flags = 0;
        cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

        // By calling vkBeginCommandBuffer, cmdBuffer is put into the recording state.
        err = vkBeginCommandBuffer(cmdBuffer, &cmd_buf_info);
        GVR_VK_CHECK(!err);

        // Before we can use the back buffer from the swapchain, we must change the
        // image layout from the PRESENT mode to the COLOR_ATTACHMENT mode.
        // PRESENT mode is optimal for sending to the screen for users to see, so the
        // image will be set back to that mode after we have completed rendering.
        VkImageMemoryBarrier preRenderBarrier = {};
        preRenderBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        preRenderBarrier.pNext = nullptr;
        preRenderBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        preRenderBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        preRenderBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        preRenderBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        preRenderBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preRenderBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preRenderBarrier.image = m_swapchainBuffers[i].image;
        preRenderBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        preRenderBarrier.subresourceRange.baseArrayLayer = 0;
        preRenderBarrier.subresourceRange.baseMipLevel = 1;
        preRenderBarrier.subresourceRange.layerCount = 0;
        preRenderBarrier.subresourceRange.levelCount = 1;

        // Thie PipelineBarrier function can operate on memoryBarriers,
        // bufferMemory and imageMemory buffers. We only provide a single
        // imageMemoryBarrier.
        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &preRenderBarrier);

        // When starting the render pass, we can set clear values.
        VkClearValue clear_values[2] = {};
        clear_values[0].color.float32[0] = 0.3f;
        clear_values[0].color.float32[1] = 0.3f;
        clear_values[0].color.float32[2] = 0.3f;
        clear_values[0].color.float32[3] = 1.0f;
        clear_values[1].depthStencil.depth = 1.0f;
        clear_values[1].depthStencil.stencil = 0;

        VkRenderPassBeginInfo rp_begin = {};
        rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin.pNext = nullptr;
        rp_begin.renderPass = m_renderPass;
        rp_begin.framebuffer = m_frameBuffers[i];
        rp_begin.renderArea.offset.x = 0;
        rp_begin.renderArea.offset.y = 0;
        rp_begin.renderArea.extent.width = m_width;
        rp_begin.renderArea.extent.height = m_height;
        rp_begin.clearValueCount = 2;
        rp_begin.pClearValues = clear_values;

        vkCmdBeginRenderPass(cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

        // Set our pipeline. This holds all major state
        // the pipeline defines, for example, that the vertex buffer is a triangle list.
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        // Bind our vertex buffer, with a 0 offset.
        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &m_vertices.buf, offsets);

        // Issue a draw command, with our 3 vertices.
        vkCmdDraw(cmdBuffer, 3, 1, 0, 0);


        // Copy Image to Buffer
        VkOffset3D off = {};
        off.x = 0;
        off.y = 0;
        off.z = 0;

        VkExtent3D extent3D = {};
        extent3D.width = m_width;
        extent3D.height = m_height;

        VkImageSubresourceLayers subResource = {};
        subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subResource.baseArrayLayer = 0;
        subResource.mipLevel = 0;
        subResource.layerCount = 1;

        VkBufferImageCopy someDetails = {};
        someDetails.bufferOffset = 0;
        someDetails.bufferRowLength = 0;
        someDetails.bufferImageHeight = 0;
        someDetails.imageSubresource = subResource;
        someDetails.imageOffset = off;
        someDetails.imageExtent = extent3D;

        VkBufferImageCopy region = { 0 };
                 region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                 region.imageSubresource.layerCount = 1;
                 region.imageExtent.width = m_width;
                 region.imageExtent.height = m_height;
                 region.imageExtent.depth = 1;


        // Now our render pass has ended.
        vkCmdEndRenderPass(cmdBuffer);
        vkCmdCopyImageToBuffer(cmdBuffer, m_swapchainBuffers[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_outputBuffers[i].imageOutputBuffer, 1,  &region);



        // As stated earlier, now transition the swapchain image to the PRESENT mode.
        VkImageMemoryBarrier prePresentBarrier = {};
        prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        prePresentBarrier.pNext = nullptr;
        prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.image = m_swapchainBuffers[i].image;
        prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        prePresentBarrier.subresourceRange.baseArrayLayer = 0;
        prePresentBarrier.subresourceRange.baseMipLevel = 1;
        prePresentBarrier.subresourceRange.layerCount = 0;

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &prePresentBarrier);

        // By ending the command buffer, it is put out of record mode.
        err = vkEndCommandBuffer(cmdBuffer);
        GVR_VK_CHECK(!err);
    }
}
void VulkanCore::DrawFrame(){

    VkResult err;
    // Get the next image to render to, then queue a wait until the image is ready
    err  = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_backBufferSemaphore, VK_NULL_HANDLE, &m_swapchainCurrentIdx);
    LOGE("Vulkan acquired image %d ", m_swapchainCurrentIdx);
    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        LOGW("VK_ERROR_OUT_OF_DATE_KHR not handled in sample");
    } else if (err == VK_SUBOPTIMAL_KHR) {
        LOGW("VK_SUBOPTIMAL_KHR not handled in sample");
    }
    GVR_VK_CHECK(!err);

	GVR_VK_CHECK(vkWaitForFences(m_device, 1, &waitFences[currentBuffer], VK_TRUE, UINT64_MAX));
    GVR_VK_CHECK(vkResetFences(m_device, 1, &waitFences[currentBuffer]));
    VkFence nullFence = VK_NULL_HANDLE;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_backBufferSemaphore;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_swapchainBuffers[m_swapchainCurrentIdx].cmdBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    err = vkQueueSubmit(m_queue, 1, &submitInfo,  waitFences[m_swapchainCurrentIdx]);
    GVR_VK_CHECK(!err);

    err = vkQueueWaitIdle(m_queue);
    if(err != VK_SUCCESS)
        LOGE("Vulkan vkQueueWaitIdle submit failed");

    LOGE("Vulkan vkQueueWaitIdle submitted");


    uint8_t * data;
    static bool printflag = true;
    if(printflag){
        uint8_t * data;
        err = vkMapMemory(m_device, m_outputBuffers[m_swapchainCurrentIdx].memory, 0, m_outputBuffers[m_swapchainCurrentIdx].size, 0, (void **)&data);
        GVR_VK_CHECK(!err);

        //void* data;

        for(int i = 0; i < (320); i++)
            finaloutput[i] = 0;

        LOGE("Vulkna size of %d", sizeof(finaloutput));
        //while(1) {
        memcpy(finaloutput, data, (m_width*m_height*4* sizeof(uint8_t)));
/*
        LOGI("Vulkan memcpy map done");
        float tt;
        for (int i = 0; i < (m_width*m_height)-4; i++) {
            //tt = (float) data[i];
            LOGI("Vulkan Data %u, %u %u %u", finaloutput[i], finaloutput[i+1], finaloutput[i+2], finaloutput[i+3]);
            i+=3;
        }
*/
        LOGE("Vulkan data reading done");
        vkUnmapMemory(m_device,m_outputBuffers[m_swapchainCurrentIdx].memory);


        printflag = false;
    }

}

void VulkanCore::initVulkanCore(ANativeWindow * newNativeWindow){
    m_Vulkan_Initialised = true;
    m_androidWindow = newNativeWindow;

    if(newNativeWindow == NULL)
        LOGE("Vulkan Bad native window");
    else
        LOGE("Vulkan Good native window");

    LOGE("Vulkan Before init methods");
    if(InitVulkan() != 1){
        m_Vulkan_Initialised = false;
        return;
    }
    LOGE("Vulkan after init methods");

    if(CreateInstance() == false){
        m_Vulkan_Initialised = false;
        return;
    }

    if(GetPhysicalDevices() == false){
        m_Vulkan_Initialised = false;
        return;
    }

    if(InitDevice() == false){
        m_Vulkan_Initialised = false;
        return;
    }

    InitSwapchain();
    InitCommandbuffers();
    InitVertexBuffers();
    InitLayouts();
    InitRenderPass();
    InitPipeline();
    InitFrameBuffers();
    InitSync();

    // Initialize our command buffers
    BuildCmdBuffer();




}
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
#include "vulkanInfoWrapper.h"
#include "util/gvr_log.h"
#include <assert.h>
#include <cstring>
#include <iostream>
#include <vector>
#include "objects/components/camera.h"
#include "objects/components/render_data.h"
#include "objects/scene.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <math.h>
#include <thread>

#include <shaderc/shaderc.hpp>
#define UINT64_MAX 99999
namespace gvr {

VulkanCore* VulkanCore::theInstance = NULL;
uint8_t *oculusTexData;
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

    // Getting CPU Thread Count
    /*
    m_threadCount = 0;
    m_threadCount = std::thread::hardware_concurrency();
    LOGE("Vulkan got threads from OS %d", m_threadCount);
    m_threadPool.setWorkerThreadCount(m_threadCount);*/

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

    //InitSurface();

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
VkResult  err;
bool pass;
VkMemoryRequirements mem_reqs;
VkMemoryAllocateInfo memoryAllocateInfo = {};

    VkResult ret = VK_SUCCESS;
       m_width = width;// 320;//surfaceCapabilities.currentExtent.width;
       m_height = height;//240;//surfaceCapabilities.currentExtent.height;

   VkImageCreateInfo imageCreateInfo = {};
       imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
       imageCreateInfo.pNext = nullptr;
       imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
       imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;//VK_FORMAT_R8G8B8A8_UNORM;//m_surfaceFormat.format;//VK_FORMAT_R32G32B32A32_SFLOAT;
       imageCreateInfo.extent = {m_width, m_height, 1};
       imageCreateInfo .mipLevels = 1;
       imageCreateInfo .arrayLayers = 1;
       imageCreateInfo .samples = VK_SAMPLE_COUNT_1_BIT;
       imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
       imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT  | VK_IMAGE_USAGE_TRANSFER_SRC_BIT ;
       imageCreateInfo .flags = 0;
       imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

       //LOGI("Vulkan Format %d ", m_surfaceFormat.format);
       // Create the image with details as imageCreateInfo
       m_swapchainImageCount = 3;
       m_swapchainBuffers = new GVR_VK_SwapchainBuffer[m_swapchainImageCount];
       outputImage = new GVR_VK_SwapchainBuffer[m_swapchainImageCount];
       GVR_VK_CHECK(m_swapchainBuffers);

       for(int i = 0; i < m_swapchainImageCount; i++) {


           bool  pass;

           //ret = vkCreateImage(m_device, &imageCreateInfo, nullptr, &m_swapchainBuffers[i].image);
           ret = vkCreateImage(
                                m_device,
                                gvr::ImageCreateInfo( VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, m_width, m_height, 1, 1, 1,
                                                      VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT  | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED ),
                                nullptr, &m_swapchainBuffers[i].image
                               );
           GVR_VK_CHECK(!ret);

           err = vkCreateBuffer(m_device, gvr::BufferCreateInfo(m_width*m_height*4* sizeof(uint8_t), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT  | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), nullptr, &m_swapchainBuffers[i].buf);
           GVR_VK_CHECK(!err);



           // discover what memory requirements are for this image.
           vkGetImageMemoryRequirements(m_device, m_swapchainBuffers[i].image, &mem_reqs);
           //LOGD("Vulkan image memreq %", mem_reqs.size);
           m_swapchainBuffers[i].size = mem_reqs.size;


           // Allocate memory according to requirements

           memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
           memoryAllocateInfo.pNext = nullptr;
           memoryAllocateInfo.allocationSize = 0;
           memoryAllocateInfo.memoryTypeIndex = 0;
           memoryAllocateInfo.allocationSize = mem_reqs.size;
           pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);
           GVR_VK_CHECK(pass);

           err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_swapchainBuffers[i].mem);
           GVR_VK_CHECK(!err);

           // Bind memory to the image
           err = vkBindImageMemory(m_device, m_swapchainBuffers[i].image, m_swapchainBuffers[i].mem, 0);
           GVR_VK_CHECK(!err);

            err = vkBindBufferMemory(m_device, m_swapchainBuffers[i].buf, m_swapchainBuffers[i].mem, 0);
                       GVR_VK_CHECK(!err);

           VkImageViewCreateInfo imageViewCreateInfo = {};
           imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
           imageViewCreateInfo.pNext = nullptr;
           imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;//VK_FORMAT_R8G8B8A8_UNORM;//m_surfaceFormat.format;
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
           imageViewCreateInfo.image = m_swapchainBuffers[i].image;

           //err = vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_swapchainBuffers[i].view);
           err = vkCreateImageView(
                                    m_device,
                                    gvr::ImageViewCreateInfo(m_swapchainBuffers[i].image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT),
                                    nullptr, &m_swapchainBuffers[i].view
                                   );

           GVR_VK_CHECK(!err);



            err = vkCreateBuffer(m_device, gvr::BufferCreateInfo(m_width*m_height*4* sizeof(uint8_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT), nullptr, &outputImage[i].buf);
                    GVR_VK_CHECK(!err);

                    // Obtain the memory requirements for this buffer.
                    vkGetBufferMemoryRequirements(m_device, outputImage[i].buf, &mem_reqs);
                    GVR_VK_CHECK(!err);

                    // And allocate memory according to those requirements.
                    memoryAllocateInfo = {};
                    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    memoryAllocateInfo.pNext = nullptr;
                    memoryAllocateInfo.allocationSize = 0;
                    memoryAllocateInfo.memoryTypeIndex = 0;
                    memoryAllocateInfo.allocationSize  = mem_reqs.size;
                    pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
                    GVR_VK_CHECK(pass);

                    outputImage[i].size = mem_reqs.size;
                    err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &outputImage[i].mem);
                    GVR_VK_CHECK(!err);

                    err = vkBindBufferMemory(m_device, outputImage[i].buf, outputImage[i].mem, 0);
                    GVR_VK_CHECK(!err);
       }






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
           //err = vkCreateImage(m_device, &imageCreateInfo, nullptr, &m_depthBuffers[i].image);
           err = vkCreateImage(
                                m_device,
                                gvr::ImageCreateInfo( VK_IMAGE_TYPE_2D, VK_FORMAT_D16_UNORM, m_width, m_height, 1, 1, 1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED ),
                                nullptr, &m_depthBuffers[i].image
                              );
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
           //err = vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_depthBuffers[i].view);
           err = vkCreateImageView(
                                    m_device,
                                    gvr::ImageViewCreateInfo(m_depthBuffers[i].image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_D16_UNORM, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT),
                                    nullptr, &m_depthBuffers[i].view
                                  );
           GVR_VK_CHECK(!err);
       }

       // Allocating memory for output image
        oculusTexData = (uint8_t*)malloc(m_width*m_height*4* sizeof(uint8_t));
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

void VulkanCore::InitTransientCmdPool(){
    VkResult ret = VK_SUCCESS;

    ret = vkCreateCommandPool(
                                m_device,
                                gvr::CmdPoolCreateInfo(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, m_queueFamilyIndex),
                                nullptr, &m_commandPoolTrans
                             );

    GVR_VK_CHECK(!ret);
}

VkCommandBuffer VulkanCore::GetTransientCmdBuffer(){
    VkResult ret = VK_SUCCESS;
    VkCommandBuffer cmdBuff;
    ret = vkAllocateCommandBuffers(
                                        m_device,
                                        gvr::CmdBufferCreateInfo(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPoolTrans),
                                        &cmdBuff
                                      );
    GVR_VK_CHECK(!ret);
    return cmdBuff;
}

void VulkanCore::InitCommandbuffers(){
    VkResult ret = VK_SUCCESS;
    // Command buffers are allocated from a pool; we define that pool here and create it.
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.queueFamilyIndex = m_queueFamilyIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    //ret = vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool);
    ret = vkCreateCommandPool(
                                m_device,
                                gvr::CmdPoolCreateInfo(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, m_queueFamilyIndex),
                                nullptr, &m_commandPool
                             );

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
        //ret = vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_swapchainBuffers[i].cmdBuffer);
        ret = vkAllocateCommandBuffers(
                                        m_device,
                                        gvr::CmdBufferCreateInfo(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool),
                                        &m_swapchainBuffers[i].cmdBuffer
                                      );


        GVR_VK_CHECK(!ret);
    }
}

void VulkanCore::InitVertexBuffersFromRenderData(const std::vector<glm::vec3>& vertices, GVR_VK_Vertices &m_vertices, GVR_VK_Indices &m_indices, const std::vector<unsigned short> & indices){

    VkResult   err;
    bool   pass;

    // Our m_vertices member contains the types required for storing
    // and defining our vertex buffer within the graphics pipeline.
    memset(&m_vertices, 0, sizeof(m_vertices));

    // Create our buffer object.
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.size = vertices.size()*sizeof(glm::vec3);//sizeof(vb);//
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCreateInfo.flags = 0;
    //err = vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &m_vertices.buf);
    err = vkCreateBuffer(m_device, gvr::BufferCreateInfo(vertices.size()*sizeof(glm::vec3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), nullptr, &m_vertices.buf);
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

    VkDeviceMemory mem_staging_vert;
    VkBuffer buf_staging_vert;
    err = vkCreateBuffer(m_device, gvr::BufferCreateInfo(vertices.size()*sizeof(glm::vec3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), nullptr, &buf_staging_vert);
    GVR_VK_CHECK(!err);

    //err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_vertices.mem);
    err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &mem_staging_vert);
    GVR_VK_CHECK(!err);

    // Now we need to map the memory of this new allocation so the CPU can edit it.
    void *data;
    //err = vkMapMemory(m_device, m_vertices.mem, 0, memoryAllocateInfo.allocationSize, 0, &data);
    err = vkMapMemory(m_device, mem_staging_vert, 0, memoryAllocateInfo.allocationSize, 0, &data);
    GVR_VK_CHECK(!err);

    // Copy our triangle verticies and colors into the mapped memory area.
    //memcpy(data, vb, sizeof(vb));
    memcpy(data, vertices.data(), vertices.size()*sizeof(glm::vec3));


    // Unmap the memory back from the CPU.
    vkUnmapMemory(m_device, mem_staging_vert);
    //vkUnmapMemory(m_device, m_vertices.mem);
    err = vkBindBufferMemory(m_device, buf_staging_vert, mem_staging_vert, 0);
    GVR_VK_CHECK(!err);

    // Create Device memory optimal
    pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);
    GVR_VK_CHECK(pass);
    err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_vertices.mem);
    GVR_VK_CHECK(!err);
    // Bind our buffer to the memory.
    err = vkBindBufferMemory(m_device, m_vertices.buf, m_vertices.mem, 0);
    GVR_VK_CHECK(!err);

    VkCommandBuffer trnCmdBuf = GetTransientCmdBuffer();
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(trnCmdBuf, &beginInfo);
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = bufferCreateInfo.size;
    vkCmdCopyBuffer(trnCmdBuf, buf_staging_vert, m_vertices.buf, 1, &copyRegion);
    vkEndCommandBuffer(trnCmdBuf);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &trnCmdBuf;

    vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue);
    vkFreeCommandBuffers(m_device, m_commandPoolTrans, 1, &trnCmdBuf);


    // The vertices need to be defined so that the pipeline understands how the
    // data is laid out. This is done by providing a VkPipelineVertexInputStateCreateInfo
    // structure with the correct information.
    m_vertices.vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_vertices.vi.pNext = nullptr;
    m_vertices.vi.vertexBindingDescriptionCount = 1;
    m_vertices.vi.pVertexBindingDescriptions = m_vertices.vi_bindings;
    m_vertices.vi.vertexAttributeDescriptionCount = 1;
    m_vertices.vi.pVertexAttributeDescriptions = m_vertices.vi_attrs;

    // We bind the buffer as a whole, using the correct buffer ID.
    // This defines the stride for each element of the vertex array.
    m_vertices.vi_bindings[0].binding = GVR_VK_VERTEX_BUFFER_BIND_ID;
    m_vertices.vi_bindings[0].stride = sizeof(glm::vec3);//sizeof(vb[0]);//
    m_vertices.vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Within each element, we define the attributes. At location 0,
    // the vertex positions, in float3 format, with offset 0 as they are
    // first in the array structure.
    m_vertices.vi_attrs[0].binding = GVR_VK_VERTEX_BUFFER_BIND_ID;
    m_vertices.vi_attrs[0].location = 0;
    m_vertices.vi_attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT; //float3
    m_vertices.vi_attrs[0].offset = 0;

    /*// The second location is the vertex colors, in RGBA float4 format.
    // These appear in each element in memory after the float3 vertex
    // positions, so the offset is set accordingly.
    m_vertices.vi_attrs[1].binding = GVR_VK_VERTEX_BUFFER_BIND_ID;
    m_vertices.vi_attrs[1].location = 1;
    m_vertices.vi_attrs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; //float4
    m_vertices.vi_attrs[1].offset = sizeof(float) * 3;*/

    // Index buffer
       // std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
        m_indices.count = static_cast<uint32_t>(indices.size());
        //m_indices.count = static_cast<uint32_t>(indexBuffer.size());
        uint32_t indexBufferSize = m_indices.count *  sizeof(unsigned short);//sizeof(uint32_t);//*

            VkBufferCreateInfo indexbufferInfo = {};
            indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            indexbufferInfo.size = indexBufferSize;
            indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

            // Copy index data to a buffer visible to the host
            //err = vkCreateBuffer(m_device, &indexbufferInfo, nullptr, &m_indices.buffer);
            err = vkCreateBuffer(m_device, gvr::BufferCreateInfo(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT), nullptr, &m_indices.buffer);
            GVR_VK_CHECK(!err);

            VkDeviceMemory mem_staging_indi;
            VkBuffer buf_staging_indi;
            err = vkCreateBuffer(m_device, gvr::BufferCreateInfo(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT), nullptr, &buf_staging_indi);
            GVR_VK_CHECK(!err);


            vkGetBufferMemoryRequirements(m_device, m_indices.buffer, &mem_reqs);
            memoryAllocateInfo.allocationSize = mem_reqs.size;
            pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
            GVR_VK_CHECK(pass);

            err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &mem_staging_indi);
            GVR_VK_CHECK(!err);
            err = vkMapMemory(m_device, mem_staging_indi, 0, indexBufferSize, 0, &data);
            GVR_VK_CHECK(!err);
            //memcpy(data, indexBuffer.data(), indexBufferSize);
            memcpy(data, indices.data(), indexBufferSize);
            vkUnmapMemory(m_device, mem_staging_indi);

            //err = vkBindBufferMemory(m_device, m_indices.buffer, m_indices.memory, 0);
            err = vkBindBufferMemory(m_device, buf_staging_indi, mem_staging_indi, 0);
            GVR_VK_CHECK(!err);

        // Create Device memory optimal
            pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);
            GVR_VK_CHECK(pass);
            err = vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_indices.memory);
            GVR_VK_CHECK(!err);

    // Bind our buffer to the memory.
        err = vkBindBufferMemory(m_device, m_indices.buffer, m_indices.memory, 0);
        GVR_VK_CHECK(!err);

        trnCmdBuf = GetTransientCmdBuffer();
        beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(trnCmdBuf, &beginInfo);
        copyRegion = {};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = indexBufferSize;
        vkCmdCopyBuffer(trnCmdBuf, buf_staging_indi, m_indices.buffer, 1, &copyRegion);
        vkEndCommandBuffer(trnCmdBuf);

        submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &trnCmdBuf;

        vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_queue);
        vkFreeCommandBuffers(m_device, m_commandPoolTrans, 1, &trnCmdBuf);
}

void VulkanCore::InitLayouts(){
    VkResult ret = VK_SUCCESS;
    // This sample has two  bindings, a sampler in the fragment shader and a uniform in the
         // vertex shader for MVP matrix.
         VkDescriptorSetLayoutBinding uniformAndSamplerBinding[2] = {};
         // Our MVP matrix
         uniformAndSamplerBinding[0].binding = 0;
         uniformAndSamplerBinding[0].descriptorCount = 1;
         uniformAndSamplerBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
         uniformAndSamplerBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
         uniformAndSamplerBinding[0].pImmutableSamplers = nullptr;
         // Our Lights
         uniformAndSamplerBinding[1].binding = 1;
         uniformAndSamplerBinding[1].descriptorCount = 1;
         uniformAndSamplerBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
         uniformAndSamplerBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
         uniformAndSamplerBinding[1].pImmutableSamplers = nullptr;

         VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
         descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
         descriptorSetLayoutCreateInfo.pNext = nullptr;
         descriptorSetLayoutCreateInfo.bindingCount = 2;
         descriptorSetLayoutCreateInfo.pBindings = &uniformAndSamplerBinding[0];

         ret = vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorLayout);
         GVR_VK_CHECK(!ret);

         VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
         pipelineLayoutCreateInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
         pipelineLayoutCreateInfo.pNext              = nullptr;
         pipelineLayoutCreateInfo.setLayoutCount     = 1;
         pipelineLayoutCreateInfo.pSetLayouts        = &m_descriptorLayout;
         ret = vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
         GVR_VK_CHECK(!ret);
}

void VulkanCore::InitUniformBuffers(){
    // the uniform in this example is a matrix in the vertex stage
    memset(&m_modelViewMatrixUniform, 0, sizeof(m_modelViewMatrixUniform));

    VkResult err = VK_SUCCESS;

    // Create our buffer object
    VkBufferCreateInfo bufferCreateInfo;
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = NULL;
    bufferCreateInfo.size = sizeof(glm::mat4);//sizeof(float)*4;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferCreateInfo.flags = 0;

    //err = vkCreateBuffer(m_device, &bufferCreateInfo, NULL, &m_modelViewMatrixUniform.buf);
    err = vkCreateBuffer(m_device, gvr::BufferCreateInfo(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), NULL, &m_modelViewMatrixUniform.buf);
    assert(!err);

    // Obtain the requirements on memory for this buffer
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_device, m_modelViewMatrixUniform.buf, &mem_reqs);
    assert(!err);

    // And allocate memory according to those requirements
    VkMemoryAllocateInfo memoryAllocateInfo;
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = NULL;
    memoryAllocateInfo.allocationSize = 0;
    memoryAllocateInfo.memoryTypeIndex = 0;
    memoryAllocateInfo.allocationSize  = mem_reqs.size;
    bool pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
    assert(pass);

    // We keep the size of the allocation for remapping it later when we update contents
    m_modelViewMatrixUniform.allocSize = memoryAllocateInfo.allocationSize;

    err = vkAllocateMemory(m_device, &memoryAllocateInfo, NULL, &m_modelViewMatrixUniform.mem);
    assert(!err);

    // Create our initial MVP matrix
    float aaa[16] = {1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1};
    glm::mat4 mvp = glm::make_mat4(aaa);;

    // Now we need to map the memory of this new allocation so the CPU can edit it.
    void *data;
    err = vkMapMemory(m_device, m_modelViewMatrixUniform.mem, 0, m_modelViewMatrixUniform.allocSize, 0, &data);
    assert(!err);

    float tempColor[4] = {0,1,0,1};
    // Copy our triangle verticies and colors into the mapped memory area
    //memcpy(data, tempColor, sizeof(float)*4);
    memcpy(data, &mvp,sizeof(mvp));

    // Unmap the memory back from the CPU
    vkUnmapMemory(m_device, m_modelViewMatrixUniform.mem);

    // Bind our buffer to the memory
    err = vkBindBufferMemory(m_device, m_modelViewMatrixUniform.buf, m_modelViewMatrixUniform.mem, 0);
    assert(!err);

    m_modelViewMatrixUniform.bufferInfo.buffer = m_modelViewMatrixUniform.buf;
    m_modelViewMatrixUniform.bufferInfo.offset = 0;
    m_modelViewMatrixUniform.bufferInfo.range = sizeof(glm::mat4);
}


void VulkanCore::InitUniformBuffersForRenderData(Uniform &m_modelViewMatrixUniform){
    // the uniform in this example is a matrix in the vertex stage
    memset(&m_modelViewMatrixUniform, 0, sizeof(m_modelViewMatrixUniform));

    VkResult err = VK_SUCCESS;

    // Create our buffer object
    VkBufferCreateInfo bufferCreateInfo;
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = NULL;
    bufferCreateInfo.size = sizeof(glm::mat4);;//sizeof(float)*4;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferCreateInfo.flags = 0;

    err = vkCreateBuffer(m_device, gvr::BufferCreateInfo(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), NULL, &m_modelViewMatrixUniform.buf);
    assert(!err);

    // Obtain the requirements on memory for this buffer
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_device, m_modelViewMatrixUniform.buf, &mem_reqs);
    assert(!err);

    // And allocate memory according to those requirements
    VkMemoryAllocateInfo memoryAllocateInfo;
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = NULL;
    memoryAllocateInfo.allocationSize = 0;
    memoryAllocateInfo.memoryTypeIndex = 0;
    memoryAllocateInfo.allocationSize  = mem_reqs.size;
    bool pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
    assert(pass);

    // We keep the size of the allocation for remapping it later when we update contents
    m_modelViewMatrixUniform.allocSize = memoryAllocateInfo.allocationSize;

    err = vkAllocateMemory(m_device, &memoryAllocateInfo, NULL, &m_modelViewMatrixUniform.mem);
    assert(!err);

    // Create our initial MVP matrix
    float aaa[16] = {1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1};
    glm::mat4 mvp = glm::make_mat4(aaa);;

    // Now we need to map the memory of this new allocation so the CPU can edit it.
    void *data;
    err = vkMapMemory(m_device, m_modelViewMatrixUniform.mem, 0, m_modelViewMatrixUniform.allocSize, 0, &data);
    assert(!err);

    float tempColor[4] = {0,1,0,1};
    // Copy our triangle verticies and colors into the mapped memory area
    //memcpy(data, tempColor, sizeof(float)*4);
    memcpy(data, &mvp,sizeof(mvp));

    // Unmap the memory back from the CPU
    vkUnmapMemory(m_device, m_modelViewMatrixUniform.mem);

    // Bind our buffer to the memory
    err = vkBindBufferMemory(m_device, m_modelViewMatrixUniform.buf, m_modelViewMatrixUniform.mem, 0);
    assert(!err);

    m_modelViewMatrixUniform.bufferInfo.buffer = m_modelViewMatrixUniform.buf;
    m_modelViewMatrixUniform.bufferInfo.offset = 0;
    m_modelViewMatrixUniform.bufferInfo.range = sizeof(glm::mat4);
}


void VulkanCore::InitUniformBuffersForRenderDataLights(Uniform &m_modelViewMatrixUniform){
    // the uniform in this example is a matrix in the vertex stage
    memset(&m_modelViewMatrixUniform, 0, sizeof(m_modelViewMatrixUniform));

    VkResult err = VK_SUCCESS;

    err = vkCreateBuffer(m_device, gvr::BufferCreateInfo(sizeof(float)*17, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), NULL, &m_modelViewMatrixUniform.buf);
    assert(!err);

    // Obtain the requirements on memory for this buffer
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_device, m_modelViewMatrixUniform.buf, &mem_reqs);
    assert(!err);

    // And allocate memory according to those requirements
    VkMemoryAllocateInfo memoryAllocateInfo;
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = NULL;
    memoryAllocateInfo.allocationSize = 0;
    memoryAllocateInfo.memoryTypeIndex = 0;
    memoryAllocateInfo.allocationSize  = mem_reqs.size;
    bool pass = GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
    assert(pass);

    // We keep the size of the allocation for remapping it later when we update contents
    m_modelViewMatrixUniform.allocSize = memoryAllocateInfo.allocationSize;

    err = vkAllocateMemory(m_device, &memoryAllocateInfo, NULL, &m_modelViewMatrixUniform.mem);
    assert(!err);

    // Create our initial MVP matrix
    float aaa[17] = {1,1,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1,1};

    // Now we need to map the memory of this new allocation so the CPU can edit it.
    void *data;
    err = vkMapMemory(m_device, m_modelViewMatrixUniform.mem, 0, m_modelViewMatrixUniform.allocSize, 0, &data);
    assert(!err);

    memcpy(data, aaa, sizeof(float)*17);

    // Unmap the memory back from the CPU
    vkUnmapMemory(m_device, m_modelViewMatrixUniform.mem);

    // Bind our buffer to the memory
    err = vkBindBufferMemory(m_device, m_modelViewMatrixUniform.buf, m_modelViewMatrixUniform.mem, 0);
    assert(!err);

    m_modelViewMatrixUniform.bufferInfo.buffer = m_modelViewMatrixUniform.buf;
    m_modelViewMatrixUniform.bufferInfo.offset = 0;
    m_modelViewMatrixUniform.bufferInfo.range = sizeof(float)*17;
}

void VulkanCore::InitRenderPass(){
// The renderpass defines the attachments to the framebuffer object that gets
    // used in the pipeline. We have two attachments, the colour buffer, and the
    // depth buffer. The operations and layouts are set to defaults for this type
    // of attachment.
    VkAttachmentDescription attachmentDescriptions[2] = {};
    attachmentDescriptions[0].flags = 0;
    attachmentDescriptions[0].format = VK_FORMAT_R8G8B8A8_UNORM;//.format;
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

VkShaderModule VulkanCore::CreateShaderModule(std::vector<uint32_t> code, uint32_t size)
{
    VkShaderModule module;
    VkResult  err;

    // Creating a shader is very simple once it's in memory as compiled SPIR-V.
    VkShaderModuleCreateInfo moduleCreateInfo = {};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.codeSize = size * sizeof(unsigned int);
    moduleCreateInfo.pCode = code.data();
    moduleCreateInfo.flags = 0;
    err = vkCreateShaderModule(m_device, gvr::ShaderModuleCreateInfo(code.data(), size * sizeof(unsigned int)), nullptr, &module);
    GVR_VK_CHECK(!err);

    return module;
}


VkShaderModule VulkanCore::CreateShaderModuleAscii(const uint32_t* code, uint32_t size)
{
    VkShaderModule module;
    VkResult  err;

    // Creating a shader is very simple once it's in memory as compiled SPIR-V.
    VkShaderModuleCreateInfo moduleCreateInfo = {};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = code;
    moduleCreateInfo.flags = 0;
    err = vkCreateShaderModule(m_device, gvr::ShaderModuleCreateInfo(code, size), nullptr, &module);
    GVR_VK_CHECK(!err);

    return module;
}

void VulkanCore::InitPipelineForRenderData(GVR_VK_Vertices &m_vertices, VkPipeline &m_pipeline){

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

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.AddMacroDefinition("MY_DEFINE", "1");

    std::string vertexShaderData = std::string("") +
    "#version 400 \n"+
    "#extension GL_ARB_separate_shader_objects : enable \n"+
    "#extension GL_ARB_shading_language_420pack : enable \n"+
    "layout (std140, set = 0, binding = 0) uniform matrix { mat4 mvp; } matrices;\n"+
    "layout (location = 0) in vec3 pos; \n"+
    "void main() { \n"+
    "  gl_Position = matrices.mvp * vec4(pos.x, pos.y, pos.z,1.0); \n"+
    "}";

    shaderc_shader_kind kind = shaderc_glsl_default_vertex_shader;
    shaderc::SpvCompilationResult module_vert = compiler.CompileGlslToSpv(vertexShaderData.c_str(), vertexShaderData.size(), kind, "VulkanShader", options);
    if (module_vert.GetCompilationStatus() != shaderc_compilation_status_success) {
    //std::cerr << module.GetErrorMessage();
    LOGI("Vulkan shader unable to compile shader %s", module_vert.GetErrorMessage().c_str());
    }
    else
    LOGE("Vulkan shader compiled shader");

    std::vector<uint32_t> result_vert(module_vert.cbegin(), module_vert.cend());


    std::string data_frag = std::string("") +
    "#version 400 \n"+
    "#extension GL_ARB_separate_shader_objects : enable \n"+
    "#extension GL_ARB_shading_language_420pack : enable \n"+
    "layout (std140, set = 0, binding = 1) uniform lightEffects {\n"+
    "vec4 ambient_color;\n"+
    "vec4 diffuse_color;\n"+
    "vec4 specular_color;\n"+
    "vec4 emissive_color;\n"+
    "float specular_exponent;\n"+
    "} lightEffectsObj;"+
    "layout (location = 0) out vec4 uFragColor;  \n"+
    "void main() {  \n"+
    " vec4 temp = vec4(1.0,0.0,1.0,1.0);\n"+
    "   uFragColor = lightEffectsObj.ambient_color;  \n"+
    "}";

    shaderc_shader_kind  kind_frag = shaderc_glsl_default_fragment_shader;
    shaderc::SpvCompilationResult module_frag = compiler.CompileGlslToSpv(data_frag.c_str(), data_frag.size(), kind_frag, "VulkanShaderFrag", options);
    if (module_frag.GetCompilationStatus() != shaderc_compilation_status_success) {
    //std::cerr << module.GetErrorMessage();
    LOGI("Vulkan shader unable to compile shader %s", module_frag.GetErrorMessage().c_str());
    }
    else
        LOGE("Vulkan shader fragcompiled shader");

    std::vector<uint32_t> result_frag(module_frag.cbegin(), module_frag.cend());

    // We define two shader stages: our vertex and fragment shader.
        // they are embedded as SPIR-V into a header file for ease of deployment.
        VkPipelineShaderStageCreateInfo shaderStages[2] = {};
        shaderStages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = CreateShaderModule( result_vert, result_vert.size());//CreateShaderModuleAscii( (const uint32_t*)&shader_tri_vert[0], shader_tri_vert_size);//
        shaderStages[0].pName  = "main";
        shaderStages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = CreateShaderModule( result_frag, result_frag.size());//CreateShaderModuleAscii( (const uint32_t*)&shader_tri_frag[0], shader_tri_frag_size);//
        shaderStages[1].pName  = "main";


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
        pipelineCreateInfo.pDepthStencilState  = &ds;
        pipelineCreateInfo.pStages             = &shaderStages[0];
        pipelineCreateInfo.renderPass          = m_renderPass;
        pipelineCreateInfo.pDynamicState       = nullptr;
        pipelineCreateInfo.stageCount          = 2; //vertex and fragment

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
        LOGI("Vulkan graphics call before");
        err = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
        GVR_VK_CHECK(!err);
        LOGI("Vulkan graphics call aftere");

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
    LOGI("Vulkan initsync start");
    VkResult ret = VK_SUCCESS;
    // For synchronization, we have semaphores for rendering and backbuffer signalling.
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;
    ret = vkCreateSemaphore(m_device, gvr::SemaphoreCreateInfo(), nullptr, &m_backBufferSemaphore);
    GVR_VK_CHECK(!ret);

    ret = vkCreateSemaphore(m_device, gvr::SemaphoreCreateInfo(), nullptr, &m_renderCompleteSemaphore);
    GVR_VK_CHECK(!ret);

    // Fences (Used to check draw command buffer completion)
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;

    waitFences.resize(m_swapchainImageCount);
    for (auto& fence : waitFences)
    {
        ret = vkCreateFence(m_device, gvr::FenceCreateInfo(), nullptr, &fence);
        GVR_VK_CHECK(!ret);
    }

    LOGI("Vulkan initsync end");
}

void VulkanCore::BuildSecondaryCmdBuffer(VkCommandBuffer secondaryCmdBuff, VkCommandBufferBeginInfo secondaryBeginInfo, RenderData* render_data_vector, VkDescriptorSet allDescriptors){

        vkBeginCommandBuffer(secondaryCmdBuff, &secondaryBeginInfo);
      vkCmdBindPipeline(secondaryCmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, render_data_vector->m_pipeline);

        //bind out descriptor set, which handles our uniforms and samplers
        vkCmdBindDescriptorSets(secondaryCmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &allDescriptors, 0, NULL);

        // Bind our vertex buffer, with a 0 offset.
        VkDeviceSize offsets[1] = {0};
        GVR_VK_Vertices vert = render_data_vector->m_vertices;
   //     if(&(vert.buf) != NULL)
   //         LOGE("buf is not nuull");
        vkCmdBindVertexBuffers(secondaryCmdBuff, VERTEX_BUFFER_BIND_ID, 1, &(vert.buf), offsets);

        // Bind triangle index buffer
                vkCmdBindIndexBuffer(secondaryCmdBuff, (render_data_vector->m_indices).buffer, 0, VK_INDEX_TYPE_UINT16);
                // Issue a draw command, with our 3 vertices.
                //vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
                //vkCmdDrawIndexed(cmdBuffer, (render_data_vector[j]->m_indices).count, 1, 0, 0, 1);
       //         LOGI("Vulkan number of indeices %d", (render_data_vector->m_indices).count);
                vkCmdDrawIndexed(secondaryCmdBuff, (render_data_vector->m_indices).count, 1, 0, 0, 1);

                vkEndCommandBuffer(secondaryCmdBuff);
}

void VulkanCore::BuildCmdBufferForRenderData(std::vector <VkDescriptorSet> &allDescriptors, int &swapChainIndex, std::vector<RenderData*>& render_data_vector)
{
 //VkPipeline &m_pipeline, GVR_VK_Vertices &m_vertices, GVR_VK_Indices &m_indices)

    // For the triangle sample, we pre-record our command buffer, as it is static.
    // We have a buffer per swap chain image, so loop over the creation process.
    //for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
    int i = swapChainIndex;
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
            preRenderBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            preRenderBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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
/*

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.pNext = nullptr;
            commandBufferAllocateInfo.commandPool = m_commandPool;
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            commandBufferAllocateInfo.commandBufferCount = 1;

            // Create render command buffers, one per swapchain image
            std::vector<VkCommandBuffer> secondaryCmdBuff(allDescriptors.size());

            for (int k = 0; k < secondaryCmdBuff.size(); k++){
                //ret = vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_swapchainBuffers[i].cmdBuffer);
                err = vkAllocateCommandBuffers(
                                                m_device,
                                                gvr::CmdBufferCreateInfo(VK_COMMAND_BUFFER_LEVEL_SECONDARY, m_commandPool),
                                                &secondaryCmdBuff[k]
                                              );
                GVR_VK_CHECK(!err);
            }

        VkCommandBufferBeginInfo secondaryBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        		VkCommandBufferInheritanceInfo inheritance = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
        		secondaryBeginInfo.flags =
        		    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        		secondaryBeginInfo.pInheritanceInfo = &inheritance;

        		// The secondary buffer doesn't really know about the primary command buffer yet, so give it some essential knowledge.
        		inheritance.renderPass = m_renderPass;
        		inheritance.framebuffer = m_frameBuffers[i];
        		inheritance.subpass = 0;
*/

        //if(m_threadCount){
        if(0){
        /*     //LOGI("Vulkan THreading start second cmd buff");
            int roundRobin = 0;
            for(int j = 0; j < allDescriptors.size(); j++){
                            m_threadPool.pushWorkToThread(roundRobin, [=]
                            {BuildSecondaryCmdBuffer(secondaryCmdBuff[j], secondaryBeginInfo, render_data_vector[j], allDescriptors[j]);});
                            roundRobin = (roundRobin + 1) % m_threadCount;
            }

            m_threadPool.waitIdle();
            vkCmdExecuteCommands(cmdBuffer, secondaryCmdBuff.size(), secondaryCmdBuff.data());
            vkFreeCommandBuffers(m_device, m_commandPool, secondaryCmdBuff.size(), secondaryCmdBuff.data());*/
        }
        else{
        for(int j = 0; j < allDescriptors.size(); j++){

        // Set our pipeline. This holds all major state
                // the pipeline defines, for example, that the vertex buffer is a triangle list.
                vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_data_vector[j]->m_pipeline);

        //bind out descriptor set, which handles our uniforms and samplers
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &allDescriptors[j], 0, NULL);

        // Bind our vertex buffer, with a 0 offset.
        VkDeviceSize offsets[1] = {0};
        GVR_VK_Vertices vert = render_data_vector[j]->m_vertices;
   //     if(&(vert.buf) != NULL)
   //         LOGE("buf is not nuull");
        vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &(vert.buf), offsets);

        // Bind triangle index buffer
                vkCmdBindIndexBuffer(cmdBuffer, (render_data_vector[j]->m_indices).buffer, 0, VK_INDEX_TYPE_UINT16);
                // Issue a draw command, with our 3 vertices.
                //vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
                //vkCmdDrawIndexed(cmdBuffer, (render_data_vector[j]->m_indices).count, 1, 0, 0, 1);
       //         LOGI("Vulkan number of indeices %d", (render_data_vector[j]->m_indices).count);
                vkCmdDrawIndexed(cmdBuffer, (render_data_vector[j]->m_indices).count, 1, 0, 0, 1);
         }
         }
        // Now our render pass has ended.
/*
        VkBufferCopy copyRegion = {};
            copyRegion.srcOffset = 0; // Optional
            copyRegion.dstOffset = 0; // Optional
            copyRegion.size = outputImage.size;
            vkCmdCopyBuffer(cmdBuffer, m_swapchainBuffers[m_swapchainCurrentIdx].buf, outputImage.buf, 1, &copyRegion);*/
  /*      VkExtent3D extent3D = {};
                      extent3D.width = m_width;
                      extent3D.height = m_height;
        VkBufferImageCopy region = { 0 };
                        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        region.imageSubresource.layerCount = 1;
                         region.imageExtent = extent3D;


        vkCmdCopyImageToBuffer(cmdBuffer, m_swapchainBuffers[m_swapchainCurrentIdx].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, outputImage.buf, 1,  &region);
*/



        vkCmdEndRenderPass(cmdBuffer);

            // As stated earlier, now transition the swapchain image to the PRESENT mode.
            VkImageMemoryBarrier prePresentBarrier = {};
            prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            prePresentBarrier.pNext = nullptr;
            prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            prePresentBarrier.image = m_swapchainBuffers[i].image;
            prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            prePresentBarrier.subresourceRange.baseArrayLayer = 0;
            prePresentBarrier.subresourceRange.baseMipLevel = 1;
            prePresentBarrier.subresourceRange.layerCount = 0;

            vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &prePresentBarrier);

            // By ending the command buffer, it is put out of record mode.
            err = vkEndCommandBuffer(cmdBuffer);
            GVR_VK_CHECK(!err);
        //}

}


int VulkanCore::AcquireNextImage(){
    imageIndex = (imageIndex + 1) % m_swapchainImageCount;
    return imageIndex;
}


void VulkanCore::DrawFrameForRenderData(int &swapChainIndex){

    VkResult err;
    // Get the next image to render to, then queue a wait until the image is ready
    int m_swapchainCurrentIdx = swapChainIndex;
  //  LOGI("Vulkna Swapchain Image number of %d", m_swapchainCurrentIdx);
    VkFence nullFence = waitFences[m_swapchainCurrentIdx];

    VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;//&m_backBufferSemaphore;
        submitInfo.pWaitDstStageMask = nullptr;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_swapchainBuffers[m_swapchainCurrentIdx].cmdBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;//&m_renderCompleteSemaphore;

  //  LOGI("Vulkan after vkqueue before submit");
    err = vkQueueSubmit(m_queue, 1, &submitInfo,  waitFences[m_swapchainCurrentIdx]);
    GVR_VK_CHECK(!err);
 //   LOGI("Vulkan after vkqueue submit %d", err);
    err = vkWaitForFences(m_device, 1, &waitFences[m_swapchainCurrentIdx], VK_TRUE, 4294967295U);
    GVR_VK_CHECK(!err);




    VkCommandBuffer trnCmdBuf = GetTransientCmdBuffer();
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(trnCmdBuf, &beginInfo);
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = outputImage[m_swapchainCurrentIdx].size;
    vkCmdCopyBuffer(trnCmdBuf, m_swapchainBuffers[m_swapchainCurrentIdx].buf, outputImage[m_swapchainCurrentIdx].buf, 1, &copyRegion);
    /* VkExtent3D extent3D = {};
              extent3D.width = m_width;
              extent3D.height = m_height;
     VkBufferImageCopy region = { 0 };
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.layerCount = 1;
                 region.imageExtent = extent3D;


    vkCmdCopyImageToBuffer(trnCmdBuf, m_swapchainBuffers[m_swapchainCurrentIdx].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, outputImage.buf, 1,  &region);*/
    vkEndCommandBuffer(trnCmdBuf);

    VkSubmitInfo ssubmitInfo = {};
    ssubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    ssubmitInfo.commandBufferCount = 1;
    ssubmitInfo.pCommandBuffers = &trnCmdBuf;

    vkQueueSubmit(m_queue, 1, &ssubmitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue);
    vkFreeCommandBuffers(m_device, m_commandPoolTrans, 1, &trnCmdBuf);










  //  LOGI("Vulkan after vkWaitForFences submit %d", err);

/*    static bool memoryAlloc = false;
    if(memoryAlloc == false){
      //  oculusTexData = (uint8_t*)malloc(m_width*m_height*4* sizeof(uint8_t));
        memoryAlloc = true;
    }
*/
    uint8_t * data;
    //err = vkMapMemory(m_device, m_swapchainBuffers[m_swapchainCurrentIdx].mem, 0, m_swapchainBuffers[m_swapchainCurrentIdx].size, 0, (void **)&data);
    err = vkMapMemory(m_device, outputImage[m_swapchainCurrentIdx].mem, 0, outputImage[m_swapchainCurrentIdx].size, 0, (void **)&data);
    GVR_VK_CHECK(!err);
    oculusTexData = data;

 //   LOGI("Vulkna size of %d", sizeof(oculusTexData));
  //  memcpy(oculusTexData, data, (m_width*m_height*4* sizeof(uint8_t)));

 //   LOGI("Vulkan memcpy map done");

    /*for (int i = 0; i < (m_width*m_height)-4; i++) {
        LOGI("Vulkan Data %u, %u %u %u", data[i], data[i+1], data[i+2], data[i+3]);
        i+=3;
        //break;
    }*/

    //oculusTexData = data;
   // texDataVulkan = data;//finaloutput;
 //   LOGI("Vulkan data reading done");
    vkUnmapMemory(m_device,outputImage[m_swapchainCurrentIdx].mem);
    //vkUnmapMemory(m_device,m_swapchainBuffers[m_swapchainCurrentIdx].mem);

    // Makes Fence Un-signalled
    err = vkResetFences(m_device, 1, &waitFences[m_swapchainCurrentIdx]);
    GVR_VK_CHECK(!err);
 //   LOGI("Vulkan after vkResetFences submit");
}


void VulkanCore::UpdateUniforms(Scene* scene, Camera* camera, RenderData* render_data)
{
    //static float spinAngle = 0.01f;

    //glm::mat4 rotationx = glm::rotate(glm::mat4(), glm::radians(spinAngle*3), glm::vec3(1.0f, 0.0f, 0.0f));
    //glm::mat4 rotationy = glm::rotate(glm::mat4(), glm::radians(spinAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    //glm::mat4 rotation = rotationx * rotationy;
    //glm::mat4 model = m_modelMatrix * rotation;
    //glm::mat4 modelView = m_viewMatrix * model;
   // glm::mat4 modelViewProjection;// =  rotation;//m_projectionMatrix * modelView;


    VkResult ret = VK_SUCCESS;
    uint8_t *pData;
        Transform* const t = render_data->owner_object()->transform();

        if (t == nullptr)
            return;

        glm::mat4 model = t->getModelMatrix();
    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 proj = camera->getProjectionMatrix();
    glm::mat4 modelViewProjection = proj * view * model;

   // LOGI("Vulkan mvp %f %f %f %f", modelViewProjection[0][0],  modelViewProjection[0][1], modelViewProjection[0][2], modelViewProjection[0][3]);

    ret = vkMapMemory(m_device, render_data->m_modelViewMatrixUniform.mem, 0, render_data->m_modelViewMatrixUniform.allocSize, 0, (void **) &pData);
    assert(!ret);

    memcpy(pData, (const void*) &modelViewProjection, sizeof(modelViewProjection));

    vkUnmapMemory(m_device, render_data->m_modelViewMatrixUniform.mem);

    //spinAngle += 0.8f;
}


void VulkanCore::InitDescriptorSetForRenderData(Uniform &m_modelViewMatrixUniform, Uniform &m_lightsUniform, VkDescriptorSet &m_descriptorSet) {
    //Create a pool with the amount of descriptors we require
    VkDescriptorPoolSize poolSize[2] = {};
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = 1;

     poolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      poolSize[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr;
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.poolSizeCount = 2;
    descriptorPoolCreateInfo.pPoolSizes = poolSize;

    VkResult  err;
    err = vkCreateDescriptorPool(m_device, &descriptorPoolCreateInfo, NULL, &m_descriptorPool);
    GVR_VK_CHECK(!err);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.pNext = nullptr;
    descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &m_descriptorLayout;

    err = vkAllocateDescriptorSets(m_device, &descriptorSetAllocateInfo, &m_descriptorSet);
    GVR_VK_CHECK(!err);

    /*VkDescriptorImageInfo descriptorImageInfo = {};

    if(m_texBricks->m_sampler == VK_NULL_HANDLE)
    LOGI("Vulkan m sampler is null");
    descriptorImageInfo.sampler = m_texBricks->m_sampler;
    descriptorImageInfo.imageView = m_texBricks->view;
    descriptorImageInfo.imageLayout = m_texBricks->m_imageLayout;*/

    VkWriteDescriptorSet writes[2] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstBinding = 0;
                         writes[0].dstSet = m_descriptorSet;
                         writes[0].descriptorCount = 1;
                         writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                         writes[0].pBufferInfo = &m_modelViewMatrixUniform.bufferInfo;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstBinding = 1;
                             writes[1].dstSet = m_descriptorSet;
                             writes[1].descriptorCount = 1;
                             writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                             writes[1].pBufferInfo = &m_lightsUniform.bufferInfo;

                         /*writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                         writes[1].dstBinding = 1;
                         writes[1].dstSet = m_descriptorSet;
                         writes[1].descriptorCount = 1;
                         writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                         writes[1].pImageInfo = &descriptorImageInfo;*/
                         LOGI("Vulkan before update sdectiptor");
                         vkUpdateDescriptorSets(m_device, 2, &writes[0], 0, nullptr);
                         LOGI("Vulkan after update sdectiptor");
}

void VulkanCore::initVulkanCore(ANativeWindow * newNativeWindow){
    m_Vulkan_Initialised = true;
    m_androidWindow = newNativeWindow;

   // LOGE("Vulkan Before init methods");
    if(InitVulkan() == 0){
   //      LOGE("Vulkan Not found at native side");
        m_Vulkan_Initialised = false;
        return;
    }
  //  LOGE("Vulkan after init methods");

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

    InitSwapchain(1024,1024);
    InitTransientCmdPool();
    InitCommandbuffers();
    //InitVertexBuffers();
     LOGE("Vulkan after InitVertexBuffers methods");
    //InitUniformBuffers();
     LOGE("Vulkan after InitUniformBuffers methods");
    InitLayouts();
     LOGE("Vulkan after InitLayouts methods");
    InitRenderPass();
     LOGE("Vulkan after InitRenderPass methods");
    //InitPipeline();
     LOGE("Vulkan after InitPipeline methods");
    //InitDescriptorSet();
    InitFrameBuffers();
    InitSync();

    // Initialize our command buffers
    //BuildCmdBuffer();
    //DrawFrame();
}
}
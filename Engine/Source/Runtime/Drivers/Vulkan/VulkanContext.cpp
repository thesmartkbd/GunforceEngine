/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|*    Copyright (C) 2023 thesmartkbd                                                *|
|*                                                                                  *|
|*    This program is free software: you can redistribute it and/or modify          *|
|*    it under the terms of the GNU General Public License as published by          *|
|*    the Free Software Foundation, either version 3 of the License, or             *|
|*    (at your option) any later version.                                           *|
|*                                                                                  *|
|*    This program is distributed in the hope that it will be useful,               *|
|*    but WITHOUT ANY WARRANTY; without even the implied warranty of                *|
|*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *|
|*    GNU General Public License for more details.                                  *|
|*                                                                                  *|
|*    You should have received a copy of the GNU General Public License             *|
|*    along with this program.  If not, see <https://www.gnu.org/licenses/>.        *|
|*                                                                                  *|
|*    This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.    *|
|*    This is free software, and you are welcome to redistribute it                 *|
|*    under certain conditions; type `show c' for details.                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|* File:           VulkanContext.cpp                                                *|
|* Create Time:    2024/01/03 01:35                                                 *|
|* Author:         thesmartkbd                                                      *|
|* EMail:          thesmartkbd@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include "VulkanContext.h"
#include "Window/Window.h"
#include "VkUtils.h"
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#define VULKAN_CONTEXT_POINTER "VK_CONTEXT_POINTER"
#define DEBUG_ONLY_RUN_ONCE() \
{\
    static bool firstDebug = true; \
    if (firstDebug) { \
        VkUtils::DebugPrintVulkanProperties(m_Instance); firstDebug = !firstDebug; \
    } \
}

VulkanContext::VulkanContext(Window *p_window) : m_Window(p_window)
{
    LOGGER_WRITE_INFO("VulkanContext initialize begin!");
    m_Window->AddWindowUserPointer(VULKAN_CONTEXT_POINTER, this);
    _InitializeVulkanContextInstance();
    DEBUG_ONLY_RUN_ONCE();
    _InitializeVulkanContextSurface();
    _InitializeVulkanContextDevice();
    _InitializeVulkanContextMemoryAllocator();
    _InitializeVulkanContextRWindow();
    _InitializeVulkanContextCommandPool();
    _InitializeVulkanContextDescriptorPool();
    LOGGER_WRITE_INFO("VulkanContext initialize end!");
}

VulkanContext::~VulkanContext()
{
    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, VkUtils::Allocator);
    vkDestroyCommandPool(m_Device, m_CommandPool, VkUtils::Allocator);
    DestroyRWindow(m_RWindow);
    vmaDestroyAllocator(m_Allocator);
    vkDestroyDevice(m_Device, VkUtils::Allocator);
    vkDestroySurfaceKHR(m_Instance, m_Surface, VkUtils::Allocator);
    vkDestroyInstance(m_Instance, VkUtils::Allocator);
    m_Window->RemoveWindowUserPointer(VULKAN_CONTEXT_POINTER);
}

void VulkanContext::CreatePipeline(const char *folder, const char *name, VulkanContext::Pipeline **ppPipeline)
{
    char buf[255];
    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;

    VulkanContext::Pipeline *newPipeline = MemoryMalloc(VulkanContext::Pipeline);

    snprintf(buf, sizeof(buf), "%s/%s.vert.spv", folder, name);
    VkUtils::LoadShaderModule(buf, m_Device, &vertexShaderModule);
    snprintf(buf, sizeof(buf), "%s/%s.frag.spv", folder, name);
    VkUtils::LoadShaderModule(buf, m_Device, &fragmentShaderModule);

    vkDestroyShaderModule(m_Device, vertexShaderModule, VkUtils::Allocator);
    vkDestroyShaderModule(m_Device, fragmentShaderModule, VkUtils::Allocator);

    *ppPipeline = newPipeline;
}

void VulkanContext::DestroyPipeline(VulkanContext::Pipeline *pPipeline)
{
    vkDestroyPipeline(m_Device, pPipeline->pipeline, VkUtils::Allocator);
    vkDestroyPipelineLayout(m_Device, pPipeline->pipelineLayout, VkUtils::Allocator);
    MemoryFree(pPipeline);
}

void VulkanContext::CreateDescriptorSet()
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
}

void VulkanContext::RecreateRWindow(VulkanContext::RWindow* pOldRWindow, VulkanContext::RWindow** ppRWindow)
{
    if (pOldRWindow != null) {
        CreateRWindow(pOldRWindow, ppRWindow);
        DestroyRWindow(pOldRWindow);
    } else {
        DestroyRWindow(*ppRWindow);
        CreateRWindow(null, ppRWindow);
    }
}

void VulkanContext::CreateRWindow(const VulkanContext::RWindow* pOldRWindow, VulkanContext::RWindow** ppRWindow)
{
    LOGGER_WRITE_INFO("VulkanContext create RWindow object");
    VulkanContext::RWindow *newRWindow = MemoryMalloc(VulkanContext::RWindow);
    VkUtils::ConfigurationSwpachainCapabilities(m_PhysicalDevice, m_Surface, newRWindow);
    LOGGER_WRITE_INFO("  - Capabilites: ");
    LOGGER_WRITE_INFO("    - Format:         %d", newRWindow->format);
    LOGGER_WRITE_INFO("    - MinImageCount:  %u", newRWindow->minImageCount);
    LOGGER_WRITE_INFO("    - ColorSpace:     %d", newRWindow->colorSpace);
    LOGGER_WRITE_INFO("    - PresentMode:    %d", newRWindow->presentMode);
    LOGGER_WRITE_INFO("    - Transform:      %d", newRWindow->transform);
    LOGGER_WRITE_INFO("    - Width:          %u", newRWindow->width);
    LOGGER_WRITE_INFO("    - Height:         %u", newRWindow->height);

    /* 设置 vector 大小 */
    newRWindow->images.resize(newRWindow->minImageCount);
    newRWindow->imageViews.resize(newRWindow->minImageCount);
    newRWindow->framebuffers.resize(newRWindow->minImageCount);

    VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
    swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKHR.surface = newRWindow->surface;
    swapchainCreateInfoKHR.minImageCount = newRWindow->minImageCount;
    swapchainCreateInfoKHR.imageFormat = newRWindow->format;
    swapchainCreateInfoKHR.imageColorSpace = newRWindow->colorSpace;
    swapchainCreateInfoKHR.imageExtent = { newRWindow->width, newRWindow->height };
    swapchainCreateInfoKHR.imageArrayLayers = 1;
    swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKHR.preTransform = newRWindow->transform;
    swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKHR.presentMode = newRWindow->presentMode;
    swapchainCreateInfoKHR.clipped = VK_TRUE;
    swapchainCreateInfoKHR.oldSwapchain = pOldRWindow != null ? pOldRWindow->swapchain : null;
    swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfoKHR.queueFamilyIndexCount = 0;
    swapchainCreateInfoKHR.pQueueFamilyIndices = nullptr;

    if (m_GraphicsQueueFamilyIndex != m_PresentQueueFamilyIndex) {
        swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        std::array<uint32_t, 2> families = {
            m_GraphicsQueueFamilyIndex,
            m_PresentQueueFamilyIndex
        };
        swapchainCreateInfoKHR.queueFamilyIndexCount = std::size(families);
        swapchainCreateInfoKHR.pQueueFamilyIndices = std::data(families);
    }
    
    VK_ERROR err = vkCreateSwapchainKHR(m_Device, &swapchainCreateInfoKHR, VkUtils::Allocator, &newRWindow->swapchain);
    LOGGER_WRITE_INFO("  Create swapchain khr: %s", ERROR_FAIL(err));

    /* create render pass  */
    CreateRenderPass(newRWindow->format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &newRWindow->renderPass);

    /* about swapcahin image */
    vkGetSwapchainImagesKHR(m_Device, newRWindow->swapchain, &newRWindow->minImageCount, std::data(newRWindow->images));

    for (uint32_t i = 0; i < newRWindow->minImageCount; i++) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = newRWindow->images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = newRWindow->format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        VK_ERROR err = vkCreateImageView(m_Device, &imageViewCreateInfo, null, &newRWindow->imageViews[i]);
        LOGGER_WRITE_INFO("Create image view(VkImageView): %s", ERROR_FAIL(err));

        CreateFramebuffer(newRWindow->renderPass, newRWindow->imageViews[i], newRWindow->width, newRWindow->height, &newRWindow->framebuffers[i]);
    }

    *ppRWindow = newRWindow;
}

void VulkanContext::DestroyRWindow(VulkanContext::RWindow* pRWindow)
{
    vkDestroyRenderPass(m_Device, pRWindow->renderPass, VkUtils::Allocator);
    for (int i = 0; i < pRWindow->minImageCount; i++) {
        vkDestroyImageView(m_Device, pRWindow->imageViews[i], VkUtils::Allocator);
        vkDestroyFramebuffer(m_Device, pRWindow->framebuffers[i], VkUtils::Allocator);
    }
    vkDestroySwapchainKHR(m_Device, pRWindow->swapchain, VkUtils::Allocator);
    MemoryFree(pRWindow);
}

void VulkanContext::CreateRenderPass(VkFormat format, VkImageLayout imageLayout, VkRenderPass* pRenderPass)
{
    VkAttachmentDescription colorAttachmentDescription = {};
    colorAttachmentDescription.format = format;
    colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout = imageLayout;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    VK_ERROR err = vkCreateRenderPass(m_Device, &renderPassCreateInfo, VkUtils::Allocator, pRenderPass);
    LOGGER_WRITE_INFO("Create render pass(VkRenderPass): %s", ERROR_FAIL(err));
}

void VulkanContext::DestroyRenderPass(VkRenderPass renderPass)
{
    vkDestroyRenderPass(m_Device, renderPass, VkUtils::Allocator);
}

void VulkanContext::CreateFramebuffer(VkRenderPass renderPass, VkImageView imageView, uint32_t width, uint32_t height, VkFramebuffer* pFramebuffer)
{
    VkImageView attachments[] = { imageView };
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = attachments;
    framebufferCreateInfo.width = width;
    framebufferCreateInfo.height = height;
    framebufferCreateInfo.layers = 1;

    VK_ERROR err = vkCreateFramebuffer(m_Device, &framebufferCreateInfo, VkUtils::Allocator, pFramebuffer);
    LOGGER_WRITE_INFO("Create frame buffer(VkFramebuffer): %s", ERROR_FAIL(err));
}

void VulkanContext::DestroyFramebuffer(VkFramebuffer framebuffer)
{
    vkDestroyFramebuffer(m_Device, framebuffer, VkUtils::Allocator);
}

void VulkanContext::CreateBuffer(uint64_t size, VkBufferUsageFlags usage, VulkanContext::Buffer *pBuffer)
{
    VulkanContext::Buffer newBuffer = MemoryMalloc(VulkanContext::Buffer_T);

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.memoryTypeBits = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    vmaCreateBuffer(m_Allocator, &bufferCreateInfo, &allocInfo, &newBuffer->buffer, &newBuffer->allocation, null);

    *pBuffer = newBuffer;
}

void VulkanContext::DestroyBuffer(VulkanContext::Buffer buffer)
{
    vmaDestroyBuffer(m_Allocator, buffer->buffer, buffer->allocation);
    MemoryFree(buffer);
}

void VulkanContext::MapMemory(VulkanContext::Buffer buffer, void **ppData)
{
    vmaMapMemory(m_Allocator, buffer->allocation, ppData);
}

void VulkanContext::UnmapMemory(VulkanContext::Buffer buffer)
{
    vmaUnmapMemory(m_Allocator, buffer->allocation);
}

void VulkanContext::DeviceWaitIdle()
{
    vkDeviceWaitIdle(m_Device);
}

void VulkanContext::BeginOneTimeCommandBuffer(VkCommandBuffer* pCommandBuffer)
{
    BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, pCommandBuffer);
}

void VulkanContext::EndOneTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
    EndCommandBuffer(commandBuffer);
}

void VulkanContext::BeginCommandBuffer(VkCommandBufferUsageFlags usage, VkCommandBuffer* pCommandBuffer)
{
    AllocateCommandBuffer(pCommandBuffer);
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = usage;
    vkBeginCommandBuffer(*pCommandBuffer, &commandBufferBeginInfo);
}

void VulkanContext::EndCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);
}

void VulkanContext::AllocateCommandBuffer(VkCommandBuffer* pCommandBuffer)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = m_CommandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_ERROR err = vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, pCommandBuffer);
    LOGGER_WRITE_INFO("Allocate command buffers(VkCommandBuffer): %s", ERROR_FAIL(err));
}

void VulkanContext::FreeCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}

void VulkanContext::_InitializeVulkanContextInstance()
{
    LOGGER_WRITE_INFO("Begin initialize instance(VkInstance)");
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = GUNFORCE_ENGINE_NAME;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = GUNFORCE_ENGINE_NAME;
    applicationInfo.apiVersion = ApiVersion;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    Vector<const char*> extensions;
    VkUtils::GetInstanceRequiredEnableExtensionProperties(extensions);
    LOGGER_WRITE_INFO("  Instance enable extension list:");
    for (const auto& extension : extensions)
        LOGGER_WRITE_INFO("    - %s", extension);
    instanceCreateInfo.enabledExtensionCount = std::size(extensions);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(extensions);

    Vector<const char*> layers;
    VkUtils::GetInstanceRequiredEnableLayerProperties(layers);
    LOGGER_WRITE_INFO("  Instance enable layer list:");
    for (const auto& layer : layers)
        LOGGER_WRITE_INFO("    - %s", layer);
    instanceCreateInfo.enabledLayerCount = std::size(layers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(layers);

    VK_ERROR err = vkCreateInstance(&instanceCreateInfo, VkUtils::Allocator, &m_Instance);
    LOGGER_WRITE_INFO("End initialize instance(VkInstance): %d", err);

    vkEnumerateInstanceVersion(&ApiVersion);
    LOGGER_WRITE_INFO("Vulkan Instance Version: %d.%d.%d", VK_VERSION_MAJOR(ApiVersion), VK_VERSION_MINOR(ApiVersion), VK_VERSION_PATCH(ApiVersion));
}

void VulkanContext::_InitializeVulkanContextSurface()
{
    LOGGER_WRITE_INFO("Begin initialize surface(VkSurface)");
    VkResult err = glfwCreateWindowSurface(m_Instance, m_Window->GetNativeWindow(), VkUtils::Allocator, &m_Surface);
    LOGGER_WRITE_INFO("End initialize surface(VkSurface): %s", ERROR_FAIL(err));
}

void VulkanContext::_InitializeVulkanContextDevice()
{
    LOGGER_WRITE_INFO("Begin initialize device(VkDevice)");
    VkUtils::GetBestPerformancePhysicalDevice(m_Instance, &m_PhysicalDevice);
    VkUtils::GetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties, &m_PhysicalDeviceFeatures);
    LOGGER_WRITE_INFO("  Use device name: %s", m_PhysicalDeviceProperties.deviceName);

    VkUtils::QueueFamilyIndices queueFamilyIndices;
    VkUtils::FindQueueFamilyIndices(m_PhysicalDevice, m_Surface, &queueFamilyIndices);
    m_GraphicsQueueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    m_PresentQueueFamilyIndex = queueFamilyIndices.presentQueueFamily;
    LOGGER_WRITE_INFO("  Query queue family indices: ");
    LOGGER_WRITE_INFO("    - GRAPHICS: %d", m_GraphicsQueueFamilyIndex);
    LOGGER_WRITE_INFO("    - PRESENT:  %d", m_PresentQueueFamilyIndex);

    float priorities = 1.0f;
    std::array<VkDeviceQueueCreateInfo, 2> deviceQueueCreateInfos = {};
    deviceQueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfos[0].queueCount = 1;
    deviceQueueCreateInfos[0].queueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    deviceQueueCreateInfos[0].pQueuePriorities = &priorities;

    deviceQueueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfos[1].queueCount = 1;
    deviceQueueCreateInfos[1].queueFamilyIndex = queueFamilyIndices.presentQueueFamily;
    deviceQueueCreateInfos[1].pQueuePriorities = &priorities;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = std::size(deviceQueueCreateInfos);
    deviceCreateInfo.pQueueCreateInfos = std::data(deviceQueueCreateInfos);
    static VkPhysicalDeviceFeatures features = {};
    deviceCreateInfo.pEnabledFeatures = &features;

    Vector<const char*> enableDeviceExtensionProperties;
    VkUtils::GetDeviceRequiredEnableExtensionProperties(m_PhysicalDevice, enableDeviceExtensionProperties);
    LOGGER_WRITE_INFO("  Device enable extension list:");
    for (const auto& extension : enableDeviceExtensionProperties)
        LOGGER_WRITE_INFO("    - %s", extension);
    deviceCreateInfo.enabledExtensionCount = std::size(enableDeviceExtensionProperties);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(enableDeviceExtensionProperties);

    Vector<const char*> enableDeviceLayerProperties;
    VkUtils::GetDeviceRequiredEnableLayerProperties(m_PhysicalDevice, enableDeviceLayerProperties);
    LOGGER_WRITE_INFO("  Device enable layer list:");
    for (const auto& layer : enableDeviceLayerProperties)
        LOGGER_WRITE_INFO("    - %s", layer);
    deviceCreateInfo.enabledLayerCount = std::size(enableDeviceLayerProperties);
    deviceCreateInfo.ppEnabledLayerNames = std::data(enableDeviceLayerProperties);

    VK_ERROR err = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, VkUtils::Allocator, &m_Device);

    /* 获取队列 */
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_PresentQueueFamilyIndex, 0, &m_PresentQueue);
    LOGGER_WRITE_INFO("End initialize device(VkDevice): %s", ERROR_FAIL(err));
}

void VulkanContext::_InitializeVulkanContextMemoryAllocator()
{
    LOGGER_WRITE_INFO("Begin initialize memory allocator(VmaAllocator)");
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.instance = m_Instance;
    allocatorCreateInfo.physicalDevice = m_PhysicalDevice;
    allocatorCreateInfo.device = m_Device;
    allocatorCreateInfo.vulkanApiVersion = ApiVersion;
    allocatorCreateInfo.pAllocationCallbacks = null;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    VK_ERROR err = vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator);
    LOGGER_WRITE_INFO("End initialize memory allocator(VmaAllocator): %s", ERROR_FAIL(err));
}

void VulkanContext::_InitializeVulkanContextRWindow()
{
    LOGGER_WRITE_INFO("Begin initialize RWindow(VulkanContext::RWindow)");
    CreateRWindow(null, &m_RWindow);
    m_Window->AddWindowResizeEventCallback([](Window *window, uint32_t width, uint32_t height) {
        LOGGER_WRITE_INFO("VulkanContext: window resize event callback - recreate VulkanContext::RWindow current size, %u, %u", width, height);
        VulkanContext *vulkanContext = (VulkanContext *) window->GetWindowUserPointer(VULKAN_CONTEXT_POINTER);
        VulkanContext::RWindow *oldRWindow = vulkanContext->m_RWindow;
        vulkanContext->RecreateRWindow(oldRWindow, &vulkanContext->m_RWindow);
    });
    LOGGER_WRITE_INFO("End initialize RWindow(VulkanContext::RWindow)");
}

void VulkanContext::_InitializeVulkanContextCommandPool()
{
    LOGGER_WRITE_INFO("Begin initialize CommandPool(VkCommandPool)");
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
    VK_ERROR err = vkCreateCommandPool(m_Device, &commandPoolCreateInfo, VkUtils::Allocator, &m_CommandPool);
    LOGGER_WRITE_INFO("  Use queue index: %d", commandPoolCreateInfo.queueFamilyIndex);
    LOGGER_WRITE_INFO("End initialize CommandPool(VkCommandPool): %s", ERROR_FAIL(err));
}

void VulkanContext::_InitializeVulkanContextDescriptorPool()
{
    LOGGER_WRITE_INFO("Begin initialize DescriptorPool(VkDescriptorPool)");
    Vector<VkDescriptorPoolSize> poolSize = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 64 },
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1024;
    descriptorPoolCreateInfo.poolSizeCount = std::size(poolSize);
    descriptorPoolCreateInfo.pPoolSizes = std::data(poolSize);

    LOGGER_WRITE_INFO("  DescriptorPool size: ");
    for (const auto &item: poolSize)
        LOGGER_WRITE_INFO("    type: %d, count: %d", item.type, item.descriptorCount);
    LOGGER_WRITE_INFO("  DescriptorPool max sets: %d", descriptorPoolCreateInfo.maxSets);

    VK_ERROR err = vkCreateDescriptorPool(m_Device, &descriptorPoolCreateInfo, VkUtils::Allocator, &m_DescriptorPool);
    LOGGER_WRITE_INFO("End initialize DescriptorPool(VkDescriptorPool): %s", ERROR_FAIL(err));
}
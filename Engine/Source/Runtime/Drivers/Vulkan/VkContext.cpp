/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|*    Copyright (C) 2023 bit-fashion                                                *|
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
|* File:           VkContext.cpp                                                    *|
|* Create Time:    2024/01/03 01:35                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include "VkContext.h"
#include "Window/Window.h"
#include "VkUtils.h"

#define VULKAN_CONTEXT_POINTER "VK_CONTEXT_POINTER"

VkContext::VkContext(Window *p_window) : m_Window(p_window)
{
    LOGGER_WRITE_INFO("VkContext initialize begin!");
    m_Window->AddWindowUserPointer(VULKAN_CONTEXT_POINTER, this);
    _InitializeVulkanContextInstance();
    _InitializeVulkanContextSurface();
    _InitializeVulkanContextDevice();
    _InitializeVulkanContextRWindow();
    _InitializeVulkanContextCommandPool();
    _InitializeVulkanContextDescriptorPool();
    LOGGER_WRITE_INFO("VkContext initialize end!");
}

VkContext::~VkContext()
{
    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, VkUtils::Allocator);
    vkDestroyCommandPool(m_Device, m_CommandPool, VkUtils::Allocator);
    vkDestroyDevice(m_Device, VkUtils::Allocator);
    vkDestroySurfaceKHR(m_Instance, m_Surface, VkUtils::Allocator);
    vkDestroyInstance(m_Instance, VkUtils::Allocator);
    m_Window->RemoveWindowUserPointer(VULKAN_CONTEXT_POINTER);
}

void VkContext::CreateFramebuffer(VkRenderPass renderPass, VkImageView imageView, uint32_t width, uint32_t height, VkFramebuffer* pFramebuffer)
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

    vkCheckCreate(Framebuffer, m_Device, &framebufferCreateInfo, VkUtils::Allocator, pFramebuffer);
}

void VkContext::DestroyFramebuffer(VkFramebuffer framebuffer)
{
    vkDestroyFramebuffer(m_Device, framebuffer, VkUtils::Allocator);
}

void VkContext::BeginOneTimeCommandBuffer(VkCommandBuffer* pCommandBuffer)
{
    BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, pCommandBuffer);
}

void VkContext::EndOneTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
    EndCommandBuffer(commandBuffer);
}

void VkContext::BeginCommandBuffer(VkCommandBufferUsageFlags usage, VkCommandBuffer* pCommandBuffer)
{
    AllocateCommandBuffer(pCommandBuffer);
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = usage;
    vkBeginCommandBuffer(*pCommandBuffer, &commandBufferBeginInfo);
}

void VkContext::EndCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);
}

void VkContext::AllocateCommandBuffer(VkCommandBuffer* pCommandBuffer)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = m_CommandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkCheckAllocate(CommandBuffers, m_Device, &commandBufferAllocateInfo, pCommandBuffer);
}

void VkContext::FreeCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}

void VkContext::_InitializeVulkanContextInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = GUNFORCE_ENGINE_NAME;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = GUNFORCE_ENGINE_NAME;
    applicationInfo.apiVersion = VK_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    Vector<const char*> extensions;
    VkUtils::GetInstanceRequiredEnableExtensionProperties(extensions);
    LOGGER_WRITE_INFO("VkContext instance enable extension list:");
    for (const auto& extension : extensions)
        LOGGER_WRITE_INFO("  - %s", extension);
    instanceCreateInfo.enabledExtensionCount = std::size(extensions);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(extensions);

    Vector<const char*> layers;
    VkUtils::GetInstanceRequiredEnableLayerProperties(layers);
    LOGGER_WRITE_INFO("VkContext instance enable layer list:");
    for (const auto& layer : layers)
        LOGGER_WRITE_INFO("  - %s", layer);
    instanceCreateInfo.enabledLayerCount = std::size(layers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(layers);

    vkCheckCreate(Instance, &instanceCreateInfo, VkUtils::Allocator, &m_Instance);
}

void VkContext::_InitializeVulkanContextSurface()
{
    glfwCreateWindowSurface(m_Instance, m_Window->GetNativeWindow(), VkUtils::Allocator, &m_Surface);
}

void VkContext::_InitializeVulkanContextDevice()
{
    VkUtils::GetBestPerformancePhysicalDevice(m_Instance, &m_PhysicalDevice);
    VkUtils::GetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties, &m_PhysicalDeviceFeatures);
    LOGGER_WRITE_INFO("VkContext physical device gpu using: {}", m_PhysicalDeviceProperties.deviceName);

    VkUtils::QueueFamilyIndices queueFamilyIndices;
    VkUtils::FindQueueFamilyIndices(m_PhysicalDevice, m_Surface, &queueFamilyIndices);
    m_GraphicsQueueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    m_PresentQueueFamilyIndex = queueFamilyIndices.presentQueueFamily;

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
    deviceCreateInfo.enabledExtensionCount = std::size(enableDeviceExtensionProperties);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(enableDeviceExtensionProperties);

    Vector<const char*> enableDeviceLayerProperties;
    VkUtils::GetDeviceRequiredEnableLayerProperties(m_PhysicalDevice, enableDeviceLayerProperties);
    deviceCreateInfo.enabledLayerCount = std::size(enableDeviceLayerProperties);
    deviceCreateInfo.ppEnabledLayerNames = std::data(enableDeviceLayerProperties);

    vkCheckCreate(Device, m_PhysicalDevice, &deviceCreateInfo, VkUtils::Allocator, &m_Device);

    /* 获取队列 */
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_PresentQueueFamilyIndex, 0, &m_PresentQueue);
}

void VkContext::_InitializeVulkanContextRWindow()
{
    LOGGER_WRITE_INFO("VulkanContext initialize RWindow object: ");
    m_RWindow = MemoryMalloc(VkContext::RWindow);
    VkUtils::ConfigurationSwpachainCapabilities(m_PhysicalDevice, m_Surface, m_RWindow);
    LOGGER_WRITE_INFO("  - capabilites: ");
    LOGGER_WRITE_INFO("    - Format:         %d", m_RWindow->format);
    LOGGER_WRITE_INFO("    - MinImageCount:  %u", m_RWindow->minImageCount);
    LOGGER_WRITE_INFO("    - ColorSpace:     %d", m_RWindow->colorSpace);
    LOGGER_WRITE_INFO("    - PresentMode:    %d", m_RWindow->presentMode);
    LOGGER_WRITE_INFO("    - Transform:      %d", m_RWindow->transform);
    LOGGER_WRITE_INFO("    - Width:          %u", m_RWindow->width);
    LOGGER_WRITE_INFO("    - Height:         %u", m_RWindow->height);

    VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
    swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKHR.surface = m_RWindow->surface;
    swapchainCreateInfoKHR.minImageCount = m_RWindow->minImageCount;
    swapchainCreateInfoKHR.imageFormat = m_RWindow->format;
    swapchainCreateInfoKHR.imageColorSpace = m_RWindow->colorSpace;
    swapchainCreateInfoKHR.imageExtent = { m_RWindow->width, m_RWindow->height };
    swapchainCreateInfoKHR.imageArrayLayers = 1;
    swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKHR.preTransform = m_RWindow->transform;
    swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKHR.presentMode = m_RWindow->presentMode;
    swapchainCreateInfoKHR.clipped = VK_TRUE;
    swapchainCreateInfoKHR.oldSwapchain = null;
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

    vkCheckCreate(SwapchainKHR, m_Device, &swapchainCreateInfoKHR, VkUtils::Allocator, &m_RWindow->swapchain);

    /* create render pass  */
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkCheckCreate(RenderPass, m_Device, &renderPassCreateInfo, VkUtils::Allocator, &m_RWindow->renderPass);

    /* about swapcahin image */
    m_RWindow->images.resize(m_RWindow->minImageCount);
    vkGetSwapchainImagesKHR(m_Device, m_RWindow->swapchain, &m_RWindow->minImageCount, std::data(m_RWindow->images));

    for (uint32_t i = 0; i < m_RWindow->minImageCount; i++) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = m_RWindow->images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = m_RWindow->format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        vkCheckCreate(ImageView, m_Device, &imageViewCreateInfo, null, &m_RWindow->imageViews[i]);

        CreateFramebuffer(m_RWindow->renderPass, m_RWindow->imageViews[i], m_RWindow->width, m_RWindow->height, &m_RWindow->framebuffers[i]);
    }
}

void VkContext::_InitializeVulkanContextCommandPool()
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
    vkCheckCreate(CommandPool, m_Device, &commandPoolCreateInfo, VkUtils::Allocator, &m_CommandPool);
}

void VkContext::_InitializeVulkanContextDescriptorPool()
{
    Vector<VkDescriptorPoolSize> poolSize = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 64 },
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1024;
    descriptorPoolCreateInfo.poolSizeCount = std::size(poolSize);
    descriptorPoolCreateInfo.pPoolSizes = std::data(poolSize);

    vkCheckCreate(DescriptorPool, m_Device, &descriptorPoolCreateInfo, VkUtils::Allocator, &m_DescriptorPool);
}
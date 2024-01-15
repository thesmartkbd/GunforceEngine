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
|* File:           VulkanContext.h                                                  *|
|* Create Time:    2024/01/03 01:35                                                 *|
|* Author:         thesmartkbd                                                      *|
|* EMail:          thesmartkbd@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include <vulkan/vulkan.h>
#include <Gunforce.h>

#define VK_API_VERSION VK_VERSION_1_3

class Window;
VK_DEFINE_HANDLE(VmaAllocator)

class GUNFORCEAPI VulkanContext {
public:
    /* Vulkan render main window context */
    struct RWindow {
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        VkRenderPass renderPass;
        Vector<VkImage> images;
        Vector<VkImageView> imageViews;
        Vector<VkFramebuffer> framebuffers;
        Window* window;
        /* capabilities */
        uint32_t width;
        uint32_t height;
        VkFormat format;
        VkColorSpaceKHR colorSpace;
        VkPresentModeKHR presentMode;
        uint32_t minImageCount;
        VkSurfaceTransformFlagBitsKHR transform;
    };

    struct Pipeline {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
    };

public:
    VulkanContext(Window *p_window);
    ~VulkanContext();

    /* RESOURCE */
    void CreatePipeline(const char *folder, const char *name, VulkanContext::Pipeline **ppPipeline);
    void DestroyPipeline(VulkanContext::Pipeline *pPipeline);
    void CreateDescriptorSet();
    void RecreateRWindow(VulkanContext::RWindow* pOldRWindow, VulkanContext::RWindow** ppRWindow);
    void CreateRWindow(const VulkanContext::RWindow* pOldRWindow, VulkanContext::RWindow** ppRWindow);
    void DestroyRWindow(VulkanContext::RWindow* pRWindow);
    void CreateRenderPass(VkFormat format, VkImageLayout imageLayout, VkRenderPass* pRenderPass);
    void DestroyRenderPass(VkRenderPass renderPass);
    void CreateFramebuffer(VkRenderPass renderPass, VkImageView imageView, uint32_t width, uint32_t height, VkFramebuffer* pFramebuffer);
    void DestroyFramebuffer(VkFramebuffer framebuffer);
    void CreateBuffer(uint64_t size, VkBufferUsageFlagBits usage, VkBuffer *pBuffer);
    void DestroyBuffer(VkBuffer buffer);

    /* OPERATE */
    void DeviceWaitIdle();
    void BeginOneTimeCommandBuffer(VkCommandBuffer* pCommandBuffer);
    void EndOneTimeCommandBuffer(VkCommandBuffer commandBuffer);
    void BeginCommandBuffer(VkCommandBufferUsageFlags usage, VkCommandBuffer *pCommandBuffer);
    void EndCommandBuffer(VkCommandBuffer commandBuffer);
    void AllocateCommandBuffer(VkCommandBuffer *pCommandBuffer);
    void FreeCommandBuffer(VkCommandBuffer commandBuffer);

private:
    void _InitializeVulkanContextInstance();
    void _InitializeVulkanContextSurface();
    void _InitializeVulkanContextDevice();
    void _InitializeVulkanContextMemoryAllocator();
    void _InitializeVulkanContextRWindow();
    void _InitializeVulkanContextCommandPool();
    void _InitializeVulkanContextDescriptorPool();

private:
    VkInstance m_Instance;
    VkSurfaceKHR m_Surface;
    VkDevice m_Device;
    VmaAllocator m_Allocator;
    VulkanContext::RWindow* m_RWindow;
    VkCommandPool m_CommandPool;
    VkDescriptorPool m_DescriptorPool;

    Window* m_Window;
    VkPhysicalDevice m_PhysicalDevice;
    VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
    VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
    uint32_t m_GraphicsQueueFamilyIndex;
    VkQueue m_GraphicsQueue;
    uint32_t m_PresentQueueFamilyIndex;
    VkQueue m_PresentQueue;
};
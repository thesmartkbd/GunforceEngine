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

class Window;
VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)

/* Vulkan render main window context */
struct VtxWindow_T {
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkRenderPass renderPass;
    Vector<VkImage> images;
    Vector<VkImageView> imageViews;
    Vector<VkFramebuffer> framebuffers;
    Vector<VkCommandBuffer> commandBuffers;
    VkSemaphore available;
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

/* pipeline */
struct VtxPipeline_T {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

enum VtxMemoryAllocateType {
    /* 使用 CPU 允许数据映射 */
    VTX_MEMORY_ALLOCATE_TYPE_CPU,
    /* 使用 GPU 分配不允许对数据进行映射 */
    VTX_MEMORY_ALLOCATE_TYPE_GPU,
};

/* buffer(VMA) */
struct VtxBuffer_T {
    VkBuffer buffer;
    uint64_t size;
    VmaAllocation allocation; /* VmaAllocation */
};

struct VtxTexture2D_T {
    VkImageView imageView;
};

VK_DEFINE_HANDLE(VtxWindow)
VK_DEFINE_HANDLE(VtxPipeline)
VK_DEFINE_HANDLE(VtxBuffer)
VK_DEFINE_HANDLE(VtxTexture2D)

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;
};

class GUNFORCEAPI VulkanContext {
public:
    VulkanContext(Window *p_window);
    ~VulkanContext();

    const VtxWindow GetCurrentContextVtxWindow() const { return m_RenderWindow; }

    /* RESOURCE */
    void CreateSemaphoreV(VkSemaphore *pSemaphore);
    void DestroySemaphoreV(VkSemaphore semaphore);
    void CreatePipeline(const char* folder, const char* name, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout, VtxPipeline* pPipeline);
    void DestroyPipeline(VtxPipeline pipeline);
    void CreateDescriptorSet(const Vector<VkDescriptorSetLayout> &layouts, VkDescriptorSet* pDescriptorSets);
    void DestroyDescriptorSet(VkDescriptorSet descriptorSet);
    void CreateDescriptorSetLayout(const Vector<VkDescriptorSetLayoutBinding> &bindings, VkDescriptorSetLayout *pDescriptorSetLayout);
    void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
    void RecreateRenderWindow(VtxWindow pOldRWindow, VtxWindow* pRWindow);
    void CreateRenderWindow(const VtxWindow oldRWindow, VtxWindow* pRWindow);
    void DestroyRenderWindow(VtxWindow window);
    void CreateRenderPass(VkFormat format, VkImageLayout imageLayout, VkRenderPass* pRenderPass);
    void DestroyRenderPass(VkRenderPass renderPass);
    void CreateFramebuffer(VkRenderPass renderPass, VkImageView imageView, uint32_t width, uint32_t height, VkFramebuffer* pFramebuffer);
    void DestroyFramebuffer(VkFramebuffer framebuffer);
    void CreateIndexBuffer(uint64_t size, uint32_t *pIndices, VtxBuffer *pBuffer);
    void CreateVertexBuffer(uint64_t size, Vertex *pVertices, VtxBuffer *pBuffer);
    void CreateBuffer(uint64_t size, VkBufferUsageFlags usage, VtxMemoryAllocateType type, VtxBuffer *pBuffer);
    void DestroyBuffer(VtxBuffer buffer);
    void CreateCommandBuffer(VkCommandBuffer *pCommandBuffer);
    void DestroyCommandBuffer(VkCommandBuffer commandBuffer);

    /* OPERATE */
    void PresentSubmitQueueKHR(VkSemaphore waitSemaphore, uint32_t index, VtxWindow window);
    void SynchronizeSubmitQueue(VkCommandBuffer commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkPipelineStageFlags waitDstStageMask);
    void QueueWaitIdle(VkQueue queue);
    void DeviceWaitIdle();
    void AcquireNextImage(VtxWindow window, uint32_t *pIndex);
    void BeginRenderPass(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkFramebuffer framebuffer, VkRenderPass renderPass);
    void EndRenderPass(VkCommandBuffer commandBuffer);
    void CopyBuffer(VtxBuffer src, uint64_t srcOffset, VtxBuffer dst, uint64_t dstOffset, uint64_t size);
    void MapMemory(VtxBuffer buffer, void **ppData);
    void UnmapMemory(VtxBuffer buffer);
    void BeginOneTimeCommandBuffer(VkCommandBuffer* pCommandBuffer);
    void EndOneTimeCommandBuffer(VkCommandBuffer commandBuffer);
    void BeginCommandBuffer(VkCommandBufferUsageFlags usage, VkCommandBuffer commandBuffer);
    void EndCommandBuffer(VkCommandBuffer commandBuffer);

private:
    void _InitializeVulkanContextInstance();
    void _InitializeVulkanContextSurface();
    void _InitializeVulkanContextDevice();
    void _InitializeVulkanContextCommandPool();
    void _InitializeVulkanContextMemoryAllocator();
    void _InitializeVulkanContextRenderWindow();
    void _InitializeVulkanContextDescriptorPool();

public:
    VkInstance m_Instance;
    VkSurfaceKHR m_Surface;
    VkDevice m_Device;
    VmaAllocator m_Allocator;
    VtxWindow m_RenderWindow;
    VkCommandPool m_CommandPool;
    VkDescriptorPool m_DescriptorPool;

    Window* m_Window;
    uint32_t ApiVersion;
    VkPhysicalDevice m_PhysicalDevice;
    VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
    VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
    uint32_t m_GraphicsQueueFamilyIndex;
    VkQueue m_GraphicsQueue;
    uint32_t m_PresentQueueFamilyIndex;
    VkQueue m_PresentQueue;
};
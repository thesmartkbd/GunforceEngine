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
    LOGGER_WRITE_DEBUG("VulkanContext initialize begin!");
    m_Window->AddWindowUserPointer(VULKAN_CONTEXT_POINTER, this);
    _InitializeVulkanContextInstance();
    DEBUG_ONLY_RUN_ONCE();
    _InitializeVulkanContextSurface();
    _InitializeVulkanContextDevice();
    _InitializeVulkanContextCommandPool();
    _InitializeVulkanContextMemoryAllocator();
    _InitializeVulkanContextRenderWindow();
    _InitializeVulkanContextDescriptorPool();
    LOGGER_WRITE_DEBUG("VulkanContext initialize end!");
}

VulkanContext::~VulkanContext()
{
    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, VkUtils::Allocator);
    DestroyRenderWindow(m_RenderWindow);
    vmaDestroyAllocator(m_Allocator);
    vkDestroyCommandPool(m_Device, m_CommandPool, VkUtils::Allocator);
    vkDestroyDevice(m_Device, VkUtils::Allocator);
    vkDestroySurfaceKHR(m_Instance, m_Surface, VkUtils::Allocator);
    vkDestroyInstance(m_Instance, VkUtils::Allocator);
    m_Window->RemoveWindowUserPointer(VULKAN_CONTEXT_POINTER);
}

void VulkanContext::CreateSemaphoreV(VkSemaphore *pSemaphore)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_Device, &semaphoreCreateInfo, VkUtils::Allocator, pSemaphore);
}

void VulkanContext::DestroySemaphoreV(VkSemaphore semaphore)
{
    vkDestroySemaphore(m_Device, semaphore, VkUtils::Allocator);
}

void VulkanContext::CreatePipeline(const char *folder, const char *name, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout, VtxPipeline *pPipeline)
{
    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;
    VtxPipeline newPipeline = MemoryMalloc(VtxPipeline_T);

    /* 着色器模块 */
    VkUtils::LoadShaderModule(tochr(strifmt("%s/%s.vert.spv", folder, name)), m_Device, &vertexShaderModule);
    VkUtils::LoadShaderModule(tochr(strifmt("%s/%s.frag.spv", folder, name)), m_Device, &fragmentShaderModule);

    VkPipelineShaderStageCreateInfo pipelineVertexShaderStageCreateInfo = {};
    pipelineVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    pipelineVertexShaderStageCreateInfo.module = vertexShaderModule;
    pipelineVertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo pipelineFragmentStageCreateInfo = {};
    pipelineFragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineFragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    pipelineFragmentStageCreateInfo.module = fragmentShaderModule;
    pipelineFragmentStageCreateInfo.pName = "main";

    Vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions = {
        { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
    };

    Vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions = {
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)   },
        { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) },
        { 2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, uv)    },
    };

    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[] = { pipelineVertexShaderStageCreateInfo, pipelineFragmentStageCreateInfo };

    /* 顶点属性内存描述 */
    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
    pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = std::size(vertexInputBindingDescriptions);
    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = std::data(vertexInputBindingDescriptions);
    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = std::size(vertexInputAttributeDescriptions);
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = std::data(vertexInputAttributeDescriptions);

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssembly = {};
    pipelineInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInputAssembly.primitiveRestartEnable = VK_FALSE;

    /* 视口裁剪 */
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_RenderWindow->width;
    viewport.height = (float) m_RenderWindow->width;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = { m_RenderWindow->width, m_RenderWindow->height };

    VkPipelineViewportStateCreateInfo pipelineViewportStateCrateInfo = {};
    pipelineViewportStateCrateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineViewportStateCrateInfo.viewportCount = 1;
    pipelineViewportStateCrateInfo.pViewports = &viewport;
    pipelineViewportStateCrateInfo.scissorCount = 1;
    pipelineViewportStateCrateInfo.pScissors = &scissor;

    /* 光栅化阶段 */
    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
    pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // Optional
    pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // Optional
    pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // Optional

    /* 多重采样 */
    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {};
    pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f; // Optional
    pipelineMultisampleStateCreateInfo.pSampleMask = nullptr; // Optional
    pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // Optional

    /* 颜色混合 */
    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
    pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
    pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
    pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    /* 帧缓冲 */
    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
    pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
    pipelineColorBlendStateCreateInfo.attachmentCount = 1;
    pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;
    pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f; // Optional
    pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f; // Optional
    pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f; // Optional
    pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f; // Optional

    /* 动态修改 */
    Vector<VkDynamicState> dynamicStates = {
            // VK_DYNAMIC_STATE_VIEWPORT,
            // VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_LINE_WIDTH,
    };

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
    pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineDynamicStateCreateInfo.dynamicStateCount = std::size(dynamicStates);
    pipelineDynamicStateCreateInfo.pDynamicStates = std::data(dynamicStates);

    /* 管道布局 */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    if (descriptorSetLayout != null) {
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    }
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, VkUtils::Allocator, &newPipeline->pipelineLayout);

    /** Create graphics pipeline in vulkan. */
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos;
    graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssembly;
    graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCrateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr; // Optional
    graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo; // Optional
    graphicsPipelineCreateInfo.layout = newPipeline->pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = renderPass;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    graphicsPipelineCreateInfo.basePipelineIndex = -1; // Optional

    VkResult err;
    err = vkCreateGraphicsPipelines(m_Device, null, 1, &graphicsPipelineCreateInfo, VkUtils::Allocator, &newPipeline->pipeline);
    LOGGER_WRITE_DEBUG("VulkanContext create pipeline(VkPipeline): handle=(VkPipeline: 0x%p, VtxPipeline=0x%p), err=%s", newPipeline->pipeline, newPipeline, ERROR_FAIL_V_MMAP(err));

    vkDestroyShaderModule(m_Device, vertexShaderModule, VkUtils::Allocator);
    LOGGER_WRITE_DEBUG("VulkanContext destroy shader module(VKShaderModule): handle=0x%p", vertexShaderModule);
    vkDestroyShaderModule(m_Device, fragmentShaderModule, VkUtils::Allocator);
    LOGGER_WRITE_DEBUG("VulkanContext destroy shader module(VKShaderModule): handle=0x%p", fragmentShaderModule);

    *pPipeline = newPipeline;
}

void VulkanContext::DestroyPipeline(VtxPipeline pipeline)
{
    LOGGER_WRITE_DEBUG("VulkanContext destroy pipeline(VkPipeline): handle=(VkPipeline: 0x%p, VtxPipeline=0x%p)", pipeline->pipeline, pipeline);
    vkDestroyPipeline(m_Device, pipeline->pipeline, VkUtils::Allocator);
    vkDestroyPipelineLayout(m_Device, pipeline->pipelineLayout, VkUtils::Allocator);
    MemoryFree(pipeline);
}

void VulkanContext::CreateDescriptorSet(const Vector<VkDescriptorSetLayout> &layouts, VkDescriptorSet *pDescriptorSet)
{
    LOGGER_WRITE_DEBUG("VulkanContext create descriptor set(VkDescriptorSet): layout count: %ld", std::size(layouts));
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = std::size(layouts);
    descriptorSetAllocateInfo.pSetLayouts = std::data(layouts);
    VkResult err = vkAllocateDescriptorSets(m_Device, &descriptorSetAllocateInfo, pDescriptorSet);
    LOGGER_WRITE_DEBUG("VulkanContext create descriptor set(VkDescriptorSet): handle=0x%p, err=%s", *pDescriptorSet, ERROR_FAIL_V_MMAP(err));
}

void VulkanContext::DestroyDescriptorSet(VkDescriptorSet descriptorSet)
{
    vkFreeDescriptorSets(m_Device, m_DescriptorPool, 1, &descriptorSet);
    LOGGER_WRITE_DEBUG("VulkanContext destroy descriptor set (VkDescriptorSet): handle=0x%p", descriptorSet);
}

void VulkanContext::CreateDescriptorSetLayout(const Vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayout* pDescriptorSetLayout)
{
    LOGGER_WRITE_DEBUG("VulkanContext create descriptor set layout(VkDescriptorSetLayout): binding count: %ld", std::size(bindings));
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = std::size(bindings);
    descriptorSetLayoutCreateInfo.pBindings = std::data(bindings);
    VkResult err;
    err = vkCreateDescriptorSetLayout(m_Device, &descriptorSetLayoutCreateInfo, VkUtils::Allocator, pDescriptorSetLayout);
    LOGGER_WRITE_DEBUG("VulkanContext create descriptor set layout(VkDescriptorSetLayout): handle=0x%p, err=%s", *pDescriptorSetLayout, ERROR_FAIL_V_MMAP(err));
}

void VulkanContext::DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
{
    vkDestroyDescriptorSetLayout(m_Device, descriptorSetLayout, VkUtils::Allocator);
    LOGGER_WRITE_DEBUG("VulkanContext destroy descriptor set layout(VkDescriptorSetLayout): handle=0x%p", descriptorSetLayout);
}

void VulkanContext::RecreateRenderWindow(VtxWindow oldRWindow, VtxWindow* pRWindow)
{
    if (oldRWindow != null) {
        CreateRenderWindow(oldRWindow, pRWindow);
        DestroyRenderWindow(oldRWindow);
    } else {
        DestroyRenderWindow(*pRWindow);
        CreateRenderWindow(null, pRWindow);
    }
}

void VulkanContext::CreateRenderWindow(const VtxWindow oldRWindow, VtxWindow* pRWindow)
{
    LOGGER_WRITE_DEBUG("VulkanContext create RWindow object");
    VtxWindow newVtxWindow = MemoryMalloc(VtxWindow_T);
    VkUtils::ConfigurationSwpachainCapabilities(m_PhysicalDevice, m_Surface, newVtxWindow);
    LOGGER_WRITE_DEBUG("  - Capabilites: ");
    LOGGER_WRITE_DEBUG("    - Format:         %d", newVtxWindow->format);
    LOGGER_WRITE_DEBUG("    - MinImageCount:  %u", newVtxWindow->minImageCount);
    LOGGER_WRITE_DEBUG("    - ColorSpace:     %d", newVtxWindow->colorSpace);
    LOGGER_WRITE_DEBUG("    - PresentMode:    %d", newVtxWindow->presentMode);
    LOGGER_WRITE_DEBUG("    - Transform:      %d", newVtxWindow->transform);
    LOGGER_WRITE_DEBUG("    - Width:          %u", newVtxWindow->width);
    LOGGER_WRITE_DEBUG("    - Height:         %u", newVtxWindow->height);

    /* 设置 vector 大小 */
    newVtxWindow->images.resize(newVtxWindow->minImageCount);
    newVtxWindow->imageViews.resize(newVtxWindow->minImageCount);
    newVtxWindow->framebuffers.resize(newVtxWindow->minImageCount);
    newVtxWindow->commandBuffers.resize(newVtxWindow->minImageCount);

    VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
    swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKHR.surface = newVtxWindow->surface;
    swapchainCreateInfoKHR.minImageCount = newVtxWindow->minImageCount;
    swapchainCreateInfoKHR.imageFormat = newVtxWindow->format;
    swapchainCreateInfoKHR.imageColorSpace = newVtxWindow->colorSpace;
    swapchainCreateInfoKHR.imageExtent = { newVtxWindow->width, newVtxWindow->height };
    swapchainCreateInfoKHR.imageArrayLayers = 1;
    swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKHR.preTransform = newVtxWindow->transform;
    swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKHR.presentMode = newVtxWindow->presentMode;
    swapchainCreateInfoKHR.clipped = VK_TRUE;
    swapchainCreateInfoKHR.oldSwapchain = oldRWindow != null ? oldRWindow->swapchain : null;
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
    
    VkResult err;
    err = vkCreateSwapchainKHR(m_Device, &swapchainCreateInfoKHR, VkUtils::Allocator, &newVtxWindow->swapchain);
    LOGGER_WRITE_DEBUG("  Create swapchain khr: %s", ERROR_FAIL_V_MMAP(err));

    /* create render pass  */
    CreateSemaphoreV(&newVtxWindow->available);
    CreateRenderPass(newVtxWindow->format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &newVtxWindow->renderPass);

    /* about swapcahin image */
    vkGetSwapchainImagesKHR(m_Device, newVtxWindow->swapchain, &newVtxWindow->minImageCount, std::data(newVtxWindow->images));

    for (uint32_t i = 0; i < newVtxWindow->minImageCount; i++) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = newVtxWindow->images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = newVtxWindow->format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        VkResult err;
        err = vkCreateImageView(m_Device, &imageViewCreateInfo, null, &newVtxWindow->imageViews[i]);
        LOGGER_WRITE_DEBUG("Create image view(VkImageView): %s", ERROR_FAIL_V_MMAP(err));

        CreateCommandBuffer(&newVtxWindow->commandBuffers[i]);
        CreateFramebuffer(newVtxWindow->renderPass, newVtxWindow->imageViews[i], newVtxWindow->width, newVtxWindow->height, &newVtxWindow->framebuffers[i]);
    }

    *pRWindow = newVtxWindow;
}

void VulkanContext::DestroyRenderWindow(VtxWindow window)
{
    DestroySemaphoreV(window->available);
    vkDestroyRenderPass(m_Device, window->renderPass, VkUtils::Allocator);
    for (int i = 0; i < window->minImageCount; i++) {
        vkDestroyImageView(m_Device, window->imageViews[i], VkUtils::Allocator);
        vkDestroyFramebuffer(m_Device, window->framebuffers[i], VkUtils::Allocator);
        DestroyCommandBuffer(window->commandBuffers[i]);
    }
    vkDestroySwapchainKHR(m_Device, window->swapchain, VkUtils::Allocator);
    MemoryFree(window);
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

    VkResult err;
    err = vkCreateRenderPass(m_Device, &renderPassCreateInfo, VkUtils::Allocator, pRenderPass);
    LOGGER_WRITE_DEBUG("Create render pass(VkRenderPass): %s", ERROR_FAIL_V_MMAP(err));
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

    VkResult err;
    err = vkCreateFramebuffer(m_Device, &framebufferCreateInfo, VkUtils::Allocator, pFramebuffer);
    LOGGER_WRITE_DEBUG("Create frame buffer(VkFramebuffer): w,h=(%u, %u), err=%s", width, height, ERROR_FAIL_V_MMAP(err));
}

void VulkanContext::DestroyFramebuffer(VkFramebuffer framebuffer)
{
    vkDestroyFramebuffer(m_Device, framebuffer, VkUtils::Allocator);
    LOGGER_WRITE_DEBUG("Destroy frame buffer(VkFramebuffer): 0x%p", framebuffer);
}

void VulkanContext::CreateIndexBuffer(uint64_t size, uint32_t *pIndices, VtxBuffer *pBuffer)
{
    VtxBuffer stagingBuffer;
    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VTX_MEMORY_ALLOCATE_TYPE_CPU, &stagingBuffer);

    void *dst;
    MapMemory(stagingBuffer, &dst);
    memcpy(dst, pIndices, size);
    UnmapMemory(stagingBuffer);

    VtxBuffer vertexBuffer;
    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VTX_MEMORY_ALLOCATE_TYPE_GPU, &vertexBuffer);
    VCmdCopyBuffer(stagingBuffer, 0, vertexBuffer, 0, size);

    DestroyBuffer(stagingBuffer);
    *pBuffer = vertexBuffer;
}

void VulkanContext::CreateVertexBuffer(uint64_t size, Vertex* pVertices, VtxBuffer* pBuffer)
{
    VtxBuffer stagingBuffer;
    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VTX_MEMORY_ALLOCATE_TYPE_CPU, &stagingBuffer);

    void *dst;
    MapMemory(stagingBuffer, &dst);
    memcpy(dst, pVertices, size);
    UnmapMemory(stagingBuffer);

    VtxBuffer vertexBuffer;
    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VTX_MEMORY_ALLOCATE_TYPE_GPU, &vertexBuffer);
    VCmdCopyBuffer(stagingBuffer, 0, vertexBuffer, 0, size);

    DestroyBuffer(stagingBuffer);
    *pBuffer = vertexBuffer;
}

void VulkanContext::CreateBuffer(uint64_t size, VkBufferUsageFlags usage, VtxMemoryAllocateType type, VtxBuffer* pBuffer)
{
    VtxBuffer newBuffer = MemoryMalloc(VtxBuffer_T);
    newBuffer->size = size;

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    switch (type) {
        case VTX_MEMORY_ALLOCATE_TYPE_CPU: {
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            allocInfo.requiredFlags = (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        } break;
        case VTX_MEMORY_ALLOCATE_TYPE_GPU: {
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        } break;
    };

    vmaCreateBuffer(m_Allocator, &bufferCreateInfo, &allocInfo, &newBuffer->buffer, &newBuffer->allocation, null);
    LOGGER_WRITE_DEBUG("VulkanContext create buffer(VtxBuffer): buffer=0x%p, allocation=0x%p, size=%llu", newBuffer->buffer, newBuffer->allocation, size);

    *pBuffer = newBuffer;
}

void VulkanContext::DestroyBuffer(VtxBuffer buffer)
{
    LOGGER_WRITE_DEBUG("VulkanContext destroy buffer(VtxBuffer): buffer=0x%p, allocation=0x%p, size=%llu", buffer->buffer, buffer->allocation, buffer->size);
    vmaDestroyBuffer(m_Allocator, buffer->buffer, buffer->allocation);
    MemoryFree(buffer);
}

void VulkanContext::VCmdCopyBuffer(VtxBuffer src, uint64_t srcOffset, VtxBuffer dst, uint64_t dstOffset, uint64_t size)
{
    VkCommandBuffer copyCommandBuffer;
    BeginOneTimeCommandBuffer(&copyCommandBuffer);

    VkBufferCopy copyInfo = {};
    copyInfo.srcOffset = srcOffset;
    copyInfo.dstOffset = dstOffset;
    copyInfo.size = size;
    vkCmdCopyBuffer(copyCommandBuffer, src->buffer, dst->buffer, 1, &copyInfo);
    LOGGER_WRITE_DEBUG("VulkanContext copy buffer: src: 0x%p, offset: %llu, dst: 0x%p, offset: %llu, size: %llu", src->buffer, srcOffset, dst->buffer, dstOffset, size);

    EndOneTimeCommandBuffer(copyCommandBuffer);
}

void VulkanContext::VCmdBindPipeline(VkCommandBuffer commandBuffer, VtxPipeline pipeline)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
}

void VulkanContext::VCmdBindVertexBuffer(VkCommandBuffer commandBuffer, VtxBuffer buffer)
{
    VkBuffer buffers[] = { buffer->buffer };
    uint64_t offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void VulkanContext::VCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VtxBuffer buffer)
{
    vkCmdBindIndexBuffer(commandBuffer, buffer->buffer, 0, VK_INDEX_TYPE_UINT32);
}

void VulkanContext::VCmdDrawIndexed(VkCommandBuffer commandBuffer, uint64_t size)
{
    vkCmdDrawIndexed(commandBuffer, size, 1, 0, 0, 0);
}

void VulkanContext::VCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t w, uint32_t h)
{
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { w, h };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanContext::VCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t w, uint32_t h)
{
    VkViewport viewport = { 0.0f, 0.0f, (float) w, (float) h, 0.0f, 1.0f };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void VulkanContext::PresentSubmitQueueKHR(VkSemaphore waitSemaphore, uint32_t index, VtxWindow window)
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;

    VkSwapchainKHR swapChains[] = { window->swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &index;
    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(m_PresentQueue, &presentInfo);
    QueueWaitIdle(m_PresentQueue);
}

void VulkanContext::SynchronizeSubmitQueue(VkCommandBuffer commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkPipelineStageFlags waitDstStageMask)
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (waitSemaphore != null) {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &waitSemaphore;
        submitInfo.pWaitDstStageMask = &waitDstStageMask;
    }

    if (signalSemaphore != null) {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphore;
    }

    VkResult err;
    err = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, null);

    if (err != VK_SUCCESS)
        throw new std::runtime_error(strifmt("VulkanContext::SynchronizeSubmitQueue() error, Cause: %s", ERROR_FAIL_V_MMAP(err)));

    QueueWaitIdle(m_GraphicsQueue);
}

void VulkanContext::QueueWaitIdle(VkQueue queue)
{
    vkQueueWaitIdle(queue);
}

void VulkanContext::DeviceWaitIdle()
{
    vkDeviceWaitIdle(m_Device);
}

void VulkanContext::AcquireNextImage(VtxWindow window, uint32_t *pIndex)
{
    vkAcquireNextImageKHR(m_Device, window->swapchain, std::numeric_limits<uint64_t>::max(), window->available, 0, pIndex);
}

void VulkanContext::BeginRenderPass(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkFramebuffer framebuffer, VkRenderPass renderPass)
{
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = framebuffer;
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = { width, height };
    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanContext::EndRenderPass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

void VulkanContext::MapMemory(VtxBuffer buffer, void **ppData)
{
    vmaMapMemory(m_Allocator, buffer->allocation, ppData);
}

void VulkanContext::UnmapMemory(VtxBuffer buffer)
{
    vmaUnmapMemory(m_Allocator, buffer->allocation);
}

void VulkanContext::BeginOneTimeCommandBuffer(VkCommandBuffer* pCommandBuffer)
{
    CreateCommandBuffer(pCommandBuffer);
    BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, *pCommandBuffer);
}

void VulkanContext::EndOneTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
    EndCommandBuffer(commandBuffer);
    SynchronizeSubmitQueue(commandBuffer, null, null, 0);
    DestroyCommandBuffer(commandBuffer);
}

void VulkanContext::BeginCommandBuffer(VkCommandBufferUsageFlags usage, VkCommandBuffer commandBuffer)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = usage;
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
}

void VulkanContext::EndCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);
}

void VulkanContext::CreateCommandBuffer(VkCommandBuffer* pCommandBuffer)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = m_CommandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkResult err;
    err = vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, pCommandBuffer);
    LOGGER_WRITE_DEBUG("VulkanContext allocate command buffer: pool=0x%p, commandBuffer=0x%p, err=%s", m_CommandPool, *pCommandBuffer, ERROR_FAIL_V_MMAP(err));
}

void VulkanContext::DestroyCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
    LOGGER_WRITE_DEBUG("VulkanContext free command buffer: pool=0x%p, commandBuffer=0x%p", m_CommandPool, commandBuffer);
}

void VulkanContext::_InitializeVulkanContextInstance()
{
    LOGGER_WRITE_DEBUG("Begin initialize instance(VkInstance)");
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
    LOGGER_WRITE_DEBUG("  Instance enable extension list:");
    for (const auto& extension : extensions)
        LOGGER_WRITE_DEBUG("    - %s", extension);
    instanceCreateInfo.enabledExtensionCount = std::size(extensions);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(extensions);

    Vector<const char*> layers;
    VkUtils::GetInstanceRequiredEnableLayerProperties(layers);
    LOGGER_WRITE_DEBUG("  Instance enable layer list:");
    for (const auto& layer : layers)
        LOGGER_WRITE_DEBUG("    - %s", layer);
    instanceCreateInfo.enabledLayerCount = std::size(layers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(layers);

    VkResult err = vkCreateInstance(&instanceCreateInfo, VkUtils::Allocator, &m_Instance);
    LOGGER_WRITE_DEBUG("End initialize instance(VkInstance): %d", err);

    vkEnumerateInstanceVersion(&ApiVersion);
    LOGGER_WRITE_DEBUG("Vulkan Instance Version: %d.%d.%d", VK_VERSION_MAJOR(ApiVersion), VK_VERSION_MINOR(ApiVersion), VK_VERSION_PATCH(ApiVersion));
}

void VulkanContext::_InitializeVulkanContextSurface()
{
    LOGGER_WRITE_DEBUG("Begin initialize surface(VkSurface)");
    VkResult err;
    err = glfwCreateWindowSurface(m_Instance, m_Window->GetNativeWindow(), VkUtils::Allocator, &m_Surface);
    LOGGER_WRITE_DEBUG("End initialize surface(VkSurface): %s", ERROR_FAIL_V_MMAP(err));
}

void VulkanContext::_InitializeVulkanContextDevice()
{
    LOGGER_WRITE_DEBUG("Begin initialize device(VkDevice)");
    VkUtils::GetBestPerformancePhysicalDevice(m_Instance, &m_PhysicalDevice);
    VkUtils::GetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties, &m_PhysicalDeviceFeatures);
    LOGGER_WRITE_DEBUG("  Use device name: %s", m_PhysicalDeviceProperties.deviceName);

    VkUtils::QueueFamilyIndices queueFamilyIndices;
    VkUtils::FindQueueFamilyIndices(m_PhysicalDevice, m_Surface, &queueFamilyIndices);
    m_GraphicsQueueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    m_PresentQueueFamilyIndex = queueFamilyIndices.presentQueueFamily;
    LOGGER_WRITE_DEBUG("  Query queue family indices: ");
    LOGGER_WRITE_DEBUG("    - GRAPHICS: %d", m_GraphicsQueueFamilyIndex);
    LOGGER_WRITE_DEBUG("    - PRESENT:  %d", m_PresentQueueFamilyIndex);

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
    LOGGER_WRITE_DEBUG("  Device enable extension list:");
    for (const auto& extension : enableDeviceExtensionProperties)
        LOGGER_WRITE_DEBUG("    - %s", extension);
    deviceCreateInfo.enabledExtensionCount = std::size(enableDeviceExtensionProperties);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(enableDeviceExtensionProperties);

    Vector<const char*> enableDeviceLayerProperties;
    VkUtils::GetDeviceRequiredEnableLayerProperties(m_PhysicalDevice, enableDeviceLayerProperties);
    LOGGER_WRITE_DEBUG("  Device enable layer list:");
    for (const auto& layer : enableDeviceLayerProperties)
        LOGGER_WRITE_DEBUG("    - %s", layer);
    deviceCreateInfo.enabledLayerCount = std::size(enableDeviceLayerProperties);
    deviceCreateInfo.ppEnabledLayerNames = std::data(enableDeviceLayerProperties);

    VkResult err;
    err = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, VkUtils::Allocator, &m_Device);

    /* 获取队列 */
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_PresentQueueFamilyIndex, 0, &m_PresentQueue);
    LOGGER_WRITE_DEBUG("End initialize device(VkDevice): %s", ERROR_FAIL_V_MMAP(err));
}

void VulkanContext::_InitializeVulkanContextCommandPool()
{
    LOGGER_WRITE_DEBUG("Begin initialize CommandPool(VkCommandPool)");
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
    VkResult err;
    err = vkCreateCommandPool(m_Device, &commandPoolCreateInfo, VkUtils::Allocator, &m_CommandPool);
    LOGGER_WRITE_DEBUG("  Use queue index: %d", commandPoolCreateInfo.queueFamilyIndex);
    LOGGER_WRITE_DEBUG("End initialize CommandPool(VkCommandPool): %s", ERROR_FAIL_V_MMAP(err));
}

void VulkanContext::_InitializeVulkanContextMemoryAllocator()
{
    LOGGER_WRITE_DEBUG("Begin initialize memory allocator(VmaAllocator)");
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
    VkResult err;
    err = vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator);
    LOGGER_WRITE_DEBUG("End initialize memory allocator(VmaAllocator): %s", ERROR_FAIL_V_MMAP(err));
}

void VulkanContext::_InitializeVulkanContextRenderWindow()
{
    LOGGER_WRITE_DEBUG("Begin initialize RWindow(VtxWindow)");
    CreateRenderWindow(null, &m_RenderWindow);
    m_Window->AddWindowResizeEventCallback([](Window *window, uint32_t width, uint32_t height) {
        LOGGER_WRITE_DEBUG("VulkanContext: window resize event callback - recreate VtxWindow current size, %u, %u", width, height);
        VulkanContext *vulkanContext = (VulkanContext *) window->GetWindowUserPointer(VULKAN_CONTEXT_POINTER);
        VtxWindow oldRWindow = vulkanContext->m_RenderWindow;
        vulkanContext->RecreateRenderWindow(oldRWindow, &vulkanContext->m_RenderWindow);
    });
    LOGGER_WRITE_DEBUG("End initialize RWindow(VtxWindow)");
}

void VulkanContext::_InitializeVulkanContextDescriptorPool()
{
    LOGGER_WRITE_DEBUG("Begin initialize DescriptorPool(VkDescriptorPool)");
    Vector<VkDescriptorPoolSize> poolSize = {
         { VK_DESCRIPTOR_TYPE_SAMPLER,                64 },
         { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64 },
         { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          64 },
         { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          64 },
         { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   64 },
         { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   64 },
         { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         64 },
         { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         64 },
         { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 64 },
         { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 64 },
         { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       64 },
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolCreateInfo.maxSets = 1024;
    descriptorPoolCreateInfo.poolSizeCount = std::size(poolSize);
    descriptorPoolCreateInfo.pPoolSizes = std::data(poolSize);

    LOGGER_WRITE_DEBUG("  DescriptorPool size: ");
    for (const auto &item: poolSize)
        LOGGER_WRITE_DEBUG("    type: %d, count: %d", item.type, item.descriptorCount);
    LOGGER_WRITE_DEBUG("  DescriptorPool max sets: %d", descriptorPoolCreateInfo.maxSets);

    VkResult err;
    err = vkCreateDescriptorPool(m_Device, &descriptorPoolCreateInfo, VkUtils::Allocator, &m_DescriptorPool);
    LOGGER_WRITE_DEBUG("End initialize DescriptorPool(VkDescriptorPool): %s", ERROR_FAIL_V_MMAP(err));
}
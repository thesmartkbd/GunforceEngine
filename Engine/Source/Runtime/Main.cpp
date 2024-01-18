/* -------------------------------------------------------------------------------- *\
|*                               Gunforce Engine                                    *|
\* -------------------------------------------------------------------------------- */

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
|* File:           Main.cpp                                                         *|
|* Create Time:    2024/01/10 16:08                                                 *|
|* Author:         thesmartkbd                                                      *|
|* EMail:          thesmartkbd@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include "Window/Window.h"
#include "Drivers/Vulkan/VulkanContext.h"
#include <Logger.h>

int main()
{
	/* 设置控制台中文字符 */
	system("chcp 65001");

	std::unique_ptr<Window> window = std::make_unique<Window>(800, 600, "枪神引擎");
	std::unique_ptr<VulkanContext> vulkanContext = std::make_unique<VulkanContext>(window.get());

    VkSemaphore semaphore;
    VtxPipeline pipeline;
    VtxWindow windowV;
    VtxBuffer vertexBuffer;
    VtxBuffer indexBuffer;

    std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f,  -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f,  0.5f,  0.0f},  {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f,  0.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    };

    std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
    };

    vulkanContext->CreateVertexBuffer(ARRAY_SIZE(vertices), std::data(vertices), &vertexBuffer);
    vulkanContext->CreateIndexBuffer(ARRAY_SIZE(indices), std::data(indices), &indexBuffer);

    windowV = vulkanContext->GetCurrentContextVtxWindow();
    vulkanContext->CreateSemaphoreV(&semaphore);
    vulkanContext->CreatePipeline("../Engine/Source/Shader", "simple", windowV->renderPass, null, &pipeline);

	while (!window->IsShouldClose()) {
        uint32_t index;
        VkCommandBuffer commandBuffer;
        vulkanContext->AcquireNextImage(windowV, &index);
        commandBuffer = windowV->commandBuffers[index];
        vulkanContext->BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, commandBuffer);
        {
            vulkanContext->BeginRenderPass(commandBuffer, windowV->width, windowV->height, windowV->framebuffers[index], windowV->renderPass);
            {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
                uint64_t offsets[] = { 0 };
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer->buffer, offsets);
                vkCmdBindIndexBuffer(commandBuffer, indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(commandBuffer, std::size(indices), 1, 0, 0, 0);
            }
            vulkanContext->EndRenderPass(commandBuffer);
        }
        vulkanContext->EndCommandBuffer(commandBuffer);
        vulkanContext->SynchronizeSubmitQueue(commandBuffer, windowV->available, semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        vulkanContext->PresentSubmitQueueKHR(semaphore, index, windowV);

		Window::PollEvents();
	}

    vulkanContext->DestroyPipeline(pipeline);
    vulkanContext->DestroySemaphoreV(semaphore);

	return 0;
}
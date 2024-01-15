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

    VulkanContext::Buffer buffer;
    vulkanContext->CreateBuffer(255, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, &buffer);

    void *data;
    vulkanContext->MapMemory(buffer, &data);
    long x = 114514;
    memcpy(data, &x, sizeof(long));
    vulkanContext->UnmapMemory(buffer);

    void *data0;
    vulkanContext->MapMemory(buffer, &data0);
    printf("data0: %ld\n", *((long *) data0));
    vulkanContext->UnmapMemory(buffer);

    vulkanContext->DestroyBuffer(buffer);

	while (!window->IsShouldClose()) {
		Window::PollEvents();
	}

	return 0;
}
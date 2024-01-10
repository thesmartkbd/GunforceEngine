/* -------------------------------------------------------------------------------- *\
|*                               Gunforce Engine                                    *|
\* -------------------------------------------------------------------------------- */

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
|* File:           Main.cpp                                                         *|
|* Create Time:    2024/01/10 16:08                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include "Window/GunforceWindow.h"
#include "Drivers/Vulkan/VulkanContext.h"

int main()
{
	/* 设置控制台中文字符 */
	system("chcp 65001");

	std::unique_ptr<GunforceWindow> window = std::make_unique<GunforceWindow>(800, 600, "枪神引擎");
	std::unique_ptr<VulkanContext> vulkanContext = std::make_unique<VulkanContext>(window.get());

	while (!window->IsShouldClose()) {
		GunforceWindow::PollEvents();
	}

	return 0;
}
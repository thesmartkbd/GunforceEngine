# /* -------------------------------------------------------------------------------- *\
# |*                               Gunforce Engine                                    *|
# \* -------------------------------------------------------------------------------- */

# /* -------------------------------------------------------------------------------- *\
# |*                                                                                  *|
# |*    Copyright (C) 2023 thesmartkbd                                                *|
# |*                                                                                  *|
# |*    This program is free software: you can redistribute it and/or modify          *|
# |*    it under the terms of the GNU General Public License as published by          *|
# |*    the Free Software Foundation, either version 3 of the License, or             *|
# |*    (at your option) any later version.                                           *|
# |*                                                                                  *|
# |*    This program is distributed in the hope that it will be useful,               *|
# |*    but WITHOUT ANY WARRANTY; without even the implied warranty of                *|
# |*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *|
# |*    GNU General Public License for more details.                                  *|
# |*                                                                                  *|
# |*    You should have received a copy of the GNU General Public License             *|
# |*    along with this program.  If not, see <https://www.gnu.org/licenses/>.        *|
# |*                                                                                  *|
# |*    This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.    *|
# |*    This is free software, and you are welcome to redistribute it                 *|
# |*    under certain conditions; type `show c' for details.                          *|
# |*                                                                                  *|
# \* -------------------------------------------------------------------------------- */

# /* -------------------------------------------------------------------------------- *\
# |*                                                                                  *|
# |* File:           CMakeLists.txt                                                   *|
# |* Create Time:    2023/12/27 16:47                                                 *|
# |* Author:         thesmartkbd                                                      *|
# |* EMail:          thesmartkbd@hotmail.com                                          *|
# |*                                                                                  *|
# \* -------------------------------------------------------------------------------- */
cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 23)
project("GunforceEngine")

set(COMPILER ${CMAKE_CXX_COMPILER_ID})
set(GLFW_PATH Engine/Source/ThirdParty/glfw3)
set(VULKAN_PATH Engine/Source/ThirdParty/vulkan)

include_directories(SYSTEM Engine/Source/Include)
include_directories(SYSTEM Engine/Source/ThirdParty)
include_directories(SYSTEM ${GLFW_PATH}/include)
include_directories(SYSTEM ${VULKAN_PATH}/Include)

include_directories(Engine/Source/Runtime)

if(COMPILER STREQUAL "MSVC")
  set(GLFW_LIBRARY_DIR ${GLFW_PATH}/lib-vc2022)
else()
  set(GLFW_LIBRARY_DIR ${GLFW_PATH}/lib-mingw-w64)
endif()

link_directories("${VULKAN_PATH}/Lib")
link_directories(${GLFW_LIBRARY_DIR})

add_executable(${PROJECT_NAME}
  Engine/Source/Include/pch.cpp
  Engine/Source/Runtime/Main.cpp
  Engine/Source/Runtime/Window/Window.cpp
  Engine/Source/Runtime/Drivers/Vulkan/VulkanContext.cpp
)

target_link_libraries(${PROJECT_NAME}
  glfw3
  vulkan-1
)
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
# |* Create Time:    2024/01/19 13:09                                                 *|
# |* Author:         thesmartkbd                                                      *|
# |* EMail:          thesmartkbd@hotmail.com                                          *|
# |*                                                                                  *|
# \* -------------------------------------------------------------------------------- */
cmake_minimum_required(VERSION 3.8)
set(CMAKE_C_STANDARD 23)
project(vkcube)

set(THIRD_PARTY_DIRECTORY ../../Source/ThirdParty)

include_directories(${THIRD_PARTY_DIRECTORY}/Vulkan/Include)
include_directories(${THIRD_PARTY_DIRECTORY}/volk)

link_directories(${THIRD_PARTY_DIRECTORY}/Vulkan/Lib)

add_executable(${PROJECT_NAME}
    cube.c
)

target_link_libraries(${PROJECT_NAME}
  vulkan-1
)
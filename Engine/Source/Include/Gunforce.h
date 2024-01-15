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
|* File:           Gunforce.h                                                       *|
|* Create Time:    2024/01/03 01:22                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include <Typedef.h>
#include <Logger.h>
#include <glm/glm.hpp>

#define GUNFORCE_ENGINE_NAME "GunforceEngine"

#ifndef GUNFORCEAPI
#  define GUNFORCEAPI
#endif

/* 开启引擎调试 */
#ifndef GUNFORCE_ENGINE_ENABLE_DEBUG
#  define GUNFORCE_ENGINE_ENABLE_DEBUG
#endif

/* 日志打印 */
#define GUNFORCE_CONSOLE_LOGGER_INFO(...) Logger::Info(GUNFORCE_ENGINE_NAME, __VA_ARGS__)
#define GUNFORCE_CONSOLE_LOGGER_DEBUG(...) Logger::Debug(GUNFORCE_ENGINE_NAME, __VA_ARGS__)
#define GUNFORCE_CONSOLE_LOGGER_WARN(...) Logger::Warn(GUNFORCE_ENGINE_NAME, __VA_ARGS__)
#define GUNFORCE_CONSOLE_LOGGER_ERROR(...) Logger::Error(GUNFORCE_ENGINE_NAME, __VA_ARGS__)
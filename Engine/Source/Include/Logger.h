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
|* File:           Logger.h                                                         *|
|* Create Time:    2023/12/30 21:07                                                 *|
|* Author:         thesmartkbd                                                      *|
|* EMail:          thesmartkbd@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include "Color.h"
#include "System.h"
#include "Date.h"
#include "IOUtils.h"
// std
#include <stdexcept>

namespace Logger
{
    static void _logger_console_write(const char *level, const char *color, const char * caller, const char* file, int line, const char *fmt, va_list va)
    {
        char time[32];
        Date::Format(time, "%Y-%m-%d %H:%M:%S", sizeof(time));
#ifdef ENABLE_LOGGER_TRACE
        System::ConsoleWrite("%s %s%s%s [%s] %s(%d): %s", time, color, level, ASCII_COLOR_RESET, caller, file, line, tochr(vstrifmt(fmt, va)));
#else
        System::ConsoleWrite("%s %s%s%s: %s", time, color, level, ASCII_COLOR_RESET, tochr(vstrifmt(fmt, va)));
#endif
    }

    static void Info(const char *caller, const char *file, int line, const char *fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        _logger_console_write("INFO ", ASCII_COLOR_GREEN, caller, file, line, fmt, va);
        va_end(va);
    }

    static void Debug(const char* caller, const char* file, int line, const char* fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        _logger_console_write("DEBUG", ASCII_COLOR_BLUE, caller, file, line, fmt, va);
        va_end(va);
    }

    static void Warn(const char* caller, const char* file, int line, const char* fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        _logger_console_write("WARN ", ASCII_COLOR_YELLOW, caller, file, line, fmt, va);
        va_end(va);
    }

    static void Error(const char* caller, const char* file, int line, const char* fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        _logger_console_write("ERROR", ASCII_COLOR_RED, caller, file, line, fmt, va);
        va_end(va);
    }
}

#if defined(ENABLE_LOGGER_WRITE)
#  define LOGGER_WRITE_INFO(fmt, ...)  Logger::Info (__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#  define LOGGER_WRITE_DEBUG(fmt, ...) Logger::Debug(__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#  define LOGGER_WRITE_WARN(fmt, ...)  Logger::Warn (__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#  define LOGGER_WRITE_ERROR(fmt, ...) Logger::Error(__FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#  define LOGGER_WRITE_INFO(fmt, ...)
#  define LOGGER_WRITE_DEBUG(fmt, ...)
#  define LOGGER_WRITE_WARN(fmt, ...)
#  define LOGGER_WRITE_ERROR(fmt, ...)
#endif
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
|* File:           Typedef.h                                                        *|
|* Create Time:    2023/12/30 21:07                                                 *|
|* Author:         thesmartkbd                                                      *|
|* EMail:          thesmartkbd@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include <array>
#include <vector>
#include <unordered_map>
#include <string>
#include <format>
#include <stdarg.h>
#include <iostream>
#include <memory>

/* 启用日志打印 */
#define ENABLE_LOGGER_WRITE
/* 日志开启文件路径追踪 */
// #define ENABLE_LOGGER_TRACE

/* 强制内联 */
#ifndef __forceinline__
#  define __forceinline__ __forceinline
#endif

/* std::vector<T> 标准库封装 */
template<typename T>
class Vector : public std::vector<T> {
public:
    using std::vector<T>::vector;
    __forceinline__
    void remove(size_t index)
      {
        this->erase(this->begin() + index);
      }
};

/* std::unordered_map<K, V> 标准库封装 */
template<typename K, typename V>
class HashMap : public std::unordered_map<K, V> {
public:
    using std::unordered_map<K, V>::unordered_map;
    __forceinline__
    void remove(const K &k)
      {
        this->erase(k);
      }
};

/* 字符串格式化 */
static std::string vstrifmt(const char* fmt, va_list va)
{
    char buf[(1024 * 4) + 1];
    vsnprintf(buf, sizeof(buf), fmt, va);
    return std::string(buf);
}

static std::string strifmt(const char *fmt, ...)
{
    std::string ret;

    va_list va;
    va_start(va, fmt);
    ret = vstrifmt(fmt, va);
    va_end(va);

    return ret;
}

#define tochr(str) ( str.c_str() )

/* NULl 宏定义 */
#ifndef null
#  if defined(VK_VERSION_1_0)
#    define null VK_NULL_HANDLE
#  else
#    define null nullptr
#  endif
#endif

/* malloc */
#define MemoryMalloc(obj) (obj *) new obj
#define MemoryFree(ptr) delete ptr
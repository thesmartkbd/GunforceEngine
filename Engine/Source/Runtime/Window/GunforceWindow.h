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
|* File:           GunforceWindow.h                                                 *|
|* Create Time:    2024/01/10 16:59                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include <Logger.h>
#include <GLFW/glfw3.h>

class GunforceWindow;

typedef void (*PFN_WindowResizeEventCallback)(GunforceWindow* win, uint32_t w, uint32_t h);

class GunforceWindow {
public:
    GunforceWindow(uint32_t width, uint32_t height, const char* title);
    ~GunforceWindow();

public:
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    bool IsShouldClose() const;
    size_t AddResizeEventCallback(PFN_WindowResizeEventCallback callback);
    void RemoveResizeEventCalback(size_t index);

public:
    /* 事件轮询 */
    static void PollEvents()
      {
        glfwPollEvents()
            ;
      }

private:
    GLFWwindow* m_HWIN;
    uint32_t m_Width;
    uint32_t m_Height;

    Vector<PFN_WindowResizeEventCallback> m_WindowResizeEventCallbacks;
};
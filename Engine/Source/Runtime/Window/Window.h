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
|* File:           Window.h                                                 *|
|* Create Time:    2024/01/10 16:59                                                 *|
|* Author:         thesmartkbd                                                      *|
|* EMail:          thesmartkbd@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include <GLFW/glfw3.h>
#include <Gunforce.h>

class Window;

typedef void (*PFN_WindowResizeEventCallback)(Window* win, uint32_t w, uint32_t h);

class Window {
public:
    Window(uint32_t width, uint32_t height, const char* title);
    ~Window();

public:
    void AddWindowUserPointer(std::string name, void *pointer);
    void* GetWindowUserPointer(const std::string &name);
    void RemoveWindowUserPointer(const std::string &name);
    GLFWwindow* GetNativeWindow() const { return m_HWIN; };
    uint32_t GetWidth() const { return m_Width; };
    uint32_t GetHeight() const { return m_Height; };
    bool IsShouldClose() const { return glfwWindowShouldClose(m_HWIN); };
    size_t AddWindowResizeEventCallback(PFN_WindowResizeEventCallback callback);
    void RemoveWindowResizeEventCalback(size_t index);

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

    HashMap<std::string, void*> m_WindowUserPointers;
    Vector<PFN_WindowResizeEventCallback> m_WindowResizeEventCallbacks;
};
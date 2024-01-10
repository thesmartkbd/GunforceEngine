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
|* File:           GunforceWindow.cpp                                                       *|
|* Create Time:    2024/01/10 16:59                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include "GunforceWindow.h"

GunforceWindow::GunforceWindow(uint32_t width, uint32_t height, const char* title) : m_Width(width), m_Height(height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if ((m_HWIN = glfwCreateWindow(width, height, title, null, null)) == null)
        Logger::Error("Create glfw3 window failed!");

    glfwSetWindowUserPointer(m_HWIN, this);

    glfwSetWindowSizeCallback(m_HWIN, [](GLFWwindow *p_glwin, int w, int h) {
        GunforceWindow *p_gunforce_window = (GunforceWindow *) glfwGetWindowUserPointer(p_glwin);
        for (const auto& callback : p_gunforce_window->m_WindowResizeEventCallbacks)
            callback(p_gunforce_window, w, h);
    });
}

GunforceWindow::~GunforceWindow()
{
    glfwDestroyWindow(m_HWIN);
    glfwTerminate();
}

uint32_t GunforceWindow::GetWidth()
{
    return m_Width;
}

uint32_t GunforceWindow::GetHeight()
{
    return m_Height;
}

bool GunforceWindow::IsShouldClose()
{
    return glfwWindowShouldClose(m_HWIN);
}

size_t GunforceWindow::AddWindowResizeEventCallback(PFN_WindowResizeEventCallback callback)
{
    m_WindowResizeEventCallbacks.push_back(callback);
    return std::size(m_WindowResizeEventCallbacks);
}

void GunforceWindow::RemoveWindowResizeEventCalback(size_t index)
{
    m_WindowResizeEventCallbacks.remove(index);
}
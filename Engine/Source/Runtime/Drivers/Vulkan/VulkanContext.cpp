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
|* File:           VulkanContext.cpp                                                *|
|* Create Time:    2024/01/03 01:35                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include "VulkanContext.h"
#include "Window/GunforceWindow.h"
#include "VulkanUtils.h"

VulkanContext::VulkanContext(GunforceWindow *p_window) : m_GunforceWindow(p_window)
{
    GUNFORCE_CONSOLE_LOGGER_INFO("VulkanContext initialize begin!");
    _InitVulkanContextInstance();
    _InitVulkanContextSurface();
    _InitVulkanContextDevice();
    _InitVulkanContextSwapchain();
    GUNFORCE_CONSOLE_LOGGER_INFO("VulkanContext initialize end!");
}

VulkanContext::~VulkanContext()
{
    vkDestroyDevice(m_Device, VulkanUtils::Allocator);
    vkDestroySurfaceKHR(m_Instance, m_Surface, VulkanUtils::Allocator);
    vkDestroyInstance(m_Instance, VulkanUtils::Allocator);
}

void VulkanContext::_InitVulkanContextInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = GUNFORCE_ENGINE_NAME;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = GUNFORCE_ENGINE_NAME;
    applicationInfo.apiVersion = VK_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    Vector<const char*> extensions;
    VulkanUtils::GetInstanceRequiredEnableExtensionProperties(extensions);
    GUNFORCE_CONSOLE_LOGGER_INFO("VulkanContext instance enable extension list:");
    for (const auto& extension : extensions)
        GUNFORCE_CONSOLE_LOGGER_INFO("  - {}", extension);
    instanceCreateInfo.enabledExtensionCount = std::size(extensions);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(extensions);

    Vector<const char*> layers;
    VulkanUtils::GetInstanceRequiredEnableLayerProperties(layers);
    GUNFORCE_CONSOLE_LOGGER_INFO("VulkanContext instance enable layer list:");
    for (const auto& layer : layers)
        GUNFORCE_CONSOLE_LOGGER_INFO("  - {}", layer);
    instanceCreateInfo.enabledLayerCount = std::size(layers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(layers);

    vkCheckCreate(Instance, &instanceCreateInfo, VulkanUtils::Allocator, &m_Instance);
}

void VulkanContext::_InitVulkanContextSurface()
{
    glfwCreateWindowSurface(m_Instance, m_GunforceWindow->GetNativeWindow(), VulkanUtils::Allocator, &m_Surface);
}

void VulkanContext::_InitVulkanContextDevice()
{
    VulkanUtils::GetBestPerformancePhysicalDevice(m_Instance, &m_PhysicalDevice);
    VulkanUtils::GetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties, &m_PhysicalDeviceFeatures);
    GUNFORCE_CONSOLE_LOGGER_INFO("VulkanContext physical device gpu using: {}", m_PhysicalDeviceProperties.deviceName);

    VulkanUtils::QueueFamilyIndices queueFamilyIndices;
    VulkanUtils::FindQueueFamilyIndices(m_PhysicalDevice, m_Surface, &queueFamilyIndices);
    m_GraphicsQueueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    m_PresentQueueFamilyIndex = queueFamilyIndices.presentQueueFamily;

    float priorities = 1.0f;
    std::array<VkDeviceQueueCreateInfo, 2> deviceQueueCreateInfos = {};
    deviceQueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfos[0].queueCount = 1;
    deviceQueueCreateInfos[0].queueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    deviceQueueCreateInfos[0].pQueuePriorities = &priorities;

    deviceQueueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfos[1].queueCount = 1;
    deviceQueueCreateInfos[1].queueFamilyIndex = queueFamilyIndices.presentQueueFamily;
    deviceQueueCreateInfos[1].pQueuePriorities = &priorities;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = std::size(deviceQueueCreateInfos);
    deviceCreateInfo.pQueueCreateInfos = std::data(deviceQueueCreateInfos);
    static VkPhysicalDeviceFeatures features = {};
    deviceCreateInfo.pEnabledFeatures = &features;

    Vector<const char*> enableDeviceExtensionProperties;
    VulkanUtils::GetDeviceRequiredEnableExtensionProperties(m_PhysicalDevice, enableDeviceExtensionProperties);
    deviceCreateInfo.enabledExtensionCount = std::size(enableDeviceExtensionProperties);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(enableDeviceExtensionProperties);

    Vector<const char*> enableDeviceLayerProperties;
    VulkanUtils::GetDeviceRequiredEnableLayerProperties(m_PhysicalDevice, enableDeviceLayerProperties);
    deviceCreateInfo.enabledLayerCount = std::size(enableDeviceLayerProperties);
    deviceCreateInfo.ppEnabledLayerNames = std::data(enableDeviceLayerProperties);

    vkCheckCreate(Device, m_PhysicalDevice, &deviceCreateInfo, VulkanUtils::Allocator, &m_Device);

    /* 获取队列 */
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_PresentQueueFamilyIndex, 0, &m_PresentQueue);
}

void VulkanContext::_InitVulkanContextSwapchain()
{

}
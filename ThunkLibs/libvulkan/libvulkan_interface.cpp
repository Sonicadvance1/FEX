#include <common/GeneratorInterface.h>

#include <type_traits>

template<auto>
struct fex_gen_config {
  unsigned version = 1;
};

// Some of Vulkan's handle types are so-called "non-dispatchable handles".
// On 64-bit, these are defined as dedicated types by default, which makes
// annotating these handle types unnecessarily complicated. Instead, setting
// the following define will make the Vulkan headers alias all handle types
// to uint64_t.
#define VK_USE_64_BIT_PTR_DEFINES 0

#define VK_USE_PLATFORM_XLIB_XRANDR_EXT
#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>

template<>
struct fex_gen_config<vkGetDeviceProcAddr> : fexgen::custom_host_impl, fexgen::custom_guest_entrypoint, fexgen::returns_guest_pointer {};
template<>
struct fex_gen_config<vkGetInstanceProcAddr> : fexgen::custom_host_impl, fexgen::custom_guest_entrypoint, fexgen::returns_guest_pointer {};

template<typename>
struct fex_gen_type {};

// internal use
void SetGuestXSync(uintptr_t, uintptr_t);
void SetGuestXGetVisualInfo(uintptr_t, uintptr_t);
void SetGuestXDisplayString(uintptr_t, uintptr_t);
template<>
struct fex_gen_config<SetGuestXSync> : fexgen::custom_guest_entrypoint, fexgen::custom_host_impl {};
template<>
struct fex_gen_config<SetGuestXGetVisualInfo> : fexgen::custom_guest_entrypoint, fexgen::custom_host_impl {};
template<>
struct fex_gen_config<SetGuestXDisplayString> : fexgen::custom_guest_entrypoint, fexgen::custom_host_impl {};

// So-called "dispatchable" handles are represented as opaque pointers.
// In addition to marking them as such, API functions that create these objects
// need special care since they wrap these handles in another pointer, which
// the thunk generator can't automatically handle.
//
// So-called "non-dispatchable" handles don't need this extra treatment, since
// they are uint64_t IDs on both 32-bit and 64-bit systems.
template<>
struct fex_gen_type<VkCommandBuffer_T> : fexgen::opaque_type {};
template<>
struct fex_gen_type<VkDevice_T> : fexgen::opaque_type {};
template<>
struct fex_gen_type<VkInstance_T> : fexgen::opaque_type {};
template<>
struct fex_gen_type<VkPhysicalDevice_T> : fexgen::opaque_type {};
template<>
struct fex_gen_type<VkQueue_T> : fexgen::opaque_type {};

// Mark union types with compatible layout as such
// TODO: These may still have different alignment requirements!
template<>
struct fex_gen_type<VkClearValue> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_type<VkClearColorValue> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_type<VkPipelineExecutableStatisticValueKHR> : fexgen::assume_compatible_data_layout {};
#ifndef IS_32BIT_THUNK
template<>
struct fex_gen_type<VkAccelerationStructureGeometryDataKHR> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_type<VkDescriptorDataEXT> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_type<VkDeviceOrHostAddressKHR> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_type<VkDeviceOrHostAddressConstKHR> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_type<VkPerformanceValueDataINTEL> : fexgen::assume_compatible_data_layout {};
#endif

// Explicitly register types that are only ever referenced through nested pointers
template<>
struct fex_gen_type<VkAccelerationStructureBuildRangeInfoKHR> {};

// Structures that contain function pointers
// TODO: Use custom repacking for these instead
template<>
struct fex_gen_type<VkDebugReportCallbackCreateInfoEXT> : fexgen::emit_layout_wrappers {};
template<>
struct fex_gen_type<VkDebugUtilsMessengerCreateInfoEXT> : fexgen::emit_layout_wrappers {};

#ifndef IS_32BIT_THUNK
// The pNext member of this is a pointer to another VkBaseOutStructure, so data layout compatibility can't be inferred automatically
template<>
struct fex_gen_type<VkBaseOutStructure> : fexgen::assume_compatible_data_layout {};
#endif


// TODO: Should not be opaque, but it's usually NULL anyway. Supporting the contained function pointers will need more work.
template<>
struct fex_gen_type<VkAllocationCallbacks> : fexgen::opaque_type {};

// X11 interop
template<>
struct fex_gen_config<&VkXcbSurfaceCreateInfoKHR::connection> : fexgen::custom_repack {};
template<>
struct fex_gen_config<&VkXlibSurfaceCreateInfoKHR::dpy> : fexgen::custom_repack {};

// Wayland interop
template<>
struct fex_gen_type<wl_display> : fexgen::opaque_type {};
template<>
struct fex_gen_type<wl_surface> : fexgen::opaque_type {};

namespace internal {

// Function, parameter index, parameter type [optional]
template<auto, int, typename = void>
struct fex_gen_param {};

template<auto>
struct fex_gen_config : fexgen::generate_guest_symtable, fexgen::indirect_guest_calls {};

template<>
struct fex_gen_config<vkCreateInstance> : fexgen::custom_host_impl {};
template<>
struct fex_gen_param<vkCreateInstance, 2, VkInstance*> : fexgen::ptr_passthrough {};
template<>
struct fex_gen_config<vkDestroyInstance> {};
template<>
struct fex_gen_config<vkEnumeratePhysicalDevices> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceFeatures> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceFormatProperties> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceImageFormatProperties> {};
// TODO: Output parameter must repack on exit!
template<>
struct fex_gen_config<vkGetPhysicalDeviceProperties> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceQueueFamilyProperties> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceMemoryProperties> {};
template<>
struct fex_gen_config<vkCreateDevice> : fexgen::custom_host_impl {};
template<>
struct fex_gen_param<vkCreateDevice, 3, VkDevice*> : fexgen::ptr_passthrough {};
template<>
struct fex_gen_config<vkDestroyDevice> {};
template<>
struct fex_gen_config<vkEnumerateInstanceExtensionProperties> {};
template<>
struct fex_gen_config<vkEnumerateDeviceExtensionProperties> {};
template<>
struct fex_gen_config<vkEnumerateInstanceLayerProperties> {};
template<>
struct fex_gen_config<vkEnumerateDeviceLayerProperties> {};
template<>
struct fex_gen_config<vkGetDeviceQueue> {};
template<>
struct fex_gen_config<vkQueueSubmit> {};
template<>
struct fex_gen_config<vkQueueWaitIdle> {};
template<>
struct fex_gen_config<vkDeviceWaitIdle> {};
template<>
struct fex_gen_config<vkAllocateMemory> : fexgen::custom_host_impl {};
template<>
struct fex_gen_config<vkFreeMemory> : fexgen::custom_host_impl {};
template<>
struct fex_gen_config<vkMapMemory> {};
template<>
struct fex_gen_config<vkUnmapMemory> {};
template<>
struct fex_gen_config<vkFlushMappedMemoryRanges> {};
template<>
struct fex_gen_config<vkInvalidateMappedMemoryRanges> {};
template<>
struct fex_gen_config<vkGetDeviceMemoryCommitment> {};
template<>
struct fex_gen_config<vkBindBufferMemory> {};
template<>
struct fex_gen_config<vkBindImageMemory> {};
template<>
struct fex_gen_config<vkGetBufferMemoryRequirements> {};
template<>
struct fex_gen_config<vkGetImageMemoryRequirements> {};
template<>
struct fex_gen_config<vkGetImageSparseMemoryRequirements> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSparseImageFormatProperties> {};
template<>
struct fex_gen_config<vkQueueBindSparse> {};
template<>
struct fex_gen_config<vkCreateFence> {};
template<>
struct fex_gen_config<vkDestroyFence> {};
template<>
struct fex_gen_config<vkResetFences> {};
template<>
struct fex_gen_config<vkGetFenceStatus> {};
template<>
struct fex_gen_config<vkWaitForFences> {};
template<>
struct fex_gen_config<vkCreateSemaphore> {};
template<>
struct fex_gen_config<vkDestroySemaphore> {};
template<>
struct fex_gen_config<vkCreateEvent> {};
template<>
struct fex_gen_config<vkDestroyEvent> {};
template<>
struct fex_gen_config<vkGetEventStatus> {};
template<>
struct fex_gen_config<vkSetEvent> {};
template<>
struct fex_gen_config<vkResetEvent> {};
template<>
struct fex_gen_config<vkCreateQueryPool> {};
template<>
struct fex_gen_config<vkDestroyQueryPool> {};
template<>
struct fex_gen_config<vkGetQueryPoolResults> {};
template<>
struct fex_gen_param<vkGetQueryPoolResults, 5, void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkCreateBuffer> {};
template<>
struct fex_gen_config<vkDestroyBuffer> {};
template<>
struct fex_gen_config<vkCreateBufferView> {};
template<>
struct fex_gen_config<vkDestroyBufferView> {};
template<>
struct fex_gen_config<vkCreateImage> {};
template<>
struct fex_gen_config<vkDestroyImage> {};
template<>
struct fex_gen_config<vkGetImageSubresourceLayout> {};
template<>
struct fex_gen_config<vkCreateImageView> {};
template<>
struct fex_gen_config<vkDestroyImageView> {};
template<>
struct fex_gen_config<vkCreateShaderModule> : fexgen::custom_host_impl {};
template<>
struct fex_gen_config<vkDestroyShaderModule> {};
template<>
struct fex_gen_config<vkCreatePipelineCache> {};
template<>
struct fex_gen_config<vkDestroyPipelineCache> {};
template<>
struct fex_gen_config<vkGetPipelineCacheData> {};
template<>
struct fex_gen_config<vkMergePipelineCaches> {};
template<>
struct fex_gen_config<vkCreateGraphicsPipelines> {};
template<>
struct fex_gen_config<vkCreateComputePipelines> {};
template<>
struct fex_gen_config<vkDestroyPipeline> {};
template<>
struct fex_gen_config<vkCreatePipelineLayout> {};
template<>
struct fex_gen_config<vkDestroyPipelineLayout> {};
template<>
struct fex_gen_config<vkCreateSampler> {};
template<>
struct fex_gen_config<vkDestroySampler> {};
template<>
struct fex_gen_config<vkCreateDescriptorSetLayout> {};
template<>
struct fex_gen_config<vkDestroyDescriptorSetLayout> {};
template<>
struct fex_gen_config<vkCreateDescriptorPool> {};
template<>
struct fex_gen_config<vkDestroyDescriptorPool> {};
template<>
struct fex_gen_config<vkResetDescriptorPool> {};
template<>
struct fex_gen_config<vkAllocateDescriptorSets> {};
template<>
struct fex_gen_config<vkFreeDescriptorSets> {};
template<>
struct fex_gen_config<vkUpdateDescriptorSets> {};
template<>
struct fex_gen_config<vkCreateFramebuffer> {};
template<>
struct fex_gen_config<vkDestroyFramebuffer> {};
template<>
struct fex_gen_config<vkCreateRenderPass> {};
template<>
struct fex_gen_config<vkDestroyRenderPass> {};
template<>
struct fex_gen_config<vkGetRenderAreaGranularity> {};
template<>
struct fex_gen_config<vkCreateCommandPool> {};
template<>
struct fex_gen_config<vkDestroyCommandPool> {};
template<>
struct fex_gen_config<vkResetCommandPool> {};
template<>
struct fex_gen_config<vkAllocateCommandBuffers> {};
template<>
struct fex_gen_config<vkFreeCommandBuffers> {};
template<>
struct fex_gen_config<vkBeginCommandBuffer> {};
template<>
struct fex_gen_config<vkEndCommandBuffer> {};
template<>
struct fex_gen_config<vkResetCommandBuffer> {};
template<>
struct fex_gen_config<vkCmdBindPipeline> {};
template<>
struct fex_gen_config<vkCmdSetViewport> {};
template<>
struct fex_gen_config<vkCmdSetScissor> {};
template<>
struct fex_gen_config<vkCmdSetLineWidth> {};
template<>
struct fex_gen_config<vkCmdSetDepthBias> {};
template<>
struct fex_gen_config<vkCmdSetBlendConstants> {};
template<>
struct fex_gen_config<vkCmdSetDepthBounds> {};
template<>
struct fex_gen_config<vkCmdSetStencilCompareMask> {};
template<>
struct fex_gen_config<vkCmdSetStencilWriteMask> {};
template<>
struct fex_gen_config<vkCmdSetStencilReference> {};
template<>
struct fex_gen_config<vkCmdBindDescriptorSets> {};
template<>
struct fex_gen_config<vkCmdBindIndexBuffer> {};
template<>
struct fex_gen_config<vkCmdBindVertexBuffers> {};
template<>
struct fex_gen_config<vkCmdDraw> {};
template<>
struct fex_gen_config<vkCmdDrawIndexed> {};
template<>
struct fex_gen_config<vkCmdDrawIndirect> {};
template<>
struct fex_gen_config<vkCmdDrawIndexedIndirect> {};
template<>
struct fex_gen_config<vkCmdDispatch> {};
template<>
struct fex_gen_config<vkCmdDispatchIndirect> {};
template<>
struct fex_gen_config<vkCmdCopyBuffer> {};
template<>
struct fex_gen_config<vkCmdCopyImage> {};
template<>
struct fex_gen_config<vkCmdBlitImage> {};
template<>
struct fex_gen_config<vkCmdCopyBufferToImage> {};
template<>
struct fex_gen_config<vkCmdCopyImageToBuffer> {};
template<>
struct fex_gen_config<vkCmdUpdateBuffer> {};
template<>
struct fex_gen_config<vkCmdFillBuffer> {};
template<>
struct fex_gen_config<vkCmdClearColorImage> {};
template<>
struct fex_gen_config<vkCmdClearDepthStencilImage> {};
template<>
struct fex_gen_config<vkCmdClearAttachments> {};
template<>
struct fex_gen_config<vkCmdResolveImage> {};
template<>
struct fex_gen_config<vkCmdSetEvent> {};
template<>
struct fex_gen_config<vkCmdResetEvent> {};
template<>
struct fex_gen_config<vkCmdWaitEvents> {};
template<>
struct fex_gen_config<vkCmdPipelineBarrier> {};
template<>
struct fex_gen_config<vkCmdBeginQuery> {};
template<>
struct fex_gen_config<vkCmdEndQuery> {};
template<>
struct fex_gen_config<vkCmdResetQueryPool> {};
template<>
struct fex_gen_config<vkCmdWriteTimestamp> {};
template<>
struct fex_gen_config<vkCmdCopyQueryPoolResults> {};
template<>
struct fex_gen_config<vkCmdPushConstants> {};
template<>
struct fex_gen_config<vkCmdBeginRenderPass> {};
template<>
struct fex_gen_config<vkCmdNextSubpass> {};
template<>
struct fex_gen_config<vkCmdEndRenderPass> {};
template<>
struct fex_gen_config<vkCmdExecuteCommands> {};
template<>
struct fex_gen_config<vkEnumerateInstanceVersion> {};
template<>
struct fex_gen_config<vkBindBufferMemory2> {};
template<>
struct fex_gen_config<vkBindImageMemory2> {};
template<>
struct fex_gen_config<vkGetDeviceGroupPeerMemoryFeatures> {};
template<>
struct fex_gen_config<vkCmdSetDeviceMask> {};
template<>
struct fex_gen_config<vkCmdDispatchBase> {};
template<>
struct fex_gen_config<vkEnumeratePhysicalDeviceGroups> {};
template<>
struct fex_gen_config<vkGetImageMemoryRequirements2> {};
template<>
struct fex_gen_config<vkGetBufferMemoryRequirements2> {};
template<>
struct fex_gen_config<vkGetImageSparseMemoryRequirements2> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceFeatures2> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceProperties2> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceFormatProperties2> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceImageFormatProperties2> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceQueueFamilyProperties2> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceMemoryProperties2> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSparseImageFormatProperties2> {};
template<>
struct fex_gen_config<vkTrimCommandPool> {};
template<>
struct fex_gen_config<vkGetDeviceQueue2> {};
template<>
struct fex_gen_config<vkCreateSamplerYcbcrConversion> {};
template<>
struct fex_gen_config<vkDestroySamplerYcbcrConversion> {};
template<>
struct fex_gen_config<vkCreateDescriptorUpdateTemplate> {};
template<>
struct fex_gen_config<vkDestroyDescriptorUpdateTemplate> {};
template<>
struct fex_gen_config<vkUpdateDescriptorSetWithTemplate> {};
template<>
struct fex_gen_param<vkUpdateDescriptorSetWithTemplate, 3, const void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceExternalBufferProperties> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceExternalFenceProperties> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceExternalSemaphoreProperties> {};
template<>
struct fex_gen_config<vkGetDescriptorSetLayoutSupport> {};
template<>
struct fex_gen_config<vkCmdDrawIndirectCount> {};
template<>
struct fex_gen_config<vkCmdDrawIndexedIndirectCount> {};
template<>
struct fex_gen_config<vkCreateRenderPass2> {};
template<>
struct fex_gen_config<vkCmdBeginRenderPass2> {};
template<>
struct fex_gen_config<vkCmdNextSubpass2> {};
template<>
struct fex_gen_config<vkCmdEndRenderPass2> {};
template<>
struct fex_gen_config<vkResetQueryPool> {};
template<>
struct fex_gen_config<vkGetSemaphoreCounterValue> {};
template<>
struct fex_gen_config<vkWaitSemaphores> {};
template<>
struct fex_gen_config<vkSignalSemaphore> {};
template<>
struct fex_gen_config<vkGetBufferDeviceAddress> {};
template<>
struct fex_gen_config<vkGetBufferOpaqueCaptureAddress> {};
template<>
struct fex_gen_config<vkGetDeviceMemoryOpaqueCaptureAddress> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceToolProperties> {};
template<>
struct fex_gen_config<vkCreatePrivateDataSlot> {};
template<>
struct fex_gen_config<vkDestroyPrivateDataSlot> {};
template<>
struct fex_gen_config<vkSetPrivateData> {};
template<>
struct fex_gen_config<vkGetPrivateData> {};
template<>
struct fex_gen_config<vkCmdSetEvent2> {};
template<>
struct fex_gen_config<vkCmdResetEvent2> {};
template<>
struct fex_gen_config<vkCmdWaitEvents2> {};
template<>
struct fex_gen_config<vkCmdPipelineBarrier2> {};
template<>
struct fex_gen_config<vkCmdWriteTimestamp2> {};
template<>
struct fex_gen_config<vkQueueSubmit2> {};
template<>
struct fex_gen_config<vkCmdCopyBuffer2> {};
template<>
struct fex_gen_config<vkCmdCopyImage2> {};
template<>
struct fex_gen_config<vkCmdCopyBufferToImage2> {};
template<>
struct fex_gen_config<vkCmdCopyImageToBuffer2> {};
template<>
struct fex_gen_config<vkCmdBlitImage2> {};
template<>
struct fex_gen_config<vkCmdResolveImage2> {};
template<>
struct fex_gen_config<vkCmdBeginRendering> {};
template<>
struct fex_gen_config<vkCmdEndRendering> {};
template<>
struct fex_gen_config<vkCmdSetCullMode> {};
template<>
struct fex_gen_config<vkCmdSetFrontFace> {};
template<>
struct fex_gen_config<vkCmdSetPrimitiveTopology> {};
template<>
struct fex_gen_config<vkCmdSetViewportWithCount> {};
template<>
struct fex_gen_config<vkCmdSetScissorWithCount> {};
template<>
struct fex_gen_config<vkCmdBindVertexBuffers2> {};
template<>
struct fex_gen_config<vkCmdSetDepthTestEnable> {};
template<>
struct fex_gen_config<vkCmdSetDepthWriteEnable> {};
template<>
struct fex_gen_config<vkCmdSetDepthCompareOp> {};
template<>
struct fex_gen_config<vkCmdSetDepthBoundsTestEnable> {};
template<>
struct fex_gen_config<vkCmdSetStencilTestEnable> {};
template<>
struct fex_gen_config<vkCmdSetStencilOp> {};
template<>
struct fex_gen_config<vkCmdSetRasterizerDiscardEnable> {};
template<>
struct fex_gen_config<vkCmdSetDepthBiasEnable> {};
template<>
struct fex_gen_config<vkCmdSetPrimitiveRestartEnable> {};
template<>
struct fex_gen_config<vkGetDeviceBufferMemoryRequirements> {};
template<>
struct fex_gen_config<vkGetDeviceImageMemoryRequirements> {};
template<>
struct fex_gen_config<vkGetDeviceImageSparseMemoryRequirements> {};
template<>
struct fex_gen_config<vkDestroySurfaceKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSurfaceSupportKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSurfaceCapabilitiesKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSurfaceFormatsKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSurfacePresentModesKHR> {};
template<>
struct fex_gen_config<vkCreateSwapchainKHR> {};
template<>
struct fex_gen_config<vkDestroySwapchainKHR> {};
template<>
struct fex_gen_config<vkGetSwapchainImagesKHR> {};
template<>
struct fex_gen_config<vkAcquireNextImageKHR> {};
template<>
struct fex_gen_config<vkQueuePresentKHR> {};
template<>
struct fex_gen_config<vkGetDeviceGroupPresentCapabilitiesKHR> {};
template<>
struct fex_gen_config<vkGetDeviceGroupSurfacePresentModesKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDevicePresentRectanglesKHR> {};
template<>
struct fex_gen_config<vkAcquireNextImage2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceDisplayPropertiesKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceDisplayPlanePropertiesKHR> {};
template<>
struct fex_gen_config<vkGetDisplayPlaneSupportedDisplaysKHR> {};
template<>
struct fex_gen_config<vkGetDisplayModePropertiesKHR> {};
template<>
struct fex_gen_config<vkCreateDisplayModeKHR> {};
template<>
struct fex_gen_config<vkGetDisplayPlaneCapabilitiesKHR> {};
template<>
struct fex_gen_config<vkCreateDisplayPlaneSurfaceKHR> {};
template<>
struct fex_gen_config<vkCreateSharedSwapchainsKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceVideoCapabilitiesKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceVideoFormatPropertiesKHR> {};
template<>
struct fex_gen_config<vkCreateVideoSessionKHR> {};
template<>
struct fex_gen_config<vkDestroyVideoSessionKHR> {};
template<>
struct fex_gen_config<vkGetVideoSessionMemoryRequirementsKHR> {};
template<>
struct fex_gen_config<vkBindVideoSessionMemoryKHR> {};
template<>
struct fex_gen_config<vkCreateVideoSessionParametersKHR> {};
template<>
struct fex_gen_config<vkUpdateVideoSessionParametersKHR> {};
template<>
struct fex_gen_config<vkDestroyVideoSessionParametersKHR> {};
template<>
struct fex_gen_config<vkCmdBeginVideoCodingKHR> {};
template<>
struct fex_gen_config<vkCmdEndVideoCodingKHR> {};
template<>
struct fex_gen_config<vkCmdControlVideoCodingKHR> {};
template<>
struct fex_gen_config<vkCmdDecodeVideoKHR> {};
template<>
struct fex_gen_config<vkCmdBeginRenderingKHR> {};
template<>
struct fex_gen_config<vkCmdEndRenderingKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceFeatures2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceProperties2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceFormatProperties2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceImageFormatProperties2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceQueueFamilyProperties2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceMemoryProperties2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSparseImageFormatProperties2KHR> {};
template<>
struct fex_gen_config<vkGetDeviceGroupPeerMemoryFeaturesKHR> {};
template<>
struct fex_gen_config<vkCmdSetDeviceMaskKHR> {};
template<>
struct fex_gen_config<vkCmdDispatchBaseKHR> {};
template<>
struct fex_gen_config<vkTrimCommandPoolKHR> {};
template<>
struct fex_gen_config<vkEnumeratePhysicalDeviceGroupsKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceExternalBufferPropertiesKHR> {};
template<>
struct fex_gen_config<vkGetMemoryFdKHR> {};
template<>
struct fex_gen_config<vkGetMemoryFdPropertiesKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceExternalSemaphorePropertiesKHR> {};
template<>
struct fex_gen_config<vkImportSemaphoreFdKHR> {};
template<>
struct fex_gen_config<vkGetSemaphoreFdKHR> {};
template<>
struct fex_gen_config<vkCmdPushDescriptorSetKHR> {};
template<>
struct fex_gen_config<vkCmdPushDescriptorSetWithTemplateKHR> {};
template<>
struct fex_gen_param<vkCmdPushDescriptorSetWithTemplateKHR, 4, const void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkCreateDescriptorUpdateTemplateKHR> {};
template<>
struct fex_gen_config<vkDestroyDescriptorUpdateTemplateKHR> {};
template<>
struct fex_gen_config<vkUpdateDescriptorSetWithTemplateKHR> {};
template<>
struct fex_gen_param<vkUpdateDescriptorSetWithTemplateKHR, 3, const void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkCreateRenderPass2KHR> {};
template<>
struct fex_gen_config<vkCmdBeginRenderPass2KHR> {};
template<>
struct fex_gen_config<vkCmdNextSubpass2KHR> {};
template<>
struct fex_gen_config<vkCmdEndRenderPass2KHR> {};
template<>
struct fex_gen_config<vkGetSwapchainStatusKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceExternalFencePropertiesKHR> {};
template<>
struct fex_gen_config<vkImportFenceFdKHR> {};
template<>
struct fex_gen_config<vkGetFenceFdKHR> {};
template<>
struct fex_gen_config<vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR> {};
template<>
struct fex_gen_config<vkAcquireProfilingLockKHR> {};
template<>
struct fex_gen_config<vkReleaseProfilingLockKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSurfaceCapabilities2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSurfaceFormats2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceDisplayProperties2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceDisplayPlaneProperties2KHR> {};
template<>
struct fex_gen_config<vkGetDisplayModeProperties2KHR> {};
template<>
struct fex_gen_config<vkGetDisplayPlaneCapabilities2KHR> {};
template<>
struct fex_gen_config<vkGetImageMemoryRequirements2KHR> {};
template<>
struct fex_gen_config<vkGetBufferMemoryRequirements2KHR> {};
template<>
struct fex_gen_config<vkGetImageSparseMemoryRequirements2KHR> {};
template<>
struct fex_gen_config<vkCreateSamplerYcbcrConversionKHR> {};
template<>
struct fex_gen_config<vkDestroySamplerYcbcrConversionKHR> {};
template<>
struct fex_gen_config<vkBindBufferMemory2KHR> {};
template<>
struct fex_gen_config<vkBindImageMemory2KHR> {};
template<>
struct fex_gen_config<vkGetDescriptorSetLayoutSupportKHR> {};
template<>
struct fex_gen_config<vkCmdDrawIndirectCountKHR> {};
template<>
struct fex_gen_config<vkCmdDrawIndexedIndirectCountKHR> {};
template<>
struct fex_gen_config<vkGetSemaphoreCounterValueKHR> {};
template<>
struct fex_gen_config<vkWaitSemaphoresKHR> {};
template<>
struct fex_gen_config<vkSignalSemaphoreKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceFragmentShadingRatesKHR> {};
template<>
struct fex_gen_config<vkCmdSetFragmentShadingRateKHR> {};
template<>
struct fex_gen_config<vkCmdSetRenderingAttachmentLocationsKHR> {};
template<>
struct fex_gen_config<vkCmdSetRenderingInputAttachmentIndicesKHR> {};
template<>
struct fex_gen_config<vkWaitForPresentKHR> {};
template<>
struct fex_gen_config<vkGetBufferDeviceAddressKHR> {};
template<>
struct fex_gen_config<vkGetBufferOpaqueCaptureAddressKHR> {};
template<>
struct fex_gen_config<vkGetDeviceMemoryOpaqueCaptureAddressKHR> {};
template<>
struct fex_gen_config<vkCreateDeferredOperationKHR> {};
template<>
struct fex_gen_config<vkDestroyDeferredOperationKHR> {};
template<>
struct fex_gen_config<vkGetDeferredOperationMaxConcurrencyKHR> {};
template<>
struct fex_gen_config<vkGetDeferredOperationResultKHR> {};
template<>
struct fex_gen_config<vkDeferredOperationJoinKHR> {};
template<>
struct fex_gen_config<vkGetPipelineExecutablePropertiesKHR> {};
template<>
struct fex_gen_config<vkGetPipelineExecutableStatisticsKHR> {};
template<>
struct fex_gen_config<vkGetPipelineExecutableInternalRepresentationsKHR> {};
template<>
struct fex_gen_config<vkMapMemory2KHR> {};
template<>
struct fex_gen_config<vkUnmapMemory2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR> {};
template<>
struct fex_gen_config<vkGetEncodedVideoSessionParametersKHR> {};
template<>
struct fex_gen_config<vkCmdEncodeVideoKHR> {};
template<>
struct fex_gen_config<vkCmdSetEvent2KHR> {};
template<>
struct fex_gen_config<vkCmdResetEvent2KHR> {};
template<>
struct fex_gen_config<vkCmdWaitEvents2KHR> {};
template<>
struct fex_gen_config<vkCmdPipelineBarrier2KHR> {};
template<>
struct fex_gen_config<vkCmdWriteTimestamp2KHR> {};
template<>
struct fex_gen_config<vkQueueSubmit2KHR> {};
template<>
struct fex_gen_config<vkCmdWriteBufferMarker2AMD> {};
template<>
struct fex_gen_config<vkGetQueueCheckpointData2NV> {};
template<>
struct fex_gen_config<vkCmdCopyBuffer2KHR> {};
template<>
struct fex_gen_config<vkCmdCopyImage2KHR> {};
template<>
struct fex_gen_config<vkCmdCopyBufferToImage2KHR> {};
template<>
struct fex_gen_config<vkCmdCopyImageToBuffer2KHR> {};
template<>
struct fex_gen_config<vkCmdBlitImage2KHR> {};
template<>
struct fex_gen_config<vkCmdResolveImage2KHR> {};
template<>
struct fex_gen_config<vkCmdTraceRaysIndirect2KHR> {};
template<>
struct fex_gen_config<vkGetDeviceBufferMemoryRequirementsKHR> {};
template<>
struct fex_gen_config<vkGetDeviceImageMemoryRequirementsKHR> {};
template<>
struct fex_gen_config<vkGetDeviceImageSparseMemoryRequirementsKHR> {};
template<>
struct fex_gen_config<vkCmdBindIndexBuffer2KHR> {};
template<>
struct fex_gen_config<vkGetRenderingAreaGranularityKHR> {};
template<>
struct fex_gen_config<vkGetDeviceImageSubresourceLayoutKHR> {};
template<>
struct fex_gen_config<vkGetImageSubresourceLayout2KHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR> {};
template<>
struct fex_gen_config<vkCmdSetLineStippleKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceCalibrateableTimeDomainsKHR> {};
template<>
struct fex_gen_config<vkGetCalibratedTimestampsKHR> {};
template<>
struct fex_gen_config<vkCmdBindDescriptorSets2KHR> {};
template<>
struct fex_gen_config<vkCmdPushConstants2KHR> {};
template<>
struct fex_gen_config<vkCmdPushDescriptorSet2KHR> {};
template<>
struct fex_gen_config<vkCmdPushDescriptorSetWithTemplate2KHR> {};
template<>
struct fex_gen_config<vkCmdSetDescriptorBufferOffsets2EXT> {};
template<>
struct fex_gen_config<vkCmdBindDescriptorBufferEmbeddedSamplers2EXT> {};
template<>
struct fex_gen_config<vkCreateDebugReportCallbackEXT> : fexgen::custom_host_impl {};
template<>
struct fex_gen_param<vkCreateDebugReportCallbackEXT, 1, const VkDebugReportCallbackCreateInfoEXT*> : fexgen::ptr_passthrough {};
template<>
struct fex_gen_config<vkDestroyDebugReportCallbackEXT> : fexgen::custom_host_impl {};
template<>
struct fex_gen_config<vkDebugReportMessageEXT> {};
template<>
struct fex_gen_config<vkDebugMarkerSetObjectTagEXT> {};
template<>
struct fex_gen_config<vkDebugMarkerSetObjectNameEXT> {};
template<>
struct fex_gen_config<vkCmdDebugMarkerBeginEXT> {};
template<>
struct fex_gen_config<vkCmdDebugMarkerEndEXT> {};
template<>
struct fex_gen_config<vkCmdDebugMarkerInsertEXT> {};
template<>
struct fex_gen_config<vkCmdBindTransformFeedbackBuffersEXT> {};
template<>
struct fex_gen_config<vkCmdBeginTransformFeedbackEXT> {};
template<>
struct fex_gen_config<vkCmdEndTransformFeedbackEXT> {};
template<>
struct fex_gen_config<vkCmdBeginQueryIndexedEXT> {};
template<>
struct fex_gen_config<vkCmdEndQueryIndexedEXT> {};
template<>
struct fex_gen_config<vkCmdDrawIndirectByteCountEXT> {};
template<>
struct fex_gen_config<vkCreateCuModuleNVX> {};
template<>
struct fex_gen_config<vkCreateCuFunctionNVX> {};
template<>
struct fex_gen_config<vkDestroyCuModuleNVX> {};
template<>
struct fex_gen_config<vkDestroyCuFunctionNVX> {};
template<>
struct fex_gen_config<vkCmdCuLaunchKernelNVX> {};
template<>
struct fex_gen_config<vkGetImageViewHandleNVX> {};
template<>
struct fex_gen_config<vkGetImageViewAddressNVX> {};
template<>
struct fex_gen_config<vkCmdDrawIndirectCountAMD> {};
template<>
struct fex_gen_config<vkCmdDrawIndexedIndirectCountAMD> {};
template<>
struct fex_gen_config<vkGetShaderInfoAMD> {};
template<>
struct fex_gen_param<vkGetShaderInfoAMD, 5, void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceExternalImageFormatPropertiesNV> {};
template<>
struct fex_gen_config<vkCmdBeginConditionalRenderingEXT> {};
template<>
struct fex_gen_config<vkCmdEndConditionalRenderingEXT> {};
template<>
struct fex_gen_config<vkCmdSetViewportWScalingNV> {};
template<>
struct fex_gen_config<vkReleaseDisplayEXT> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSurfaceCapabilities2EXT> {};
template<>
struct fex_gen_config<vkDisplayPowerControlEXT> {};
template<>
struct fex_gen_config<vkRegisterDeviceEventEXT> {};
template<>
struct fex_gen_config<vkRegisterDisplayEventEXT> {};
template<>
struct fex_gen_config<vkGetSwapchainCounterEXT> {};
template<>
struct fex_gen_config<vkGetRefreshCycleDurationGOOGLE> {};
template<>
struct fex_gen_config<vkGetPastPresentationTimingGOOGLE> {};
template<>
struct fex_gen_config<vkCmdSetDiscardRectangleEXT> {};
template<>
struct fex_gen_config<vkCmdSetDiscardRectangleEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetDiscardRectangleModeEXT> {};
template<>
struct fex_gen_config<vkSetHdrMetadataEXT> {};
template<>
struct fex_gen_config<vkSetDebugUtilsObjectNameEXT> {};
template<>
struct fex_gen_config<vkSetDebugUtilsObjectTagEXT> {};
template<>
struct fex_gen_config<vkQueueBeginDebugUtilsLabelEXT> {};
template<>
struct fex_gen_config<vkQueueEndDebugUtilsLabelEXT> {};
template<>
struct fex_gen_config<vkQueueInsertDebugUtilsLabelEXT> {};
template<>
struct fex_gen_config<vkCmdBeginDebugUtilsLabelEXT> {};
template<>
struct fex_gen_config<vkCmdEndDebugUtilsLabelEXT> {};
template<>
struct fex_gen_config<vkCmdInsertDebugUtilsLabelEXT> {};
template<>
struct fex_gen_config<vkCreateDebugUtilsMessengerEXT> : fexgen::custom_host_impl {};
template<>
struct fex_gen_param<vkCreateDebugUtilsMessengerEXT, 1, const VkDebugUtilsMessengerCreateInfoEXT*> : fexgen::ptr_passthrough {};
template<>
struct fex_gen_config<vkDestroyDebugUtilsMessengerEXT> {};
template<>
struct fex_gen_config<vkSubmitDebugUtilsMessageEXT> {};
template<>
struct fex_gen_config<vkCmdSetSampleLocationsEXT> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceMultisamplePropertiesEXT> {};
template<>
struct fex_gen_config<vkGetImageDrmFormatModifierPropertiesEXT> {};
template<>
struct fex_gen_config<vkCreateValidationCacheEXT> {};
template<>
struct fex_gen_config<vkDestroyValidationCacheEXT> {};
template<>
struct fex_gen_config<vkMergeValidationCachesEXT> {};
template<>
struct fex_gen_config<vkGetValidationCacheDataEXT> {};
template<>
struct fex_gen_param<vkGetValidationCacheDataEXT, 3, void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkCmdBindShadingRateImageNV> {};
template<>
struct fex_gen_config<vkCmdSetViewportShadingRatePaletteNV> {};
template<>
struct fex_gen_config<vkCmdSetCoarseSampleOrderNV> {};
template<>
struct fex_gen_config<vkCreateAccelerationStructureNV> {};
template<>
struct fex_gen_config<vkDestroyAccelerationStructureNV> {};
template<>
struct fex_gen_config<vkGetAccelerationStructureMemoryRequirementsNV> {};
template<>
struct fex_gen_config<vkBindAccelerationStructureMemoryNV> {};
template<>
struct fex_gen_config<vkCmdBuildAccelerationStructureNV> {};
template<>
struct fex_gen_config<vkCmdCopyAccelerationStructureNV> {};
template<>
struct fex_gen_config<vkCmdTraceRaysNV> {};
template<>
struct fex_gen_config<vkCreateRayTracingPipelinesNV> {};
template<>
struct fex_gen_config<vkGetRayTracingShaderGroupHandlesKHR> {};
template<>
struct fex_gen_param<vkGetRayTracingShaderGroupHandlesKHR, 5, void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkGetRayTracingShaderGroupHandlesNV> {};
template<>
struct fex_gen_param<vkGetRayTracingShaderGroupHandlesNV, 5, void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkGetAccelerationStructureHandleNV> {};
template<>
struct fex_gen_param<vkGetAccelerationStructureHandleNV, 3, void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkCmdWriteAccelerationStructuresPropertiesNV> {};
template<>
struct fex_gen_config<vkCompileDeferredNV> {};
template<>
struct fex_gen_config<vkGetMemoryHostPointerPropertiesEXT> {};
template<>
struct fex_gen_param<vkGetMemoryHostPointerPropertiesEXT, 2, const void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkCmdWriteBufferMarkerAMD> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceCalibrateableTimeDomainsEXT> {};
template<>
struct fex_gen_config<vkGetCalibratedTimestampsEXT> {};
template<>
struct fex_gen_config<vkCmdDrawMeshTasksNV> {};
template<>
struct fex_gen_config<vkCmdDrawMeshTasksIndirectNV> {};
template<>
struct fex_gen_config<vkCmdDrawMeshTasksIndirectCountNV> {};
template<>
struct fex_gen_config<vkCmdSetExclusiveScissorEnableNV> {};
template<>
struct fex_gen_config<vkCmdSetExclusiveScissorNV> {};
template<>
struct fex_gen_config<vkCmdSetCheckpointNV> {};
template<>
struct fex_gen_param<vkCmdSetCheckpointNV, 1, const void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkGetQueueCheckpointDataNV> {};
template<>
struct fex_gen_config<vkInitializePerformanceApiINTEL> {};
template<>
struct fex_gen_config<vkUninitializePerformanceApiINTEL> {};
template<>
struct fex_gen_config<vkCmdSetPerformanceMarkerINTEL> {};
template<>
struct fex_gen_config<vkCmdSetPerformanceStreamMarkerINTEL> {};
template<>
struct fex_gen_config<vkCmdSetPerformanceOverrideINTEL> {};
template<>
struct fex_gen_config<vkAcquirePerformanceConfigurationINTEL> {};
template<>
struct fex_gen_config<vkReleasePerformanceConfigurationINTEL> {};
template<>
struct fex_gen_config<vkQueueSetPerformanceConfigurationINTEL> {};
template<>
struct fex_gen_config<vkGetPerformanceParameterINTEL> {};
template<>
struct fex_gen_config<vkSetLocalDimmingAMD> {};
template<>
struct fex_gen_config<vkGetBufferDeviceAddressEXT> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceToolPropertiesEXT> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceCooperativeMatrixPropertiesNV> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV> {};
template<>
struct fex_gen_config<vkCreateHeadlessSurfaceEXT> {};
template<>
struct fex_gen_config<vkCmdSetLineStippleEXT> {};
template<>
struct fex_gen_config<vkResetQueryPoolEXT> {};
template<>
struct fex_gen_config<vkCmdSetCullModeEXT> {};
template<>
struct fex_gen_config<vkCmdSetFrontFaceEXT> {};
template<>
struct fex_gen_config<vkCmdSetPrimitiveTopologyEXT> {};
template<>
struct fex_gen_config<vkCmdSetViewportWithCountEXT> {};
template<>
struct fex_gen_config<vkCmdSetScissorWithCountEXT> {};
template<>
struct fex_gen_config<vkCmdBindVertexBuffers2EXT> {};
template<>
struct fex_gen_config<vkCmdSetDepthTestEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetDepthWriteEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetDepthCompareOpEXT> {};
template<>
struct fex_gen_config<vkCmdSetDepthBoundsTestEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetStencilTestEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetStencilOpEXT> {};
template<>
struct fex_gen_config<vkCopyMemoryToImageEXT> {};
template<>
struct fex_gen_config<vkCopyImageToMemoryEXT> {};
template<>
struct fex_gen_config<vkCopyImageToImageEXT> {};
template<>
struct fex_gen_config<vkTransitionImageLayoutEXT> {};
template<>
struct fex_gen_config<vkGetImageSubresourceLayout2EXT> {};
template<>
struct fex_gen_config<vkReleaseSwapchainImagesEXT> {};
template<>
struct fex_gen_config<vkGetGeneratedCommandsMemoryRequirementsNV> {};
template<>
struct fex_gen_config<vkCmdPreprocessGeneratedCommandsNV> {};
template<>
struct fex_gen_config<vkCmdExecuteGeneratedCommandsNV> {};
template<>
struct fex_gen_config<vkCmdBindPipelineShaderGroupNV> {};
template<>
struct fex_gen_config<vkCreateIndirectCommandsLayoutNV> {};
template<>
struct fex_gen_config<vkDestroyIndirectCommandsLayoutNV> {};
template<>
struct fex_gen_config<vkCmdSetDepthBias2EXT> {};
template<>
struct fex_gen_config<vkAcquireDrmDisplayEXT> {};
template<>
struct fex_gen_config<vkGetDrmDisplayEXT> {};
template<>
struct fex_gen_config<vkCreatePrivateDataSlotEXT> {};
template<>
struct fex_gen_config<vkDestroyPrivateDataSlotEXT> {};
template<>
struct fex_gen_config<vkSetPrivateDataEXT> {};
template<>
struct fex_gen_config<vkGetPrivateDataEXT> {};
template<>
struct fex_gen_config<vkCreateCudaModuleNV> {};
template<>
struct fex_gen_config<vkGetCudaModuleCacheNV> {};
template<>
struct fex_gen_config<vkCreateCudaFunctionNV> {};
template<>
struct fex_gen_config<vkDestroyCudaModuleNV> {};
template<>
struct fex_gen_config<vkDestroyCudaFunctionNV> {};
template<>
struct fex_gen_config<vkCmdCudaLaunchKernelNV> {};
template<>
struct fex_gen_config<vkGetDescriptorSetLayoutSizeEXT> {};
template<>
struct fex_gen_config<vkGetDescriptorSetLayoutBindingOffsetEXT> {};
template<>
struct fex_gen_config<vkGetDescriptorEXT> {};
template<>
struct fex_gen_config<vkCmdBindDescriptorBuffersEXT> {};
template<>
struct fex_gen_config<vkCmdSetDescriptorBufferOffsetsEXT> {};
template<>
struct fex_gen_config<vkCmdBindDescriptorBufferEmbeddedSamplersEXT> {};
template<>
struct fex_gen_config<vkGetBufferOpaqueCaptureDescriptorDataEXT> {};
template<>
struct fex_gen_config<vkGetImageOpaqueCaptureDescriptorDataEXT> {};
template<>
struct fex_gen_config<vkGetImageViewOpaqueCaptureDescriptorDataEXT> {};
template<>
struct fex_gen_config<vkGetSamplerOpaqueCaptureDescriptorDataEXT> {};
template<>
struct fex_gen_config<vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT> {};
template<>
struct fex_gen_config<vkCmdSetFragmentShadingRateEnumNV> {};
template<>
struct fex_gen_config<vkGetDeviceFaultInfoEXT> {};
template<>
struct fex_gen_config<vkCmdSetVertexInputEXT> {};
template<>
struct fex_gen_config<vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI> {};
template<>
struct fex_gen_config<vkCmdSubpassShadingHUAWEI> {};
template<>
struct fex_gen_config<vkCmdBindInvocationMaskHUAWEI> {};
template<>
struct fex_gen_config<vkGetMemoryRemoteAddressNV> {};
template<>
struct fex_gen_config<vkGetPipelinePropertiesEXT> {};
template<>
struct fex_gen_config<vkCmdSetPatchControlPointsEXT> {};
template<>
struct fex_gen_config<vkCmdSetRasterizerDiscardEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetDepthBiasEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetLogicOpEXT> {};
template<>
struct fex_gen_config<vkCmdSetPrimitiveRestartEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetColorWriteEnableEXT> {};
template<>
struct fex_gen_config<vkCmdDrawMultiEXT> {};
template<>
struct fex_gen_config<vkCmdDrawMultiIndexedEXT> {};
template<>
struct fex_gen_config<vkCreateMicromapEXT> {};
template<>
struct fex_gen_config<vkDestroyMicromapEXT> {};
template<>
struct fex_gen_config<vkCmdBuildMicromapsEXT> {};
template<>
struct fex_gen_config<vkBuildMicromapsEXT> {};
template<>
struct fex_gen_config<vkCopyMicromapEXT> {};
template<>
struct fex_gen_config<vkCopyMicromapToMemoryEXT> {};
template<>
struct fex_gen_config<vkCopyMemoryToMicromapEXT> {};
template<>
struct fex_gen_config<vkWriteMicromapsPropertiesEXT> {};
template<>
struct fex_gen_param<vkWriteMicromapsPropertiesEXT, 5, void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkCmdCopyMicromapEXT> {};
template<>
struct fex_gen_config<vkCmdCopyMicromapToMemoryEXT> {};
template<>
struct fex_gen_config<vkCmdCopyMemoryToMicromapEXT> {};
template<>
struct fex_gen_config<vkCmdWriteMicromapsPropertiesEXT> {};
template<>
struct fex_gen_config<vkGetDeviceMicromapCompatibilityEXT> {};
template<>
struct fex_gen_config<vkGetMicromapBuildSizesEXT> {};
template<>
struct fex_gen_config<vkCmdDrawClusterHUAWEI> {};
template<>
struct fex_gen_config<vkCmdDrawClusterIndirectHUAWEI> {};
template<>
struct fex_gen_config<vkSetDeviceMemoryPriorityEXT> {};
template<>
struct fex_gen_config<vkGetDescriptorSetLayoutHostMappingInfoVALVE> {};
template<>
struct fex_gen_config<vkGetDescriptorSetHostMappingVALVE> {};
template<>
struct fex_gen_config<vkCmdCopyMemoryIndirectNV> {};
template<>
struct fex_gen_config<vkCmdCopyMemoryToImageIndirectNV> {};
template<>
struct fex_gen_config<vkCmdDecompressMemoryNV> {};
template<>
struct fex_gen_config<vkCmdDecompressMemoryIndirectCountNV> {};
template<>
struct fex_gen_config<vkGetPipelineIndirectMemoryRequirementsNV> {};
template<>
struct fex_gen_config<vkCmdUpdatePipelineIndirectBufferNV> {};
template<>
struct fex_gen_config<vkGetPipelineIndirectDeviceAddressNV> {};
template<>
struct fex_gen_config<vkCmdSetDepthClampEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetPolygonModeEXT> {};
template<>
struct fex_gen_config<vkCmdSetRasterizationSamplesEXT> {};
template<>
struct fex_gen_config<vkCmdSetSampleMaskEXT> {};
template<>
struct fex_gen_config<vkCmdSetAlphaToCoverageEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetAlphaToOneEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetLogicOpEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetColorBlendEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetColorBlendEquationEXT> {};
template<>
struct fex_gen_config<vkCmdSetColorWriteMaskEXT> {};
template<>
struct fex_gen_config<vkCmdSetTessellationDomainOriginEXT> {};
template<>
struct fex_gen_config<vkCmdSetRasterizationStreamEXT> {};
template<>
struct fex_gen_config<vkCmdSetConservativeRasterizationModeEXT> {};
template<>
struct fex_gen_config<vkCmdSetExtraPrimitiveOverestimationSizeEXT> {};
template<>
struct fex_gen_config<vkCmdSetDepthClipEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetSampleLocationsEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetColorBlendAdvancedEXT> {};
template<>
struct fex_gen_config<vkCmdSetProvokingVertexModeEXT> {};
template<>
struct fex_gen_config<vkCmdSetLineRasterizationModeEXT> {};
template<>
struct fex_gen_config<vkCmdSetLineStippleEnableEXT> {};
template<>
struct fex_gen_config<vkCmdSetDepthClipNegativeOneToOneEXT> {};
template<>
struct fex_gen_config<vkCmdSetViewportWScalingEnableNV> {};
template<>
struct fex_gen_config<vkCmdSetViewportSwizzleNV> {};
template<>
struct fex_gen_config<vkCmdSetCoverageToColorEnableNV> {};
template<>
struct fex_gen_config<vkCmdSetCoverageToColorLocationNV> {};
template<>
struct fex_gen_config<vkCmdSetCoverageModulationModeNV> {};
template<>
struct fex_gen_config<vkCmdSetCoverageModulationTableEnableNV> {};
template<>
struct fex_gen_config<vkCmdSetCoverageModulationTableNV> {};
template<>
struct fex_gen_config<vkCmdSetShadingRateImageEnableNV> {};
template<>
struct fex_gen_config<vkCmdSetRepresentativeFragmentTestEnableNV> {};
template<>
struct fex_gen_config<vkCmdSetCoverageReductionModeNV> {};
template<>
struct fex_gen_config<vkGetShaderModuleIdentifierEXT> {};
template<>
struct fex_gen_config<vkGetShaderModuleCreateInfoIdentifierEXT> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceOpticalFlowImageFormatsNV> {};
template<>
struct fex_gen_config<vkCreateOpticalFlowSessionNV> {};
template<>
struct fex_gen_config<vkDestroyOpticalFlowSessionNV> {};
template<>
struct fex_gen_config<vkBindOpticalFlowSessionImageNV> {};
template<>
struct fex_gen_config<vkCmdOpticalFlowExecuteNV> {};
template<>
struct fex_gen_config<vkCreateShadersEXT> {};
template<>
struct fex_gen_config<vkDestroyShaderEXT> {};
template<>
struct fex_gen_config<vkGetShaderBinaryDataEXT> {};
template<>
struct fex_gen_config<vkCmdBindShadersEXT> {};
template<>
struct fex_gen_config<vkGetFramebufferTilePropertiesQCOM> {};
template<>
struct fex_gen_config<vkGetDynamicRenderingTilePropertiesQCOM> {};
template<>
struct fex_gen_config<vkSetLatencySleepModeNV> {};
template<>
struct fex_gen_config<vkLatencySleepNV> {};
template<>
struct fex_gen_config<vkSetLatencyMarkerNV> {};
template<>
struct fex_gen_config<vkGetLatencyTimingsNV> {};
template<>
struct fex_gen_config<vkQueueNotifyOutOfBandNV> {};
template<>
struct fex_gen_config<vkCmdSetAttachmentFeedbackLoopEnableEXT> {};
template<>
struct fex_gen_config<vkCreateAccelerationStructureKHR> {};
template<>
struct fex_gen_config<vkDestroyAccelerationStructureKHR> {};
template<>
struct fex_gen_config<vkCmdBuildAccelerationStructuresKHR> {};
template<>
struct fex_gen_config<vkCmdBuildAccelerationStructuresIndirectKHR> {};
template<>
struct fex_gen_config<vkBuildAccelerationStructuresKHR> {};
template<>
struct fex_gen_config<vkCopyAccelerationStructureKHR> {};
template<>
struct fex_gen_config<vkCopyAccelerationStructureToMemoryKHR> {};
template<>
struct fex_gen_config<vkCopyMemoryToAccelerationStructureKHR> {};
template<>
struct fex_gen_config<vkWriteAccelerationStructuresPropertiesKHR> {};
template<>
struct fex_gen_param<vkWriteAccelerationStructuresPropertiesKHR, 5, void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkCmdCopyAccelerationStructureKHR> {};
template<>
struct fex_gen_config<vkCmdCopyAccelerationStructureToMemoryKHR> {};
template<>
struct fex_gen_config<vkCmdCopyMemoryToAccelerationStructureKHR> {};
template<>
struct fex_gen_config<vkGetAccelerationStructureDeviceAddressKHR> {};
template<>
struct fex_gen_config<vkCmdWriteAccelerationStructuresPropertiesKHR> {};
template<>
struct fex_gen_config<vkGetDeviceAccelerationStructureCompatibilityKHR> {};
template<>
struct fex_gen_config<vkGetAccelerationStructureBuildSizesKHR> {};
template<>
struct fex_gen_config<vkCmdTraceRaysKHR> {};
template<>
struct fex_gen_config<vkCreateRayTracingPipelinesKHR> {};
template<>
struct fex_gen_config<vkGetRayTracingCaptureReplayShaderGroupHandlesKHR> {};
template<>
struct fex_gen_param<vkGetRayTracingCaptureReplayShaderGroupHandlesKHR, 5, void*> : fexgen::assume_compatible_data_layout {};
template<>
struct fex_gen_config<vkCmdTraceRaysIndirectKHR> {};
template<>
struct fex_gen_config<vkGetRayTracingShaderGroupStackSizeKHR> {};
template<>
struct fex_gen_config<vkCmdSetRayTracingPipelineStackSizeKHR> {};
template<>
struct fex_gen_config<vkCmdDrawMeshTasksEXT> {};
template<>
struct fex_gen_config<vkCmdDrawMeshTasksIndirectEXT> {};
template<>
struct fex_gen_config<vkCmdDrawMeshTasksIndirectCountEXT> {};

// vulkan_xlib_xrandr.h
template<>
struct fex_gen_config<vkAcquireXlibDisplayEXT> : fexgen::custom_host_impl {};
template<>
struct fex_gen_param<vkAcquireXlibDisplayEXT, 1, Display*> : fexgen::ptr_passthrough {};
template<>
struct fex_gen_config<vkGetRandROutputDisplayEXT> : fexgen::custom_host_impl {};
template<>
struct fex_gen_param<vkGetRandROutputDisplayEXT, 1, Display*> : fexgen::ptr_passthrough {};

// vulkan_wayland.h
template<>
struct fex_gen_config<vkCreateWaylandSurfaceKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceWaylandPresentationSupportKHR> {};

// vulkan_xcb.h
template<>
struct fex_gen_config<vkCreateXcbSurfaceKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceXcbPresentationSupportKHR> : fexgen::custom_host_impl {};
template<>
struct fex_gen_param<vkGetPhysicalDeviceXcbPresentationSupportKHR, 2, xcb_connection_t*> : fexgen::ptr_passthrough {};

// vulkan_xlib.h
template<>
struct fex_gen_config<vkCreateXlibSurfaceKHR> {};
template<>
struct fex_gen_config<vkGetPhysicalDeviceXlibPresentationSupportKHR> : fexgen::custom_host_impl {};
template<>
struct fex_gen_param<vkGetPhysicalDeviceXlibPresentationSupportKHR, 2, Display*> : fexgen::ptr_passthrough {};

} // namespace internal

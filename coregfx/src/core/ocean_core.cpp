#include <coregfx/core/ocean_core.hpp>

namespace oceancore
{
	VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice)
	{
		assert(physicalDevice);
		this->physicalDevice = physicalDevice;

		// Store Properties features, limits and properties of the physical device for later use
		// Device properties also contain limits and sparse properties
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		// Features should be checked by the examples before using them
		vkGetPhysicalDeviceFeatures(physicalDevice, &features);
		// Memory properties are used regularly for creating all kinds of buffers
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
		// Queue family properties, used for setting up requested queues upon device creation
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		assert(queueFamilyCount > 0);
		queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
	}

	/**
	 * Default destructor
	 *
	 * @note Frees the logical device
	 */
	VulkanDevice::~VulkanDevice()
	{
		if (commandPool)
		{
			vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
		}
		if (logicalDevice)
		{
			vkDestroyDevice(logicalDevice, nullptr);
		}
	}

	/**
	 * Get the index of a memory type that has all the requested property bits set
	 *
	 * @param typeBits Bitmask with bits set for each memory type supported by the resource to request for (from VkMemoryRequirements)
	 * @param properties Bitmask of properties for the memory type to request
	 * @param (Optional) memTypeFound Pointer to a bool that is set to true if a matching memory type has been found
	 *
	 * @return Index of the requested memory type
	 *
	 * @throw Throws an exception if memTypeFound is null and no memory type could be found that supports the requested properties
	 */
	/*uint32_t VulkanDevice::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
	{
		for (size_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					if (memTypeFound)
					{
						*memTypeFound = true;
					}
					return i;
				}
			}
			typeBits >>= 1;
		}

		if (memTypeFound)
		{
			*memTypeFound = false;
			return 0;
		}
		else
		{
			throw std::runtime_error("Could not find a matching memory type");
		}
	}*/
	// chatgpt
	uint32_t VulkanDevice::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
	{
		// Cache values for performance and clarity
		const uint32_t memoryTypeCount = memoryProperties.memoryTypeCount;
		const VkMemoryType* memoryTypes = memoryProperties.memoryTypes;

		for (uint32_t i = 0; i < memoryTypeCount && typeBits; ++i, typeBits >>= 1) {
			if (typeBits & 1) {  // Check if the bit is set for this memory type
				if ((memoryTypes[i].propertyFlags & properties) == properties) {  // Check if properties match
					if (memTypeFound) {
						*memTypeFound = VK_TRUE;
					}
					return i;
				}
			}
		}

		// No matching memory type found
		if (memTypeFound) {
			*memTypeFound = VK_FALSE;
			return 0;
		}

		throw std::runtime_error("Could not find a matching memory type");
	}

	/**
	 * Get the index of a queue family that supports the requested queue flags
	 *
	 * @param queueFlags Queue flags to find a queue family index for
	 *
	 * @return Index of the queue family index that matches the flags
	 *
	 * @throw Throws an exception if no queue family index could be found that supports the requested flags
	 */
	uint32_t VulkanDevice::getQueueFamilyIndex(VkQueueFlagBits queueFlags)
	{
		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if (queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			for (size_t i = 0; i < static_cast<size_t>(queueFamilyProperties.size()); i++)
			{
				if ((queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
					break;
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (size_t i = 0; i < static_cast<size_t>(queueFamilyProperties.size()); i++)
		{
			if (queueFamilyProperties[i].queueFlags & queueFlags)
			{
				return i;
				break;
			}
		}

		throw std::runtime_error("Could not find a matching queue family index");
	}

	/**
	 * Create the logical device based on the assigned physical device, also gets default queue family indices
	 *
	 * @param enabledFeatures Can be used to enable certain features upon device creation
	 * @param requestedQueueTypes Bit flags specifying the queue types to be requested from the device
	 *
	 * @return VkResult of the device creation call
	 */
	VkResult VulkanDevice::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char *> enabledExtensions, VkQueueFlags requestedQueueTypes)
	{
		// Desired queues need to be requested upon logical device creation
		// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
		// requests different queue types

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

		// Get queue family indices for the requested queue family types
		// Note that the indices may overlap depending on the implementation

		const float defaultQueuePriority(0.0f);

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			queueFamilyIndices.graphics = 0;
		}

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
			{
				// If compute family index differs, we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			queueFamilyIndices.compute = queueFamilyIndices.graphics;
		}

		// Create the logical device representation
		std::vector<const char *> deviceExtensions(enabledExtensions);
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
		deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

		if (deviceExtensions.size() > 0)
		{
			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);

		if (result == VK_SUCCESS)
		{
			commandPool = createCommandPool(queueFamilyIndices.graphics);
		}

		this->enabledFeatures = enabledFeatures;

		return result;
	}
	/**
	 * Create a buffer on the device
	 *
	 * @param usageFlags Usage flag bit mask for the buffer (i.e. index, vertex, uniform buffer)
	 * @param memoryPropertyFlags Memory properties for this buffer (i.e. device local, host visible, coherent)
	 * @param size Size of the buffer in byes
	 * @param buffer Pointer to the buffer handle acquired by the function
	 * @param memory Pointer to the memory handle acquired by the function
	 * @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
	 *
	 * @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
	 */
	VkResult VulkanDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data)
	{
		// Create the buffer handle
		VkBufferCreateInfo bufferCreateInfo = oceaninitializer::bufferCreateInfo(usageFlags, size);
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, buffer));

		// Create the memory backing up the buffer handle
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = oceaninitializer::memoryAllocateInfo();
		vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReqs);

		memAlloc.allocationSize = memReqs.size;

		// If we need to copy data, ensure we have HOST_VISIBLE memory
		VkMemoryPropertyFlags requiredMemoryProperties = memoryPropertyFlags;
		if (data != nullptr) {
			// For data copying, we need HOST_VISIBLE and HOST_COHERENT memory
			requiredMemoryProperties |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}

		// Find a memory type index that fits the properties of the buffer
		memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, requiredMemoryProperties);

		// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
		if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
		{
			allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memAlloc.pNext = &allocFlagsInfo;
		}

		// Log compact buffer creation info (only on first buffer to avoid spam)
		static bool firstBufferLogged = false;
		if (!firstBufferLogged) {
			logMemoryInfo();
			firstBufferLogged = true;
		}

		// Determine buffer type from usage flags
		std::string bufferType = "UNKNOWN";
		if (usageFlags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) bufferType = "VERTEX";
		else if (usageFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) bufferType = "INDEX";
		else if (usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) bufferType = "UNIFORM";
		else if (usageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) bufferType = "STORAGE";
		else if (usageFlags & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) bufferType = "TRANSFER_SRC";
		else if (usageFlags & VK_BUFFER_USAGE_TRANSFER_DST_BIT) bufferType = "TRANSFER_DST";

		// Single compact log line for buffer creation
		ocean::info("VkBuffer created: type=" + bufferType + ", size=" + std::to_string(size) + "B (" + std::to_string(size / 1024.0) + "KB)" + (data ? ", with_data" : ""));

		VK_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, memory));

		// If a pointer to the buffer data has been passed, map the buffer and copy over the data
		if (data != nullptr)
		{
			void *mapped;
			VK_CHECK_RESULT(vkMapMemory(logicalDevice, *memory, 0, size, 0, &mapped));
			memcpy(mapped, data, size);

			// If host coherency hasn't been requested, do a manual flush to make writes visible
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedRange = oceaninitializer::mappedMemoryRange();
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size = size;
				vkFlushMappedMemoryRanges(logicalDevice, 1, &mappedRange);
			}
			vkUnmapMemory(logicalDevice, *memory);
		}

		// Attach the memory to the buffer object
		VK_CHECK_RESULT(vkBindBufferMemory(logicalDevice, *buffer, *memory, 0));

		return VK_SUCCESS;
	}
	/**
	 * Create a buffer on the device
	 *
	 * @param usageFlags Usage flag bitmask for the buffer (i.e. index, vertex, uniform buffer)
	 * @param memoryPropertyFlags Memory properties for this buffer (i.e. device local, host visible, coherent)
	 * @param size Size of the buffer in bytes
	 * @param buffer Pointer to the buffer handle acquired by the function
	 * @param memory Pointer to the memory handle acquired by the function
	 * @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
	 *
	 * @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
	 */
	VkResult VulkanDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
										VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, std::string msg, void *data)
	{
		// std::cout << "VkDevice.createBuffer msg=" << msg << " byteSize = " << size << std::endl;
		//  Create the buffer handle
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = usageFlags;
		bufferCreateInfo.size = size;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, buffer));

		// Create the memory backing up the buffer handle
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc{};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		// Find a memory type index that fits the properties of the buffer
		memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		// std::cout << "memReqs.size=" << memReqs.size << std::endl;
		VK_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, memory));

		// If a pointer to the buffer data has been passed, map the buffer and copy over the data
		if (data != nullptr)
		{
			void *mapped;
			VK_CHECK_RESULT(vkMapMemory(logicalDevice, *memory, 0, size, 0, &mapped));
			memcpy(mapped, data, size);
			// If host coherency hasn't been requested, do a manual flush to make writes visible
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedRange{};
				mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size = size;
				vkFlushMappedMemoryRanges(logicalDevice, 1, &mappedRange);
			}
			vkUnmapMemory(logicalDevice, *memory);
		}

		// Attach the memory to the buffer object
		VK_CHECK_RESULT(vkBindBufferMemory(logicalDevice, *buffer, *memory, 0));

		return VK_SUCCESS;
	}
	// chatgpt
	// Methode zum Kopieren von Daten von einem Buffer in ein Image
	void VulkanDevice::copyBufferToImage(
		VkBuffer buffer,
		VkImage image,
		uint32_t width,
		uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0; // Zeilenl�nge ist nicht spezifiziert, bedeutet, dass das gesamte Bild kopiert wird
		region.bufferImageHeight = 0; // H�he der Bildreihe ist nicht spezifiziert
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		endSingleTimeCommands(commandBuffer);
	}
	// ChatGPT
	// Methode zur Speicherallokation
	void VulkanDevice::createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory)
	{
		// 1. Vulkan-Buffer erstellen
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;  // Gr��e des Buffers
		bufferInfo.usage = usage;  // Art der Nutzung des Buffers (Transfer, Vertex, etc.)
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // Exklusiv f�r eine Queue-Familie

		if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create buffer!");
		}

		// 2. Speicheranforderungen des Buffers ermitteln
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

		// 3. Speicherallokation f�r den Buffer
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate buffer memory!");
		}

		// 4. Speicher mit dem Buffer verkn�pfen
		vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
	}
	// ChatGPT
	// Hilfsfunktion, um den passenden MemoryType zu finden
	uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type!");
	}

	// Function to log detailed memory information
	void VulkanDevice::logMemoryInfo() {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		// Build compact single-line summary
		std::string memInfo = "Vulkan Memory: heaps=" + std::to_string(memProperties.memoryHeapCount) + ", types=" + std::to_string(memProperties.memoryTypeCount);

		// Add heap info
		for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++) {
			VkMemoryHeap heap = memProperties.memoryHeaps[i];
			memInfo += ", Heap" + std::to_string(i) + "=" + std::to_string(heap.size / 1024 / 1024) + "MB";
			if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) memInfo += "(DEVICE_LOCAL)";
		}

		ocean::info(memInfo);
	}
	// ChatGPT
	// Methode zur Erstellung eines Vulkan-Images
	void VulkanDevice::createImage(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory)
	{
		// 1. Vulkan-Image erstellen
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;  // Typ des Images, hier 2D
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;  // Tiefe f�r 2D-Bilder ist immer 1
		imageInfo.mipLevels = 1;  // Anzahl der Mipmap-Stufen
		imageInfo.arrayLayers = 1;  // Anzahl der Array-Schichten
		imageInfo.format = format;  // Format des Images, z.B. VK_FORMAT_R8G8B8A8_SRGB
		imageInfo.tiling = tiling;  // Tiling-Modus, z.B. VK_IMAGE_TILING_OPTIMAL
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // Start-Layout des Images
		imageInfo.usage = usage;  // Verwendungszweck des Images, z.B. VK_IMAGE_USAGE_TRANSFER_DST_BIT
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // Bild wird von nur einer Queue-Familie verwendet
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;  // Anzahl der Samples (Multisampling)

		if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}

		// 2. Speicheranforderungen des Images ermitteln
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

		// 3. Speicherallokation f�r das Image
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory!");
		}

		// 4. Speicher mit dem Image verkn�pfen
		vkBindImageMemory(logicalDevice, image, imageMemory, 0);
	}
	// chatgpt
	void VulkanDevice::transitionImageLayout(
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);
	}

	// ChatGPT
	VkCommandBuffer VulkanDevice::beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}
	// ChatGPT
	void VulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
	}

	/**
	 * Create a command pool for allocation command buffers from
	 *
	 * @param queueFamilyIndex Family index of the queue to create the command pool for
	 * @param createFlags (Optional) Command pool creation flags (Defaults to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
	 *
	 * @note Command buffers allocated from the created pool can only be submitted to a queue with the same family index
	 *
	 * @return A handle to the created command buffer
	 */
	VkCommandPool VulkanDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
		cmdPoolInfo.flags = createFlags;
		VkCommandPool cmdPool;
		VK_CHECK_RESULT(vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &cmdPool));
		return cmdPool;
	}

	/**
	 * Allocate a command buffer from the command pool
	 *
	 * @param level Level of the new command buffer (primary or secondary)
	 * @param (Optional) begin If true, recording on the new command buffer will be started (vkBeginCommandBuffer) (Defaults to false)
	 *
	 * @return A handle to the allocated command buffer
	 */
	VkCommandBuffer VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, bool begin)
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = commandPool;
		cmdBufAllocateInfo.level = level;
		cmdBufAllocateInfo.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(logicalDevice, &cmdBufAllocateInfo, &cmdBuffer));

		// If requested, also start recording for the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo commandBufferBI{};
			commandBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &commandBufferBI));
		}

		return cmdBuffer;
	}

	void VulkanDevice::beginCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo commandBufferBI{};
		commandBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBI));
	}

	/**
	 * Finish command buffer recording and submit it to a queue
	 *
	 * @param commandBuffer Command buffer to flush
	 * @param queue Queue to submit the command buffer to
	 * @param free (Optional) Free the command buffer once it has been submitted (Defaults to true)
	 *
	 * @note The queue that the command buffer is submitted to must be from the same family index as the pool it was allocated from
	 * @note Uses a fence to ensure command buffer has finished executing
	 */
	void VulkanDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
	{
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		VkFence fence;
		VK_CHECK_RESULT(vkCreateFence(logicalDevice, &fenceInfo, nullptr, &fence));

		// Submit to the queue
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
		// Wait for the fence to signal that command buffer has finished executing
		VK_CHECK_RESULT(vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, 100000000000));

		vkDestroyFence(logicalDevice, fence, nullptr);

		if (free)
		{
			vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
		}
	}

	void Camera::updateViewMatrix()
	{
		// std::string ps = std::to_string(position.x) + " " + std::to_string(position.y) + " " + std::to_string(position.z) + " " + std::to_string(rotation.x) + " " + std::to_string(rotation.y) + " " + std::to_string(rotation.z);
		// printf("%s\n", ps.c_str());
		glm::mat4 rotM = glm::mat4(1.0f);
		glm::mat4 transM;

		rotM = glm::rotate(rotM, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		transM = glm::translate(glm::mat4(1.0f), position * glm::vec3(1.0f, 1.0f, -1.0f));

		if (type == CameraType::firstperson)
		{
			matrices.view = rotM * transM;
		}
		else
		{
			matrices.view = transM * rotM;
		}

		updated = true;
	}

	bool Camera::moving()
	{
		return keys.left || keys.right || keys.up || keys.down;
	}

	float Camera::getNearClip()
	{
		return znear;
	}

	float Camera::getFarClip()
	{
		return zfar;
	}
	std::string Camera::debugString()
	{
		return "pos=[" + std::to_string(position.x) + "," + std::to_string(position.y) + "," + std::to_string(position.z) + "] rot=[" + std::to_string(rotation.x) + "," + std::to_string(rotation.y) + "," + std::to_string(rotation.z) + "]";
	}

	void Camera::setPerspective(float fov, float aspect, float znear, float zfar)
	{
		this->fov = fov;
		this->znear = znear;
		this->zfar = zfar;
		matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	};

	void Camera::updateAspectRatio(float aspect)
	{
		matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	}

	void Camera::setPosition(glm::vec3 position)
	{
		this->position = position;
		updateViewMatrix();
	}

	void Camera::setRotation(glm::vec3 rotation)
	{
		this->rotation = rotation;
		updateViewMatrix();
	};

	void Camera::rotate(glm::vec3 delta)
	{
		this->rotation += delta;
		updateViewMatrix();
	}

	void Camera::setTranslation(glm::vec3 translation)
	{
		this->position = translation;
		updateViewMatrix();
	};

	void Camera::translate(glm::vec3 delta)
	{
		this->position += delta;
		updateViewMatrix();
	}

	void Camera::update(float deltaTime)
	{
		updated = false;
		if (type == CameraType::firstperson)
		{
			if (moving())
			{
				glm::vec3 camFront;
				camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
				camFront.y = sin(glm::radians(rotation.x));
				camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
				camFront = glm::normalize(camFront);

				float moveSpeed = deltaTime * movementSpeed;

				if (keys.up)
					position += camFront * moveSpeed;
				if (keys.down)
					position -= camFront * moveSpeed;
				if (keys.left)
					position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
				if (keys.right)
					position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

				updateViewMatrix();
			}
		}
	};

	/**
	 * @brief Update camera passing separate axis data (gamepad)
	 * @return Returns true if view or position has been changed
	 */
	bool Camera::updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)
	{
		bool retVal = false;

		if (type == CameraType::firstperson)
		{
			// Use the common console thumbstick layout
			// Left = view, right = move

			const float deadZone = 0.0015f;
			const float range = 1.0f - deadZone;

			glm::vec3 camFront;
			camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
			camFront.y = sin(glm::radians(rotation.x));
			camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
			camFront = glm::normalize(camFront);

			float moveSpeed = deltaTime * movementSpeed * 2.0f;
			float rotSpeed = deltaTime * rotationSpeed * 50.0f;

			// Move
			if (fabsf(axisLeft.y) > deadZone)
			{
				float pos = (fabsf(axisLeft.y) - deadZone) / range;
				position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
				retVal = true;
			}
			if (fabsf(axisLeft.x) > deadZone)
			{
				float pos = (fabsf(axisLeft.x) - deadZone) / range;
				position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
				retVal = true;
			}

			// Rotate
			if (fabsf(axisRight.x) > deadZone)
			{
				float pos = (fabsf(axisRight.x) - deadZone) / range;
				rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
				retVal = true;
			}
			if (fabsf(axisRight.y) > deadZone)
			{
				float pos = (fabsf(axisRight.y) - deadZone) / range;
				rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
				retVal = true;
			}
		}
		else
		{
			// todo: move code from example base class for look-at
		}

		if (retVal)
		{
			updateViewMatrix();
		}

		return retVal;
	}

	void Buffer::create(VulkanDevice *device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
						VkDeviceSize size, bool map)
	{
		this->device = device->logicalDevice;
		device->createBuffer(usageFlags, memoryPropertyFlags, size, &buffer, &memory, "uniformBuffer");
		descriptor = {buffer, 0, size};
		if (map)
		{
			VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, memory, 0, size, 0, &mapped));
		}
	}

	void Buffer::destroy()
	{
		if (mapped)
		{
			unmap();
		}
		vkDestroyBuffer(device, buffer, nullptr);
		vkFreeMemory(device, memory, nullptr);
		buffer = VK_NULL_HANDLE;
		memory = VK_NULL_HANDLE;
	}

	void Buffer::map()
	{
		VK_CHECK_RESULT(vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &mapped));
	}

	void Buffer::unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(device, memory);
			mapped = nullptr;
		}
	}

	void Buffer::flush(VkDeviceSize size)
	{
		VkMappedMemoryRange mappedRange{};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.size = size;
		VK_CHECK_RESULT(vkFlushMappedMemoryRanges(device, 1, &mappedRange));
	}
}
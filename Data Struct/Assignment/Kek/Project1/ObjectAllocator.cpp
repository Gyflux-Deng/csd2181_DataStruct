///*!************************************************************************
//\file   ObjectAllocator.cpp
//\author Maojie Deng (2200840)
//\par    SIT email: 2200840@sit.singaporetech.edu.sg
//\par    DP email: maojie.deng@digipen.edu
//\par    Course: csd2183
//\par    Assignment 1
//\date   31-01-2023
//
//\brief
//**************************************************************************/
#include "ObjectAllocator.h"
#include <iostream>
#include <cstring>
#include <stdio.h>


//******Definition of object allocator*****//

//****Object Allocator Constructors*****//
//Constructor with size and OAConfig as input

/**
 * @brief Constructs an ObjectAllocator.
 * Initializes an ObjectAllocator with a specific object size and configuration settings.
 * It calculates the total page size based on the object size, padding, alignment, and header
 * information. If the C++ memory manager is not being used, it immediately allocates one page
 * to start the allocation process.
 * @param ObjectSize The size of each object to be managed by the allocator.
 * @param config Configuration settings for the allocator, including padding and alignment.
 */
ObjectAllocator::ObjectAllocator(size_t ObjectSize, const OAConfig& config) :PageList_{ nullptr }, FreeList_{ nullptr }, Config_{ config }, Stats_{}
{
    Stats_.ObjectSize_ = ObjectSize;
    const size_t pointer_size = sizeof(void*);

    //calculate the size of the middle block including header, object, and padding
    size_t midBlock = Config_.HBlockInfo_.size_ + (2 * Config_.PadBytes_) + Stats_.ObjectSize_;

    //if alignment is required, adjust the middle block size
    if (Config_.Alignment_ > 0)
    {
        size_t lfAlight = CalculatePadding(pointer_size + Config_.HBlockInfo_.size_ + Config_.PadBytes_, Config_.Alignment_);
        Config_.LeftAlignSize_ = static_cast<unsigned int>(lfAlight);
        size_t lfAlight2 = CalculatePadding(midBlock, Config_.Alignment_);
        Config_.InterAlignSize_ = static_cast<unsigned int>(lfAlight2);

        //adjust the block size to include the inter-object alignment padding
        midBlock += Config_.InterAlignSize_;
    }

    //calculate total page size
    Stats_.PageSize_ = CalculateTotalPageSize(pointer_size, Config_.LeftAlignSize_, midBlock,
        Config_.ObjectsPerPage_, Config_.InterAlignSize_);

    if (!Config_.UseCPPMemManager_)
    {
        try
        {
            AllocateNewPage();
        }
        catch (OAException& exception)
        {
            throw(exception);
        }
    }

}

/**
 * @brief Calculates the total page size needed based on configuration and object size.
 * This function computes the total size of a page, including space for the object, header,
 * padding, alignment, and pointers for list management. It ensures that the page layout adheres
 * to the specified alignment and padding requirements. 
 * @param pointerSize Size of a pointer, used for page list link.
 * @param leftAlignSize Size of left alignment padding.
 * @param block_size Size of a single block, including object, header, and padding.
 * @param objectsPerPage Number of objects per page.
 * @param interAlignSize Size of alignment padding between objects.
 * @return size_t The total size of a page in bytes.
 */
size_t ObjectAllocator::CalculateTotalPageSize(size_t pointerSize, size_t leftAlignSize, size_t block_size, size_t objectsPerPage,
    size_t interAlignSize) const
{
    size_t pageSize = pointerSize + leftAlignSize;
    pageSize += block_size * objectsPerPage;
    if (objectsPerPage > 1)
    { //subtract inter-object alignment for the last object
        pageSize -= interAlignSize;
    }
    return pageSize;
}

/**
 * @brief Calculates the padding needed to satisfy the alignment requirement.
 * This utility function computes how much padding is required to align the next block
 * or structure according to the specified alignment. It is used to ensure that all objects
 * and headers are correctly aligned in memory.
 * @param size Current size before alignment.
 * @param alignment Required alignment boundary.
 * @return size_t The size of padding in bytes needed to achieve the alignment.
 */
size_t ObjectAllocator::CalculatePadding(size_t size, size_t alignment)
{
    size_t padding = alignment - (size % alignment);
    return (padding == alignment) ? 0 : padding;
}


//Destructor
//Destroys the object manager
/**
 * @brief Destructor for the ObjectAllocator.
 * Cleans up all allocated pages by the allocator. It iterates through the page list,
 * deallocating each page to release all resources before the allocator is destroyed.
 */
ObjectAllocator::~ObjectAllocator()
{
    while (PageList_ != NULL)
    {
        GenericObject* temp = PageList_->Next;
        delete[] PageList_;
        PageList_ = temp;
    }
}


//****End Object Allocator Constructors*****//


//****Object Allocator Member Functions*****//
//Takes an object from free and give it to client (Simulates new)
//THrows exception is the object can't be allocated
/**
 * @brief Allocates a block of memory for an object.
 * This function allocates a block of memory of size defined in the allocator's
 * configuration. It can use the C++ memory manager or the allocator's custom
 * mechanism depending on the configuration. It updates allocator statistics upon
 * successful allocation and performs necessary bookkeeping.
 * @param label Optional label for the allocated block, used for debugging purposes.
 * @return void* Pointer to the allocated block of memory.
 * @throw OAException Throws an exception if memory cannot be allocated due to
 *        reaching the limit of pages or no system memory available.
 */
void* ObjectAllocator::Allocate(const char* label)
{
    //using their own allocate
    if (Config_.UseCPPMemManager_)
    {
        try {
            // Allocate new block using c++ new
            void* newBlock = new char[Stats_.ObjectSize_];

            // Update statistics
            ++Stats_.ObjectsInUse_;
            ++Stats_.Allocations_;
            ++Stats_.MostObjects_;
            --Stats_.FreeObjects_;

            // Return allocated block
            return newBlock;
        }
        catch (const std::bad_alloc&)
        {
            throw OAException(OAException::E_NO_MEMORY, "Allocate: No system memory available.");
        }
    }

    //If the next free list is outside of the page limit
    if (Stats_.FreeObjects_ <= 0)
    {
        //Check if reached pages limit
        if (Config_.MaxPages_ > 0 && Stats_.PagesInUse_ >= Config_.MaxPages_)
        {
            throw OAException(OAException::E_NO_PAGES, "Allocate:  You have reached maximum pages limit.");
        }
        AllocateNewPage();
    }

    //Give the current free block to client
    void* allocatedPtr = FreeList_;
    FreeList_ = FreeList_->Next;
    // //Set object blocks to free
    memset(allocatedPtr, ALLOCATED_PATTERN, Stats_.ObjectSize_);

    ++Stats_.Allocations_;
    ++Stats_.ObjectsInUse_;
    --Stats_.FreeObjects_;
    if (Stats_.ObjectsInUse_ > Stats_.MostObjects_)
    {
        Stats_.MostObjects_ = Stats_.ObjectsInUse_;
    }

    BlockHeaderCheck(allocatedPtr, label);

    return allocatedPtr;
}

/**
 * @brief Allocates a new page of blocks and adds them to the free list.
 * This function is called when there are no free blocks available for allocation.
 * It allocates a new page, initializes it, and links the blocks within the page
 * into the allocator's free list. It updates the allocator's statistics accordingly.
 * @throw OAException Throws an exception if a new page cannot be allocated due
 *        to system memory constraints.
 */
void ObjectAllocator::AllocateNewPage()
{
    //// Allocate a new page
    char* newPage = nullptr;

    try
    {
        newPage = new char[Stats_.PageSize_] {};
    }
    catch (const std::bad_alloc&)
    {
        throw OAException(OAException::E_NO_MEMORY, "allocate_new_page: No system memory available");

    }

    // Initialize the new page by setting up the free list within the page
    char* currentBlock = newPage;

    // Skip the space for the page list link pointer
    currentBlock += sizeof(GenericObject*);

    // Apply left alignment pattern if necessary
    if (Config_.LeftAlignSize_ > 0)
    {
        memset(currentBlock, ALIGN_PATTERN, Config_.LeftAlignSize_);
        currentBlock += Config_.LeftAlignSize_;
    }

    // Construct the free list for the new page
    for (size_t i = 0; i < Config_.ObjectsPerPage_; ++i)
    {
        // Apply header info and padding before the object
        if (Config_.HBlockInfo_.size_ > 0)
        {
            //  memset(currentBlock, PAD_PATTERN, Config_.HBlockInfo_.size_);
            currentBlock += Config_.HBlockInfo_.size_;
        }

        if (Config_.PadBytes_ > 0)
        {
            memset(currentBlock, PAD_PATTERN, Config_.PadBytes_);
            currentBlock += Config_.PadBytes_;
        }

        // Mark the object area with the unallocated pattern
        memset(currentBlock, UNALLOCATED_PATTERN, Stats_.ObjectSize_);

        // Link this block into the free list
        GenericObject* newObject = reinterpret_cast<GenericObject*>(currentBlock);
        newObject->Next = FreeList_;
        FreeList_ = newObject;
        currentBlock += Stats_.ObjectSize_;

        // Apply padding after the object
        if (Config_.PadBytes_ > 0)
        {
            memset(currentBlock, PAD_PATTERN, Config_.PadBytes_);
            currentBlock += Config_.PadBytes_;
        }

        // Apply inter-object alignment pattern if necessary and not the last object
        if (Config_.InterAlignSize_ > 0 && i < Config_.ObjectsPerPage_ - 1)
        {
            memset(currentBlock, ALIGN_PATTERN, Config_.InterAlignSize_);
            currentBlock += Config_.InterAlignSize_;
        }

        // Update the number of free objects
        ++Stats_.FreeObjects_;
    }

    // Link the new page into the page list
    GenericObject* pageHeader = reinterpret_cast<GenericObject*>(newPage);
    pageHeader->Next = PageList_;
    PageList_ = pageHeader;
    ++Stats_.PagesInUse_;

}

/**
 * @brief Checks and updates the block header during allocation.
 * This function updates the block header with allocation information, which can
 * include allocation count, use count, and optional external information such as
 * a debug label. The exact behavior depends on the allocator's header block configuration.
 * @param allocatedBlock Pointer to the allocated block whose header is to be updated.
 * @param label Optional debug label for the block.
 */
void ObjectAllocator::BlockHeaderCheck(void* allocatedBlock, const char* label) const
{
    if (Config_.HBlockInfo_.type_ == Config_.hbNone)
    {
        return;
    }
    char* headerBlock = reinterpret_cast<char*>(allocatedBlock) - Config_.PadBytes_ - Config_.HBlockInfo_.size_;

    switch (Config_.HBlockInfo_.type_)
    {
    case OAConfig::HBLOCK_TYPE::hbBasic:
    {
        unsigned* allocation = reinterpret_cast<unsigned*>(headerBlock);
        *allocation = Stats_.Allocations_;
        *(headerBlock += sizeof(unsigned)) = true;
        break;
    }
    case OAConfig::HBLOCK_TYPE::hbExtended:
    {
        headerBlock += Config_.HBlockInfo_.additional_;
        short* useCount = reinterpret_cast<short*>(headerBlock);
        ++(*useCount);
        headerBlock += sizeof(short);

        // Set alloc number
        unsigned* allocation = reinterpret_cast<unsigned*>(headerBlock);
        *allocation = Stats_.Allocations_;
        *(headerBlock += sizeof(unsigned)) = true; 

        // Set flag
        //*headerBlock = 1;
        break;
    }
    case OAConfig::HBLOCK_TYPE::hbExternal:
    {
       
        MemBlockInfo** externalHeader = reinterpret_cast<MemBlockInfo**>(headerBlock);
        try
        {
            // Allocate external header block
            (*externalHeader) = new MemBlockInfo{};

            // Allocate memory for label
            if (label) {
                (*externalHeader)->label = new char[std::strlen(label) + 1] {};
            }
        }
        catch (std::bad_alloc&)
        {
            throw(OAException(OAException::E_NO_MEMORY, "Allocate: No system memory available."));
        }

        // Assign external header members
        (*externalHeader)->in_use = true;
        (*externalHeader)->alloc_num = Stats_.Allocations_;
        if (label) 
        {
            std::strcpy((*externalHeader)->label, label);
        }
        break;
    }
    case OAConfig::HBLOCK_TYPE::hbNone:
    {
        break;
    }


    }
}

/**
 * @brief Resets the block header during deallocation.
 * This function clears the block header information, preparing the block for reuse.
 * It handles different header configurations by resetting allocation count, use count,
 * and external information as necessary.
 * @param allocatedBlock Pointer to the block being deallocated.
 */
void ObjectAllocator::BlockHeaderCheckFree(void* allocatedBlock) const
{
    if (Config_.HBlockInfo_.type_ == Config_.hbNone)
    {
        return;
    }
    char* headerBlock = reinterpret_cast<char*>(allocatedBlock) - Config_.PadBytes_ - Config_.HBlockInfo_.size_;

    switch (Config_.HBlockInfo_.type_)
    {
    case OAConfig::HBLOCK_TYPE::hbBasic:
    {
       
        // Update alloc num
        unsigned* allocation = reinterpret_cast<unsigned*>(headerBlock);
        *allocation = 0;
        *(headerBlock += sizeof(unsigned)) = false;

    
        break;
    }
    case OAConfig::HBLOCK_TYPE::hbExtended:
    {
        headerBlock += Config_.HBlockInfo_.additional_;
        headerBlock += sizeof(short);

        // Set alloc number
        unsigned* allocation = reinterpret_cast<unsigned*>(headerBlock);
        *allocation = 0;
        *(headerBlock += sizeof(unsigned)) = false;

        break;
    }
    case OAConfig::HBLOCK_TYPE::hbExternal:
    {
        
        MemBlockInfo** externalHeader = reinterpret_cast<MemBlockInfo**>(headerBlock);

        // Allocate external header block
        (*externalHeader) = new MemBlockInfo{};

        // Allocate memory for label
        if ((*externalHeader) && (*externalHeader)->label)
        {
            delete[](*externalHeader)->label;
        }
        if ((externalHeader))
        {
            delete (*externalHeader);
            (*externalHeader) = nullptr;
        }
        break;
    }
    case OAConfig::HBLOCK_TYPE::hbNone:
    {
        break;
    }


    }
}

/**
 * @brief Frees a previously allocated object and updates allocator statistics.
 * This function releases an object back to the allocator's free list, allowing it to be reused.
 * It performs several checks to ensure the object can be safely freed, including whether the
 * object is already freed, if it's within the allocator's managed memory boundaries, and if the
 * object's memory is corrupted. It updates the allocator's statistics upon successful deallocation.
 * @param Object Pointer to the object to be freed.
 * @throw OAException Throws an exception if the object has already been freed, is not within the
 *        allocator's boundaries, or its memory is corrupted.
 */
void ObjectAllocator::Free(void* Object)
{
    //return if pointer is empty
    if (!Object)
    {
        return;
    }

    //using theirs 
    if (Config_.UseCPPMemManager_)
    {
        delete[] reinterpret_cast<char*>(Object);

        //update allocator statistics
        ++Stats_.Deallocations_;
        ++Stats_.FreeObjects_;
        //decrement the count of objects in use
        --Stats_.ObjectsInUse_;
        return;
    }

    //check for free
    if (CheckErrorFree(reinterpret_cast<GenericObject*>(Object)))
    {
        throw OAException(OAException::E_MULTIPLE_FREE, "Free: Object has already been freed.");
    }
    //check the block boundary
    if (CheckBlockBoundary(Object))
    {
        throw OAException(OAException::E_BAD_BOUNDARY, "Boundary: Object has bad boundary.");
    }
    //check for the corruption 
    if (CorruptedCheck(reinterpret_cast<GenericObject*>(Object)))
    {
        throw OAException(OAException::E_CORRUPTED_BLOCK, "Corrupted: Object has corruption.");
    }


    //set object blocks to free
    memset(Object, FREED_PATTERN, Stats_.ObjectSize_);

    //add the object back to the free list
    GenericObject* addBlock = reinterpret_cast<GenericObject*>(Object);
    addBlock->Next = FreeList_;
    FreeList_ = addBlock;

    //update allocator statistics
    ++Stats_.Deallocations_;
    ++Stats_.FreeObjects_;
    //decrement the count of objects in use
    --Stats_.ObjectsInUse_;
    BlockHeaderCheckFree(addBlock);

}

/**
 * @brief Checks if the specified block has already been freed.
 * Iterates through the free list to determine if the block is already present,
 * indicating it has been previously freed. This helps in detecting double free errors.
 * @param block Pointer to the block to check.
 * @return bool True if the block is found in the free list, indicating it's already been freed; false otherwise.
 */
bool ObjectAllocator::CheckErrorFree(GenericObject* block) const
{
    for (GenericObject* current = FreeList_; current != nullptr; current = current->Next)
    {
        if (current == block)
        {   //found the block in the free list
            return true;
        }
    }
    //block not found in the free list
    return false;
}

/**
 * @brief Checks for memory corruption around a block's boundaries.
 * Verifies the integrity of padding bytes around the allocated block to detect memory corruption.
 * This function is part of the allocator's debugging and validation mechanisms to ensure memory safety.
 * @param block Pointer to the block to check for corruption.
 * @return bool True if corruption is detected in the padding bytes; false if the padding is intact.
 */
bool ObjectAllocator::CorruptedCheck(GenericObject* block) const
{
    if (Config_.PadBytes_ == 0)
    {
        return false;
    }

    //convert
    unsigned char* blockStart = reinterpret_cast<unsigned char*>(block);

    //calculate padding
    unsigned char* leftPadding = blockStart - Config_.PadBytes_;
    unsigned char* rightPadding = blockStart + Stats_.ObjectSize_;

    //check if any byte in the left padding
    for (size_t i = 0; i < Config_.PadBytes_; ++i)
    {
        //check left padding 
        if (leftPadding[i] != PAD_PATTERN)
        {
            return true;
        }
    }

    //check if any byte in the right padding
    for (size_t i = 0; i < Config_.PadBytes_; ++i)
    {
        //check right padding
        if (rightPadding[i] != PAD_PATTERN)
        {
            //corrupted
            return true;
        }
    }

    //if none return false
    return false;
}

/**
 * @brief Checks if the given block is within the boundary of any allocated pages.
 * This function iterates through all pages managed by the ObjectAllocator to determine
 * if the provided block address falls within the boundaries of these pages. It's used
 * to verify that a block being freed or used is actually part of the allocator's memory
 * pool, helping to prevent errors related to invalid memory access.
 * @param block Pointer to the block whose boundary is to be checked.
 * @return bool Returns true if the block is not within the boundary of any allocated page,
 * indicating a potential error; otherwise, false.
 */
bool ObjectAllocator::CheckBlockBoundary(void* block)
{

    char* blockCharPtr = static_cast<char*>(block);

    //loop
    for (GenericObject* currentPage = PageList_; currentPage != nullptr; currentPage = currentPage->Next)
    {

        //get the address
        char* pageStart = reinterpret_cast<char*>(currentPage);
        char* pageEnd = pageStart + Stats_.PageSize_;

        //check if the block within boundaries
        if (blockCharPtr >= pageStart && blockCharPtr < pageEnd)
        {
            //the block is within boundary
            return false;
        }
    }

    //the block not within the boundary
    return true;
}

/**
 * @brief Iterates through each allocated block in use and calls a provided callback function.
 * This function traverses all pages and blocks managed by the ObjectAllocator, invoking
 * a user-provided callback function for each block that is currently in use. It can be
 * used for debugging, logging, or performing custom actions on active blocks.
 * @param fn A callback function that takes two parameters: a pointer to the block and its size.
 * @return unsigned The count of blocks in use that the callback function was called for.
 */
unsigned ObjectAllocator::DumpMemoryInUse(DUMPCALLBACK fn) const
{
    unsigned count = 0;


    for (GenericObject* currentPage = PageList_; currentPage != nullptr; currentPage = currentPage->Next)
    {
        // Calculate the starting position of the first block in the page
        char* currentBlockPtr = reinterpret_cast<char*>(currentPage) + sizeof(GenericObject*) + Config_.LeftAlignSize_
            + Config_.HBlockInfo_.size_ + Config_.PadBytes_;

        // Iterate through each block in the page
        for (unsigned i = 0; i < Config_.ObjectsPerPage_; ++i)
        {
            // Check if the block's header indicates it's in use
            GenericObject* currentBlock = reinterpret_cast<GenericObject*>(currentBlockPtr);
            if (!CheckErrorFree(currentBlock))
            {
                count++; // Increment the count of blocks in use
                fn(currentBlockPtr, Stats_.ObjectSize_);
            }

            // Move to the next block
            currentBlockPtr += Config_.HBlockInfo_.size_ + (2 * Config_.PadBytes_) + Stats_.ObjectSize_;
            if (i < Config_.ObjectsPerPage_ - 1)
            {
                currentBlockPtr += Config_.InterAlignSize_;
            }
        }
    }

    return count;
}

/**
 * @brief Validates each page and block for potential corruption, calling a callback for each.
 * This function checks all pages and blocks managed by the ObjectAllocator for signs of
 * corruption, such as incorrect padding bytes. For each block that is potentially corrupted,
 * a user-provided callback function is invoked, allowing for custom handling or logging.
 * @param fn A callback function that takes two parameters: a pointer to the potentially corrupted block and its size.
 * @return unsigned The count of potentially corrupted blocks that the callback function was called for.
 */
unsigned ObjectAllocator::ValidatePages(VALIDATECALLBACK fn) const
{
    //tracker
    unsigned corruptedCount = 0;
    //loop base off the pages
    for (GenericObject* currentPage = PageList_; currentPage != nullptr; currentPage = currentPage->Next)
    {
        //assign the first block
        char* currentBlockPtr = reinterpret_cast<char*>(currentPage) + sizeof(GenericObject*) + Config_.LeftAlignSize_
            + Config_.HBlockInfo_.size_ + Config_.PadBytes_;

        //loop
        for (unsigned i = 0; i < Config_.ObjectsPerPage_; ++i)
        {
            //check if the current block is corrupted
            GenericObject* currentBlock = reinterpret_cast<GenericObject*>(currentBlockPtr);
            if (CorruptedCheck(currentBlock))
            {

                ++corruptedCount;
                //call the callback
                fn(currentBlockPtr, Stats_.ObjectSize_);
            }

            //move to the next block in the page
            currentBlockPtr += Config_.HBlockInfo_.size_ + (2 * Config_.PadBytes_) + Stats_.ObjectSize_;

            if (i < Config_.ObjectsPerPage_ - 1)
            {   //add back allignment
                currentBlockPtr += Config_.InterAlignSize_;
            }
        }
    }

    return corruptedCount;
}

/**
 * @brief Frees all empty pages from the allocator's page list.
 * Iterates through the page list to identify and free pages that do not contain any allocated blocks.
 * This can help in reducing memory usage by releasing pages that are no longer needed. The function
 * updates the allocator's statistics to reflect changes in the number of pages in use and the number
 * of free objects.
 * @return unsigned int The number of pages that were freed during the operation.
 */
unsigned ObjectAllocator::FreeEmptyPages()
{
    unsigned int freedPageCount = 0;
    GenericObject** currentPtrRef = &PageList_; // Pointer to pointer to iterate and modify the page list

    while (*currentPtrRef)
    {
        GenericObject* currentPage = *currentPtrRef;
        if (PageIsEmpty(currentPage))
        {
            GenericObject* nextPage = currentPage->Next;

            // Free all blocks in the page, assuming freeBlocks is a function that properly clears blocks from the free list
            freeBlocks(currentPage);

            // Deallocate the page
            delete[] reinterpret_cast<char*>(currentPage);

            // Update the page list to bypass the deleted page
            *currentPtrRef = nextPage;

            //update statistics
            ++freedPageCount;
            --Stats_.PagesInUse_;
            Stats_.FreeObjects_ -= Config_.ObjectsPerPage_;

        }
        else
        {
            currentPtrRef = &(*currentPtrRef)->Next;
        }
    }

    return freedPageCount;
}
/**
 * @brief Checks if a given page is empty (i.e., all blocks within the page are free).
 * This function iterates through all blocks in a specified page to determine if the page
 * contains no allocated blocks. A page is considered empty if all its blocks are part of
 * the free list.
 * @param page A pointer to the page to check.
 * @return bool True if the page is empty; otherwise, false.
 */
bool ObjectAllocator::PageIsEmpty(GenericObject* page) const
{
    char* blockPtr = reinterpret_cast<char*>(page) + sizeof(GenericObject*) + Config_.LeftAlignSize_ + Config_.HBlockInfo_.size_ + Config_.PadBytes_;
    for (unsigned i = 0; i < Config_.ObjectsPerPage_; ++i)
    {
        GenericObject* currentBlock = reinterpret_cast<GenericObject*>(blockPtr);

        if (!IsBlockFree(currentBlock))
        {
            return false;
        }


        blockPtr += Config_.HBlockInfo_.size_ + (2 * Config_.PadBytes_) + Stats_.ObjectSize_;

        //check if its not last
        if (i < Config_.ObjectsPerPage_ - 1)
        {
            blockPtr += Config_.InterAlignSize_;
        }
    }
    return true;
}
/**
 * @brief Determines if a specific block is free (i.e., part of the free list).
 * This function checks whether a given block is currently part of the allocator's
 * free list, indicating that the block is available for allocation.
 * @param block A pointer to the block to check.
 * @return bool True if the block is part of the free list; otherwise, false.
 */
bool ObjectAllocator::IsBlockFree(GenericObject* block) const
{
    // Iterate through the FreeList_ to see if the block is part of it
    for (GenericObject* freeBlock = FreeList_; freeBlock != nullptr; freeBlock = freeBlock->Next)
    {
        if (freeBlock == block)
        {
            return true; // Found the block in the FreeList_
        }
    }
    return false; // Block is not in the FreeList_, hence it's allocated
}
/**
 * @brief Removes all free blocks within a specified page from the free list.
 * This function iterates through the free list and removes any blocks that are
 * located within the boundaries of a specified page. This is typically called
 * before deallocating a page to ensure that no dangling pointers remain in the
 * free list.
 * @param block A pointer to the page from which to remove blocks.
 */
void ObjectAllocator::freeBlocks(GenericObject* block)
{
    GenericObject** freePtrRef = &FreeList_; // Pointer to a pointer for direct list manipulation

    while (*freePtrRef)
    {
        // Calculate the start of the page for the current block
        char* startOfPage = reinterpret_cast<char*>(block);
        char* endOfPage = startOfPage + Stats_.PageSize_;

        // Check if the current free block is within the page to be freed
        if (reinterpret_cast<char*>(*freePtrRef) >= startOfPage && reinterpret_cast<char*>(*freePtrRef) < endOfPage)
        {
            // Remove the block from the free list
            GenericObject* toBeRemoved = *freePtrRef; // Temporarily store the block to be removed
            *freePtrRef = toBeRemoved->Next; // Bypass the block in the list
        }
        else
        {
            // Move to the next block if not within the page to be freed
            freePtrRef = &(*freePtrRef)->Next;
        }
    }
}

//******Testing member functions********/
/**
 * @brief Sets the debug state of the allocator.
 * This function enables or disables debugging for the allocator, allowing for
 * detailed tracking and analysis of memory allocation and deallocation operations.
 * @param state If true, debugging is enabled. If false, debugging is disabled.
 */
void ObjectAllocator::SetDebugState(bool state)
{
    Config_.DebugOn_ = state;
}


/**
 * @brief Gets a pointer to the internal free list.
 * The free list contains blocks of memory that have been allocated but are
 * currently not in use. This function provides read-only access to the start
 * of the free list, allowing inspection without modification.
 * @return const void* A constant pointer to the first block in the free list.
 */
const void* ObjectAllocator::GetFreeList() const
{
    return FreeList_;
}

/**
 * @brief Gets a pointer to the page list.
 * The page list contains the pages of memory that have been allocated to hold
 * objects. This function provides read-only access to the start of the page list,
 * enabling inspection of the page structure without modification.
 * @return const void* A constant pointer to the first page in the page list.
 */
const void* ObjectAllocator::GetPageList() const
{
    return PageList_;
}

/**
 * @brief Retrieves the current configuration settings of the allocator.
 * This function returns a copy of the allocator's configuration settings, which
 * include various parameters controlling its behavior, such as object size, padding,
 * and alignment requirements.
 * @return OAConfig A copy of the current configuration settings.
 */
OAConfig ObjectAllocator::GetConfig() const
{
    return Config_;
}

/**
 * @brief Gets the statistics of the allocator.
 * This function returns a structure containing various statistics about the allocator,
 * such as the number of objects currently allocated, the total number of allocations
 * and deallocations, and the size of objects being managed.
 * @return OAStats A structure containing the allocator's statistics.
 */
OAStats ObjectAllocator::GetStats() const
{
    return Stats_;
}




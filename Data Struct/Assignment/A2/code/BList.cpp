/*!************************************************************************
\file   BList.cpp
\author Maojie Deng (2200840)
\par    SIT email: 2200840@sit.singaporetech.edu.sg
\par    DP email: maojie.deng@digipen.edu
\par    Course: csd2183
\par    Assignment 2
\date   16-02-2023

\brief
This is a Blist assignment cpp file 
**************************************************************************/

#include "BList.h"

/**
 * @brief Default constructor for BList.
 * Initializes an empty BList with head and tail pointers set to nullptr. Also sets the node size
 * and array size statistics for the list based on the template parameters.
 * @tparam T The type of elements stored in the BList.
 * @tparam Size The fixed size of the array within each BNode.
 */
template <typename T, unsigned Size>
BList<T, Size>::BList() : head_{nullptr}, tail_{nullptr} 
{
  // Sets the size of a node.
  stats.NodeSize = sizeof(BNode);
  // Sets the maximum number of elements in each node's array.
  stats.ArraySize = Size; 
}

/**
 * @brief Copy constructor for BList.
 * Creates a deep copy of an existing BList. The new list will have its own copy of the nodes
 * and elements contained in the `rhs` list. Node connections (next and prev pointers) are
 * recreated to mirror those in the `rhs` list.
 * @param rhs Reference to the BList object to copy from.
 * @tparam T The type of elements stored in the BList.
 * @tparam Size The fixed size of the array within each BNode.
 */
template <typename T, unsigned Size>
BList<T, Size>::BList(const BList &rhs) : head_{nullptr}, tail_{nullptr}
{
    *this = rhs;
}

 

/**
 * @brief Destructor for the BList class.
 * Cleans up all resources used by the BList, effectively deleting all nodes
 * and clearing the list. This is done by calling the clear() method which
 * iteratively removes each node from the list.
 * @tparam T The type of elements stored in the BList.
 * @tparam Size The fixed size of the array within each BNode.
 */
template <typename T, unsigned Size>
BList<T, Size>::~BList()
{ 
  // Calls clear method to free all nodes.
  clear(); 
}

/**
 * @brief Copy assignment operator.
 * Assigns the contents of another BList object to this one. This involves
 * clearing the current list, if any, and deep copying each node from the
 * source list to this one. The stats of the list are also copied, ensuring
 * the new list has the same statistics as the source list. This method ensures
 * a deep copy, where the list and all its nodes are duplicated.
 * @param rhs A constant reference to the BList object to copy from.
 * @return BList<T, Size>& A reference to the current object after copying.
 * @tparam T The type of elements stored in the BList.
 * @tparam Size The fixed size of the array within each BNode.
 */
template <typename T, unsigned Size>
BList<T, Size>& BList<T, Size>::operator=(const BList<T, Size>&rhs)
{
  
    // Clear the current list to prepare for the copy.
    clear();

    // thisItr will iterate through the current list as we copy nodes.
    BNode* tempPtr = nullptr;

    // rhsItr iterates through the source list.
    BNode* ptrHead = rhs.head_;

    // Loop through each node in the source list.
    while (ptrHead)
    {
        // If this is not the first node, prepare the next node.
        if (tempPtr)
        {
            // Create a new node and link it to the current node's next.
            tempPtr->next = CreateNewNode();
            // Store the current node temporarily to update the previous link later.
            BNode* tempPrev = tempPtr;
            // Move to the new node.
            tempPtr = tempPtr->next;
            // Link back to the previous node.
            tempPtr->prev = tempPrev;
        }
        else
        {
            // For the first node, just create a new node and set it as the head.
            tempPtr = CreateNewNode();
            head_ = tempPtr;
        }

        // Copy the count from the source node.
        tempPtr->count = ptrHead->count;
        // Copy the values from the source node.
        for (int i = 0; i < tempPtr->count; ++i)
        {
            tempPtr->values[i] = ptrHead->values[i];
        }
        // Move to the next node in the source list.
        ptrHead = ptrHead->next;
    }
    // After the loop, set the last node as the tail.
    tail_ = tempPtr;
    // Copy the stats from the source list.
    stats = rhs.stats;

    // Return a reference to the current object.
    return *this;
}

/**
 * @brief Inserts an element at the front of the list.
 * If the list is empty, a new node is created and becomes both the head and tail of the list.
 * If the head node has space, the value is inserted at the front of the head node, shifting
 * existing elements right. If the head node is full, a new head node is created for the value.
 * @param value The value to be inserted at the front of the list.
 * @tparam T The type of elements stored in the BList.
 * @tparam Size The fixed size of the array within each BNode.
 */
template <typename T, unsigned Size>
void BList<T, Size>::push_front(const T& value)
{
  // Check if the list is empty
  if(!head_)
  {
    // Create a new node and initialize as head and tail
    BNode* newNode = CreateNewNode();
    head_ = tail_ = newNode;
    // Insert the value and increment the node and item counts
    Increament(newNode,value); // Assuming a typo here, should probably be Increment
    ++stats.ItemCount;
  }
  else
  {
    // Check if the head node has space
    if (head_->count < stats.ArraySize)
    {
      // Shift existing values to make space for the new value at the front
      for(int i = head_->count; i > 0; --i)
      {
        head_->values[i] = head_->values[i - 1];
      }
      // Insert the value at the front
      Increament(head_,value);
      ++stats.ItemCount;
    } 
    else 
    {
      // Create a new node, insert it at the front of the list
      BNode* newNode = CreateNewNode();
      newNode->next = head_;
      head_->prev = newNode;
      head_ = newNode;
      // Insert the value into the new head node
      Increament(newNode,value);
      ++stats.ItemCount;
    }
  }
}

/**
 * @brief Inserts an element at the back of the list.
 * If the list is empty, a new node is created to store the value, becoming both the head and tail
 * of the list. If the tail node has space, the value is added to it. Otherwise, a new node is created
 * and linked as the new tail of the list, with the value inserted as the first element.
 * @param value The value to be inserted at the back of the list.
 * @tparam T The type of elements stored in the BList.
 * @tparam Size The fixed size of the array within each BNode.
 */
template <typename T, unsigned Size>
void BList<T, Size>::push_back(const T& value)
{
  // Check if the list is empty
  if(!head_)
  {
    // Create a new node and initialize as head and tail
    BNode* newNode = CreateNewNode();
    head_ = newNode;
    tail_ = newNode;
    // Insert the value and increment the node and item counts
    Increament(newNode,value);
    ++stats.ItemCount;
  }
  else
  {
    // Check if the tail node has space
    if(tail_->count < stats.ArraySize)
    {
      // Add the value to the tail node
      tail_->values[tail_->count] = value;
      ++tail_->count;
      ++stats.ItemCount;
    }
    else
    {
      // Create a new node, link it as the new tail
      BNode* newNode = CreateNewNode();
      tail_->next = newNode;
      newNode->prev = tail_;
      tail_ = newNode;
      // Insert the value into the new tail node
      Increament(newNode,value);
      ++stats.ItemCount;
    }
  }
}
/**
 * @brief Inserts a value into the list in a sorted manner.
 * This method inserts a value such that the list maintains its sorted order. 
 * It traverses the list to find the correct position for the new value. If the list is empty, 
 * it directly calls `push_front` to add the value. If the list is not empty but all current nodes are full or the 
 * value is greater than all existing values, it either appends the value to the end or splits the 
 * tail node to accommodate the new value.
 * @param value The value to insert into the list.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
void BList<T, Size>::insert(const T& value)
{
  BNode* ptrHead = head_; // Start at the head of the list
  bool inserted = false; // Flag to track if the value has been inserted
  int index{}; // Initialize index to 0

  // If the list is empty, insert the value at the front
  if(ptrHead == nullptr)
  {
    push_front(value);
    return;
  } 

  // Traverse the list to find the correct position for the new value
  while(ptrHead != nullptr)
  {
    // Compare the value with each value in the current node
    for(int i = 0; i < ptrHead->count; ++i)
    {
      // If the value is less than the current value, mark as inserted
      if(value < ptrHead->values[i])
      {
        inserted = true;
        break;
      }
      // Special case: check if the value fits before the first value of the next node
      if (i == ptrHead->count - 1 && ptrHead->next && value < ptrHead->next->values[0])
      {
        inserted = true;
        break;
      }
    }
    if(inserted == true)
    {
      break; // Exit the loop if the insertion point is found
    }
    ptrHead = ptrHead->next; // Move to the next node
  }

  // If no suitable position was found, the value is the largest and needs to be added at the end
  if (inserted == false)
  {
    // Check if there is space in the tail node
    if (tail_->count < stats.ArraySize)
    {
      push_back(value); // Add the value to the end of the list
    }
    else
    {
      Split(tail_, value); // Split the tail node to make space for the new value
    }
  }
  else // A suitable position was found
  {
    // Check if the current node is full
    if (ptrHead->count >= stats.ArraySize)
    {
      // Check if the value can be added to the next node
      if (ptrHead->values[ptrHead->count - 1] < value && ptrHead->next && ptrHead->next->count < stats.ArraySize) 
      {
        BNode* nextNode = ptrHead->next;

        // Shift values in the next node to make space for the new value at the beginning
        for(int i = nextNode->count; i > 0; --i) 
        {
          nextNode->values[i] = nextNode->values[i - 1];
        }
        Increament(nextNode,value);
        ++stats.ItemCount; // Update the total item count
      }
      else
      {
        Split(ptrHead, value); // Split the current node to accommodate the new value
      }
    }
    else // There is space in the current node
    {
      // Find the insertion point in the current node
      for(; index < ptrHead->count; ++index)
      {
        if (value < ptrHead->values[index])
        {
          break;
        }  
      }

      // Shift values to make space and insert the new value
      for(int i = ptrHead->count; i > index; --i)
      {
        ptrHead->values[i] = ptrHead->values[i - 1];
      }
      ptrHead->values[index] = value;
      ++ptrHead->count; // Update the count of values in the node
      ++stats.ItemCount; // Update the total item count
    }
  }
}

/**
 * @brief Removes an element at a specified index from the list.
 * This method traverses the list to find and remove the element at the given index. 
 * If the index points to a valid element, that element is removed, 
 * and the elements that follow are shifted to fill the gap. If removing an element results in an empty node, the node is deleted to maintain the integrity of the list.
 * @param index The index of the element to remove.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
void BList<T, Size>::remove(int index) //index
{

  BNode* ptrHead = head_; // Start at the head of the list
  int tempIndex{0}; // Initialize a temporary index to track the global index across nodes

  // Traverse nodes in the list
  while(ptrHead != nullptr)
  {
    // Traverse elements within the current node
    for(int i = 0; i < ptrHead->count; ++i)
    {
      // Check if the current global index matches the target index
      if(tempIndex == index)
      {
        // Shift elements to remove the target element
        for(int j = i; j < ptrHead->count - 1; ++j)
        {
          ptrHead->values[j] = ptrHead->values[j + 1];
        }
        --ptrHead->count; // Decrement the count of elements in the node
        --stats.ItemCount; // Decrement the global item count

        // Check if the current node is now empty
        if (ptrHead->count == 0)
        {
          // Special handling if the empty node is the head
          if (ptrHead == head_)
          {
            head_ = head_->next;
            if(head_) head_->prev = nullptr;
          }
          // Special handling if the empty node is the tail
          else if (ptrHead == tail_)
          {
            tail_ = tail_->prev;
            tail_->next = nullptr;
          }
          // Handle the case where the empty node is in the middle
          else
          {
            ptrHead->prev->next = ptrHead->next;
            ptrHead->next->prev = ptrHead->prev;
          }

          delete ptrHead; // Delete the empty node
          --stats.NodeCount; // Decrement the global node count
        }
        return; // Exit the function as the element has been removed
      }
      ++tempIndex; // Increment the global index
    }
    ptrHead = ptrHead->next; // Move to the next node
  }
}

/**
 * @brief Removes the first occurrence of a value from the list.
 * Iterates through each node and their elements in search of the specified value. 
 * Once found, the value is removed, and subsequent elements are shifted left to fill the gap. 
 * If this action results in an empty node, the node itself is removed from the list. 
 * Only the first occurrence of the value is removed.
 * @param value The value to be removed from the list.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
void BList<T, Size>::remove_by_value(const T& value)
{
  BNode* ptrHead = head_; // Initialize pointer to start at the head of the list
  
  // Traverse through all nodes in the list
  while(ptrHead != nullptr)
  {
    // Iterate over elements within the current node
    for(int i = 0; i < ptrHead->count; ++i)
    {
      // Check if the current element matches the target value
      if(ptrHead->values[i] == value)
      {
        // Shift subsequent elements to the left to overwrite the target value
        for(int j = i; j < ptrHead->count - 1; ++j)
        {
          ptrHead->values[j] = ptrHead->values[j + 1];
        }
        --ptrHead->count; // Decrement the count of elements in the current node
        --stats.ItemCount; // Decrement the total item count in the list

        // Check if the node is now empty
        if (ptrHead->count == 0)
        {
          // Special handling if the empty node is the head
          if (ptrHead == head_)
          {
            head_ = head_->next;
            if(head_) head_->prev = nullptr; // Prevent dangling pointer
          }
          // Special handling if the empty node is the tail
          else if (ptrHead == tail_)
          {
            tail_ = tail_->prev;
            tail_->next = nullptr; // Ensure the new tail has no next node
          }
          // Handle removal of a middle node
          else
          {
            ptrHead->prev->next = ptrHead->next; // Link previous node to next node
            ptrHead->next->prev = ptrHead->prev; // Link next node to previous node
          }

          delete ptrHead; // Delete the now empty node
          --stats.NodeCount; // Decrement the node count
        }
        return; // Exit after removing the first occurrence
      }
    }
    ptrHead = ptrHead->next; // Move to the next node if the value wasn't found or already removed
  }
}

/**
 * @brief Searches for the first occurrence of a given value within the BList and returns its index.
 * This method iterates through the nodes of the BList,
 * examining each element within the nodes for a match with the specified value.
 * If found, the method returns the overall index of the value within the list, 
 * considering all nodes as a contiguous array. If the value is not found, the method returns -1.
 * @param value The value to search for within the BList.
 * @return The zero-based index of the first occurrence of the value, or -1 if the value is not found.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
int BList<T, Size>::find(const T& value) const // returns index, -1 if not found
{
  BNode* ptrHead = head_; // Start the search at the head of the list
  int index = 0; // Initialize the index to keep track of the overall position

  // Iterate through each node in the list
  while(ptrHead != nullptr)
  {
    // Search for the value within the current node
    for(int i = 0; i < ptrHead->count; ++i)
    {
      // Check if the current element matches the search value
      if(ptrHead->values[i] == value)
      {
        // Return the calculated index if a match is found
        return index + i;
      }
    }
    
    // Update the index to account for the elements in the current node
    index += ptrHead->count;
    // Move to the next node in the list
    ptrHead = ptrHead->next;
  }

  // Return -1 if the value is not found in any of the nodes
  return -1;
}

/**
 * @brief Accesses an element at a given index within the BList for l-value use.
 * This operator allows direct access to elements stored within the BList, 
 * similar to array indexing. It navigates through the linked list of nodes to find the node that contains 
 * the element at the specified index, considering the list as a contiguous array. 
 * If the index is valid, it returns a reference to the element, allowing modification (hence, for l-values). 
 * If the index is out of range, it throws an exception.
 * @param index The zero-based index of the element to access.
 * @return A reference to the element at the specified index, allowing modification.
 * @throws BListException If the index is out of range.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
T& BList<T, Size>::operator[](int index) // for l-values
{
     // Check if the index is within valid bounds
    if (index < 0 || static_cast<int>(index) >= stats.ItemCount) 
    {
        // Throw an exception if the index is out of range
        throw BListException(BListException::E_BAD_INDEX, "Index out of range");
    }

    // Start searching from the head of the list
    BNode* current = head_;
    // Keep track of the cumulative count to determine the node containing the index
    int currentIndex = 0;

    // Traverse the list to find the node containing the index
    while (current != nullptr) 
    {
        // Check if the index falls within the current node
        if (index < currentIndex + current->count) 
        {
            // Calculate the adjusted index within the node and return the value
            return current->values[index - currentIndex];
        }
        // Update the cumulative index and move to the next node
        currentIndex += current->count;
        current = current->next;
    }

    // If the function reaches this point, it means the index was incorrectly calculated
    // as being within bounds but wasn't found in the list. Throw an exception.
    throw BListException(BListException::E_BAD_INDEX, "Index computation failed");
}

/**
 * @brief Accesses an element at a given index within the BList for r-value use.
 * This const version of the operator allows read-only access to elements stored within the BList, 
 * similar to array indexing. It traverses the linked list of nodes to locate the node that contains 
 * the element at the specified index, considering the list as a contiguous array. If the index is valid
 * it returns a const reference to the element, suitable for r-value contexts. If the index is out of range, it throws an exception.
 * @param index The zero-based index of the element to access.
 * @return A const reference to the element at the specified index, suitable for read-only access.
 * @throws BListException If the index is out of the valid range.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
const T& BList<T, Size>::operator[](int index) const // for r-values
{
       // Check if the index is within valid bounds
    if (index < 0 || static_cast<int>(index) >= stats.ItemCount) 
    {
        // Throw an exception if the index is out of range
        throw BListException(BListException::E_BAD_INDEX, "Index out of range");
    }

    // Start searching from the head of the list
    BNode* current = head_;
    // Keep track of the cumulative count to determine the node containing the index
    int currentIndex = 0;

    // Traverse the list to find the node containing the index
    while (current != nullptr) 
    {
        // Check if the index falls within the current node
        if (index < currentIndex + current->count) 
        {
            // Calculate the adjusted index within the node and return the value
            return current->values[index - currentIndex];
        }
        // Update the cumulative index and move to the next node
        currentIndex += current->count;
        current = current->next;
    }

    // If the function reaches this point, it means the index was incorrectly calculated
    // as being within bounds but wasn't found in the list. Throw an exception.
    throw BListException(BListException::E_BAD_INDEX, "Index computation failed");
}

/**
 * @brief Returns the total number of items in the BList.
 * This function provides the total count of items stored across all nodes within the BList, not just the number of nodes.
 * @return The total number of items in the list.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
size_t BList<T, Size>::size() const   // total number of items (not nodes)
{
  return stats.ItemCount;
}

/**
 * @brief Clears the BList, deleting all nodes and resetting statistics.
 * This function iteratively deletes all nodes in the BList from head to tail, 
 * effectively clearing the list. It also resets the internal statistics tracking the number of nodes and items to zero.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
void BList<T, Size>::clear()          // delete all nodes
{
  BNode* ptrHead = head_;
  // Iterate through each node, deleting them one by one.
  while(ptrHead)
  {
    BNode* nxtNode = ptrHead->next; // Store next node before deletion.
    delete ptrHead; // Delete the current node.
    ptrHead = nxtNode; // Move to the next node.
  }
  // Reset head and tail pointers as well as list statistics.
  head_ = nullptr;
  tail_ = nullptr;
  stats.ItemCount = 0;
  stats.NodeCount = 0;
}

/**
 * @brief Returns the size of a BNode structure.
 * This utility function is primarily used for internal purposes to know the memory size of a single node, 
 * which may be useful for memory allocation and management.
 * @return The size (in bytes) of the BNode structure.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
size_t BList<T, Size>::nodesize()
{
  return sizeof(BNode);
}

/**
 * @brief Provides access to the head node of the BList.
 * This function is mainly used for debugging and testing purposes, 
 * allowing direct read-only access to the first node in the list.
 * @return A constant pointer to the head node of the list.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
const typename BList<T, Size>::BNode* BList<T, Size>::GetHead() const
{
  return head_;
}

template <typename T, unsigned Size>
BListStats BList<T, Size>::GetStats() const
{
  return stats;
}

/**
 * @brief Retrieves the current statistics of the BList.
 * This function returns a BListStats structure containing various statistics about the list, including the size of nodes,
 * the number of nodes, the array size within nodes, and the total item count. 
 * @return A BListStats structure with the current statistics of the list.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template<typename T, unsigned Size>
void BList<T, Size>::Split(BNode* node, T const& value)
{
  //create a blank temp
  BNode* tempNode = CreateNewNode();

  //check if node is next
  if (node->next) 
  {
    tempNode->next = node->next;
    node->next->prev = tempNode;
  }
  else
  {
    tail_ = tempNode;
  }

  node->next = tempNode;
  tempNode->prev = node;


  if (stats.ArraySize == 1) 
  {
    if (node->values[0] < value)
    {
      Increament(tempNode,value);
      ++stats.ItemCount;
    }
    else
    {
      tempNode->values[0] = node->values[0];
      ++tempNode->count;
      ++stats.ItemCount;

      node->values[0] = value;
    }
    return;
  }

  //if the array is bigger than 1
  int rightCount = 0;
  int i = stats.ArraySize / 2;
  for(; i < stats.ArraySize; ++i) //first half of the node
  {
    tempNode->values[rightCount] = node->values[i];
    ++rightCount;

    ++tempNode->count;
    --node->count;
  }

  if (value < tempNode->values[0])
  {
    int index{0};
    for(; index < node->count; ++index)
    {
      if (value < node->values[index]) break;
    }

    for(int i = node->count; i > index; --i)
    {
      node->values[i] = node->values[i - 1];
    }
    node->values[index] = value;
    ++node->count;
    ++stats.ItemCount;
  }
  else
  {
    int index{0};
    for(; index < tempNode->count; ++index)
    {
      if (value < tempNode->values[index]) break;
    }
  
    for(int i = tempNode->count; i > index; --i)
    {
      tempNode->values[i] = tempNode->values[i - 1];
    }
    tempNode->values[index] = value;
    ++tempNode->count;
    ++stats.ItemCount;
  }
}

/**
 * @brief Creates a new node for the BList.
 * This method attempts to allocate memory for a new BNode structure.
 * If the allocation fails, it throws a BListException indicating a memory allocation error. 
 * It also initializes the new node's links to nullptr and increments the node count in the list's statistics.
 * @exception BListException Thrown if memory allocation for the new node fails.
 * @return A pointer to the newly created BNode.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template<typename T, unsigned Size>
typename BList<T, Size>::BNode* BList<T, Size>::CreateNewNode()
{
  BNode* newNode = nullptr;
  try
  {
      // Attempt to allocate memory for a new node.
      newNode = new BNode{};
  }
  catch (const std::exception& _excep)
  {
      // Throw a BListException if memory allocation fails.
      throw(BListException(BListException::E_NO_MEMORY, _excep.what()));
  }

    // Initialize the next and prev pointers of the new node to nullptr.
    newNode->next = nullptr;
    newNode->prev = nullptr;
    // Increment the count of nodes in the list's statistics.
    stats.NodeCount++;
    // Return the pointer to the newly allocated node.
    return newNode;
}

/**
 * @brief Inserts a value at the beginning of a node.
 * This method is used to insert a given value at the first position (index 0) within a specified node.
 *  It also increments the count of items within the node. This method assumes that the node has enough capacity for the new item.
 * @param node A pointer to the node where the value should be inserted.
 * @param value The value to be inserted into the node.
 * @tparam T The type of elements stored in the list.
 * @tparam Size The fixed size of the array within each node.
 */
template <typename T, unsigned Size>
void BList<T, Size>::Increament(BNode* node, T const& value)
{
   // Assign the provided value to the first position in the node.
    node->values[0] = value;
    // Increment the item count within the node.
    ++node->count;
}

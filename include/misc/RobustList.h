/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ROBUSTLIST_H
#define ROBUSTLIST_H

#include <stdlib.h>

/// One list element
template<typename T>
class RobustListNode {
public:
    RobustListNode* next;
    RobustListNode* prev;
    T data;
};

template<typename T> class RobustList;
template<typename T> class RobustListIterator;
template<typename T> class RobustListConstIterator;

/**
    This iterator is designed for the robust list class. The iterator registers at the list
    and thus can be updated when a list element is removed from the list.
*/
template<typename T>
class RobustListIterator {
public:
    /**
        Default constructor. Does not register at the list.
    */
    RobustListIterator<T>() {
        current = nullptr;
        pList = nullptr;
        AdvancingAllowed = true;
    }

    /**
        Copy constructor.
    */
    RobustListIterator<T>(const RobustListIterator<T>& x) {
        current = nullptr;
        pList = nullptr;
        AdvancingAllowed = true;
        *this = x;
    }

    /**
        This constructor constructs a iterator that points at start in the list List.
        \param  start   Start element to point at.
        \param  List    this is the list of start element
    */
    RobustListIterator<T>(RobustListNode<T>* start, const RobustList<T>* List) {
        current = start;
        pList = List;
        AdvancingAllowed = true;
        registerAtList();
    }

    /**
        destructor
    */
    ~RobustListIterator<T>() {
        unregisterFromList();
    }

    /**
        This operator returns the element the iterator is currently pointing to
        \return a reference to the element the iterator is currently pointing to
    */
    T& operator*() const {
        return current->data;
    }

    /**
        This operator returns a pointer to the element the iterator is currently pointing to
        \return a pointer to the element the iterator is currently pointing to
    */
    T* operator->() const {
        return &(current->data);
    }

    /**
        This operator advances to the next element in the list. If the element the iterator is
        currently pointing to is deleted it will be automatically advanced to the next element and
        a flag is set. If this flag is set this method will do nothing except clearing this flag.
        If the flag is not set this method advances the iterator.
        \return A reference to this iterator
    */
    RobustListIterator<T>& operator++() {
        if(current->next != nullptr) {
            if(AdvancingAllowed == true) {
                current = current->next;
            } else {
                AdvancingAllowed = true;
            }
        }
        return *this;
    }

    /**
        This operator advances to the next element in the list. If the element the iterator is
        currently pointing to is deleted it will be automatically advanced to the next element and
        a flag is set. If this flag is set this method will do nothing except clearing this flag.
        If the flag is not set this method advances the iterator.
    */
    void operator++(int) {
        if(current->next != nullptr) {
            if(AdvancingAllowed == true) {
                current = current->next;
            } else {
                AdvancingAllowed = true;
            }
        }
    }

    /**
        This operator goes backwards one element in the list.
        \return A reference to this iterator
    */
    RobustListIterator<T>& operator--() {
        if(current->prev != pList->head) {
            current = current->prev;
            AdvancingAllowed = true;
        }
        return *this;
    }

    /**
        This operator goes backwards one element in the list.
    */
    void operator--(int) {
        if(current->prev != pList->head) {
            current = current->prev;
            AdvancingAllowed = true;
        }
    }

    /**
        This operator compares to iterators.
        \param  x   the other iterator
        \return true if both iterators point to the same element, false otherwise
    */
    bool operator==(const RobustListIterator<T>& x) const {
        if((pList == nullptr) && (x.pList == nullptr)) {
            return true;
        } else if((pList == nullptr) && (x.current == x.pList->tail)) {
            return true;
        } else if((x.pList == nullptr) && (current == pList->tail)) {
            return true;
        } else {
            return (current == x.current);
        }
    }

    /**
        This operator compares to iterators.
        \param  x   the other iterator
        \return false if both iterators point to the same element, true otherwise
    */
    bool operator!=(const RobustListIterator<T>& x) const {
        return !(operator==(x));
    }

    /**
        This operator copys the iterator x to this iterat.
        \param  x   the other iterator
        \return A reference to this iterator
    */
    RobustListIterator<T>& operator=(const RobustListIterator<T>& x) {
        if(this == &x) {
            return *this;
        }

        if(pList != x.pList) {
            unregisterFromList();
            current = x.current;
            pList = x.pList;
            AdvancingAllowed = x.AdvancingAllowed;
            registerAtList();
        } else {
            current = x.current;
        }
        return *this;
    }

    /**
        Returns the list of this iterator
        \return the list of this iterator
    */
    inline RobustList<T>* getList() {
        return pList;
    }

private:
    /**
        Registers this iterator at the list
    */
    void registerAtList() {
        if(pList != nullptr) {
            pList->RegisterIterator(this);
        }
    }

    /**
        Unregisters this iterator from the list
    */
    void unregisterFromList() {
        if(pList != nullptr) {
            pList->UnregisterIterator(this);
            pList = nullptr;
        }
    }

    friend class RobustList<T>;
    friend class RobustListConstIterator<T>;

    RobustListNode<T>* current;
    const RobustList<T>* pList;
    bool AdvancingAllowed;
};

/**
    This const iterator is designed for the robust list class. The iterator registers at the list
    and thus can be updated when a list element is removed from the list.
*/
template<typename T>
class RobustListConstIterator {
public:
    /**
        Default constructor. Does not register at the list.
    */
    RobustListConstIterator<T>() {
        current = nullptr;
        pList = nullptr;
        AdvancingAllowed = true;
    }

    /**
        Copy constructor.
    */
    RobustListConstIterator<T>(const RobustListConstIterator<T>& x) {
        current = nullptr;
        pList = nullptr;
        AdvancingAllowed = true;
        *this = x;
    }

    /**
        Copy constructor.
    */
    RobustListConstIterator<T>(const RobustListIterator<T>& x) {
        current = x.current;
        pList = x.pList;
        AdvancingAllowed = x.AdvancingAllowed;
        registerAtList();
    }

    /**
        This constructor constructs a iterator that points at start in the list List.
        \param  start   Start element to point at.
        \param  List    this is the list of start element
    */
    RobustListConstIterator<T>(const RobustListNode<T>* start, const RobustList<T>* List) {
        current = start;
        pList = List;
        AdvancingAllowed = true;
        registerAtList();
    }

    /**
        destructor
    */
    ~RobustListConstIterator<T>() {
        unregisterFromList();
    }

    /**
        This operator returns the element the iterator is currently pointing to
        \return a reference to the element the iterator is currently pointing to
    */
    const T& operator*() const {
        return current->data;
    }

    /**
        This operator returns a pointer to the element the iterator is currently pointing to
        \return a pointer to the element the iterator is currently pointing to
    */
    const T* operator->() const {
        return &(current->data);
    }

    /**
        This operator advances to the next element in the list. If the element the iterator is
        currently pointing to is deleted it will be automatically advanced to the next element and
        a flag is set. If this flag is set this method will do nothing except clearing this flag.
        If the flag is not set this method advances the iterator.
        \return A reference to this iterator
    */
    RobustListConstIterator<T>& operator++() {
        if(current->next != nullptr) {
            if(AdvancingAllowed == true) {
                current = current->next;
            } else {
                AdvancingAllowed = true;
            }
        }
        return *this;
    }

    /**
        This operator advances to the next element in the list. If the element the iterator is
        currently pointing to is deleted it will be automatically advanced to the next element and
        a flag is set. If this flag is set this method will do nothing except clearing this flag.
        If the flag is not set this method advances the iterator.
    */
    void operator++(int) {
        if(current->next != nullptr) {
            if(AdvancingAllowed == true) {
                current = current->next;
            } else {
                AdvancingAllowed = true;
            }
        }
    }

    /**
        This operator goes backwards one element in the list.
        \return A reference to this iterator
    */
    RobustListConstIterator<T>& operator--() {
        if(current->prev != pList->head) {
            current = current->prev;
            AdvancingAllowed = true;
        }
        return *this;
    }

    /**
        This operator goes backwards one element in the list.
    */
    void operator--(int) {
        if(current->prev != pList->head) {
            current = current->prev;
            AdvancingAllowed = true;
        }
    }

    /**
        This operator compares to iterators.
        \param  x   the other iterator
        \return true if both iterators point to the same element, false otherwise
    */
    bool operator==(const RobustListConstIterator<T>& x) const {
        if((pList == nullptr) && (x.pList == nullptr)) {
            return true;
        } else if((pList == nullptr) && (x.current == x.pList->tail)) {
            return true;
        } else if((x.pList == nullptr) && (current == pList->tail)) {
            return true;
        } else {
            return (current == x.current);
        }
    }

    /**
        This operator compares to iterators.
        \param  x   the other iterator
        \return false if both iterators point to the same element, true otherwise
    */
    bool operator!=(const RobustListConstIterator<T>& x) const {
        return !(operator==(x));
    }

    /**
        This operator copys the iterator x to this iterat.
        \param  x   the other iterator
        \return A reference to this iterator
    */
    RobustListConstIterator<T>& operator=(const RobustListConstIterator<T>& x) {
        if(this == &x) {
            return *this;
        }

        if(pList != x.pList) {
            unregisterFromList();
            current = x.current;
            pList = x.pList;
            AdvancingAllowed = x.AdvancingAllowed;
            registerAtList();
        } else {
            current = x.current;
        }
        return *this;
    }

    /**
        Returns the list of this iterator
        \return the list of this iterator
    */
    inline RobustList<T>* getList() {
        return pList;
    }

private:
    /**
        Registers this iterator at the list
    */
    void registerAtList() {
        if(pList != nullptr) {
            pList->RegisterConstIterator(this);
        }
    }

    /**
        Unregisters this iterator from the list
    */
    void unregisterFromList() {
        if(pList != nullptr) {
            pList->UnregisterConstIterator(this);
            pList = nullptr;
        }
    }

    friend class RobustList<T>;
    friend class RobustListIterator<T>;

    const RobustListNode<T>* current;
    const RobustList<T>* pList;
    bool AdvancingAllowed;
};

/**
        A robust list class. While iterator though the list it is allowed to modify the list in any way.
        The class is designed to be very similar to the stl list class but has only basic functionality.
*/
template<typename T>
class RobustList {
public:
    typedef RobustListIterator<T> iterator;
    typedef RobustListConstIterator<T> const_iterator;
    /**
        Default constructor
    */
    RobustList() {
        numElements = 0;
        head = new RobustListNode<T>();
        tail = new RobustListNode<T>();
        head->next = tail;
        head->prev = nullptr;
        tail->next = nullptr;
        tail->prev = head;
        IteratorListHead = nullptr;
        ConstIteratorListHead = nullptr;
    }

    /**
        Copy constructor
    */
    RobustList(const RobustList<T>& x) {
        numElements = 0;
        head = new RobustListNode<T>();
        tail = new RobustListNode<T>();
        head->next = tail;
        head->prev = nullptr;
        tail->next = nullptr;
        tail->prev = head;
        IteratorListHead = nullptr;
        ConstIteratorListHead = nullptr;
        *this = x;
    }

    /**
        Destructor
    */
    ~RobustList() {
        clear();
        delete head;
        delete tail;

        while(IteratorListHead != nullptr) {
            IteratorListNode* tmp = IteratorListHead;
            tmp->iter->unregisterFromList();
            /*IteratorListHead = IteratorListHead->next;
            delete tmp;*/
        }

        while(ConstIteratorListHead != nullptr) {
            ConstIteratorListNode* tmp = ConstIteratorListHead;
            tmp->iter->unregisterFromList();
            /*ConstIteratorListHead = ConstIteratorListHead->next;
            delete tmp;*/
        }
    }

    /**
        Returns the number of items (elements) currently stored in the list.
        \return number of elements in the list
    */
    int size() const {
        return numElements;
    }

    /**
        Checks whether this list is empty.
        \returns true if the number of elements is zero, false otherwise.
    */
    bool empty() const {
        return (numElements == 0);
    }

    /**
        Adds the element x at the beginning of the list.
        \param  x   Element to add
    */
    void push_front(const T& x) {
        RobustListNode<T>* newElement = new RobustListNode<T>();
        newElement->data = x;
        newElement->next = head->next;
        newElement->prev = head;
        head->next->prev = newElement;
        head->next = newElement;
        numElements++;
    }

    /**
        Adds the element x at the end of the list.
        \param  x   Element to add
    */
    void push_back(const T& x) {
        RobustListNode<T>* newElement = new RobustListNode<T>();
        newElement->data = x;
        newElement->next = tail;
        newElement->prev = tail->prev;
        tail->prev->next = newElement;
        tail->prev = newElement;
        numElements++;
    }

    /**
        Erases the first element from a list. These operations are illegal if the list is empty.
    */
    void pop_front() {
        removeElement(head->next);
    }

    /**
        Erases the last element from a list. These operations are illegal if the list is empty.
    */
    void pop_back() {
        removeElement(tail->prev);
    }

    /**
        Obtain a reference to the first element in the list (valid only if the list is not empty).
        This reference may be used to access the first element in the list.
        \return First element in the list.
    */
    T& front() {
        return head->next->data;
    }

    /**
        Obtain a const reference to the first element in the list (valid only if the list is not empty).
        This reference may be used to access the first element in the list.
        \return First element in the list.
    */
    const T& front() const {
        return head->next->data;
    }

    /**
        Obtain a reference to the last element in the list (valid only if the list is not empty).
        This reference may be used to access the last element in the list.
        \return Last element in the list.
    */
    T& back() {
        return tail->prev->data;
    }

    /**
        Obtain a const reference to the last element in the list (valid only if the list is not empty).
        This reference may be used to access the last element in the list.
        \return Last element in the list.
    */
    const T& back() const {
        return tail->prev->data;
    }

    /**
        Returns an iterator that references the beginning of the list.
        \return Iterator that points to the beginning of the list
    */
    iterator begin() {
        return iterator(head->next,this);
    }

    /**
        Returns an const iterator that references the beginning of the list.
        \return Iterator that points to the beginning of the list
    */
    const_iterator begin() const {
        return const_iterator(head->next,this);
    }

    /**
        Returns an iterator that references a position just past the last element in the list.
        \return Iterator that points to the end of the list
    */
    iterator end() {
        iterator iter;
        return iter;
    }

    /**
        Returns an const iterator that references a position just past the last element in the list.
        \return Iterator that points to the end of the list
    */
    const_iterator end() const {
        const_iterator iter;
        return iter;
    }

    /**
        Erase all elements from this list.
    */
    void clear() {
        RobustListNode<T>* current = head->next;
        while(current != tail) {
            RobustListNode<T>* temp = current;
            current = current->next;
            removeElement(temp);
        }
    }

    /**
        Erases all list elements that are equal to value.
        The equality operator (==) must be defined for T, the type of element stored in the list.
        \param  value   value to delete
    */
    void remove(const T& value) {
        RobustListNode<T>* current = head->next;
        while(current != tail) {
            RobustListNode<T>* temp = current;
            current = current->next;
            if(temp->data == value) {
                removeElement(temp);
            }
        }
    }

    /**
        This method inserts value before position.
        \param  position    Position to insert before
        \param  value       value to insert
    */
    void insert(iterator position, T& value) {
        RobustListNode<T>* newElement = new RobustListNode<T>();
        newElement->data = value;
        // avoid inserting before head
        if(position.current == head) {
            newElement->prev = head;
            newElement->next = head->next;
            head->next->prev = newElement;
            head->next = newElement;

            //Advance this iterators
            position.current = position.current->next;
        } else {
            newElement->next = position.current;
            newElement->prev = position.current->prev;
            position.current->prev = newElement;
            newElement->prev->next = newElement;
        }
        numElements++;
    }

    void erase(iterator position) {
        removeElement(position.current);
    }

    /**
        Copys a list to this list. This will first delete all elements in this list
        and then copy all elements from the other list.
        \param  x   The other list
        \return this list
    */
    RobustList<T>& operator=(const RobustList<T>& x) {
        if(this == &x) {
            return *this;
        }

        clear();
        RobustListNode<T>* current = x.head->next;
        RobustListNode<T>* old = head;
        while(current != x.tail) {
            RobustListNode<T>* tmp = new RobustListNode<T>();
            tmp->data = current->data;
            old->next = tmp;
            tmp->prev = old;
            old = tmp;
            current = current->next;
        }
        old->next = tail;
        tail->prev = old;
        numElements = x.numElements;
        return *this;
    }

    /**
        Tests whether two lists have the same content (element-by-element comparison for all elements)
        \param  x   The other list
        \return true if both list contain the same elements in the same order, false otherwise
    */
    bool operator==(const RobustList<T>& x) const {
        if(this == &x) {
            return true;
        }

        if(numElements != x.numElements) {
            return false;
        }

        RobustListNode<T>* current1 = head->next;
        RobustListNode<T>* current2 = x.head->next;
        while((current1 != tail) && (current2 != x.tail)) {
            if(current1->data != current2->data) {
                return false;
            }
        }
        return true;
    }

    /**
        Tests whether two lists do not have the same content (element-by-element comparison for all elements)
        \param  x   The other list
        \return false if both list contain the same elements in the same order, true otherwise
    */
    bool operator!=(const RobustList<T>& x) const {
        return !this->operator==(x);
    }

private:
    /**
        Registers an iterator at the list
        \param  pIterator   the iterator to register
    */
    void RegisterIterator(RobustListIterator<T>* pIterator) const {
        IteratorListNode* newNode = new IteratorListNode();
        newNode->next = IteratorListHead;
        newNode->iter = pIterator;
        IteratorListHead = newNode;
    }

    /**
        Registers an const iterator at the list
        \param  pIterator   the iterator to register
    */
    void RegisterConstIterator(RobustListConstIterator<T>* pIterator) const {
        ConstIteratorListNode* newNode = new ConstIteratorListNode();
        newNode->next = ConstIteratorListHead;
        newNode->iter = pIterator;
        ConstIteratorListHead = newNode;
    }

    /**
        Unregisters a iterator from the list.
        \param  pIterator   the iterator to unregister
    */
    void UnregisterIterator(RobustListIterator<T>* pIterator) const {
        IteratorListNode* current = IteratorListHead;
        IteratorListNode* old = nullptr;
        while(current != nullptr) {
            if(current->iter == pIterator) {
                if(old == nullptr) {
                    IteratorListHead = current->next;
                } else {
                    old->next = current->next;
                }
                delete current;
                return;
            }
            old = current;
            current = current->next;
        }
    }

    /**
        Unregisters a const iterator from the list.
        \param  pIterator   the iterator to unregister
    */
    void UnregisterConstIterator(RobustListConstIterator<T>* pIterator) const {
        ConstIteratorListNode* current = ConstIteratorListHead;
        ConstIteratorListNode* old = nullptr;
        while(current != nullptr) {
            if(current->iter == pIterator) {
                if(old == nullptr) {
                    ConstIteratorListHead = current->next;
                } else {
                    old->next = current->next;
                }
                delete current;
                return;
            }
            old = current;
            current = current->next;
        }
    }

    /**
        This method removes one element from the list and adjusts all
        iterators that point to this element.
        \param  element element to remove
    */
    void removeElement(RobustListNode<T>* element) {
        if((element == head)||(element == tail)) {
            return;
        }

        IteratorListNode* current = IteratorListHead;
        while(current != nullptr) {
            if(current->iter->current == element) {
                current->iter->current = current->iter->current->next;
                current->iter->AdvancingAllowed = false;
            }
            current = current->next;
        }

        ConstIteratorListNode* constcurrent = ConstIteratorListHead;
        while(constcurrent != nullptr) {
            if(constcurrent->iter->current == element) {
                constcurrent->iter->current = constcurrent->iter->current->next;
                constcurrent->iter->AdvancingAllowed = false;
            }
            constcurrent = constcurrent->next;
        }

        element->prev->next = element->next;
        element->next->prev = element->prev;
        delete element;
        numElements--;
    }

    friend class RobustListIterator<T>;
    friend class RobustListConstIterator<T>;

    int numElements;
    RobustListNode<T>* head;
    RobustListNode<T>* tail;

    class IteratorListNode {
    public:
        RobustListIterator<T>* iter;
        IteratorListNode* next;
    };

    class ConstIteratorListNode {
    public:
        RobustListConstIterator<T>* iter;
        ConstIteratorListNode* next;
    };

    mutable IteratorListNode* IteratorListHead;
    mutable ConstIteratorListNode* ConstIteratorListHead;
};


#endif //ROBUSTLIST_H

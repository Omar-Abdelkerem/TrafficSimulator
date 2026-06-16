#ifndef LINKEDLIST_CPP
#define LINKEDLIST_CPP

#include "LinkedList.h"

template <typename T>
LinkedList<T>::LinkedList() : head(nullptr), tail(nullptr), count(0) {}

template <typename T>
LinkedList<T>::~LinkedList() {
    Clear();
}

template <typename T>
void LinkedList<T>::InsertEnd(const T& value) {
    Node<T>* newNode = new Node<T>(value);
    if (tail == nullptr) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
    count++;
}

template <typename T>
bool LinkedList<T>::Remove(const T& value) {
    Node<T>* current = head;
    Node<T>* prev = nullptr;
    
    while (current != nullptr) {
        if (current->data == value) {
            if (prev == nullptr) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            if (current == tail) {
                tail = prev;
            }
            delete current;
            count--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

template <typename T>
bool LinkedList<T>::RemoveNode(Node<T>* nodePtr) {
    if (nodePtr == nullptr) return false;
    
    Node<T>* prev = findPrevious(nodePtr);
    if (prev == nullptr) {
        head = nodePtr->next;
    } else {
        prev->next = nodePtr->next;
    }
    if (nodePtr == tail) {
        tail = prev;
    }
    delete nodePtr;
    count--;
    return true;
}

template <typename T>
Node<T>* LinkedList<T>::findPrevious(Node<T>* target) {
    Node<T>* current = head;
    while (current != nullptr && current->next != target) {
        current = current->next;
    }
    return current;
}

template <typename T>
bool LinkedList<T>::IsEmpty() const {
    return head == nullptr;
}

template <typename T>
int LinkedList<T>::Count() const {
    return count;
}

template <typename T>
Node<T>* LinkedList<T>::GetHead() const {
    return head;
}

template <typename T>
Node<T>* LinkedList<T>::GetTail() const {
    return tail;
}

template <typename T>
void LinkedList<T>::Clear() {
    Node<T>* current = head;
    while (current != nullptr) {
        Node<T>* next = current->next;
        delete current;
        current = next;
    }
    head = tail = nullptr;
    count = 0;
}

#endif


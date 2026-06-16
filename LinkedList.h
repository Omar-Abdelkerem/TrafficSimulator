#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "Node.h"

template <typename T>
class LinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
    int count;
    Node<T>* findPrevious(Node<T>* target);

public:
    LinkedList();
    ~LinkedList();
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    void InsertEnd(const T& value);
    bool Remove(const T& value);
    bool RemoveNode(Node<T>* nodePtr);
    bool IsEmpty() const;
    int Count() const;
    Node<T>* GetHead() const;
    Node<T>* GetTail() const;
    void Clear();
};

#include "LinkedList.cpp"

#endif


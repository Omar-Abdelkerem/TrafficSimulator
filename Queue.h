#ifndef QUEUE_H
#define QUEUE_H

#include "LinkedList.h"

template <typename T>
class Queue {
private:
    LinkedList<T> list;

public:
    Queue() {}
    ~Queue() {}
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void Enqueue(const T& value) { list.InsertEnd(value); }
    T Dequeue() {
        if (list.IsEmpty()) return T();
        Node<T>* headNode = list.GetHead();
        T value = headNode->data;
        list.RemoveNode(headNode);
        return value;
    }
    T Peek() const {
        if (list.IsEmpty()) return T();
        return list.GetHead()->data;
    }
    bool IsEmpty() const { return list.IsEmpty(); }
    int Count() const { return list.Count(); }
    bool Remove(const T& value) { return list.Remove(value); }
    Node<T>* GetHead() const { return list.GetHead(); }
};

#endif


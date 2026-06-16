#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include "LinkedList.h"
#include <type_traits>

template <typename T>
class PriorityQueue {
private:
    LinkedList<T> list;
    bool (*compareFunc)(const T& a, const T& b);

public:
    PriorityQueue(bool (*compare)(const T& a, const T& b)) 
        : compareFunc(compare) {}
    
    template<typename U>
    PriorityQueue(bool (*compare)(U* const& a, U* const& b),
                   typename std::enable_if<std::is_pointer<T>::value && 
                                          std::is_same<typename std::remove_pointer<T>::type, U>::value>::type* = nullptr)
        : compareFunc(reinterpret_cast<bool (*)(const T&, const T&)>(compare)) {}
    ~PriorityQueue() {}

    void Enqueue(const T& value) {
        LinkedList<T> tempList;
        Node<T>* current = list.GetHead();
        bool inserted = false;

        while (current != nullptr) {
            if (!inserted && !compareFunc(current->data, value)) {
                tempList.InsertEnd(value);
                inserted = true;
            }
            tempList.InsertEnd(current->data);
            current = current->next;
        }

        if (!inserted) {
            tempList.InsertEnd(value);
        }

        list.Clear();
        current = tempList.GetHead();
        while (current != nullptr) {
            list.InsertEnd(current->data);
            current = current->next;
        }
    }

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


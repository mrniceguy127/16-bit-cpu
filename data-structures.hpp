#ifndef DATASTRUCTURES_HPP
#define DATASTRUCTURES_HPP

#include <iostream>

namespace DataStructures {
  template<class T>
  class LinkedList;

  template <class T>
  class LinkedListNode {
  public:
    LinkedListNode<T> * next;
    LinkedListNode<T> * prev;
    T data;
    LinkedListNode() : next(nullptr), prev(nullptr) {}
    inline bool operator==(const LinkedListNode<T>& node) { return data == node.data; }
    inline bool operator!=(const LinkedListNode<T>& node) { return data != node.data; }
    inline bool operator==(const LinkedListNode<LinkedList<T>>& list) {
      bool same = true;
      unsigned i;
      for (i = 0; i < static_cast<unsigned>(this->size()); i++) {
        if (this->get(i) != list.get(i)) {
          same = false;
          break;
        }
      }
      return same && list.size() == this->size();
    }
    inline bool operator!=(const LinkedListNode<LinkedList<T>>& list) {
      bool same = true;
      unsigned i;
      for (i = 0; i < static_cast<unsigned>(this->size()); i++) {
        if (data.get(static_cast<int>(i)) != list.data.get(static_cast<int>(i))) {
          same = false;
          break;
        }
      }
      return !(same && list.data.size() == data.size());
    }
  };

  template <class T>
  class LinkedList {
    LinkedListNode<T> * head, * tail;
  public:
    LinkedList() {
      head = nullptr;
      tail = nullptr;
    }
    inline bool operator==(const LinkedList<T>& list) {
      bool same = true;
      unsigned i;
      for (i = 0; i < static_cast<unsigned>(this->size()); i++) {
        if (this->get(i) != list.get(i)) {
          same = false;
          break;
        }
      }
      return same && list.size() == this->size();
    }
    inline bool operator!=(const LinkedList<T>& list) {
      bool same = true;
      unsigned i;
      for (i = 0; i < static_cast<unsigned>(this->size()); i++) {
        if (this->get(static_cast<int>(i)) != list.get(static_cast<int>(i))) {
          same = false;
          break;
        }
      }
      return !(same && list.size() == this->size());
    }

    T first() const {
      if (tail != nullptr) {
        return tail->data;
      } else {
        return T();
      }
    }

    T last() const {
      return head->data;
      if (head != nullptr) {
        return head->data;
      } else {
        return T();
      }
    }

    int size() const {
      int size = 0;
      LinkedListNode<T> * temp = tail;
      while (temp != nullptr) {
        size++;
        temp = temp->next;
      }
      return size;
    }

    void push(T data) {
      LinkedListNode<T> * node = new LinkedListNode<T>();
      node->data = data;
      node->prev = head;
      node->next = nullptr;
      if (head != nullptr) {
        head->next = node;
      } else {
        tail = node;
      }
      head = node;
    }

    void pop() {
      if (head != nullptr) {
        LinkedListNode<T> * temp = head->prev;
        if (temp != nullptr) {
          temp->next = nullptr;
          delete head;
          head = temp;
          if (head->prev != nullptr) {
            head->prev->next = head;
          }
        } else {
          delete head;
          head = nullptr;
          tail = nullptr;
        }
      }
    }

    T get(int index) const {
      if (index > this->size() - 1) {
        return T();
      } else if (tail != nullptr) {
        int i = 0;
        LinkedListNode<T> * temp = tail;
        while (i < index) {
          temp = temp->next;
          i++;
        }
        return temp->data;
      } else {
        return T();
      }
    }


    void set(int index, T data) {
      int listSize = this->size();
      if (listSize == 0 || index > listSize - 1) {
        pop();
        push(data);
      } else {
        int i = 0;
        LinkedListNode<T> * temp = tail;
        LinkedListNode<T> * newNode = new LinkedListNode<T>();
        LinkedListNode<T> * nodeToDelete;
        newNode->data = data;
        while (i < index-1) {
          temp = temp->next;
          i++;
        }
        nodeToDelete = temp->next;
        newNode->prev = temp;
        if (nodeToDelete->next != nullptr) {
          newNode->next = nodeToDelete->next;
        } else {
          head = newNode;
        }
        temp->next = newNode;
        delete nodeToDelete;
      }
    }

    T shift() {
      if (tail != nullptr) {
        LinkedListNode<T> * temp = tail->next;
        if (temp != nullptr) {
          temp->prev = nullptr;
        } else {
          head = nullptr;
        }
        int data = tail->data;
        delete tail;
        tail = temp;
        return data;
      } else {
        return T();
      }
    }

    void unshift(T data) {
      LinkedListNode<T> * node = new LinkedListNode<T>();
      node->data = data;
      node->next = tail;
      if (tail != nullptr) {
        tail->prev = node;
      } else {
        head = node;
      }
      tail = node;
    }

    void insert(int index, T data) {
      int listSize = this->size();
      LinkedListNode<T> * newNode = new LinkedListNode<T>();
      newNode->data = data;
      if (index > listSize - 1 || listSize == 0) {
        push(data);
      } else if (index < 0) {
        unshift(data);
      } else {
        int i = 0;
        LinkedListNode<T> * temp = tail;
        while (i < index) {
          temp = tail->next;
          i++;
        }
        newNode->prev = temp->prev;
        newNode->next = temp;
        temp->prev->next = newNode;
        temp->prev = newNode;
      }
    }

    LinkedList<T> slice(int a, int b) {
      LinkedList<T> temp;
      int i = 0;
      int range = b-a;
      if (b >= a) {
        while (i < range) {
          temp.push(this->get(a+i));
          i++;
        }
      } else {
        while (i > range) {
          temp.push(this->get(b-i));
          i--;
        }
      }

      return temp;
    }

    void forEach(void (*func)(T, int)) {
      for (int i = 0; i < this->size(); i++) {
        func(this->get(i), i);
      }
    }

    LinkedList<T> filter(bool (*filter)(T)) {
      LinkedList<T> filtered;
      for (int i = 0; i < this->size(); i++) {
        if (filter(this->get(i))) {
          filtered.push(this->get(i));
        }
      }
      return filtered;
    }

    LinkedList<T> map(T (*map)(T)) {
      LinkedList<T> mapped;
      int i;
      for (i = 0; i < this->size(); i++) {
        mapped.push(map(this->get(i)));
      }

      return mapped;
    }

    void concat(LinkedList<T> * newData) {
      int i;
      for (i = 0; i < newData->size(); i++) {
        this->push(newData->get(i));
      }
    }

    void print();
  };
};

#endif // DATASTRUCTURES_HPP

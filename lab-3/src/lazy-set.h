#pragma once

#include <pthread.h>

template <class T>
class LazySet {
 public:
  struct Node {
    Node next;
    int key;
    bool marked;
    T data;
    pthread_mutex_t mutex;

    inline void lock();
    inline void unlock();
  };

  bool add(T item);
  bool remove(T item);
  bool contains(T item);

 private:
  Node* m_head;

  bool validate(Node*& pred, Node*& cur);
};

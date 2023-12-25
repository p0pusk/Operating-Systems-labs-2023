#include "lazy-set.h"

#include <algorithm>
#include <exception>
#include <iostream>

template <typename T>
void LazySet<T>::Node::lock() {
  int res = pthread_mutex_lock(&mutex);
  if (res) {
    throw std::runtime_error("Failed to lock mutex, code: " +
                             std::to_string(res));
  }
}

template <typename T>
void LazySet<T>::Node::unlock() {
  int res{pthread_mutex_unlock(&mutex)};
  if (res) {
    throw std::runtime_error("Failed to unlock mutex, code: " +
                             std::to_string(res));
  }
}

template <typename T>
bool LazySet<T>::validate(Node*& pred, Node*& curr) {
  return !pred->marked && !curr->marked && pred->next == curr;
}

template <typename T>
bool LazySet<T>::add(T item) {
  int key;
  // int key = std::hash<T::int>

  while (true) {
    Node* pred = m_head;
    Node* curr = m_head->next;

    while (curr->key < key) {
      pred = curr;
      curr = curr->next;
    }

    try {
      pred->lock();
      curr->lock();
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      exit(1);
    }

    if (validate(pred, curr)) {
      if (curr->key == key) {
        curr->unlock();
        pred->unlock();
        return false;
      } else {
        Node* node = new Node(item);
        node->next = curr;
        pred->next = node;
        curr->unlock();
        pred->unlock();
        return true;
      }
    }
  }
}

template <typename T>
bool LazySet<T>::remove(T item) {
  int key;
  while (true) {
    Node* pred = m_head;
    Node* curr = pred->next;

    while (curr->key < key) {
      pred = curr;
      curr = curr->next;
    }

    try {
      pred->lock();
      curr->lock();
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
      exit(1);
    }

    if (validate(pred, curr)) {
      if (curr->key != key) {
        pred->unlock();
        curr->unlock();
        return false;
      } else {
        curr->marked = true;
        pred->next = curr->next;
        pred->unlock();
        curr->unlock();
        return true;
      }
    }
  }
}

template <typename T>
bool LazySet<T>::contains(T item) {
  int key;
  Node* curr = m_head;
  while (curr->key < key) {
    curr = curr->next;
  }

  return curr->key == key && !curr->marked;
}

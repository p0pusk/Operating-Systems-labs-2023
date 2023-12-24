
#pragma once

template <class T>
class LazySet {
 public:
  bool add(T item);
  bool remove(T item);
  bool contains(T item);
};

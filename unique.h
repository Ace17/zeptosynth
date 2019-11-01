#pragma once

struct Exception
{
  const char* msg;
};

// lightweight unique_ptr replacement
template<typename T>
struct Unique
{
  Unique() = default;
  Unique(T* ptr_) : ptr(ptr_) {}
  ~Unique() { delete ptr; }

  template<typename V>
  Unique(Unique<V>&& other)
  {
    delete ptr;
    ptr = other.ptr;
    other.ptr = nullptr;
  }

  // disable copy
  Unique(Unique const &) = delete;
  Unique& operator = (Unique const&) = delete;

  T* operator -> ()
  {
    return ptr;
  }

  T* ptr {};
};

template<typename T, typename... Args>
Unique<T> makeUnique(Args && ... args)
{
  return Unique<T>(new T(args...));
}


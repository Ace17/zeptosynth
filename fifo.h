#pragma once

#include <stdint.h>
#include <atomic>
#include <vector>

template<typename T>
struct Fifo
{
  Fifo(int maxCount = 1024) : data(maxCount) {}

  void push(const T& element)
  {
    const auto currPos = m_writePos.load();
    const auto nextPos = (currPos + 1) % (int)data.size();

    if(nextPos == m_readPos.load())
      return; // queue full

    data[currPos] = element;
    m_writePos.store(nextPos);
  }

  bool pop(T& element)
  {
    const auto currPos = m_readPos.load();
    const auto nextPos = (currPos + 1) % (int)data.size();

    if(currPos == m_writePos.load())
      return false; // nothing to pop

    element = std::move(data[currPos]);
    m_readPos.store(nextPos);
    return true;
  }

private:
  std::vector<T> data;

  std::atomic<int> m_readPos {};
  std::atomic<int> m_writePos {};
};


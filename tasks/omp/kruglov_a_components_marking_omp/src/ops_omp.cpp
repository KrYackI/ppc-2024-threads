// Copyright 2023 Kruglov Alexey

#include "omp/kruglov_a_components_marking_omp/include/ops_omp.hpp"

#include <omp.h>

using namespace std::chrono_literals;

namespace KruglovOmpTask {

class RecursivePtr {
  RecursivePtr *ptr;
  int value;

 public:
  RecursivePtr() : ptr(nullptr), value(0) {}
  RecursivePtr(int _value) : ptr(nullptr), value(_value) {}
  RecursivePtr *getParent() { return ptr; }
  void set(RecursivePtr *_ptr) {
    RecursivePtr *p = _ptr;
    while (p->getParent() != nullptr) {
      p = p->getParent();
    }
    if (!ptr) {
      ptr = p;
      value = 0;
      return;
    }
    ptr->set(p);
  }
  int getValue() {
    if (!ptr) {
      return value;
    }
    return ptr->getValue();
  }
  bool hasVal() { return (getValue() != 0); }
};

bool imgMarkingOmp::pre_processing() {
  internal_order_test();
  // Init value for input and output
  h = reinterpret_cast<uint32_t *>(taskData->inputs[0])[0];
  w = reinterpret_cast<uint32_t *>(taskData->inputs[0])[1];
  src.resize(h);
  dst.resize(h);
  for (size_t i = 0; i < h; ++i) {
    for (size_t j = 0; j < w; ++j) src[i].push_back(reinterpret_cast<uint8_t *>(taskData->inputs[1])[i * w + j]);
    dst[i].resize(w, 0);
  }
  return true;
}

bool imgMarkingOmp::validation() {
  internal_order_test();
  // Check count elements of output
  h = reinterpret_cast<uint32_t *>(taskData->inputs[0])[0];
  w = reinterpret_cast<uint32_t *>(taskData->inputs[0])[1];
  return (h * w == taskData->inputs_count[1] && taskData->inputs_count[1] == taskData->outputs_count[0]);
}

bool imgMarkingOmp::run() {
  internal_order_test();
  imgMarking();
  return true;
}

bool imgMarkingOmp::post_processing() {
  internal_order_test();
  for (size_t i = 0; i < h; ++i)
    for (size_t j = 0; j < w; ++j) reinterpret_cast<uint32_t *>(taskData->outputs[0])[i * w + j] = dst[i][j];
  return true;
}

void imgMarkingOmp::imgMarking() {
  std::vector<std::vector<RecursivePtr *>> ptrMap;
  ptrMap.resize(h);
  
  omp_lock_t *lock = new omp_lock_t;
  omp_init_lock(lock);

  uint32_t num_threads = omp_get_thread_limit();
  if (num_threads > h) num_threads = 1;

#pragma omp parallel num_threads(1)
  {
#pragma omp for
    for (size_t i = 0; i < h; ++i) ptrMap[i].resize(w, nullptr);

    std::list<uint32_t> localVec;

    int32_t d = h / omp_get_num_threads();
    size_t h0;
    size_t h1;
    if (omp_get_thread_num()) {
      h0 = d * (omp_get_thread_num() - 1);
      h1 = d * (omp_get_thread_num()) - 1;
    } else {
      h0 = d * (omp_get_num_threads() - 1);
      h1 = h;
    }
    int32_t offset = h0 * w + 1;
    RecursivePtr *localPtr = nullptr;
    RecursivePtr *ptr = nullptr;

    for (size_t i = 0; i < w; ++i) {
      if (src[h0][i] == 0) {
        if (i == 0 || ptrMap[h0][i - 1] == nullptr) {
          ptrMap[h0][i] = new RecursivePtr(offset++);
          localPtr = ptrMap[h0][i];
        } else {
          ptrMap[h0][i] = new RecursivePtr;
          ptrMap[h0][i]->set(localPtr);
        }
        if (i == w - 1 || src[h0][i + 1] == 1) localPtr = nullptr;
      }
    }
    localPtr = ptrMap[h0][0];

    for (size_t i = 1 + h0; i < h1; ++i) {
      if (src[i][0] == 0) {
        if (ptrMap[i - 1][0] == nullptr) {
          ptrMap[i][0] = new RecursivePtr(offset++);
          localPtr = ptrMap[i][0];
        } else {
          ptrMap[i][0] = new RecursivePtr;
          ptrMap[i][0]->set(localPtr);
        }
        if (i == h1 - 1 || src[i + 1][0] == 1) localPtr = nullptr;
      }

      for (size_t j = 1; j < w; ++j) {
        if (src[i][j] == 0) {
          ptr = nullptr;
          if (ptrMap[i - 1][j] != nullptr) ptr = ptrMap[i - 1][j];
          if (ptrMap[i][j - 1] != nullptr) {
            if (ptr != nullptr && ptrMap[i][j - 1]->getValue() != ptr->getValue())
              ptrMap[i][j - 1]->set(ptrMap[i - 1][j]);
            ptr = ptrMap[i][j - 1];
          }

          if (ptr == nullptr) {
            ptrMap[i][j] = new RecursivePtr(offset++);
          } else {
            ptrMap[i][j] = new RecursivePtr;
            ptrMap[i][j]->set(ptr);
          }
        }
      }
    }

    if (localPtr != nullptr) delete localPtr;

#pragma omp barrier
    if (omp_get_thread_num()) {
      for (size_t j = 0; j < w; ++j) {
        if (src[h1][j] == 0) {
          if (ptrMap[h1 - 1][j] != nullptr && ptrMap[h1][j] != nullptr &&
              ptrMap[h1 - 1][j]->getValue() != ptrMap[h1][j]->getValue()) {
            omp_set_lock(lock);
            ptrMap[h1 - 1][j]->set(ptrMap[h1][j]);
            omp_unset_lock(lock);
          }
        }
      }
    }

    omp_destroy_lock(lock);
    delete lock;

#pragma omp for
    for (size_t i = 0; i < h; ++i)
      for (size_t j = 0; j < w; ++j)
        if (ptrMap[i][j] != nullptr) dst[i][j] = ptrMap[i][j]->getValue();
    #pragma omp for
        for (size_t i = 0; i < h; ++i)
            for (size_t j = 0; j < w; ++j)
                if (ptrMap[i][j] != nullptr) delete ptrMap[i][j];

    // if (ptr != nullptr) delete ptr;
    // if (localPtr != nullptr) delete localPtr;
  }

}

}  // namespace KruglovOmpTask
// Copyright 2023 Kruglov Alexey

#include "omp/kruglov_a_components_marking_omp/include/ops_omp.hpp"

#include <omp.h>

using namespace std::chrono_literals;

namespace KruglovOmpTask {

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
  uint32_t curLabel = 1;
  std::list<uint32_t> labelVec;  // critical data
  std::list<uint32_t *> groupVec;
  std::vector<std::vector<uint32_t **>> ptrMap;
  ptrMap.resize(h);

  omp_lock_t *lock = new omp_lock_t;
  omp_init_lock(lock);

  uint32_t num_threads = omp_get_thread_limit();
  if (num_threads > h) num_threads = 1;

#pragma omp parallel num_threads(num_threads)
  {
#pragma omp for
    for (size_t i = 0; i < h; ++i) ptrMap[i].resize(w, nullptr);

    int32_t d = h / omp_get_num_threads();
    size_t h0;
    size_t h1;
    if (omp_get_thread_num()) {
      h0 = d * (omp_get_thread_num() - 1);
      h1 = d * (omp_get_thread_num());
    } else {
      h0 = d * (omp_get_num_threads() - 1);
      h1 = h;
    }

    for (size_t i = 0; i < w; ++i) {
      if (src[h0][i] == 0) {
        if (i == 0 || ptrMap[h0][i - 1] == nullptr) {
          omp_set_lock(lock);
          labelVec.push_back(curLabel++);
          groupVec.push_back(&labelVec.back());
          ptrMap[h0][i] = &groupVec.back();
          omp_unset_lock(lock);
        } else
          ptrMap[h0][i] = ptrMap[h0][i - 1];
      }
    }
    for (size_t i = 1 + h0; i < h1; ++i) {
      if (src[i][0] == 0) {
        if (ptrMap[i - 1][0] == nullptr) {
          omp_set_lock(lock);
          labelVec.push_back(curLabel++);
          groupVec.push_back(&labelVec.back());
          ptrMap[i][0] = &groupVec.back();
          omp_unset_lock(lock);
        } else
          ptrMap[i][0] = ptrMap[i - 1][0];
      }
      for (size_t j = 1; j < w; ++j) {
        if (src[i][j] == 0) {
          uint32_t **ptr = nullptr;
          if (ptrMap[i - 1][j] != nullptr) ptr = ptrMap[i - 1][j];
          if (ptrMap[i][j - 1] != nullptr) {
            if (ptr != nullptr && *ptrMap[i][j - 1] != *ptr) *ptrMap[i - 1][j] = *ptrMap[i][j - 1];
            ptr = ptrMap[i][j - 1];
          }

          if (ptr == nullptr) {
            omp_set_lock(lock);
            labelVec.push_back(curLabel++);
            groupVec.push_back(&labelVec.back());
            ptrMap[i][j] = &groupVec.back();
            omp_unset_lock(lock);
          } else
            ptrMap[i][j] = ptr;
        }
      }
    }
#pragma omp barrier
    if (omp_get_thread_num()) {
      for (size_t j = 0; j < w; ++j) {
        if (src[h1][j] == 0) {
          if (ptrMap[h1 - 1][j] != nullptr && ptrMap[h1][j] != nullptr && *ptrMap[h1 - 1][j] != *ptrMap[h1][j]) {
            omp_set_lock(lock);
            *ptrMap[h1 - 1][j] = *ptrMap[h1][j];
            ptrMap[h1][j] = ptrMap[h1 - 1][j];
            omp_unset_lock(lock);
          }
        }
      }
    }

    omp_destroy_lock(lock);
#pragma omp single
    {
      size_t cur = 1;
      groupVec.sort();
      groupVec.unique();
      for (auto &label : groupVec) {
        *label = cur++;
      }
    }

#pragma omp for
    for (size_t i = 0; i < h; ++i)
      for (size_t j = 0; j < w; ++j)
        if (ptrMap[i][j] != nullptr) dst[i][j] = **(ptrMap[i][j]);
  }
}
}  // namespace KruglovOmpTask
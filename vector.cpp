//==============================================================
// Copyright © 2020 Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#include <CL/sycl.hpp>
using namespace sycl;

static const size_t N = 100;

#include <chrono>
#include <vector>
#include <numeric>
#include <algorithm>

void work(queue &q) {
  std::cout << "Device : "
            << q.get_device().get_info<info::device::name>()
            << std::endl;
            
  std::vector<int> vector1(N);
  std::iota(vector1.begin(), vector1.end(), 1);

  std::cout << "vector1  : ";
  for (const auto& elem: vector1) std::cout << elem << ' ';
  std::cout << std::endl;

  std::vector<int> vector2(N);
  std::generate(vector2.begin(), vector2.end(), [] {
      static int i = 0;
      return (++i * N);
  });  

  std::cout << "vector2  : ";
  for (const auto& elem: vector2) std::cout << elem << ' ';
  std::cout << std::endl;


  buffer vector1_buffer {vector1};
  buffer vector2_buffer {vector2};
    
  q.submit([&](handler &h) {

    auto vector1_accessor = vector1_buffer.get_access<access::mode::read_write>(h);
    auto vector2_accessor = vector2_buffer.get_access <access::mode::read >(h);

    h.parallel_for<class test>(range<1>(N), [=](id<1> index) {

      vector1_accessor[index] += vector2_accessor[index];
    });
  });
  q.wait();
  vector1_buffer.get_access<access::mode::read>();

  std::cout << "vector1 + vector2  : ";
  for (const auto& elem: vector1) std::cout << elem << ' ';
  std::cout << std::endl;
}

int main() {  
  try {
    queue q;
    auto start = std::chrono::high_resolution_clock::now();
    work(q);
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    std::cout << "Elapsed microseconds : " << microseconds << std::endl;
  } catch (exception e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    std::terminate();
  } catch (...) {
    std::cerr << "Unknown exception" << std::endl;
    std::terminate();
  }
}
// SIMD_Tests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <intrin.h>
#include <iostream>
#include <cassert>
#include <thread>
#include <future>
#include <vector>
#include <numeric>

static const bool printVectors = false;
static const bool printIterInfo = false;

static const unsigned int iterations = 1000000;
static const unsigned int vecSize = 4 * 10000;

void printNFloatVector(const char* name, unsigned int n, float* v) {
  if (!printVectors)
    return;

  std::cout << name << ": ";

  for (unsigned int i = 0; i < n; ++i) {
    std::cout << *(v + i);

    if(i < (n -1))
      std::cout << ", ";
    else
      std::cout << std::endl;
  }
}

std::chrono::duration<double, std::milli> operate(float* res, float* v1, float* v2) {
  auto start = std::chrono::high_resolution_clock::now();
  for (unsigned int i = 0; i < vecSize; ++i) {
    res[i] = v1[i] * v2[i];
  }
  auto end = std::chrono::high_resolution_clock::now();
  return end - start;
}

std::chrono::duration<double, std::milli> operate_simd(float* res, float* v1, float* v2) {
  auto start = std::chrono::high_resolution_clock::now();
  for (unsigned int i = 0; i < vecSize; i += 4) {
    __m128 v1_128 = _mm_set_ps(v1[i + 3], v1[i + 2], v1[i + 1], v1[i]); //Load 4 bytes to v1_128 (Loads in reverse order so we input in erverse too)
    __m128 v2_128 = _mm_set_ps(v2[i + 3], v2[i + 2], v2[i + 1], v2[i]); //Load 4 bytes to v2_128 (Loads in reverse order so we input in erverse too)
    __m128 res_128 = _mm_mul_ps(v1_128, v2_128); //Add v1_128 & v2_128 and save in res_128
    memcpy((res + i), &res_128, sizeof(res_128)); //Save result from res_128 to res
  }
  auto end = std::chrono::high_resolution_clock::now();
  return end - start;
}

int main()
{
  float v1[vecSize];
  float v2[vecSize];
  float res[vecSize];
  float res_simd[vecSize];

  assert((sizeof(v1) / sizeof(float)) % 4 == 0);
  assert((sizeof(v2) / sizeof(float)) % 4 == 0);
  assert((sizeof(res) / sizeof(float)) % 4 == 0);
  assert((sizeof(res_simd) / sizeof(float)) % 4 == 0);

  for (unsigned int i = 0; i < vecSize; ++i) {
    v1[i] = static_cast<float>(i);
    v2[i] = static_cast<float>(vecSize - 1 - i);
  }
  memset(res, 0, sizeof(res));
  memset(res_simd, 0, sizeof(res_simd));

  printNFloatVector("v1", vecSize, v1);
  printNFloatVector("v2", vecSize, v2);

  std::vector<double> diffs_vector;

  for (int i = 0; i < iterations; ++i) {
    std::chrono::duration<double, std::milli> elapsed_normal;
    std::chrono::duration<double, std::milli> elapsed_simd;

    auto future = std::async(operate, res, v1, v2);
    auto future_simd = std::async(operate_simd, res, v1, v2);
    elapsed_normal = future.get();
    elapsed_simd = future_simd.get();
    /*
    std::packaged_task<std::chrono::duration<double, std::milli>(float*, float*, float*)> job1(operate);
    std::packaged_task<std::chrono::duration<double, std::milli>(float*, float*, float*)> job2(operate_simd);
    std::future<std::chrono::duration<double, std::milli>> future = job1.get_future();
    std::future<std::chrono::duration<double, std::milli>> future_simd = job2.get_future();
    std::thread worker1(std::move(job1));
    std::thread worker2(std::move(job2));
    worker1.join();
    worker2.join();
    elapsed_normal = future.get();
    elapsed_simd = future_simd.get();
    */
    double diff = elapsed_normal.count() - elapsed_simd.count();
    diffs_vector.push_back(diff);

    std::cout << "Iteration: " << i + 1 << std::endl;
    if (printIterInfo) {
      std::cout << std::fixed;
      std::cout << "\tOperate elapsed time: " << elapsed_normal.count() << std::endl;
      std::cout << "\tOperate SIMD elapsed time: " << elapsed_simd.count() << std::endl;
      std::cout << "\tDIFF: " << diff << std::endl;
      std::cout.unsetf(std::ios::fixed);
    }
  }

  double total = std::accumulate(diffs_vector.begin(), diffs_vector.end(), 0.0f);
  std::cout << "TOTAL: " << total << std::endl;
  std::cout << "AVERAGE: " << total / iterations << std::endl;

  printNFloatVector("res", vecSize, res);
  printNFloatVector("res_simd", vecSize, res);

  std::string a;
  std::cin >> a;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu
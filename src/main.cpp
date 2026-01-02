#include "Simulator.hpp"
#include <iomanip>
#include <iostream>
#include <vector>

int main() {
  std::cout << "=== CrossLayerARQ Simulator ===" << std::endl;

  // Parameter Sweep Configuration
  std::vector<uint32_t> W_values = {32}; // {2, 4, 8, 16, 32, 64};
  std::vector<size_t> L_values = {1024}; // {128, 256, 512, 1024, 2048, 4096};
  const int NUM_SEEDS = 1;               // 10;

  // Output Header
  std::cout << "W,L,Seed,Duration(s),Goodput(bps)" << std::endl;

  for (auto W : W_values) {
    for (auto L : L_values) {
      for (int seed = 0; seed < NUM_SEEDS; ++seed) {
        ARQ::Simulator sim;
        auto res = sim.Run(W, L);

        double dur = res.total_time.count() / 1'000'000.0;
        std::cout << W << "," << L << "," << seed << "," << std::fixed
                  << std::setprecision(4) << dur << "," << std::fixed
                  << std::setprecision(2) << res.goodput_bps << std::endl;
      }
    }
  }

  return 0;
}
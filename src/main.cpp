#include "Simulator.hpp"
#include <iomanip>
#include <iostream>
#include <vector>

int main() {
  std::cout << "=== CrossLayerARQ Simulator ===" << std::endl;

  // Parameter Sweep Configuration
  std::vector<uint32_t> W_values = {64}; // {1, 16, 64}
  std::vector<size_t> L_values = {1024}; // {512, 1024, 4096}

  // Output Header
  std::cout << "W,L,Duration(s),Goodput(bps)" << std::endl;

  for (auto W : W_values) {
    for (auto L : L_values) {
      ARQ::Simulator sim;
      // Run
      auto res = sim.Run(W, L);

      // Print Result
      double dur = res.total_time.count() / 1'000'000.0;
      std::cout << W << "," << L << "," << std::fixed << std::setprecision(4)
                << dur << "," << std::fixed << std::setprecision(2)
                << res.goodput_bps << std::endl;
    }
  }

  return 0;
}
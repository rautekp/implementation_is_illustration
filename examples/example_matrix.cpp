#include <DemoCases.hpp> // Include shared demos via iii_demos library
#include <iii/Recorder.hpp>
#include <iostream>


int main() {
  std::cout << "Starting Matrix Verification..." << std::endl;

  // Use the shared demo logic
  // This function records events to the singleton Recorder instance
  DemoCases::generate_matrix_demo();

  std::cout << "Trace generation complete." << std::endl;

  iii::Recorder::get().dump("trace_matrix.json");
  return 0;
}

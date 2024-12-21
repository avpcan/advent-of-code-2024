#include <memory>
#include <unordered_map>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

typedef std::pair<int64_t, int64_t> StatePair;

class ValueStateMachine {
  // The value on the stones will go through a deterministic path.
  // This state machine should get updated as new values are discovered.

  // For every value, two "next" values are stored;
  // - The first is the new stone's value
  // - The second is the split stone's value (or null if no split)

  // If the new stone's value isn't in the map yet, this class adds it to the state machine and returns its next states.

  // The pointers that are not to -1 will be returned in a list.
  // The size of the array represents num_stones

  protected:
    std::unordered_map<int64_t, StatePair> unitedStates_;

  public:
    ValueStateMachine() {
      unitedStates_[-1] = std::make_pair(-1,-1);
    }

    bool HasState(int64_t stateValue) {
      if (unitedStates_.find(stateValue) == unitedStates_.end()) {return false; }
      else { return true; }
    }

    void NewState (int64_t value, StatePair next_states) {
      unitedStates_[value] = next_states;
    }

    StatePair AddState (int64_t value) {
      StatePair next_states = CalculateNextStates(value);
      unitedStates_[value] = next_states;
      return next_states;
    }

    StatePair GetNextStates (int64_t stateValue) {
      if(HasState(stateValue)) {
        return unitedStates_[stateValue];
      }
      else {
        return AddState(stateValue);
      }
    }

    StatePair CalculateNextStates (int64_t stateValue) {
      // The number 0 is replaced by a 1
      if (stateValue == 0) {return StatePair(1, -1);}

      // Determine the number of digits
      int numDigits = 0;
      int64_t valueLeft = stateValue;
      while(valueLeft > 0) {
        numDigits++;
        valueLeft = valueLeft / 10;
      }

      if (numDigits % 2 == 0) {
        // An even number of digits is replaced by two values:
        // The left half of the digits and the right half of the digits
        // (The new numbers don't keep extra leading zeroes: 1000 would become stones 10 and 0.)
        int64_t state1 = stateValue;
        int64_t state2 = 0;
        int state1Digits = numDigits/2;
        int state2Digits = numDigits/2;

        while (state1Digits > 0) {
          state1 = std::floor(state1 / 10);
          state1Digits--;
        }

        int multiplier = 1;
        while (state2Digits > 0) {
          multiplier = multiplier * 10;
          state2Digits--;
        }
        state2 = stateValue - (multiplier * state1);

        return StatePair(state1, state2);
      }

      else {
        // The number is multiplied by 2024.
        return StatePair(stateValue * 2024, -1);
      }
    }

}; // ValueStateMachine

std::unordered_map<int64_t, int64_t> PopulateHistogramFromFile(const std::string& filePath) {
    std::unordered_map<int64_t, int64_t> valueHistogram;
    std::ifstream inputFile(filePath);

    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return valueHistogram;
    }

    int64_t value;
    while (inputFile >> value) {
      auto histogramItr = valueHistogram.find(value);
      if (histogramItr != valueHistogram.end()) {
        valueHistogram[value] += 1;
      }
      else {
        valueHistogram[value] = 1;
      }
    }

    inputFile.close();
    return valueHistogram;
}

int main(int argc, char** argv) {

  // Read inputs
  std::string filename = argv[1];
  int numIterations = atoi(argv[2]);
  ValueStateMachine state_machine;

  // Create a histogram of the stones
  std::unordered_map<int64_t, int64_t> valueHistogram = PopulateHistogramFromFile(filename);
  std::cout << "~~INITIAL HISTOGRAM~~" << std::endl;
  for ( const auto &entry : valueHistogram) {
    std::cout << "{" << entry.first << ", " << entry.second << "}" << std::endl;
  }

  // for each iteration, make a new histogram with the "next" state(s) of each stone
  int iterationsLeft = numIterations;
  while(iterationsLeft > 0) {
    std::unordered_map<int64_t, int64_t> updatedHistogram;

    for (const auto &entry : valueHistogram) {
      StatePair nextStates = state_machine.GetNextStates(entry.first);
      int64_t numWithValue = entry.second;
      // The replaced stone; will always exist
      int64_t value = nextStates.first;
      auto updatedHistogramItr = updatedHistogram.find(value);
      if (updatedHistogramItr != updatedHistogram.end()) {
        updatedHistogram[value] += numWithValue;
      }
      else {
        updatedHistogram[value] = numWithValue;
      }

      // The split stone (if applicable)
      value = nextStates.second;
      if (value != -1) {
        auto updatedHistogramItr = updatedHistogram.find(value);
        if (updatedHistogramItr != updatedHistogram.end()) {
          updatedHistogram[value] += numWithValue;
        }
        else {
          updatedHistogram[value] = numWithValue;
        }
      }

    }
    // auto histogramItr = valueHistogram.find(value);
    // if (histogramItr != valueHistogram.end()) {
    //   valueHistogram[value] = 1;
    // }
    // else {
    //   valueHistogram[value] += 1;
    // }

    valueHistogram = updatedHistogram;
    iterationsLeft--;
  }

  std::cout << "~~END HISTOGRAM~~" << std::endl;
  int64_t count = 0;
  for ( const auto &entry : valueHistogram) {
    std::cout << "{" << entry.first << ", " << entry.second << "}" << std::endl;
    count += entry.second;
  }

  std::cout << "~~Total Count~~" << std::endl;
  std::cout << count << std::endl;


}

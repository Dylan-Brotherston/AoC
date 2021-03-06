#include <iostream>
#include <fstream>
#include <set>
#include <vector>


uint32_t task_1(std::ifstream input_fd) {
    int32_t sum{}, tmp_val{};

    while (input_fd >> tmp_val) sum += tmp_val;

    return sum;
}

uint32_t task_2(std::ifstream input_fd) {
    int32_t sum{}, tmp_val{};
    std::set<int32_t> freq_set{};
    std::vector<int> freqs;

    while (input_fd >> tmp_val) freqs.emplace_back(tmp_val);


    while (true) {
        for (const auto &tmp_val : freqs) {
            sum += tmp_val;

            if (freq_set.find(sum) != freq_set.end()) {
                return sum;
            }
            else {
                freq_set.emplace(sum);
            }
        }    

    }
    return 0;
   

}

int main() {

    std::cout << "sum of frequencies in task 1 is : " << task_1(std::ifstream{ "input/day01_input.txt" }) << std::endl;
    std::cout << "first repeated frequency in task 2 is : " << task_2(std::ifstream{ "input/day01_input.txt" }) << std::endl;

    return 0;
}

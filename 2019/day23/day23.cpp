#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <chrono>
#include <set>
#include <map>
#include <queue>


class timer {
public:
    timer(std::string point) : m_point{ point } {
        m_start_point = std::chrono::high_resolution_clock::now();
    }

    ~timer() {
        auto endclock = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start_point).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endclock).time_since_epoch().count();

        auto duration = end - start;
        double ms = duration * 0.001;

        std::cout << "time used by : " << m_point << " was : " << ms << " ms" << std::endl;

    }

private:
    std::string m_point;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_point;
};


class intcode_computer {
public:
    intcode_computer(std::vector<int64_t> p_cmd) : m_memory{ p_cmd }, m_initial_size{ p_cmd.size() }{
        std::vector<int64_t> padding(3000, 0);
        std::copy(padding.begin(), padding.end(), std::back_inserter(m_memory));
    }

    void set_next_input(int64_t p_input) {
        if (m_address == -1) {
            m_address = static_cast<int16_t>(p_input);
        }
        else
        {
            m_next_input.push(p_input);
        }
    }

    bool is_still_running()const { return !m_has_returned; }
    bool is_queue_empty()const { return m_next_input.empty(); }
    bool is_idle()const { return m_is_idle; }

    std::vector<int64_t> get_output() {
        std::vector<int64_t> ret = m_output;
        m_output.clear();
        return ret;
    }

    void run_cycle() {
        while (!m_has_returned) {

            auto opcode = m_memory[idx];
            uint16_t instruction = opcode % 100;


            int64_t* param1_ptr{};
            int64_t* param2_ptr{};
            int64_t* param3_ptr{};


            if (instruction != 99) {
                opcode /= 100;
                switch (opcode % 10) {
                case 0:
                    param1_ptr = &m_memory[m_memory[idx + 1]];
                    break;

                case 1:
                    param1_ptr = &m_memory[idx + 1];
                    break;

                case 2:
                    param1_ptr = &m_memory[m_relative_base + m_memory[idx + 1]];
                    break;
                }
            }

            if (m_instructions_with_3_params.find(static_cast<uint8_t>(instruction)) != m_instructions_with_3_params.end()) {
                opcode /= 10;

                switch (opcode % 10) {
                case 0:
                    param2_ptr = &m_memory[m_memory[idx + 2]];
                    break;

                case 1:
                    param2_ptr = &m_memory[idx + 2];
                    break;

                case 2:
                    param2_ptr = &m_memory[m_relative_base + m_memory[idx + 2]];
                    break;
                }
            }

            if (instruction == 1 || instruction == 2 || instruction == 7 || instruction == 8) {
                opcode /= 10;

                switch (opcode % 10) {
                case 0:
                    param3_ptr = &m_memory[m_memory[idx + 3]];
                    break;

                case 2:
                    param3_ptr = &m_memory[m_relative_base + m_memory[idx + 3]];
                    break;
                }

            }

            std::size_t old_idx = idx;
            switch (instruction)
            {

            case 1:
                *param3_ptr = *param1_ptr + *param2_ptr;
                idx = old_idx != m_memory[idx + 4] ? idx + 4 : idx;
                break;

            case 2:
                *param3_ptr = *param1_ptr * *param2_ptr;
                idx = old_idx != m_memory[idx + 4] ? idx + 4 : idx;
                break;

            case 3:
                if (!m_is_address_set) {
                    *param1_ptr = m_address;
                    m_is_address_set = true;
                }
                else {
                    if (m_next_input.empty()) {
                        //*param1_ptr = -1;
                        m_is_idle = true;
                        return;
                    }
                    else {
                        *param1_ptr = m_next_input.front();
                        m_next_input.pop();
                        m_is_idle = false;
                    }
                }
                idx += 2;
                break;

            case 4:
                m_output.push_back(*param1_ptr);
                idx += 2;
                if (m_output.size() == 3)
                    return;
                break;

            case 5:
                idx = *param1_ptr != 0 ? *param2_ptr : idx + 3;
                break;

            case 6:
                idx = *param1_ptr == 0 ? *param2_ptr : idx + 3;
                break;

            case 7:
                *param3_ptr = *param1_ptr < *param2_ptr;
                idx = old_idx != m_memory[idx + 4] ? idx + 4 : idx;
                break;

            case 8:
                *param3_ptr = *param1_ptr == *param2_ptr;
                idx = old_idx != m_memory[idx + 4] ? idx + 4 : idx;
                break;

            case 9:
                m_relative_base += *param1_ptr;
                idx += 2;
                break;

            case 99:
                m_has_returned = true;
                m_is_idle = true;
                idx += 1;
                break;

            default:
                throw std::runtime_error("wrong instruction in op code");
            }
        }

    }

private:
    std::vector<int64_t> m_memory;
    std::queue<int64_t> m_next_input{};
    std::vector<int64_t> m_output;
    bool m_has_returned{};
    size_t idx{};
    size_t m_relative_base{};
    const static std::set<uint8_t> m_instructions_with_3_params;
    size_t m_initial_size{};
    int16_t m_address{ -1 };
    bool m_is_address_set{ false };
    bool m_is_idle{ false };

};

const std::set<uint8_t> intcode_computer::m_instructions_with_3_params{ 1,2,5,6,7,8 };

std::vector<int64_t> string2vector(std::string input_txt) {
    std::vector<int64_t> ret;
    size_t pos{};
    int64_t cmd_instance{};
    while ((pos = input_txt.find(',')) != std::string::npos) {
        std::stringstream{ input_txt.substr(0, pos) } >> cmd_instance;
        ret.push_back(cmd_instance);
        input_txt.erase(0, pos + 1);
    }
    std::stringstream{ input_txt.substr(0, pos) } >> cmd_instance;
    ret.push_back(cmd_instance);
    return ret;
}



void task_1(std::vector<int64_t> p_cmds) {
    std::vector<intcode_computer> network;

    for (uint8_t idx{}; idx < 50; ++idx) {
        network.push_back({ p_cmds });
        network.back().set_next_input(idx);
        network.back().set_next_input(-1);
    }

    bool msg_fnd{};
    while (!msg_fnd) {
        for (auto& machine : network) {

            machine.run_cycle();
            auto output = machine.get_output();
            if (output.size() == 0)
                continue;

            if (output[0] == 255) {
                std::cout << "task 1 return was : " << output[2] << std::endl;
                msg_fnd = true;
                break;
            }

            for (uint16_t msg_idx{}; msg_idx < output.size(); msg_idx += 3) {
                network[output[msg_idx]].set_next_input(output[msg_idx + 1]);
                network[output[msg_idx]].set_next_input(output[msg_idx + 2]);
            }


        }
    }

}

void task_2(std::vector<int64_t> p_cmds) {

    std::vector<intcode_computer> network;

    intcode_computer nat{ p_cmds };
    nat.set_next_input(255);

    for (uint8_t idx{}; idx < 50; ++idx) {
        network.push_back({ p_cmds });
        network.back().set_next_input(idx);
        network.back().set_next_input(-1);
    }
    std::vector<int64_t> nat_cmds_history;
    int64_t nx{}, ny{};

    while (true) {
        for (auto& machine : network) {

            machine.run_cycle();
            auto output = machine.get_output();
            if (output.size() == 0)
                continue;

            if (output[0] == 255) {
                nx = output[1];
                ny = output[2];

                continue;
            }

            for (int16_t msg_idx{}; msg_idx < output.size(); msg_idx += 3) {
                network[output[msg_idx]].set_next_input(output[msg_idx + 1]);
                network[output[msg_idx]].set_next_input(output[msg_idx + 2]);
            }
            
        }
        if (std::all_of(network.cbegin(), network.cend(), [](const intcode_computer& obj) { return obj.is_idle() ; })) {
            if (ny == 0)
                continue;

            if (nat_cmds_history.size() != 0 && ny == nat_cmds_history.back()) {
                std::cout << "task 2 output was :" << ny << std::endl;
                return;
            }

            network[0].set_next_input(nx);
            network[0].set_next_input(ny);
            network[0].run_cycle();
            nat_cmds_history.push_back(ny);

        }
    }

}

int main() {

    std::ifstream input_fd{ "input\\day23_input.txt" };

    std::string tmp;
    input_fd >> tmp;

    auto cmds = string2vector(tmp);
    {
        timer t1("task 1");
        task_1(cmds);
    }

    {
        timer t1("task 2");
        task_2(cmds);
    }

    return 0;
}

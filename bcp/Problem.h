
#ifndef MAPFW_PROBLEM_H
#define MAPFW_PROBLEM_H



#include <nlohmann/json.hpp>
#include <vector>
#include <chrono>

class Problem
{
private:

public:
    struct coord{
        int x;
        int y;

        coord(int x, int y):x(x), y(y){}
    };

    struct Solution{
        int cost;
        std::vector<int> costs;
        std::vector<std::vector<coord>> paths;
    };

    int width;
    int height;
    int id;
    int agent_n;
    bool* grid;
    bool solved = false;
    std::vector<coord> start_coords;
    std::vector<coord> goal_coords;
    Solution* solution;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::duration<long int, std::ratio<1, 1000>> elapsed;


    Problem(const nlohmann::json& problem_json);
    ~Problem();
    void create_solution();
    void start_clock();
    void stop_clock();
    void to_json(nlohmann::json& return_json);
    void print();


};

#endif





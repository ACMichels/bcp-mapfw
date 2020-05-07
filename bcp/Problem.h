
#ifndef MAPFW_PROBLEM_H
#define MAPFW_PROBLEM_H



#include <nlohmann/json.hpp>
#include <vector>

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
        bool solved = false;
    };

    int width;
    int height;
    int id;
    int agent_n;
    bool* grid;
    std::vector<coord> start_coords;
    std::vector<coord> goal_coords;
    Solution* solution;

    Problem(const nlohmann::json& problem_json);
    ~Problem();
    void create_solution();


};

#endif





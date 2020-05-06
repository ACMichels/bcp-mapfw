
#ifndef MAPFW_PROBLEM_H
#define MAPFW_PROBLEM_H



#include <nlohmann/json.hpp>
#include <vector>

class Problem
{
private:
    struct coord{
        int x;
        int y;

        coord(int x, int y):x(x), y(y){}
    };

public:
    int width;
    int height;
    int id;
    int agent_n;
    bool* grid;
    std::vector<coord> start_coords;
    std::vector<coord> goal_coords;

    Problem(const nlohmann::json& problem_json);
    ~Problem();



};

#endif





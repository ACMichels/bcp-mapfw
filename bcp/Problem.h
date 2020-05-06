#include <nlohmann/json.hpp>
#include <vector>

class Problem
{
struct coord{
    int x;
    int y;

    coord(int x, int y):x(x), y(y){}
};

private:
    int width;
    int height;
    int id;
    int agent_n;
    bool* grid;
    std::vector<coord> start_coords;
    std::vector<coord> goal_coords;

public:
    Problem(const nlohmann::json problem_json);
    ~Problem();



};





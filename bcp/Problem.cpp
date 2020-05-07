
#include "Problem.h"
#include "iostream"
#include "string"
#include <algorithm>
#include "Includes.h"

Problem::Problem(const nlohmann::json& problem_json)
{
    // Reformat json string in json
    std::string problem_data_string = problem_json["problem"].dump();
    problem_data_string.erase(std::remove(problem_data_string.begin(), problem_data_string.end(), '\\'), problem_data_string.end());
    problem_data_string[0] = problem_data_string[problem_data_string.length()-1] = ' ';
    nlohmann::json problem_data_json = nlohmann::json::parse(problem_data_string);

    // Read data from json
    id = problem_json["id"];
    width = problem_data_json["width"];
    height = problem_data_json["height"];
    agent_n = problem_data_json["starts"].size();

    // Read the grid
    grid = (bool*) malloc(width * height);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            grid[width*i+j] = problem_data_json["grid"][i][j] == 1 ? false : true;
        }
    }

    // Read start and end locations
    for (int i = 0; i < agent_n; i++)
    {
        start_coords.push_back(coord(problem_data_json["starts"][i][0], problem_data_json["starts"][i][1]));
        goal_coords.push_back(coord(problem_data_json["goals"][i][0], problem_data_json["goals"][i][1]));
    }

    // Check
    release_assert(start_coords.size() == (unsigned int)agent_n, "start_coords does not have the right amount of coords ({} /= {})", start_coords.size(), agent_n);
    release_assert(goal_coords.size() == (unsigned int)agent_n, "goal_coords does not have the right amount of coords ({} /= {})", goal_coords.size(), agent_n);
}

void Problem::to_json(nlohmann::json& return_json)
{
    return_json["problem"] = id;
    return_json["time"] = elapsed.count();
    return_json["solution"];
    for (int a = 0; a < agent_n; a++)
    {
        nlohmann::json j2;
        for (unsigned int c = 0; c < solution->paths[a].size(); c++)
        {
            j2[c].push_back(solution->paths[a][c].x);
            j2[c].push_back(solution->paths[a][c].y);
        }
        return_json["solution"].push_back(j2);
    }
}

void Problem::create_solution()
{
    solution = new Solution();
    solution->costs.reserve(agent_n);
    solution->paths.reserve(agent_n);

}

void Problem::start_clock()
{
    start = std::chrono::high_resolution_clock::now();
}

void Problem::stop_clock()
{
    auto stop = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
}

void Problem::print()
{
    println("******************* Problem {} *******************", id);
    if (solved)
    {
        println("SOLVED");
        println("Time:       {}", elapsed.count());
        println("Total cost: {}", solution->cost);

        for (int a = 0; a < solution->costs.size(); a++)
        {
            fmt::print("{:>3}: {:<5} ", a, solution->costs[a]);
            for (int i = 0; i < solution->paths[a].size(); i++)
            {
                fmt::print("({:<2}, {:<2})  ", solution->paths[a][i].x, solution->paths[a][i].y);
            }
            println("");
        }

    } else {
        println("UNSOLVED");
    }
    println("");
}

Problem::~Problem()
{
    delete[] grid;
    delete solution;
}

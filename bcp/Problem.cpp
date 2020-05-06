
#include "Problem.h"
#include "iostream"
#include "string"
#include <algorithm>

Problem::Problem(const nlohmann::json problem_json)
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

    for (int i = 0; i < agent_n; i++)
    {
        start_coords.push_back(coord(problem_data_json["starts"][i][0], problem_data_json["starts"][i][1]));
        goal_coords.push_back(coord(problem_data_json["goals"][i][0], problem_data_json["goals"][i][1]));
    }
}

Problem::~Problem()
{
    delete[] grid;
}

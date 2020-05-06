
#ifndef MAPFW_BENCHMARKER_H
#define MAPFW_BENCHMARKER_H


#include <fstream>
#include "Includes.h"
#include "iostream"
#include <stdlib.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <Problem.h>

class Benchmarker
{
public:
    int benchmark_id;
    std::vector<Problem*> problems;


    Benchmarker();
    ~Benchmarker();

    void load(std::vector<int> problem_id);

    void submit();

};

#endif

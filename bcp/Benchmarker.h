
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
private:
    char API_key[32];

public:
    int benchmark_id;
    double timeout;
    std::vector<Problem*> problems;


    Benchmarker();
    ~Benchmarker();

    void load(int problem_id, bool debug);

    bool submit();

};

#endif

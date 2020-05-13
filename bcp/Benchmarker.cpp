
#include <fstream>
#include "Includes.h"
#include "Benchmarker.h"
#include "iostream"
#include <stdlib.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <Problem.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = (char*) realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

Benchmarker::Benchmarker()
{
    // Load API key
    std::string API_key_file_location = "/home/andor/Documents/BCP-MAFPW/bcp-mapfw/credentials.txt";
    std::ifstream API_key_file;
    API_key_file.open(API_key_file_location, std::ios::in);
    release_assert(API_key_file.good(), "Invalid credentials file {}", API_key_file_location);

    // Read credentials.
    API_key_file.getline(API_key, 32);
}

Benchmarker::~Benchmarker()
{
    for (unsigned int i = 0; i < problems.size(); i++)
    {
        delete problems[i];
    }
}

void Benchmarker::load(std::vector<int> problem_id, bool debug)
{
//    nlohmann::json problem_jsonn = "{\"id\":3451,\"problem\":\"{\\\"grid\\\": [[1, 1, 1, 1, 1, 1, 1], [1, 0, 1, 1, 0, 0, 1], [1, 0, 0, 0, 0, 0, 1], [1, 0, 1, 1, 1, 1, 1], [1, 1, 1, 1, 1, 1, 1]], \\\"width\\\": 7, \\\"height\\\": 5, \\\"starts\\\": [[5, 1], [1, 2], [2, 2]], \\\"goals\\\": [[4, 1], [5, 2], [1, 1]], \\\"waypoints\\\": [[[1, 3]], [], []]}\"}"_json;
//
//    problems.push_back(new Problem(problem_jsonn));
//    return;

    struct MemoryStruct chunk;
    chunk.memory = (char*) malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    CURL *curl;
    CURLcode res;

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) {

        // Setup header
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, fmt::format("X-API-Token:{}", API_key).c_str());
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "charset: utf-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* First set the URL that is about to receive our POST. This URL can
           just as well be a https:// URL if that is what should receive the
           data. */
        curl_easy_setopt(curl, CURLOPT_URL, fmt::format("https://mapfw.nl/api/benchmarks/{}/problems", problem_id[0]).c_str());
        /* Now specify the POST data */
        std::string opts = fmt::format("{{\"algorithm\" : \"BCP\", \"version\": \"heuristic-based waypoints\", \"debug\": {} }}", (debug?"true":"false"));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, opts.c_str());

        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK){
            err("curl_easy_perform() failed: {}\n", curl_easy_strerror(res));
        } else {
            nlohmann::json json_data = nlohmann::json::parse(chunk.memory);

            /* Setup problems */
            benchmark_id = json_data["attempt"];
            for (nlohmann::json json_problem : json_data["problems"])
            {
                problems.push_back(new Problem(json_problem));
            }
        }

        /* always cleanup */
        curl_easy_cleanup(curl); // Also deletes API_token
        delete headers;


    }
    else
    {
        err("curl setup failed");
    }

    free(chunk.memory);
    curl_global_cleanup();


}

void Benchmarker::submit()
{
    // put all data in json format
    nlohmann::json return_json;

    for (unsigned int i = 0; i < problems.size(); i++)
    {
        problems[i]->print();
        if (problems[i]->solved)
        {
            nlohmann::json j;
            problems[i]->to_json(j);
            return_json["solutions"].push_back(j);
        }
    }
//    return;

    // Send data
    struct MemoryStruct chunk;
    chunk.memory = (char*) malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    CURL *curl;
    CURLcode res;

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) {

        // Setup header
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, fmt::format("X-API-Token:{}", API_key).c_str());
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "charset: utf-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* First set the URL that is about to receive our POST. This URL can
           just as well be a https:// URL if that is what should receive the
           data. */
        curl_easy_setopt(curl, CURLOPT_URL, fmt::format("https://mapfw.nl/api/attempts/{}/solutions", benchmark_id).c_str());
        /* Now specify the POST data */
        std::string return_string = return_json.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, return_string.c_str());

        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK){
            err("curl_easy_perform() failed: {}\n", curl_easy_strerror(res));
        } else {
            fmt::print("Successfully sent with returncode");
        }

        /* always cleanup */
        curl_easy_cleanup(curl); // Also deletes API_token
        delete headers;

    }
}

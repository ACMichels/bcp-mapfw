
#include <fstream>
#include "Includes.h"
#include "Benchmarker.h"
#include "iostream"
#include <curl/curl.h>

class Benchmarker
{
public:
    static void load()
    {
        std::string API_key_file_location = "/home/andor/Documents/BCP-MAFPW/bcp-mapfw/credentials.txt";
        std::ifstream API_key_file;
        API_key_file.open(API_key_file_location, std::ios::in);
        release_assert(API_key_file.good(), "Invalid credentials file {}", API_key_file_location);

        // Read credentials.
        char API_key[32];
        API_key_file.getline(API_key, 32);




        CURL *curl;
        CURLcode res;

        /* In windows, this will init the winsock stuff */
        curl_global_init(CURL_GLOBAL_ALL);

        /* get a curl handle */
        curl = curl_easy_init();
        if(curl) {

            // Setup header
            struct curl_slist *headers = NULL;
            std::string API_token_string = std::string("X-API-Token:") + API_key;
            const char* API_token = API_token_string.c_str();
            headers = curl_slist_append(headers, API_token);
            headers = curl_slist_append(headers, "Accept: application/json");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "charset: utf-8");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            /* First set the URL that is about to receive our POST. This URL can
               just as well be a https:// URL if that is what should receive the
               data. */
            curl_easy_setopt(curl, CURLOPT_URL, "https://mapfw.nl/api/benchmarks/1/problems");
            /* Now specify the POST data */
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"algorithm\" : \"BCP\", \"version\": \"1\", \"debug\": true}");

            /* Perform the request, res will get the return code */
            res = curl_easy_perform(curl);
            /* Check for errors */
            if(res != CURLE_OK)
                err("curl_easy_perform() failed: {}\n", curl_easy_strerror(res));

            /* always cleanup */
            curl_easy_cleanup(curl);
            std::cout << res << "\n";
        }
        else
        {
            err("curl setup failed");
        }
        curl_global_cleanup();



    }

    void submit()
    {

    }

};

void testfunction()
{
    std::cout << "Test\n";
    Benchmarker::load();
}

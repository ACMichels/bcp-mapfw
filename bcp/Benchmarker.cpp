
#include <fstream>
#include "Includes.h"
#include "Benchmarker.h"
#include "iostream"

using namespace std;

class Benchmarker
{
public:
    static void load()
    {
        string API_key_file_location = "/home/andor/Documents/BCP-MAFPW/bcp-mapfw/credentials.txt";
        std::ifstream API_key_file;
        API_key_file.open(API_key_file_location, std::ios::in);
        release_assert(API_key_file.good(), "Invalid credentials file {}", API_key_file_location);

        // Read credentials.
        char buf[1024];
        API_key_file.getline(buf, 1024);
        cout << '!' << buf << '!' << '\n';
    }

    void submit()
    {

    }

};

void testfunction()
{
    cout << "Test\n";
    Benchmarker::load();
}

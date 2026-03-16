#pragma once
#include <string>
using namespace std;

struct ServerConfig {
    string ip = "0.0.0.0";
    int port = 12345;
    int bufferSize = 4096;
};
#include "webserv.hpp"

void compressSlashesHandler(std::string str, std::string expected)
{
    std::string url = str;
    compressSlashes(url);
    if (url != expected)
        std::cout << "Expected: " << expected << " Got: "<< url << "\n";
}

void compressSlashesTests()
{
    compressSlashesHandler("/api/truc", "/api/truc");
    compressSlashesHandler("/api////truc", "/api/truc");
    compressSlashesHandler("///////api////truc", "/api/truc");
    compressSlashesHandler("///////////api////////truc", "/api/truc");
    compressSlashesHandler("///////////api////////truc//", "/api/truc/");
    compressSlashesHandler("///////////api////////truc/////////", "/api/truc/");
}

int main()
{
    compressSlashesTests();
}
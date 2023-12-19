#include "../parsing.hpp"

int fill_port(const string &file, ServerBlock *server) {
    string listen = file.substr(0, file.find(';'));
    string ipAddress;

    if (listen.find(':') != string::npos) {
        ipAddress = listen.substr(0, listen.find(':'));
        listen = listen.substr(listen.find(':') + 1);
    } else
        ipAddress = "";
    server->set(ipAddress, listen, true);
    return (file.find(';') + 1);
}

int fill_host_name(const string &file, ServerBlock *server) {
    int start = 0;

    while (file[start] != ';') {
        if (file[start] == ',')
            start++;
        server->addHostName(file.substr(start, file.find_first_of(",;", start) - start));
        start = file.find_first_of(",;", start);
    }
    return (file.find(';') + 1);
}

bool fill_data(string file, Server *server) {
    int start = 0;
    int index = 0;
    ServerBlock *block;

    file.erase(std::remove_if(file.begin(), file.end(), delete_for_fill), file.end());
    while (file[index]) {
        redirect_priority = 0;
        index = file.find("server{", start);
        if (file.find("server{", start) == string::npos)
            break ;
        block = server->addBlock();
        index += 7;
        start = index;
        while (file[index] && file[index] != '}') {
            index += found_data(file.substr(index), block);
        }
    }
    if (!check_different_server(server))
        return (false);
    return (true);
}
#include "../parsing.hpp"

int redirect_priority;

int fill_redirect(const string &file, LocationBlock *location) {
    int i = 0;

    redirect_priority = 1;
    int number = atoi(file.c_str());
    i = file.find(',');
    i++;
    string url = file.substr(i, file.find(';') - i);
    location->setRedirection(number, url);
    return (file.find(';') + 1);
}

int fill_methods(const string &file, LocationBlock *location) {
    int i = 0;

    String methods = file.substr(0, file.find(';'));
    vector<String>  arrayMethods;
    while (methods[i]) {
        if (methods[i] == ',')
            i++;
        String method = methods.substr(i, methods.find(',', i) - i);
        i += method.size();
        arrayMethods.push_back(method);
    }
    arrayMethods.push_back("");
    if (arrayMethods[0] != "" || i == 0)
        location->setAllowedMethods(arrayMethods);
    return (i + 1);
}

int fill_proxy_pass(const string &file, LocationBlock *server) {
    if (redirect_priority == 1)
        return (file.find(';') + 1);
    server->setProxyPass(file.substr(0, file.find(';')));
    return (file.find(';') + 1);
}

int fill_location(const string &file, ServerBlock *server) {
    int i = 0;
    redirect_priority = 0;
    LocationBlock *location;

    location = server->addLocation();
    bool exact = false;
    if (file[i] == '=') {
        exact = true;
        i++;
    }
    location->setPath(file.substr(i, file.find('{') - i), exact);
    while (file[i] != '{')
        i++;
    i++;
    while (file[i] != '}')
        i += found_location_data(&file[i], location);
    location = NULL;
    return (i + 1);
}

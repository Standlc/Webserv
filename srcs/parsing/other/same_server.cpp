#include "../parsing.hpp"

void    erase_duplicates(vector<String> &host_names)
{
    sort(host_names.begin(), host_names.end());
    host_names.erase(unique(host_names.begin(), host_names.end()), host_names.end());
}

bool same_host_names(vector<String> host_names1, vector<String> host_names2)
{
    if (host_names1.size() == 0 && host_names2.size() == 0)
        return (true);
    erase_duplicates(host_names1);
    erase_duplicates(host_names2);
    host_names1.insert(host_names1.end(), host_names2.begin(), host_names2.end());
    if (std::adjacent_find(host_names1.begin(), host_names1.end()) != host_names1.end())
        return (true);
    return (false);
}

bool same_data(ServerBlock block1, ServerBlock block2)
{
    if (block1.getIpAddress() == block2.getIpAddress()
        && block1.getPort() == block2.getPort()
        && same_host_names(block1.getHostNames(), block2.getHostNames()))
        return (true);
    return (false);
}

bool    check_different_server(Server *server)
{
    int index = 0;
    int index_checker;
    int size = server->getServerSize();

    while (index < size)
    {
        index_checker = -1;
        while (++index_checker < size)
            if (index_checker != index && same_data(server->getServerBlock(index), server->getServerBlock(index_checker)))
                return (false);
        index++;
    }
    return (true);
}

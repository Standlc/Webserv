#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "../HttpRequest.hpp"
#include "../HttpResponse.hpp"
#include "../PollEvents/PollFd.hpp"
#include "../Server.hpp"
#include "../webserv.hpp"

typedef struct Redirection {
    String url;
    int statusCode;
} Redirection;

class Block {
   protected:
    String _index;
    bool _autoIndex;
    String _root;
    String _uploadRoot;
    size_t _reqBodyMaxSize;
    Redirection _redirection;

    std::unordered_multimap<String, String> _headers;
    std::unordered_map<int, std::string> _errorFiles;
    std::unordered_map<std::string, std::string> _cgiCommands;
    std::string _sessionCookieName;

   public:
    Block();
    Block(const Block &b);
    Block &operator=(const Block &b);
    virtual ~Block();

    void loadErrPage(int statusCode, HttpResponse &res, HttpRequest &req);
    String getResourcePath(const String &reqUrl, const String &file = "");
    String getUploadFilePath(const String &filename);
    bool hasErrorPage(int statusCode);
    void addErrorPage(int statusCode, String pagePath);

    void setRedirection(int statusCode, String redirectionUrl);
    void addCgiCommand(String extension, String absoluteCommandPath);
    void setRoot(String root);
    void setUploadRoot(String dir);
    void setIndex(String index);
    void setMaxBodySize(size_t size);
    void setAutoIndex(bool isOn);
    void addHeader(String key, String value);
    void setSessionCookie(String name);
};

class ServerBlock : public Block {
   private:
    String _ipAddress;
    String _port;
    bool _isDefault;
    std::vector<String> _hostNames;
    std::vector<LocationBlock> _locationBlocks;
    int _locationBlockSize;

   public:
    ServerBlock();
    ServerBlock &operator=(const ServerBlock &b);

    clientPollHandlerType execute(Server &server, ClientPoll &client);
    LocationBlock *findLocationBlockByPath(const String &reqPath);

    void setHostName(String name);
    bool isHost(String hostName);
    String getPort();
    String getIpAddress();
    bool isDefault();

    void set(String ipAddress, String port, bool isDefault);
    LocationBlock &getLocationBlock(int index);
    void addLocationBlocks(int size);
};

typedef void (*pathHandlerType)(LocationBlock &block, HttpRequest &req, HttpResponse &res);

class LocationBlock : public Block {
   private:
    String _path;
    bool _isExact;
    std::map<String, pathHandlerType> _handlers;

   public:
    LocationBlock(const Block &b);
    LocationBlock &operator=(const LocationBlock &b);

    void throwReqErrors(HttpRequest &req);
    bool handlesHttpMethod(String httpMethod);
    String assembleRedirectionUrl(HttpRequest &req);
    bool exceedsReqMaxSize(size_t size);
    String generateSessionCookie();

    clientPollHandlerType execute(Server &server, ClientPoll &client);
    clientPollHandlerType handleCgi(Server &server, ClientPoll &client, const String &cgiScriptPath);
    void checkCgiScriptAccess(const String &cgiScriptPath);
    String isCgiScriptRequest(HttpRequest &req);
    void setenvCgi(HttpRequest &req, const String &cgiScriptPath);

    // void proxyPass(Server &server, ClientPoll &client) {
    //     // Change host name
    //     // change connection to close
    //     // Send 502 if request fails or res status is error
    // }

    const String &getPath();
    const String &getIndex();
    bool isExactPath();
    bool isAutoIndex();
    void setHandlers(pathHandlerType getMethod, pathHandlerType postMethod, pathHandlerType deleteMethod);
    void setPath(const String &path, bool isExact = false);
};

#endif
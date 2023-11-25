#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "../PollEvents/PollFd.hpp"
#include "../Server.hpp"
#include "../ServerStreams/HttpRequest.hpp"
#include "../ServerStreams/HttpResponse.hpp"
#include "../webserv.hpp"

class Block {
   protected:
    String _index;
    bool _autoIndex;
    String _root;
    String _uploadRoot;
    size_t _reqBodyMaxSize;

    Headers _headers;
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
    bool isHost(const String &hostName);
    const String &port();
    const String &ipAddress();
    const std::vector<std::string> &hostNames();
    bool isDefault();

    void addHostName(String name);
    void set(String ipAddress, String port, bool isDefault);
    LocationBlock &getLocationBlock(int index);
    void addLocationBlocks(int size);
};

typedef void (LocationBlock::*serverMethodHandler)(HttpRequest &req, HttpResponse &res);
typedef clientPollHandlerType (LocationBlock::*requestHandlerType)(ClientPoll &client);

typedef struct Redirection {
    String url;
    int statusCode;
} Redirection;

class LocationBlock : public Block {
   private:
    std::map<String, serverMethodHandler> _serverMethodshandlers;
    requestHandlerType _requestHandler;
    std::vector<String> _allowedMethods;
    String _path;
    bool _isExact;
    String _proxyPass;
    ServerBlock &_serverBlock;
    Redirection _redirection;

   public:
    LocationBlock(ServerBlock &serverBlock);
    LocationBlock &operator=(const LocationBlock &b);

    ServerBlock &serverBlock();
    void throwReqErrors(HttpRequest &req);
    bool handlesHttpMethod(const String &httpMethod);
    bool isMethodAllowed(const String &httpMethod);
    String assembleRedirectionUrl(HttpRequest &req);
    bool exceedsReqMaxSize(size_t size);
    String generateSessionCookie();
    void handleMethod(const String &httpMethod, HttpRequest &req, HttpResponse &res);

    void getMethod(HttpRequest &req, HttpResponse &res);
    void postMethod(HttpRequest &req, HttpResponse &res);
    void deleteMethod(HttpRequest &req, HttpResponse &res);
    clientPollHandlerType serverMethodHandler(ClientPoll &client);
    clientPollHandlerType redirectionHandler(ClientPoll &client);
    clientPollHandlerType proxyHandler(ClientPoll &client);

    clientPollHandlerType reverseProxy(Server &server, ClientPoll &client);
    clientPollHandlerType execute(Server &server, ClientPoll &client);
    clientPollHandlerType handleCgi(ClientPoll &client, const String &cgiScriptPath);
    void checkCgiScriptAccess(const String &cgiScriptPath);
    String isCgiScriptRequest(HttpRequest &req);
    void setenvCgi(HttpRequest &req, const String &cgiScriptPath);

    void setRedirection(int statusCode, String redirectionUrl);
    void setProxyPass(const String &proxyPass);
    const String &getPath();
    const String &getIndex();
    bool isExactPath();
    bool isAutoIndex();
    void setAllowedMethods(String methods[]);
    void setPath(const String &path, bool isExact = false);
};

#endif
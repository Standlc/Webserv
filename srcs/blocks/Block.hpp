#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "../PollEvents/PollFd.hpp"
#include "../Server.hpp"
#include "../ServerStreams/HttpRequest.hpp"
#include "../ServerStreams/HttpResponse.hpp"
#include "../webserv.hpp"
#include "LocationBlock/utils.hpp"

class Block {
   protected:
    String _index;
    bool _autoIndex;
    String _root;
    String _uploadRoot;
    size_t _reqBodyMaxSize;
    String _fallBack;

    Headers _headers;
    std::map<int, String> _errorFiles;
    std::map<String, String> _cgiCommands;
    std::vector<String> _sessionCookies;

   public:
    Block();
    Block(const Block &b);
    Block &operator=(const Block &b);
    virtual ~Block();

    void loadErrPage(int statusCode, HttpResponse &res, const String &locationPath);
    String getResourcePath(const String &reqUrl, const String &file = "");
    String getUploadFilePath(const String &file);
    bool hasErrorPage(int statusCode);
    void addErrorPage(int statusCode, String pagePath);

    void addCgiCommand(String extension, String absoluteCommandPath);
    void setRoot(String root);
    void setUploadRoot(String dir);
    void setIndex(String index);
    void setFallBack(String file);
    void setMaxBodySize(size_t size);
    void setAutoIndex(bool isOn);
    void addHeader(String key, String value);
    void addSessionCookie(String name);
};

class ServerBlock : public Block {
   private:
    String _ipAddress;
    String _port;
    bool _isDefault;
    std::vector<String> _hostNames;
    std::vector<LocationBlock> _locations;

   public:
    ServerBlock();
    ServerBlock(const ServerBlock &b) {
        *this = b;
    }
    ServerBlock &operator=(const ServerBlock &b);

    clientPollHandlerType execute(ClientPoll &client);
    LocationBlock *findLocationBlockByPath(const String &reqPath);
    bool isHost(const String &hostName);
    const String &port();
    const String &ipAddress();
    const std::vector<String> &hostNames();
    bool isDefault();

    void addHostName(String name);
    void set(String ipAddress, String port, bool isDefault);
    LocationBlock &getLocationBlock(int index);
    LocationBlock *addLocation();
    LocationBlock *addLocation(LocationBlock &location);

    String getIpAddress();
    String getPort();
    std::vector<String> getHostNames();
};

typedef void (LocationBlock::*serverMethodHandlerType)(HttpRequest &req, HttpResponse &res);
typedef clientPollHandlerType (LocationBlock::*requestHandlerType)(ClientPoll &client);

typedef struct Redirection {
    String url;
    int statusCode;
} Redirection;

class LocationBlock : public Block {
   private:
    std::map<String, serverMethodHandlerType> _serverMethodshandlers;
    requestHandlerType _requestHandler;
    std::vector<String> _allowedMethods;
    String _path;
    bool _isExact;
    ProxyUrl *_proxyPass;
    ServerBlock *_serverBlock;
    Redirection _redirection;

   public:
    LocationBlock();
    LocationBlock(ServerBlock &serverBlock);
    LocationBlock(const LocationBlock &b);
    ~LocationBlock();
    LocationBlock &operator=(const LocationBlock &b);
    ServerBlock *serverBlock();

    bool handlesHttpMethod(const String &httpMethod);
    bool isMethodAllowed(const String &httpMethod);
    String assembleRedirectionUrl(HttpRequest &req);
    bool exceedsReqMaxSize(size_t size);
    String generateSessionCookie();
    void handleMethod(const String &httpMethod, HttpRequest &req, HttpResponse &res);
    void handleSessionCookies(ClientPoll &client);

    String getPathTranslated(const String &pathInfo, const String &cgiResourcePath);
    String cgiScriptResourcePath(const String &cgiScriptPath);
    String getReqPathInfo(HttpRequest &req);
    void getMethod(HttpRequest &req, HttpResponse &res);
    void postMethod(HttpRequest &req, HttpResponse &res);
    void deleteMethod(HttpRequest &req, HttpResponse &res);
    clientPollHandlerType serverMethodHandler(ClientPoll &client);
    clientPollHandlerType redirectionHandler(ClientPoll &client);
    clientPollHandlerType proxyHandler(ClientPoll &client);

    clientPollHandlerType reverseProxy(Server &server, ClientPoll &client);
    clientPollHandlerType execute(ClientPoll &client);
    clientPollHandlerType handleCgi(ClientPoll &client, const String &cgiScriptPath);
    void checkCgiScriptAccess(const String &cgiScriptPath);
    String isCgiScriptRequest(HttpRequest &req);
    void setenvCgi(HttpRequest &req, const String &cgiResourcePath, const String &cgiScriptPath);

    void setRedirection(int statusCode, String redirectionUrl);
    void setProxyPass(const String &proxyPass);
    const String &getPath();
    const String &getIndex();
    bool isExactPath();
    bool isAutoIndex();
    void setAllowedMethods(std::vector<String> methods);
    void setPath(const String &path, bool isExact = false);
};

#endif
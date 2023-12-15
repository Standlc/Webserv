#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include "srcs/webserv.hpp"
#include "srcs/Server.hpp"

using namespace std;

#define ERR -1

#define NOARG "1 argument is requiered"
#define NOFILE "File not found"
#define MBRACK "Cannot open or close the brackets"
#define MPORT "Port not valid"
#define MHOST "Host name not found"
#define MINDEX "Index files not found"
#define MROOT "Root not found"
#define MERRPAGE "Error pages not found"
#define MERRNUM "Number of error isn't accept"
#define MSYNT "Error syntaxes"
#define MMETHOD "Unrecognized method"
#define MLOCFOL "File location for location block not found"
#define MDATA "Unrecognized identifier"
#define MAUTIND "Error on the instruction for auto_index"
#define MEXT "Extention not found"
#define MHEAD "Invalid header"
#define MFORCE "File location for force not found"
#define MREDIRECT "Redirect not valid"
#define MBODYSIZE "Size of body not valid"
#define MPROXY "Proxy pass not valid"
#define MCOOKIE "Cookie syntaxes not valid"
#define MUPLOAD "Upload root not found"
#define MQUOTES "Opening of quotes not closed"

#define NOEX ""

#define EXPORT "listen: 80;"
#define EXHOST "host_name: www.apple.com;"
#define EXINDEX "index_file: index.html;"
#define EXROOT "root: /var/www;"
#define EXERRPAGE "404: /404.html;"
#define EXMETHOD "methods: GET;"
#define EXAUTIND "auto_index: on;"
#define EXREDIRECT "redirect: 303, http://apple.com;"
#define EXBODYSIZE "body_max_size: 20MB;"
#define EXPROXY "proxy_pass: http://apple.com;"
#define EXCOOKIE "cookie:blabla;"
#define EXUPLOAD "upload_root:download/"

#define NOLINE 0

void    fill_data(std::string file, Server *server);
int     parsing(int argc, char **argv, Server *server);
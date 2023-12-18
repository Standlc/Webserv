#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include "../webserv.hpp"
#include "../Server.hpp"

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
#define MFALLBACK "File location for fallback not found"
#define MREDIRECT "Redirect not valid"
#define MBODYSIZE "Size of body not valid"
#define MPROXY "Proxy pass not valid"
#define MCOOKIE "Cookie syntaxes not valid"
#define MUPLOAD "Upload root not found"
#define MQUOTES "Opening of quotes not closed"
#define MNOSERV "No server"
#define MNOLISTEN "No listen"
#define MNOROOT "No root"
#define MERRPAGESYNT "Syntaxe error in the error pages"
#define MSAMESERV "2 Servers have the same : ipAddress, Port and Hostname"

#define NOEX ""

#define EXPORT "listen: 80;"
#define EXHOST "host_name: www.apple.com;"
#define EXINDEX "index: index.html;"
#define EXROOT "root: /var/www;"
#define EXERRPAGE "404:/404.html;"
#define EXMETHOD "methods: GET;"
#define EXAUTIND "auto_index: on;"
#define EXREDIRECT "redirect: 303, http://apple.com;"
#define EXBODYSIZE "body_max_size: 20MB;"
#define EXPROXY "proxy_pass: http://apple.com;"
#define EXCOOKIE "cookie:blabla;"
#define EXUPLOAD "upload_root:download/"
#define EXHEADER "Content-Disposition:blabla;"
#define EXEXT ".php:/bin/php;"
#define EXFALLBACK "fallback:/index"

#define NOLINE 0

extern int              isDebug;
extern int              redirect_priority;
extern map<string, int> required;
extern int              error_parsing;
extern int              simple_quotes;
extern int              double_quotes;

int check_error_page(const string &content);
int check_root(const string &content);
int check_index(const string &content);
int check_upload_root(const string &content);
int good_extention(const string &content, int i);
int check_cgi_extensions(const string &content);
int check_auto_index(const string &content);
int good_header(const string &content, int i);
int check_add_header(const string &content);
int check_cookie(const string &content);
int check_body_max_size(const string &content);
int check_fallback(const string &content);

int	check_proxy_pass(const string &content);
int check_methods(const string &content);
int check_redirect(const string &content);
int check_location(const string &content);

int check_listen(const string &content);
int check_host_name(const string &content);
int check_server(const string &content);
int check_error(string &file);

int check_block(const string &content);
int check_content(const string &content);

int fill_auto_index(const string &file, Block *location);
int fill_cgi_extensions(const string &file, Block *location);
int fill_add_header(const string &file, Block *location);
int fill_body_max_size(const string &file, Block *location);
int fill_fallback(const string &file, Block *server);
int fill_cookie(const string &file, Block *server);
int fill_index(const string &file, Block *server);
int fill_upload_root(const string &file, Block *server);
int fill_root(const string &file, Block *server);
int fill_error_pages(const string &file, Block *server);

int fill_redirect(const string &file, LocationBlock *location);
int fill_methods(const string &file, LocationBlock *location);
int fill_proxy_pass(const string &file, LocationBlock *server);
int fill_location(const string &file, ServerBlock *server);

int     fill_port(const string &file, ServerBlock *server);
int     fill_host_name(const string &file, ServerBlock *server);
bool    fill_data(std::string file, Server *server);

int found_location_data(const string &file, LocationBlock *location);
int found_data(string const &file, ServerBlock *server);

int	check_no_backspace(const string &content);
int check_quotes();
int check_brackets(const string &content);

bool delete_for_fill(char c);
bool delete_for_parse(char c);
void delete_comment(string &file);

int line_tracker(const string &error, string initializer = "");
int error_message(int row, string message, string example);

void	set_required();
int     check_required();

void    erase_duplicates(vector<String> &host_names);
bool    same_host_names(vector<String> host_names1, vector<String> host_names2);
bool    same_data(ServerBlock block1, ServerBlock block2);
bool    check_different_server(Server *server);

int     endline(string line);
int     next_semicolon(string line);
int     next_coma(string line);
int     next_bracket(string line);

int     parsing(int argc, char **argv, Server *server);
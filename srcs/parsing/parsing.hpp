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

#define NOARG "One configuration file is requiered"
#define NOFILE "File not found"
#define MBRACK "bracket not valid (left open or extra closing bracket)"
#define MPORT "port not valid"
#define MHOST "host_names not valid"
#define MINDEX "index not valid"
#define MROOT "root not valid"
#define MERRPAGE "error_pages not valid"
#define MERRNUM "Error status in error_pages not valid (must be between 300 and 599))"
#define MSYNT "Syntax error in file"
#define MMETHOD "Unrecognized method"
#define MLOCFOL "Location path not found"
#define MDATA "Unrecognized identifier"
#define MAUTIND "auto_index not valid"
#define MEXT "cgi_extension not valid"
#define MHEAD "header not valid in add_headers"
#define MFALLBACK "fallback not valid"
#define MREDIRECT "Location redirection not valid"
#define MBODYSIZE "body_max_size not valid"
#define MPROXY "proxy_pass not valid"
#define MCOOKIE "cookie not valid"
#define MUPLOAD "upload_root not valid"
#define MQUOTES "Quote not valid (not closed)"
#define MNOSERV "Found no server directives"
#define MNOLISTEN "Found no listen directive"
#define MNOROOT "Found no root directive"
#define MERRPAGESYNT "error_pages not valid"
#define MSAMESERV "2 Servers (or more) have the same: IP address, port and hostname"

#define NOEX ""

#define EXPORT "listen: 0.0.0.0:3000;"
#define EXHOST "host_name: your_hostname;"
#define EXINDEX "index: /index.html; (absolute from root or relative)"
#define EXROOT "root: /var/www; (absolute or relative)"
#define EXERRPAGE "404: /404.html; (absolute from root or relative)"
#define EXMETHOD "methods: GET, POST, DELETE;"
#define EXAUTIND "auto_index: on/off;"
#define EXREDIRECT "redirect: 303, http://example.com;"
#define EXBODYSIZE "body_max_size: <size>MB/KB (or raw size in bytes);"
#define EXPROXY "proxy_pass: http://example.com;"
#define EXCOOKIE "cookie: <cookie_name>;"
#define EXUPLOAD "upload_root: /var/www/upload; (absolute or relative)"
#define EXHEADER "Content-Disposition: attachment;"
#define EXEXT ".php: /bin/php; (must be absolute)"
#define EXFALLBACK "fallback: /index.html; (absolute from root or relative)"

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
int fill_cgi_extensions(const string &finstructionsile, Block *location);
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
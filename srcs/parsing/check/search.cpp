#include "../parsing.hpp"

int check_block(const string &content)
{
	if (content[0] == '\n')
		return (1);
	if (!strncmp(content.c_str(), "root:", strlen("root:")))
		return (check_root(content));
	if (!strncmp(content.c_str(), "methods:", strlen("methods:")))
		return (check_methods(content));
	if (!strncmp(content.c_str(), "index:", strlen("index:")))
		return (check_index(content));
	if (!strncmp(content.c_str(), "auto_index:", strlen("auto_index:")))
		return (check_auto_index(content));
	if (!strncmp(content.c_str(), "cgi_extensions:", strlen("cgi_extensions:")))
		return (check_cgi_extensions(content));
	if (!strncmp(content.c_str(), "add_headers:", strlen("add_headers:")))
		return (check_add_header(content));
	if (!strncmp(content.c_str(), "body_max_size:", strlen("body_max_size:")))
		return (check_body_max_size(content));
	if (!strncmp(content.c_str(), "redirect:", strlen("redirect:")))
		return (check_redirect(content));
	if (!strncmp(content.c_str(), "fallback:", strlen("fallback:")))
		return (check_fallback(content));
	if (!strncmp(content.c_str(), "proxy_pass:", strlen("proxy_pass:")))
		return (check_proxy_pass(content));
	if (!strncmp(content.c_str(), "error_pages:", strlen("error_pages:")))
		return (check_error_page(content));
	if (!strncmp(content.c_str(), "cookie:", strlen("cookie:")))
		return (check_cookie(content));
	if (!strncmp(content.c_str(), "upload_root:", strlen("upload_root:")))
		return (check_upload_root(content));
	return (error_message(line_tracker(content), MDATA, NOEX));
}

int check_content(const string &content)
{
	if (content[0] == '\n')
		return (1);
	if (!strncmp(content.c_str(), "server", strlen("server")))
		return (check_server(content));
	if (!strncmp(content.c_str(), "listen:", strlen("listen:")))
		return (check_listen(content));
	if (!strncmp(content.c_str(), "root:", strlen("root:")))
		return (check_root(content));
	if (!strncmp(content.c_str(), "host_names:", strlen("host_names:")))
		return (check_host_name(content));
	if (!strncmp(content.c_str(), "index:", strlen("index:")))
		return (check_index(content));
	if (!strncmp(content.c_str(), "error_pages:", strlen("error_pages:")))
		return (check_error_page(content));
	if (!strncmp(content.c_str(), "location:", strlen("location:")))
		return (check_location(content));
	if (!strncmp(content.c_str(), "auto_index:", strlen("auto_index:")))
		return (check_auto_index(content));
	if (!strncmp(content.c_str(), "cgi_extensions:", strlen("cgi_extensions:")))
		return (check_cgi_extensions(content));
	if (!strncmp(content.c_str(), "body_max_size:", strlen("body_max_size:")))
		return (check_body_max_size(content));
	if (!strncmp(content.c_str(), "add_headers:", strlen("add_headers:")))
		return (check_add_header(content));
	if (!strncmp(content.c_str(), "cookie:", strlen("cookie:")))
		return (check_cookie(content));
	if (!strncmp(content.c_str(), "upload_root:", strlen("upload_root:")))
		return (check_upload_root(content));
	if (!strncmp(content.c_str(), "fallback:", strlen("fallback:")))
		return (check_fallback(content));
	return (error_message(line_tracker(content), MDATA, NOEX));
}
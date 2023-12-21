# AlmondServ - A Web Server in C++

AlmondServ is a lightweight web server made in C++98 style. It provides essential built-in functionalities, supporting the GET, POST and DELETE methods.

## Features
- **HTTP Methods:** Supports the core HTTP methods:
  - **GET:** Retrieve resources.
  - **POST:** Create resources with multi-part/form-data body.
  - **DELETE:** Delete resources.

- **CGI Execution:** Ability to execute CGI programs based on provided extensions and launch commands.

- **Proxy Pass:** Forward requests to another server, extending the server's capability to act as a proxy for various backend servers.

# Usage
Compile AlmondServ with the provided Makefile:
```bash
make
```

Run the server with the desired configuration file:
```bash
./AlmondServ [config_file]
```
Replace [config_file] with the path to your configuration file (AlmondServ must be run from the project's root).

# Configuration File
AlmondServ uses a configuration file to specify the servers's settings.
It features directives inheritance, meaning server blocks' directives can be passed down to location blocks or overitten by if redeclared.
```
server {
    listen: 127.0.0.1:8080;
    host_names: localhost, yourdomain.com;
    root: /path/to/your/web/root;

    location: / {
        index: /index.html;
    }

    # Additional location blocks...
}
# Additional server blocks...
```

## Proxy Pass
AlmondServ supports proxying any HTTP method.
To use it as a proxy server, configure the ```proxy_pass``` directive in your configuration file:
```
server {
    listen: 127.0.0.1:8080;

    location: /api {
        proxy_pass: http://some_server;
    }
}
```
Keep in mind that AlmondServ, being an HTTP server does not support HTTPS proxies.

## CGI Execution
Configure CGI execution by specifying the CGI extension and command in the ```cgi_extensions``` directive:
```
server {
    listen: 127.0.0.1:8080;

    location: /cgi-bin/ {
        cgi_extensions: {
            .py: /usr/bin/python3;
            .php: /bin/php-cgi;
        }
    }
}
```

# Directives
| Directive | Meaning | Syntax | Example |
| -------- | -------- | ------ | ------- |
| root | Root directory where your website files are located. | absolute or relative path | ```root: /some/dir;```
| upload_root | Root where uploaded files using the core webserv POST method will be stored. | absolute or relative path | ```uploaded_root: /some/dir;```
| listen | IP address and/or port for a server block. | [ip_address]:[port] | ```listen: 0.0.0.0:3000;```
| host_names | Host names your server block will respond to. | list of [host_name] | ```host_names: random.io;```
| location | Declaration of a location block. | [location_path] {...} | ```location: /some/path {index: index.html}```
| index | File to be maped by a request's url if it maps to a directory. | absolute or relative path | ```index: /index.html;``` 
| fallback | File or directory to be maped by a request's url if the requested resource doesn't exits. | absolute or relative path | ```fallback: /index.html;``` 
| methods | HTTP methods allowed for a specific path. | list of [valid_HTTP_method] | ```methods: GET, POST, PUT;```
| error_pages | Your error pages to send. | { list of [error_status]: [path/to/page]; } | ```error_pages: {404: 404.html}```
| auto_index | Listing of a directory if index file doesn't exist in it. | on/off | ```auto_index: on;```
| add_headers | Headers to add if client request was successfull. | { list of [field]: [value]; } | ```add_headers: {Content-Disposition: attachment;}```
| redirect | Client redirection for a specific path.  | absolute or local URI | ```redirect: </index.html>;```
| body_max_size | Client's body request maximum size in bytes. | [some_limit] (MB/KB/ or plain bytes) | ```body_max_size: 10MB;```
| proxy_pass | Absolute URI a specific location will proxy its requests to. | HTTP absolute URI | ```proxy_pass: http://localhost:5000;```

# Resource mapping
### Definition
The process of turning an HTTP request's path to an actual resource path on the server.

## Leading slash
A leading slash in a specified path will result in the location's path discarded for resource mapping.
Without it, the location's path will be included.
### Example
```location: /public/ { index: /index.html }```, with this request ```/public```, will result in ```/your/root + /index.html``` not ```/your/root + /public + /file.png```.

## Trailing slash
To include a location block's path in the resource's mapping, you must specify a trailing slash in the path.
To discard the location's path, leave the path as is. In this case the location's path will be discarded when mapping the request. 
### Example
```location: /public {...}```, with this request ```/public/file.png```, will result in ```/your/root + /file.png``` not ```/your/root + /public + /file.png```.

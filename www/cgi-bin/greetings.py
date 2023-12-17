import sys
import os
import cgi

# Parse query string parameters
form = cgi.FieldStorage()
username = form.getvalue("username")

# Print the Content-type header
print("Content-type: text/html\n")

variables = [
        'GATEWAY_INTERFACE',
        'SERVER_SOFTWARE',
        'SERVER_NAME',
        'SERVER_PROTOCOL',
        'SERVER_PORT',
        'REQUEST_METHOD',
        'REQUEST_URI',
        'QUERY_STRING',
        'SCRIPT_NAME',
        'SCRIPT_FILENAME',
        'PATH_INFO',
        'PATH_TRANSLATED',
        'REMOTE_ADDR',
        'REMOTE_HOST',
        'CONTENT_LENGTH',
        'CONTENT_TYPE',
        'HTTP_COOKIE',
        ]
envVariablesList = "<div style='display:flex;flex-direction:column;gap:5px;'>"

for var in variables:
    envVariablesList += f"<span>{var}: {os.getenv(var)}</span>"

envVariablesList += "</div>"


# Output the greeting
html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Greeting</title>
    <link rel="stylesheet" href="/styles.css" />
</head>
<body>
    <div class="wrapper">
      <div class="flex-col">
        <a href="/index.html">
          <img src="/icons/back.svg" alt="" />
          Home</a
        >
        <h2>Hello, {username}!</h2>
        {envVariablesList}
        <span style="color:gray;">Powered by Python</span>
      </div>
    </div>
</body>
</html>
"""

print(html_content)

# input_string = sys.stdin.read()
# print("Content-Type: text/plain");
# print("Content-Length: " + str(len(input_string)));
# print("")
# print(input_string)
# print("REQUEST_METHOD", os.getenv("REQUEST_METHOD"))
# print("SERVER_SOFTWARE", os.getenv("SERVER_SOFTWARE"))
# print("GATEWAY_INTERFACE", os.getenv("GATEWAY_INTERFACE"))
# print("SERVER_NAME", os.getenv("SERVER_NAME"))
# print("SERVER_PORT", os.getenv("SERVER_PORT"))
# print("SERVER_PROTOCOL", os.getenv("SERVER_PROTOCOL"))
# print("REMOTE_ADDR", os.getenv("REMOTE_ADDR"))
# print("HTTP_ACCEPT", os.getenv("HTTP_ACCEPT"))
# print("HTTP_USER_AGENT", os.getenv("HTTP_USER_AGENT"))
# print("HTTP_ACCEPT_LANGUAGE", os.getenv("HTTP_ACCEPT_LANGUAGE"))
# print("HTTP_COOKIE", os.getenv("HTTP_COOKIE"))
# print("HTTP_REFERER", os.getenv("HTTP_REFERER"))
# print("SCRIPT_NAME", os.getenv("SCRIPT_NAME"))
# print("PATH_INFO", os.getenv("PATH_INFO"))
# print("PATH_TRANSLATED", os.getenv("PATH_TRANSLATED"))
# print("QUERY_STRING", os.getenv("QUERY_STRING"))
# print("CONTENT_LENGTH", os.getenv("CONTENT_LENGTH"))
# print("CONTENT_TYPE", os.getenv("CONTENT_TYPE"))
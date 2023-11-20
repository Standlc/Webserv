import sys
import os

# print("Content-Type: text/plain\r\nTransfer-Encoding: chunked\r\n\r\n8\r\nMozilla \r\n11\r\nDeveloper Network\r\n0\r\n\r\n", end='')

os.write(2, str.encode("CGI: READING\n"))
input_string = sys.stdin.read()
os.write(2, str.encode("CGI: WRITTING\n"))
print("Content-Type: text/plain");
print("Content-Length: " + str(len(input_string)));
print("")
print("REQUEST_METHOD", os.getenv("REQUEST_METHOD"))
print("SERVER_SOFTWARE", os.getenv("SERVER_SOFTWARE"))
print("GATEWAY_INTERFACE", os.getenv("GATEWAY_INTERFACE"))
print("SERVER_NAME", os.getenv("SERVER_NAME"))
print("SERVER_PORT", os.getenv("SERVER_PORT"))
print("SERVER_PROTOCOL", os.getenv("SERVER_PROTOCOL"))
print("REMOTE_ADDR", os.getenv("REMOTE_ADDR"))
print("HTTP_ACCEPT", os.getenv("HTTP_ACCEPT"))
print("HTTP_USER_AGENT", os.getenv("HTTP_USER_AGENT"))
print("HTTP_ACCEPT_LANGUAGE", os.getenv("HTTP_ACCEPT_LANGUAGE"))
print("HTTP_COOKIE", os.getenv("HTTP_COOKIE"))
print("HTTP_REFERER", os.getenv("HTTP_REFERER"))
print("SCRIPT_NAME", os.getenv("SCRIPT_NAME"))
print("PATH_INFO", os.getenv("PATH_INFO"))
print("PATH_TRANSLATED", os.getenv("PATH_TRANSLATED"))
print("QUERY_STRING", os.getenv("QUERY_STRING"))
print("CONTENT_LENGTH", os.getenv("CONTENT_LENGTH"))
print("CONTENT_TYPE", os.getenv("CONTENT_TYPE"))
print(input_string)
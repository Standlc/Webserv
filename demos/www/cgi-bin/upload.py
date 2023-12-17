import cgi
from fileinput import filename
import os

# Specify the directory where uploaded files will be saved
upload_directory = "../public/"

# Create the upload directory if it doesn't exist

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

os.makedirs(upload_directory, exist_ok=True)

# Print the Content-type header
print("Content-type: text/html\n")

# Get the uploaded file data
form = cgi.FieldStorage()
uploaded_file = form['file']

# Check if a file was uploaded
if uploaded_file.filename:
    # Construct the full path to save the file
    save_path = os.path.join(upload_directory, os.path.basename(uploaded_file.filename))

    # Save the file
    with open(save_path, 'wb') as file:
        file.write(uploaded_file.file.read())

    print(f"""
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
                <h2>File uploaded successfully</h2>
                {envVariablesList}
                <a href="{save_path}">{uploaded_file.filename}</a>
                <span style="color:gray;">Powered by Python</span>
            </div>
            </div>
        </body>
        </html>
        """)
else:
    print("Please select a file to upload.")
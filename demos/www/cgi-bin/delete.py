import cgi
import os

# Specify the directory where files are stored
files_directory = "../public/"

# Print the Content-type header
print("Content-type: text/html\n")

# Parse query string parameters
form = cgi.FieldStorage()
file_to_delete = form.getvalue("fileToDelete")

# Check if a filename was provided
if file_to_delete:
    # Construct the full path to the file
    file_path = os.path.join(files_directory, file_to_delete)

    # Check if the file exists before attempting to delete
    if os.path.exists(file_path):
        # Attempt to delete the file
        try:
            os.remove(file_path)
            print(print(f"""
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
                        <h2>File deleted successfully</h2>
                        <span style='color:red;'>{file_to_delete}</span>
                        <a href="/public">Uploaded files</a>
                        <span style="color:gray;">Powered by Python</span>
                    </div>
                    </div>
                </body>
                </html>
                """))
        except Exception as e:
            print(f"Error deleting file: {e}")
    else:
        print(f"File not found: {file_to_delete}")
else:
    print("Please provide a filename to delete.")
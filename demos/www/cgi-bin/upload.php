<?php

// Specify the directory where uploaded files will be saved
$uploadDirectory = '../public/';

// Check if the form was submitted
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Check if a file was uploaded
    if (isset($_FILES['file'])) {
        $file = $_FILES['file'];

        // Check for errors during file upload
        if ($file['error'] === UPLOAD_ERR_OK) {
            // Move the uploaded file to the specified directory
            $uploadedFilePath = $uploadDirectory . basename($file['name']);
            move_uploaded_file($file['tmp_name'], $uploadedFilePath);

            echo <<<HTML
            <html lang="en">
                <head>
                    <link rel="stylesheet" href="/styles.css">
                    <title>File Upload Form</title>
                </head>
                <body>
                    <div class="wrapper">
                        <div class="flex-col">
                            <a href="/index.html">
                                <img src="/icons/back.svg" alt="" />
                                Home
                            </a>
                            <h2>File was uploaded</h2>
            HTML;
                            $link = "/public/" . basename($file['name']);
                            echo "<a href=";echo $link;echo ">";echo basename($file['name']);echo "</a>";
                            $cgiVariables = array(
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
                            );
                            echo "<div style='display:flex;flex-direction:column;gap:5px;'>";
                            // Print the values of the specified CGI environment variables
                            foreach ($cgiVariables as $variable) {
                                echo "<span>";echo "$variable: " . (isset($_SERVER[$variable]) ? $_SERVER[$variable] : '');echo "</span>";
                            }
                            echo "</div>";
                            echo <<<HTML
                            <span style="color:gray;">Powered by PHP/8.2.12</span>
                        </div>
                    </div>
                </body>
            </html>
            HTML;
        } else {
            // Display an error message
            echo "Error uploading file. Code: {$file['error']}";
        }
    } else {
        // Display an error message if no file was uploaded
        echo "Please select a file to upload.";
    }
} else {
    // Display the HTML form for file upload
    echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <link rel="stylesheet" href="/styles.css">
    <title>File Upload Form</title>
</head>
<body>
    <div class="wrapper">
        <form action="upload.php" method="post" enctype="multipart/form-data">
            <div class="flex-col">
                <a href="/">
                    <img src="/icons/back.svg" alt="" />
                    Home
                </a>
                <h1 for="file">Select a file to upload</h1>
                <input type="file" id="file" name="file" required>
                <button type="submit">Upload</button>
            </div>
        </form>
    </div>
</body>
</html>
HTML;
}

?>
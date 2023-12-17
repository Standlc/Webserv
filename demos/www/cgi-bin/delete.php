<?php

// Specify the directory where files are stored
$filesDirectory = '../public/';

// Check if the form was submitted
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Check if a file was selected for deletion
    if (isset($_POST['fileToDelete'])) {
        $fileToDelete = $_POST['fileToDelete'];

        // Construct the full path to the file
        $filePath = $filesDirectory . $fileToDelete;

        // Check if the file exists before attempting to delete
        if (file_exists($filePath)) {
            // Attempt to delete the file
            if (unlink($filePath)) {
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
                                <h2>File was deleted</h2>
            HTML;
                                echo "<span style='color:red;'>";echo $fileToDelete;echo "</span>";
                                echo <<<HTML
                                <a href="/public">Uploaded files</a>
                                <span style="color:gray;">Powered by PHP/8.2.12</span>
                            </div>
                        </div>
                    </body>
                </html>
                HTML;
            } else {
                echo "Error deleting file: $fileToDelete";
            }
        } else {
            echo "File not found: $fileToDelete";
        }
    } else {
        echo "Please select a file to delete.";
    }
} else {
    // Display the HTML form for selecting and deleting a file
    $files = scandir($filesDirectory);
    $action = htmlspecialchars($_GET['action']);

    echo <<<HTML
<html lang="en">
<head>
    <link rel="stylesheet" href="/styles.css">
    <title>File Upload Form</title>
</head>
<body>
    <div class="wrapper">
        <form action=$action method="post">
            <div class="flex-col">
                <a href="/">
                    <img src="/icons/back.svg" alt="" />
                    Home
                </a>
                <h1 for="file">Select a file to delete</h1>
                <select id="fileToDelete" name="fileToDelete" required>
HTML;

                foreach ($files as $file) {
                    if ($file != '.' && $file != '..') {
                        echo "<option value=\"$file\">$file</option>";
                    }
                }
                echo <<<HTML
                    </select>
                    <button type="submit">Delete</button>
            </div>
        </form>
    </div>
</body>
</html>
HTML;
}

?>
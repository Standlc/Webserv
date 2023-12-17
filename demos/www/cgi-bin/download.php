#!/usr/bin/php-cgi
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
                <h1 for="file">Click on a file to download</h1>
                <div style="display:flex;flex-wrap: wrap;gap:5px;">
                    <?php
                    $directory = '../public/';
                    if ($handle = opendir($directory)) {
                        while (false !== ($file = readdir($handle))) {
                            if ($file != "." && $file != "..") {
                                echo '<a href="/public/download/' . $file . '">' . $file . '</a><br>';
                            }
                        }
                        closedir($handle);
                    }
                    ?>
                </div>
        </div>
    </div>
  </body>
</html>
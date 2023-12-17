#!/opt/homebrew/bin/php-cgi
<?php

// Read the incoming POST data
$postData = file_get_contents('php://input');

// Parse the POST data (assuming it's URL-encoded)
parse_str($postData, $parsedData);

// Set the Content-Type header to text/plain
header('Content-Type: text/plain');
// header("Transfer-Encoding: chunked");
// header('Content-Length: text/plain');

// Output the parsed data
echo "Received POST data:\n";
print_r($parsedData);

?>
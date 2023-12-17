<HTML>
    <LINK rel="stylesheet" href="/styles.css"></LINK>

    <DIV>
        <div class="wrapper">
            <DIV class="flex-col">
                    <?php
                        if (isset($_GET['username'])) {
                            $username = htmlspecialchars($_GET['username']); // Sanitize the input to prevent XSS
                            echo "<h2>Hello, $username!</h2>";
                            
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
                        } else {
                            echo "Please provide a username in the query string.";
                        }
                    ?>
                <span style="color:gray;">Powered by PHP/8.2.12</span>
            </DIV>
        </div>
    </DIV>
</HTML>
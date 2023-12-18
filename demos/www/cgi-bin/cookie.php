<html>
  <head>
    <title>Webserv</title>
    <link rel="stylesheet" href="/styles.css" />
  </head>
  <body>
    <div class="wrapper">
      <div class="flex-col">
        <?php
            if (isset($_COOKIE['username']) && $_COOKIE['username'] != "") {
                $username = $_COOKIE['username'];
                echo "<h2>Welcome back $username!</h2>";
            } else {
                echo "<h2>Awe you didn't get a cookie?</h2>";
            }
        ?>
      </div>
    </div>
  </body>
</html>
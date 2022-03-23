<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<title>FELIX KING</title>
</head>
<body>
	<h1>Hello, <?php echo $_GET['name'];?></h1>
	<h1>Hello, <?php echo $_GET['age'];?></h1>
</body>
</html>

./php-cgi -f echo.php name=felix age=1
/Users/fharing/42/webserv/server/cgi/php-cgi -f echo.php name=felix age=18 > out
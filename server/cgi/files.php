<!DOCTYPE html>
<html>
	<head>
		<title>Webserv</title>
		<style>
			body {
				margin: 0;
				padding: 0;
				text-align: center;
				/* background-image: url(./img/compbg-min.jpg); */
				background-size: cover;
				background-attachment: fixed;
				background-position: center center;
				background-repeat: no-repeat;
				width: 100%;
				height: 100%;
				font-family: 'Segoe UI', Tahoma, sans-serif;
			}

			.container {
				display: flex;
				flex-direction: column;
				align-items: center;
				position: absolute;
				top: 50%;
				left: 50%;
				transform: translate(-50%, -50%);
				background-color: black;
				box-shadow: 0px 5px 40px 0px #3A3B3C;
				border: 1px solid #FFFFFF;
				color: white;
				min-width: 500px;
				padding: 24px;
			}

			.line {
				color: #FFFFFF;
				width: 80%;
				height: 12px;
				border-bottom: 1px solid white;
				margin-bottom: 30px;
			}

			/* navbar start */
			.navbar {
				color: #FFFFFF;
				border-bottom: 1px solid white;
				position: relative;
			}

			.navbar ul {
				list-style-type: none;
				margin: 0;
				padding: 0;
				overflow: hidden;
				background-color: #000000;
			}

			.navbar li {
				display: inline;
			}

			.navbar a {
				display: block;
				background-color: #000000;
				color: #FFFFFF;
				text-align: center;
				font-size: 20px;
				text-decoration: none;
				padding: 20px 32px;
			}

			.navbar a:hover {
				color: #00e600;
			}
			/* navbar end */

			/* team start */

			.files a {
				text-decoration: none;
				color: #FFFFFF;
			}
			
			.files a:hover {
				color: #00e600;
			}

			/* team end */
		</style>
	</head>
	<body>
		<div class="navbar">
			<ul>
				<li><a href="" style="float: left;">web7000</a></li>
				<li><a href="" style="float: right;">Team</a></li>
				<li><a href="" style="float: right;">Files</a></li>
			</ul>
		</div>
		<div class="container">
			<!-- <img src="./img/42logo.png" alt=""> -->
			<!-- <div class="line"></div> -->
			<div class="files">
				<p><?php 
					$fileList = glob('./*');
					foreach($fileList as $filename){
						if(is_file($filename)){
							// echo '<a href="',$filename,'" >',$filename,'</a><br>'; 
							echo '<a href="" >',$filename,'</a><br>'; 
						}
					}
				?></p>
			</div>
		</div>
	</body>
</html>
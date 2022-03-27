<?php

// Initialize URL to the variable
$url = 'https://www.geeksforgeeks.org/register?name=Amit&email=amit1998@gmail.com';

// Use parse_url() function to parse the URL
// and return an associative array which
// contains its various components
$url_components = parse_url($url);

// Use parse_str() function to parse the
// string passed via URL
parse_str($url_components['query'], $params);

// Display result
echo 'Hi '.$params['name'].' your emailID is '.$params['email'];

?>


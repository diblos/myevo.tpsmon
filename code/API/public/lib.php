<?php
define("valTRUE",1);
define("valFALSE",0);
define("RECORD_LIMIT",50);

// date_default_timezone_set("UTC");

function isUpdateConfig($arr)
{
    return valFALSE;
}

function login($username, $password) 
{
	$db = connect_db();
    $sql = "SELECT name FROM tbl_user WHERE username='".$username."' AND password=sha('".$password."');";
    $result = $db->query($sql);
	$db->close();

	if ($result->num_rows > 0) {
	 
	 	$row = $result->fetch_assoc();
   		$arrRtn['user'] = $row["name"]; //Just return the user name for reference
        $arrRtn['token'] = bin2hex(openssl_random_pseudo_bytes(16)); //generate a random token

        $tokenExpiration = date('Y-m-d H:i:s', strtotime('+12 hour'));//the expiration date will be in one hour from the current moment

        updateToken($username, $arrRtn['token'], $tokenExpiration); //This function can update the token on the database and set the expiration date-time, implement your own
        // return json_encode($arrRtn);

	    return $arrRtn;
	} else {
	    return false;
	}

}

function updateToken($uid,$token,$expire)
{
    archiveToken($uid);
	$db = connect_db();
    $sql = "UPDATE tbl_user SET token='".$token."', token_expire='".$expire."' WHERE username='".$uid."';";

    $result = $db->query($sql);
	$db->close();
    
    if($result){    
        return 0;
    }else{
     	return 1;
    }
}

function archiveToken($uid)
{
    $db = connect_db();
    $sql = "INSERT INTO tbl_token_audit (username,token,token_expire) SELECT username,token,token_expire FROM tbl_user WHERE username = '".$uid."';";

    $result = $db->query($sql);
    $db->close();
    
    if($result){    
        return 0;
    }else{
        return 1;
    }
}

function checkToken($token) 
{
	$db = connect_db();
    $sql = "SELECT id FROM tbl_user WHERE token='".$token."' AND token_expire > now();";
    // echo($sql);
    $result = $db->query($sql);
	$db->close();

	if ($result->num_rows > 0) {
		$row = $result->fetch_assoc();
	    return $row["id"];
	} else {
	    return false;
	}
}

function Accesslog($token,$route,$status)
{
    $origin = get_client_ip_server();
    $method = $_SERVER['REQUEST_METHOD'];
	$db = connect_db();

    if($status==='EXPIRED'){
        $sql = "INSERT INTO tbl_user_access (origin,username,token,route,method,status) VALUES ('$origin',IFNULL((SELECT username from tbl_token_audit WHERE token='$token'),''),'$token','$route','$method','$status')";
    }else{
        $sql = "INSERT INTO tbl_user_access (origin,username,token,route,method,status) VALUES ('$origin',IFNULL((SELECT username from tbl_user WHERE token='$token'),''),'$token','$route','$method','$status')";    
    }   

    $result = $db->query($sql);
	$db->close();
    
    if($result){    
        return 0;
    }else{
     	return 1;
    }
}

// Function to get the client ip address
function get_client_ip_server() {
    $ipaddress = '';
    if ($_SERVER['HTTP_CLIENT_IP'])
        $ipaddress = $_SERVER['HTTP_CLIENT_IP'];
    else if($_SERVER['HTTP_X_FORWARDED_FOR'])
        $ipaddress = $_SERVER['HTTP_X_FORWARDED_FOR'];
    else if($_SERVER['HTTP_X_FORWARDED'])
        $ipaddress = $_SERVER['HTTP_X_FORWARDED'];
    else if($_SERVER['HTTP_FORWARDED_FOR'])
        $ipaddress = $_SERVER['HTTP_FORWARDED_FOR'];
    else if($_SERVER['HTTP_FORWARDED'])
        $ipaddress = $_SERVER['HTTP_FORWARDED'];
    else if($_SERVER['REMOTE_ADDR'])
        $ipaddress = $_SERVER['REMOTE_ADDR'];
    else
        $ipaddress = 'UNKNOWN';
 
    return $ipaddress;
}
?>

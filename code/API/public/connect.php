<?php

function connect_db() {
	$server="localhost"; // Host name
	$user="mydev"; // Mysql username
	$pass="123@qwe"; // Mysql password
	$database="tps"; // Database name
	$connection = new mysqli($server, $user, $pass, $database);

	return $connection;
}

function execSQL($sql, $params, $close){
		$mysqli = connect_db();           
       	$stmt = $mysqli->prepare($sql) or die ("Failed to prepared the statement!");           
		call_user_func_array(array($stmt, 'bind_param'), refValues($params));           
        $stmt->execute();           
		if($close){
		   $result = $mysqli->affected_rows;
		}else{
		   $meta = $stmt->result_metadata();
			while ( $field = $meta->fetch_field() ) {
			   $parameters[] = &$row[$field->name];
			}
			call_user_func_array(array($stmt, 'bind_result'), refValues($parameters));		   
			while ( $stmt->fetch() ) {  
			   $x = array();  
			   foreach( $row as $key => $val ) {  
			      $x[$key] = $val;  
			   }  
			   $results[] = $x;  
			}

			if(isset($results)){$result = $results;}else{$result = [];}	
			// if(isset($results)){$result = $results;}else{$result = array(0);}	

			// echo ("xxxxxxxxxxxxx");

		}           
           $stmt->close();
           $mysqli->close();
           return  $result;
}

function querySQL($sql){
		$mysqli = connect_db();           
       	$stmt = $mysqli->prepare($sql) or die ("Failed to prepared the statement!");           
        $stmt->execute();           
		
		   $meta = $stmt->result_metadata();
			while ( $field = $meta->fetch_field() ) {
			   $parameters[] = &$row[$field->name];
			}
			call_user_func_array(array($stmt, 'bind_result'), refValues($parameters));	
			while ( $stmt->fetch() ) {  
			   $x = array();  
			   foreach( $row as $key => $val ) {  
			      $x[$key] = $val;  
			   }  
			   $results[] = $x;  
			}

			if(isset($results)){$result = $results;}else{$result = [];}	
			// if(isset($results)){$result = $results;}else{$result = array(0);}	

			// echo ("xxxxxxxxxxxxx");

		     
           $stmt->close();
           $mysqli->close();
           return  $result;
}
   
function refValues($arr){
    if (strnatcmp(phpversion(),'5.3') >= 0) //Reference is required for PHP 5.3+
    {
        $refs = array();
        foreach($arr as $key => $value)
            $refs[$key] = &$arr[$key];
        return $refs;
    }
    return $arr;
}
?>

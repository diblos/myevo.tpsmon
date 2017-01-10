<?php
define('SERVER_ADDRESS', '127.0.0.1');
define('SERVER_PORT', 57000);
define('dTRUE', 1);
define('dFALSE', 0);
define('DRIFT_SECONDS',60);
define('SOCKET_RESET_INTERVAL',3600);
define('SOCKET_READ_TIMEOUT',30);

ini_set('error_reporting', E_ERROR);
ini_set('date.timezone', 'Asia/Kuala_Lumpur');
// ini_set('date.timezone', 'UTC');

// define('TAGS', array('DEV','DTM','EVT','AON','BON','TMP','TAG','STT','STP'));
$EVENTS = array('900'=>'No Event','901'=>'No Power','902'=>'No Battery','903'=>'Low Battery','904'=>'Tampered Case');

// require('public/connect.php');// DEV HERE
require('../connect.php');// LIVE HERE

function log_incoming($msg,$origin){

		$db = connect_db();
		$query=mysqli_query($db,"INSERT INTO post_log (origin,content) VALUES ('$origin' , '$msg');");
		$db->close();

}

function event_lookup($eventCode){
	global $EVENTS;
	try {
		return $EVENTS[$eventCode];
	} catch (Exception $e) {
		echo "Err: ".$e->getMessage();
	    echo ', Event Code '.$eventCode." not available.".PHP_EOL;
	    return 'Unknown';
	}
}

function get_config($deviceid){

   	$data = execSQL("SELECT UNIX_TIMESTAMP(hb_time) AS HBT,hb_duration AS HBD,'' AS SNC,IF(update_hb_eperiod='N' ,0, 1) AS HBU FROM device_configuration WHERE deviceid=? LIMIT 1;", array('i',$deviceid), false);

    // 
   	if(!isset($data)){ return false; };
    
    // echo(json_encode($r).PHP_EOL);
    update_config_flag($deviceid);
    
    // CFG:4{s:3:HBT;i:1476147600;s:3:HBD;i:1454;s:3:SNC;i:1483972940;s:3:HBU;i:1;}
    $str='CFG:4{s:3:HBT;i:' . $data[0]["HBT"] . ';s:3:HBD;i:' . $data[0]["HBD"] .';s:3:SNC;i:'. time() . ';s:3:HBU;i:'. $data[0]["HBU"] .';}';
    return $str;
    
}

function update_config($deviceid,$drift){

   	// $data = execSQL("SELECT DATEDIFF(last_updated,last_conf_request) AS dtime FROM device_configuration WHERE deviceid=? LIMIT 1;", array('i',$deviceid), false);
	
	$data = querySQL("SELECT DATEDIFF(last_updated,last_conf_request) AS dtime,UNIX_TIMESTAMP(now())-$drift AS drift FROM device_configuration WHERE deviceid='$deviceid' LIMIT 1;");

   	$r = new stdClass();
    if(!isset($data)){
        $r->device = $deviceid;
        $r->UPD = dFALSE;
    }else{
        $r->device = $deviceid;
        if(!$data){
			$r->UPD = dFALSE;
        }else{
        	if( (int) $data[0]['dtime']>0){
        		$r->UPD = dTRUE;
        		echo "Force device to Update : new update=".$data[0]['dtime'].PHP_EOL;	
        	}else{
        		if(( (int) $data[0]['drift'] < DRIFT_SECONDS)&&( (int) $data[0]['drift'] > (DRIFT_SECONDS*-1)) ){ // Device Time is different from server
        			$r->UPD = dFALSE;
        		}else{
					$r->UPD = dTRUE;
					echo "Force device to Update : drift=".$data[0]['drift'].PHP_EOL;
        		}        		
        	}        	
        }
        $r->data->diff=$data[0]['dtime'];
    }

    
    $str = "ATT:2{s:3:UPD;i:" . $r->UPD . ";}";
    //echo "my string is ($str)".PHP_EOL;

    return $str;
}

function update_config_flag($deviceid){
   	execSQL("UPDATE device_configuration SET last_conf_request=NOW() WHERE deviceid=? LIMIT 1;", array('i',$deviceid), true);
}

function encode_message($data){

}

function decode_message($input_msg){

}

function getCommand($input){
	return strleft($input,3);
}

function strleft($leftstring, $length) {
  return(substr($leftstring, 0, $length));
}

function strright($rightstring, $length) {
  return(substr($rightstring, -$length));
}

function decodeMessage($input){
   // todo manage empty input

	$str='';
	$a = array();
	for ($i = 0; $i <= strlen($input); $i++) {
		$tmp = substr($input,$i,1);
	    // echo $tmp.PHP_EOL ;
	    if(($tmp=="{")||($tmp==":")||($tmp==";")||($tmp=="}")){
	    	// echo $str.PHP_EOL;
	    	array_push($a,$str);
	    	$str='';
	    }else{
	    	$str = $str.$tmp;
	    }
	}

	if(isset($a)){

		switch ($a[0]) {
		    case 'CFG':
		    	$deviceId='';
		    	$deviceTime=0;
		    	$item=$a[1];
		    	for($i=2;$i<sizeof($a);$i++) {
		    		switch ($a[$i]){
		    			case 's':
		    				if($item % 2 ==0){
		    					$curTag = $a[$i+2];
		    				}else{
		    					if($curTag=='DEV') $deviceId = $a[$i+2];
		    				}
		    				// echo $a[$i].' value = '.$a[$i+2].PHP_EOL;
		    				$item=$item-1;
		    				break;
		    			case 'i':
		    				if($item % 2 !=0){
		    					if($curTag=='DTM') $deviceTime = $a[$i+1];
		    				}
		    				// echo 'i value = '.$a[$i+1].PHP_EOL;
		    				$item=$item-1;
		    				break;
		    			default:
		    		}
		    		
		    		// echo $curTag;
				}

				echo 'deviceId: '.$deviceId.PHP_EOL;
				echo 'deviceTime: '.$deviceTime.PHP_EOL;

				// return get_config($deviceId);
				$return = get_config($deviceId);
				return ($return) ? $return : '1';
		        break;
		    case 'STS':
		    	
		    	$deviceId  = '';
		    	$eventCode = '';
		    	$ac_on     = 0;
		    	$batt_on   = 0;
		    	$tamper_on = 0;
		    	$deviceTime= 0;
		    	$item=$a[1];
		    	for($i=2;$i<sizeof($a);$i++) {
		    		switch ($a[$i]){
		    			case 's':
		    				if($item % 2 ==0){
		    					$curTag = $a[$i+2];
		    				}else{
		    					if($curTag=='DEV') $deviceId = $a[$i+2];
		    					if($curTag=='EVT') $eventCode = $a[$i+2];
		    				}
		    				// echo $a[$i].' value = '.$a[$i+2].PHP_EOL;
		    				$item=$item-1;
		    				break;
		    			case 'i':
		    				if($item % 2 !=0){
		    					if($curTag=='AON') $ac_on = $a[$i+1];
		    					if($curTag=='BON') $batt_on = $a[$i+1];
		    					if($curTag=='TMP') $tamper_on = $a[$i+1];
		    					if($curTag=='DTM') $deviceTime = $a[$i+1];
		    				}
		    				// echo 'i value = '.$a[$i+1].PHP_EOL;
		    				$item=$item-1;
		    				break;
		    			default:
		    		}
		    		
		    		// echo $curTag;
				}

				echo 'deviceId: '.$deviceId.PHP_EOL;
				echo 'eventCode: '.$eventCode.PHP_EOL;
				echo 'ac_on: '.$ac_on.PHP_EOL;
				echo 'batt_on: '.$batt_on.PHP_EOL;
				echo 'tamper_on: '.$tamper_on.PHP_EOL;
				echo 'deviceTime: '.$deviceTime.PHP_EOL;
				$eventdesc = event_lookup($eventCode);

				try {

					$db = connect_db();
					$query=mysqli_query($db,"INSERT INTO device_status_log (deviceid,ac_on,batt_on,tamper,dev_timestamp) VALUES ('$deviceId' , '$ac_on', '$batt_on', '$tamper_on', from_unixtime($deviceTime));");
					$query=mysqli_query($db,"INSERT INTO device_event_log (deviceid,eventid,eventdesc,dev_timestamp) VALUES ('$deviceId' , '$eventCode', '$eventdesc',from_unixtime($deviceTime));");
					$db->close();

				} catch (Exception $e) {
				    echo 'Caught exception: ',  $e->getMessage(), PHP_EOL;
				}

				return 'STS:0';
		        break;

		    case 'ATT':
		    	
		    	$deviceId  = '';
		    	$tagID = '';
		    	$deviceTime = 0;
		    	$HBStart = 0;
		    	$HBStop = 0;
		    	$item=$a[1];
		    	for($i=2;$i<sizeof($a);$i++) {
		    		switch ($a[$i]){
		    			case 's':
		    				if($item % 2 ==0){
		    					$curTag = $a[$i+2];
		    				}else{
		    					if($curTag=='DEV') $deviceId = $a[$i+2];
		    					if($curTag=='TAG') $tagID = $a[$i+2];
		    				}
		    				// echo $a[$i].' value = '.$a[$i+2].PHP_EOL;
		    				$item=$item-1;
		    				break;
		    			case 'i':
		    				if($item % 2 !=0){
		    					if($curTag=='DTM') $deviceTime = $a[$i+1];
		    					if($curTag=='STT') $HBStart = $a[$i+1];
		    					if($curTag=='STP') $HBStop = $a[$i+1];
		    				}
		    				// echo 'i value = '.$a[$i+1].PHP_EOL;
		    				$item=$item-1;
		    				break;
		    			default:
		    		}
		    		// echo $curTag.' '.$item.PHP_EOL;
		    		
				}

				echo 'deviceId: '.$deviceId.PHP_EOL;
				echo 'tagID: '.$tagID.PHP_EOL;
				echo 'deviceTime: '.$deviceTime.PHP_EOL;
				echo 'HBStart: '.$HBStart.PHP_EOL;
				echo 'HBStop: '.$HBStop.PHP_EOL;	

				$db = connect_db();
				$query=mysqli_query($db,"INSERT INTO time_attend_log (deviceid,cardid,dev_timestamp,prev_hb,next_hb) VALUES ('$deviceId' , '$tagID', from_unixtime($deviceTime),from_unixtime($HBStart),from_unixtime($HBStop));");
				$db->close();

				return update_config($deviceId,$deviceTime);
		        break;
		   	default:
		   		echo 'ERR:Invalid data'.PHP_EOL;
		   		return '1';
		}
	}
}

function connection_on(&$sock)
{
	// Set the ip and port we will listen on
	$address = SERVER_ADDRESS;
	$port = SERVER_PORT;
	// Create a TCP Stream socket
	$sock = socket_create(AF_INET, SOCK_STREAM, 0); // 0 for  SQL_TCP
	// Set socket option to reuse same port, avoiding system waiting
	if(!socket_set_option($sock, SOL_SOCKET, SO_REUSEADDR, 1)){echo 'Unable to set SO_REUSEADDR option on socket: '.socket_strerror(socket_last_error()).PHP_EOL;};
	// Set read timeout to the socket - to avoid server - client wait
	if(!socket_set_option($sock,SOL_SOCKET, SO_RCVTIMEO, array("sec"=>SOCKET_READ_TIMEOUT, "usec"=>0))){echo 'Unable to set SO_RCVTIMEO option on socket: '.socket_strerror(socket_last_error()).PHP_EOL;};
	// Bind the socket to an address/port
	socket_bind($sock, 0, $port) or die('Could not bind to address');  // 0 for localhost
	// Start listening for connections
	socket_listen($sock);
	echo 'listening on port '.$port.PHP_EOL;
}

function connection_off(&$sock)
{
	// Close the master sockets
	socket_close($sock);
	echo 'port closed.'.PHP_EOL;
}

function getNextDT($currentDT,$startDT,$period){
	try {
		return ($currentDT -(($currentDT-$startDT) % $period) + $period);
	} catch (Exception $e) {
	    return -9999;
	}
}
/********Socket Server*********************/
set_time_limit (0);
// set_time_limit (10);
$startTime = time();

connect:connection_on($sock);
$endTime = getNextDT(time(),$startTime,SOCKET_RESET_INTERVAL);

//loop and listen
while (true) {
   	
	if(time() > $endTime)goto disconnect;
	$endTime = getNextDT(time(),$startTime,SOCKET_RESET_INTERVAL);
	
	/* Accept incoming  requests and handle them as child processes */
	$client =  socket_accept($sock);
	if ($client === false) { continue; };

	echo "--New connection from " . serialize($client) . PHP_EOL;
	// Read the input  from the client – 1024000 bytes

	$input =  socket_read($client, 1024000);


	


	if($input!==false && !empty($input)){
		echo "-----------------------------------------".PHP_EOL;
		// Strip all white  spaces from input
		// $output =  ereg_replace("[ \t\n\r]","",$input)."\0";
		// $output =  preg_replace("[ \t\n\r]","",$input)."\0";
		// ===============================================================
		socket_getpeername($client, $a, $p);

		echo $a.' << '.$input.PHP_EOL;

		log_incoming($input,"$a:$p");//LOG
		$cmd = getCommand($input);
		if($cmd){
			$str = decodeMessage($input);
			$response = $str;			
		}else{
			$response = '1';	
		}
		echo SERVER_ADDRESS.' >> '.$response.PHP_EOL;
		// Display output  back to client
		socket_write($client, $response);
	}else{
		echo "wrong input: ".serialize($input).PHP_EOL;
	}
	
	echo "closing".PHP_EOL;
	socket_close($client);
	// $input = null;
	// $output = null;
	// $cmd = null;
	// $str = null;
	// $response = null;
	// $client = null;

}
// ===============================================================
disconnect:
connection_off($sock);
// sleep(3);
goto connect;

?>
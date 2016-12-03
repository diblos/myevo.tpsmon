<?php
// Routes
$app->get('/admin', function ($request, $response, $args) {
    // Sample log message
    // $this->logger->info("Slim-Skeleton '/admin' route");
    // Render index view
    return $this->renderer->render($response, 'admin-attendance.phtml', $args);
});

$app->get('/admin/apicall', function ($request, $response, $args) {
    // Sample log message
    //$this->logger->info("Slim-Skeleton '/admin/tpsapi' route");
    // Render index view
    return $this->renderer->render($response, 'admin-api-access.phtml', $args);
});

$app->get('/admin/apilist/[{limit}]', function ($request, $response, $args) {
    // Sample log message
    $this->logger->info("Slim-Skeleton '/admin/list/[{limit}]' route");

    $limit = $request->getAttribute('limit');
    $r = new stdClass();
    try {
            $data = execSQL("SELECT origin,username,token,route,method,status,svr_timestamp FROM tbl_user_access ORDER BY ID DESC LIMIT ?;", array('i',$limit), false);
            if(!isset($data)){
                $r->result = 'fail';
                $r->description = 'No Device exists'; 
                $newResponse = $response
                ->withJson($r,204);               
            }else{
                $r->result = 'success';
                $r->data=$data;
                //$r->user=$user;
                $newResponse = $response->withHeader('Access-Control-Allow-Origin','*')
                ->withJson($r,200);
            }

    } catch (Exception $e) {
        $r->result = 'fail';
        $r->description = 'Internal server error';
        $newResponse = $response
        ->withJson($r,500);  
        
    }
    // echo(json_encode($r));  
    
    return $newResponse;

});

$app->get('/admin/list/[{limit}]', function ($request, $response, $args) {
    // Sample log message
    $this->logger->info("Slim-Skeleton '/admin/list/[{limit}]' route");

    $limit = $request->getAttribute('limit');
    $r = new stdClass();
    try {
            $data = execSQL("SELECT deviceid,cardid,dev_timestamp,prev_hb,next_hb FROM time_attend_log ORDER BY ID DESC LIMIT ?;", array('i',$limit), false);
            if(!isset($data)){
                $r->result = 'fail';
                $r->description = 'No Device exists'; 
                $newResponse = $response
                ->withJson($r,204);               
            }else{
                $r->result = 'success';
                $r->data=$data;
                //$r->user=$user;
                $newResponse = $response->withHeader('Access-Control-Allow-Origin','*')
                ->withJson($r,200);
            }

    } catch (Exception $e) {
        $r->result = 'fail';
        $r->description = 'Internal server error';
        $newResponse = $response
        ->withJson($r,500);  
        
    }
    // echo(json_encode($r));  
    
    return $newResponse;

});

$app->get('/[{name}]', function ($request, $response, $args) {
    // Sample log message
    $this->logger->info("Slim-Skeleton '/[{name}]' route");

    // Render index view
    return $this->renderer->render($response, 'index.phtml', $args);
});

$app->get('/hello/[{name}]', function($request, $response, $args){
	$name = $request->getAttribute('name');
	$response->getBody()->write("Hello, $name");
	//return $response;
	
	$data = array('name' => $name, 'age' => 40);
	//$newResponse = $response->withJson($data);
	$newResponse = $response
					->withHeader('Access-Control-Allow-Origin','*')
					->withJson($data,201);
	
	return $newResponse;
	
})->add( new ExampleMiddleware() );

// ==================================================================================================================================================
// NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH NORTH
// ==================================================================================================================================================
// USERS LOGIN
// ==================================================================================================================================================
$app->post(
    '/api/user/login',
    function ($request, $response, $arguments) {
        $_POST = json_decode(file_get_contents('php://input'), true);

        $uid=$_POST["uid"];
        $pwd=$_POST["pwd"];
        
        $uid = filter_var($uid, FILTER_SANITIZE_EMAIL);

        $result = login($uid,$pwd);

        $r = new stdClass();
        if($result){    
            $r->result = 'success';
            $r->data=$result;
            $newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($result,200);
        }else{
            $r->result = 'fail';
            $r->description = 'Invalid credentials';
            $newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,401);
        }

        // echo json_encode($r);
        return $newResponse;
    }
);
// ==================================================================================================================================================
// RETRIEVE LIST OF DEVICES
// ==================================================================================================================================================
$app->get(
    '/api/device/list',//PAGE??
    function ($request, $response, $arguments) {   
    $r = new stdClass();
    try {
    		$user = $_SESSION["user"];
            $data = execSQL("SELECT deviceid AS device_id FROM device_configuration WHERE user_id=? AND deleted=0;", array('i',$user), false);
            if(!isset($data)){
                $r->result = 'fail';
                $r->description = 'No Device exists'; 
            	$newResponse = $response
				->withHeader('Access-Control-Allow-Origin','*')
				->withJson($r,204);               
            }else{
                $r->result = 'success';
                $r->data=$data;
                //$r->user=$user;
            	$newResponse = $response
				->withHeader('Access-Control-Allow-Origin','*')
				->withJson($r,200);
            }

    } catch (Exception $e) {
        $r->result = 'fail';
        $r->description = 'Internal server error';
    	$newResponse = $response
		->withHeader('Access-Control-Allow-Origin','*')
		->withJson($r,500);  
        
    }
    // echo(json_encode($r));  
	
	return $newResponse;
}
)->add( new sTokenAuth() );
// ==================================================================================================================================================
// CREATE A SINGLE DEVICE
// ==================================================================================================================================================
$app->post(
    '/api/device/{id}',
    function ($request, $response, $arguments) {
        $_POST = json_decode(file_get_contents('php://input'), true);

        // VALIDATE DATA
        $user = $_SESSION["user"];
        $hb_time=$_POST["heartbeat_time"];
        $hb_duration=$_POST["heartbeat_duration"];
        $update_hb_eperiod=$_POST["reset_heartbeat_every_period"];

        if (!strtotime($hb_time)) {// validate time string
            $hb_time = null;
        }
        if( ! preg_match('/^\d+$/', $hb_duration) ){// Your variable is not an integer
            $hb_duration = null;    
        }
        if( ! preg_match('/\b([ny]|[NY])\b/', $update_hb_eperiod)){// n or y / N or Y
            $update_hb_eperiod = null;
        }
        
        $id = $request->getAttribute('id');
        $id = filter_var($id, FILTER_SANITIZE_STRING);

        $r = new stdClass();
        if((!$user)||(!$hb_time)||(!$hb_duration)||(!$update_hb_eperiod)){
	       	$r->result = 'fail';
            $r->description = 'Invalid data';
            $newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,400);
        }else{
	        $db = connect_db();
	        // CHECK DEVICE
	        
            $update_hb_eperiod = strtoupper($update_hb_eperiod);//SET TO UPPERCASE
	        $sql = "INSERT INTO device_configuration (deviceid,hb_time,hb_duration,update_hb_eperiod,user_id) VALUES ('$id','$hb_time',$hb_duration,'$update_hb_eperiod','$user')";
	        $result = $db->query($sql);

	        if($result){    
	            $r->result = 'success';
	            $r->data=$_POST;
	            $newResponse = $response
				->withHeader('Access-Control-Allow-Origin','*')
				->withJson($r,201);
	        }else{
	            $r->result = 'fail';
	            $r->description = 'Device already exists';
	            // $r->meme = $sql;
	            $newResponse = $response
				->withHeader('Access-Control-Allow-Origin','*')
				->withJson($r,409);
	        }
        }


        // echo(json_encode($r));
        return $newResponse;
    }
)->add( new sTokenAuth() );
// ==================================================================================================================================================
// UPDATE A SINGLE DEVICE
// ==================================================================================================================================================
$app->put(
    '/api/device/{id}',
    function ($request, $response, $arguments) use ($app) {
        $_POST = json_decode(file_get_contents('php://input'), true);

        // VALIDATE DATA
        $user = $_SESSION["user"];
        $hb_time=$_POST["heartbeat_time"];
        $hb_duration=$_POST["heartbeat_duration"];
        $update_hb_eperiod=$_POST["reset_heartbeat_every_period"];
        $id = $request->getAttribute('id');

        if (!strtotime($hb_time)) {// validate time string
            $hb_time = null;
        }
        if( ! preg_match('/^\d+$/', $hb_duration) ){// Your variable is not an integer
            $hb_duration = null;    
        }
        if( ! preg_match('/\b([ny]|[NY])\b/', $update_hb_eperiod)){// n or y / N or Y
            $update_hb_eperiod = null;
        }

        $id = filter_var($id, FILTER_SANITIZE_STRING);

        $r = new stdClass();
        if((!$user)||(!$hb_time)||(!$hb_duration)||(!$update_hb_eperiod)){
  			$r->result = 'fail';
            $r->description = 'Invalid data';
            $newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,400);
        }else{
	        // CHECK DEVICE
	        $db=connect_db();
            $update_hb_eperiod = strtoupper($update_hb_eperiod);//SET TO UPPERCASE
			$result = $db->query("UPDATE device_configuration SET hb_time='$hb_time',hb_duration='$hb_duration',update_hb_eperiod='$update_hb_eperiod',last_updated=SYSDATE() WHERE deviceid='$id' AND user_id='$user' AND deleted=0;");

	        if($db->affected_rows != 0){
	            $r->result = 'success';
	            $d = new stdClass();
	            $d->device_id=$id;
	            $r->data=$d;
	            $newResponse = $response
				->withHeader('Access-Control-Allow-Origin','*')
				->withJson($r,201);
	        }else{
	            $r->result = 'fail';
	            $r->description = 'Device doesn\'t exists'; 
	            $newResponse = $response
				->withHeader('Access-Control-Allow-Origin','*')
				->withJson($r,404);
	        }
        }

        // echo(json_encode($r));
		return $newResponse;
    }
)->add( new sTokenAuth() );
// ==================================================================================================================================================
// READ A SINGLE DEVICE
// ==================================================================================================================================================
$app->get('/api/device/{id}', function ($request, $response, $arguments) {
    $r = new stdClass();
    try {
    	$id = $request->getAttribute('id');
        $id = filter_var($id, FILTER_SANITIZE_STRING);

        $user = $_SESSION["user"];
        $ndata = querySQL("SELECT deviceid AS device_id,hb_time AS heartbeat_time,hb_duration AS heartbeat_duration,update_hb_eperiod AS reset_heartbeat_every_period FROM device_configuration WHERE deviceid='$id' AND user_id='$user' AND deleted=0;");
        if(empty($ndata)){
            if(sizeof($ndata)==0){
                $r->result = 'fail';
                $r->description = 'Device doesn\'t exists';
                $newResponse = $response
                    ->withHeader('Access-Control-Allow-Origin','*')
                    ->withJson($r,404);                
                }

        }else{
            $r->result = 'success';
            $r->data=$ndata;
            //$r->user=$user;            
        	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,200);
        }

    } catch (Exception $e) {
        // echo 'Caught exception: ',  $e->getMessage(), "\n";
        $r->result = 'fail';
        $r->description = $e->getMessage();
        // $r->error_message = 'Invalid Data';
    	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,500);
    }
    // echo(json_encode($r)); 	
	return $newResponse;
})->add( new sTokenAuth() );
// ==================================================================================================================================================
// DELETE A SINGLE DEVICE
// ==================================================================================================================================================
$app->delete(
    '/api/device/{id}',
    function ($request, $response, $arguments) {
        // CHECK DEVICE
        $user = $_SESSION["user"];
        $id = $request->getAttribute('id');
        $id = filter_var($id, FILTER_SANITIZE_STRING);

		$db=connect_db();
        $result = $db->query("UPDATE device_configuration SET deleted=1, date_deleted=SYSDATE() WHERE deviceid='$id' AND user_id='$user';");
        $r = new stdClass();
        if($result){    
            $r->result = 'success';
            $d = new stdClass();
            $d->device_id=$id;
            $r->data=$d;
        	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,204);
        }else{
            $r->result = 'fail';
            $r->description = 'Device doesn\'t exists';
        	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,204);
        }
        // echo(json_encode($r));
        return $newResponse;
    }
)->add( new sTokenAuth() );
// ==================================================================================================================================================
// RETRIEVE TIME ATTENDANCE BY DEVICE / TAG
// ==================================================================================================================================================
$app->get('/api/attendance/{dev}/{id}', function ($request, $response, $arguments) {//PAGE
    $r = new stdClass();
    try {
    	$user = $_SESSION["user"];
		$dev = $request->getAttribute('dev');
		$id = $request->getAttribute('id');
        
        $id = filter_var($id, FILTER_SANITIZE_STRING);

        $start_time = null;
        $end_time = null;
        $start_counter = null;

        if ((isset($_REQUEST['start_time'])) && (!empty($_REQUEST['start_time'])))
        {
            $start_time = $_GET['start_time'];
            if (!strtotime($start_time)) {// validate time string
                $start_time = null;
            }
        }
        if ((isset($_REQUEST['end_time'])) && (!empty($_REQUEST['end_time'])))
        {
            $end_time = $_GET['end_time'];
            if (!strtotime($end_time)) {// validate time string
                $end_time = null;
            }
        }
        if ((isset($_REQUEST['start_counter'])) && (!empty($_REQUEST['start_counter'])))
        {
            $start_counter = $_GET['start_counter'];
            if( ! preg_match('/^\d+$/', $start_counter) ){// Your variable is not an integer
                $start_counter = 0;    
            }
        }else{
            $start_counter = 0;
        }

        if(!(($start_time)&&($end_time))){
            $r->result = 'fail';
            $r->description = 'Invalid data';
            $newResponse = $response
            ->withHeader('Access-Control-Allow-Origin','*')
            ->withJson($r,400);
        }else{
            if($dev==='device'){
                $ndata = querySQL("SELECT dev_timestamp AS timestamp,cardid AS tag_id FROM time_attend_log WHERE deviceid IN (SELECT DISTINCT deviceid FROM device_configuration WHERE user_id='$user' AND deviceid='$id' AND deleted=0) AND dev_timestamp>='$start_time' AND dev_timestamp<='$end_time' ORDER BY dev_timestamp LIMIT $start_counter,".RECORD_LIMIT.";");
            }else{
                $ndata = querySQL("SELECT dev_timestamp AS timestamp,deviceid FROM time_attend_log WHERE cardid='$id' AND deviceid IN (SELECT DISTINCT deviceid FROM device_configuration WHERE user_id='$user' AND deleted=0) AND dev_timestamp>='$start_time' AND dev_timestamp<='$end_time' ORDER BY dev_timestamp LIMIT $start_counter,".RECORD_LIMIT.";");
            }

            if(!$ndata){
                $r->result = 'fail';
                if($dev==='device'){
                    $r->description = 'Device doesn\'t exists';
                }else{
                    $r->description = 'Tag doesn\'t exists';
                }
                $newResponse = $response
                ->withHeader('Access-Control-Allow-Origin','*')
                ->withJson($r,404);
            }else{
                $r->result = 'success';
                $d = new stdClass();
                if($dev==='device'){
                    $d->device_id=$id;
                }else{
                    $d->tag_id=$id;
                }            
                $d->attendance=$ndata;

                try {
                    $d->last_counter=(sizeof($ndata) < RECORD_LIMIT) ? sizeof($ndata)+$start_counter : RECORD_LIMIT+$start_counter;
                } catch (Exception $e) {
                    //echo 'Caught exception: ',  $e->getMessage(), "\n";
                    $d->last_counter=0;
                }
                
                $r->data=$d;
                 $newResponse = $response
                ->withHeader('Access-Control-Allow-Origin','*')
                ->withJson($r,200);  
            }
        }

    } catch (Exception $e) {
        $r->result = 'fail';
        $r->description = $e->getMessage();
        // $r->error_message = 'Invalid Data';        
         	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,400);  
    }
    // echo(json_encode($r)); 
    return $newResponse;
})->add( new sTokenAuth() );
// ==================================================================================================================================================
// RETRIEVE ALL DEVICE SERVICES
// ==================================================================================================================================================
$app->get('/api/devicestatus', function ($request, $response, $arguments) {//PAGE??
    $r = new stdClass();
    try {
    	$user = $_SESSION["user"];
        // $ndata = querySQL("SELECT deviceid,ac_on,tamper AS tamper_on,MAX(dev_timestamp) AS dev_timestamp FROM device_status_log WHERE deviceid IN (SELECT deviceid FROM device_configuration WHERE user_id='$user') GROUP BY deviceid;");
        $ndata = querySQL("SELECT deviceid,ac_on,MAX(dev_timestamp) AS dev_timestamp FROM device_status_log WHERE deviceid IN (SELECT deviceid FROM device_configuration WHERE user_id='$user') GROUP BY deviceid;");
        if(!$ndata){
            $r->result = 'fail';
            $r->description = 'No device to list';
         	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,204);           
        }else{
            $r->result = 'success';
            $r->data=$ndata;
            // $r->data->device_id=$ndata['device_id'];
         	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,200);  
        }            
    } catch (Exception $e) {
        $r->result = 'fail';
        $r->description = $e->getMessage();
        // $r->error_message = 'Invalid Data'; 
     	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,400);        
    }
    // echo(json_encode($r));
    return $newResponse;
})->add( new sTokenAuth() );
// ==================================================================================================================================================
// RETRIEVE SINGLE DEVICE SERVICES
// ==================================================================================================================================================
$app->get('/api/devicestatus/{id}', function ($request, $response, $arguments) {
    $r = new stdClass();
    try {
    	$user = $_SESSION["user"];
        $id = $request->getAttribute('id');
        $id = filter_var($id, FILTER_SANITIZE_STRING);

        // $ndata = querySQL("SELECT deviceid AS device_id,ac_on,tamper AS tamper_on,MAX(dev_timestamp) AS dev_timestamp FROM device_status_log WHERE deviceid IN (SELECT deviceid FROM device_configuration WHERE user_id='$user' AND deviceid='$id') HAVING deviceid IS NOT NULL;");
        $ndata = querySQL("SELECT deviceid AS device_id,ac_on,MAX(dev_timestamp) AS dev_timestamp FROM device_status_log WHERE deviceid IN (SELECT deviceid FROM device_configuration WHERE user_id='$user' AND deviceid='$id') HAVING deviceid IS NOT NULL;");
        if(!$ndata){
            $r->result = 'fail';
            $r->description = 'Device doesn\'t exists';
            $newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,404);          
        }else{
            $r->result = 'success';
            $r->data=$ndata;
            // $r->data->device_id=$ndata['device_id'];
         	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,200);  
        }

    } catch (Exception $e) {
        $r->result = 'fail';
        $r->description = $e->getMessage();
        // $r->error_message = 'Invalid Data';   
        $newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,400);  
    }
    // echo(json_encode($r)); 
    return $newResponse;
})->add( new sTokenAuth() );
// ==================================================================================================================================================
// SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH SOUTH 
// ==================================================================================================================================================
// POST TIME ATTENDANCE / PAIRINGS SERVICES
// ==================================================================================================================================================
$app->post('/device/{id}/attendance',function ($request, $response, $arguments) {
        
		$id = $request->getAttribute('id');    
        $_POST = json_decode(file_get_contents('php://input'), true);
        $pairings = $_POST["pairings"];// VALIDATE DATE
        
        $r = new stdClass();
		try {

	        $db = connect_db();
			foreach ($pairings as $value){// THROW EXCEPTION INVALID DATA
		        $tag_id=$value["tag_id"];		     
		        $timestamp=   $value["timestamp"];
		        $previous_heartbeat_start=$value["previous_heartbeat_start"];
		        $previous_heartbeat_stop=$value["previous_heartbeat_stop"];

				$query=mysqli_query($db,"INSERT INTO time_attend_log (deviceid,cardid,dev_timestamp,prev_hb,next_hb) VALUES ('$id' , '$tag_id', '$timestamp','$previous_heartbeat_start','$previous_heartbeat_stop');");

		        if ($query){
		            $r->result = 'success';
		            $r->update_config=isUpdateConfig($id);
                 	$newResponse = $response
						->withHeader('Access-Control-Allow-Origin','*')
						->withJson($r,201);   
		        }else{         
		            $r->result = 'fail';
		            $r->description = 'Device doesn\'t exists';
                 	$newResponse = $response
						->withHeader('Access-Control-Allow-Origin','*')
						->withJson($r,404); 
		        }
			}

		} catch (Exception $e) {
        	$r->result = 'fail';
            $r->description = 'Internal server error';
         	$newResponse = $response
				->withHeader('Access-Control-Allow-Origin','*')
				->withJson($r,500); 
		}
        // echo(json_encode($r));
        return $newResponse;
    }
);
// ==================================================================================================================================================
// POST DEVICE EVENT/STATUS SERVICES
// ==================================================================================================================================================
$app->post('/device/{id}/status',function ($request, $response, $arguments) {

		$id = $request->getAttribute('id');
        $_POST = json_decode(file_get_contents('php://input'), true);
        $events = $_POST["events"];// VALIDATE DATA
        
        $r = new stdClass();
		try {

	        $db = connect_db();
			foreach ($events as $value){// THROW EXCEPTION INVALID DATA
		        $e_code=$value["event_code"];
		        $e_desc=$value["event_description"];
		        $ac=$value["ac_on"];
		        $batt=$value["batt_on"];
		        $tamper=$value["tamper_on"];
		        $timestamp=   $value["timestamp"];
		        //$timestamp=   str_replace("T"," ",$value["timestamp"]);
		        //$timestamp=date('Y-m-d H:i:s');

				// echo $value["timestamp"];
		 	 //       echo $timestamp;

				$query=mysqli_query($db,"INSERT INTO device_status_log (deviceid,ac_on,batt_on,tamper,dev_timestamp) VALUES ('$id' , '$ac', '$batt', '$tamper', '$timestamp');");
				$query=mysqli_query($db,"INSERT INTO device_event_log (deviceid,eventid,eventdesc,dev_timestamp) VALUES ('$id' , '$e_code', '$e_desc','$timestamp');");

		        // $query=false;
		        if ($query){
		            $r->result = 'success';
		            $r->data=$_POST;
                    $newResponse = $response
                    ->withHeader('Access-Control-Allow-Origin','*')
                    ->withJson($r,404); 
		        }else{         
		            $r->result = 'fail';
		            $r->description = 'Device doesn\'t exists';
                    $newResponse = $response
                    ->withHeader('Access-Control-Allow-Origin','*')
                    ->withJson($r,404); 
		        }

			}


		} catch (Exception $e) {
        	$r->result = 'fail';
            $r->description = 'Device doesn\'t exists';
            $newResponse = $response
            ->withHeader('Access-Control-Allow-Origin','*')
            ->withJson($r,404); 
		}
        // echo(json_encode($r));
        return $newResponse;
    }
);
// ==================================================================================================================================================
// RETRIEVE DEVICE CONFIGURATIONS SERVICES
// ==================================================================================================================================================
$app->get('/device/{id}/config', function ($request, $response, $arguments) {
    $r = new stdClass();
    try {
        $id = $request->getAttribute('id');
        $ndata = execSQL("SELECT deviceid AS device_id,hb_time AS heartbeat_time,hb_duration AS heartbeat_duration,NOW() AS new_clock,update_hb_eperiod AS update_heartbeat_each_pairing FROM device_configuration WHERE deviceid=?;", array('i', $id), false);

        if(!$ndata){
            $r->result = 'fail';
            $r->description = 'Device doesn\'t exists';
         	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,404);         
        }else{
            $r->result = 'success';
            $r->data=$ndata;
         	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,200); 
        }

    } catch (Exception $e) {
        $r->result = 'fail';
        $r->description = $e->getMessage();
        // $r->error_message = 'Invalid Data';
     	$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,500);   
    }
    // echo(json_encode($r));
    return $newResponse;
});
<?php
// Application middleware

// e.g: $app->add(new \Slim\Csrf\Guard);
class ExampleMiddleware
{
    /**
     * Example middleware invokable class
     *
     * @param  \Psr\Http\Message\ServerRequestInterface $request  PSR7 request
     * @param  \Psr\Http\Message\ResponseInterface      $response PSR7 response
     * @param  callable                                 $next     Next middleware
     *
     * @return \Psr\Http\Message\ResponseInterface
     */
    public function __invoke($request, $response, $next)
    {
        $response->getBody()->write('BEFORE');
        $response = $next($request, $response);
        // $response->getBody()->write('AFTER');

        return $response;
    }
}
//$subject->add( new ExampleMiddleware() );


class sTokenAuth
{
    /**
     * Example middleware invokable class
     *
     * @param  \Psr\Http\Message\ServerRequestInterface $request  PSR7 request
     * @param  \Psr\Http\Message\ResponseInterface      $response PSR7 response
     * @param  callable                                 $next     Next middleware
     *
     * @return \Psr\Http\Message\ResponseInterface
     */
    public function __invoke($request, $response, $next)
    {
    	$route = $request->getAttribute('route')->getPattern();
		if ($request->hasHeader('Authorization')) {
		    $tokenAuth = $request->getHeaderLine('Authorization');
		    $user =checkToken($tokenAuth);
			if ($user)
			{
				Accesslog($tokenAuth,$route,'SUCCESS');
				// session_start();
				$_SESSION["user"] = $user;
			    $response = $next($request, $response);
			    // $response->getBody()->write('AFTER');
		    	return $response;
			}else{
				Accesslog($tokenAuth,$route,'EXPIRED');
			    $r = new stdClass();
				$r->result="fail";
				$r->description="expired token";
				$newResponse = $response
				->withHeader('Access-Control-Allow-Origin','*')
				->withJson($r,401); 

			    return $response;
			}

		}else{
			Accesslog('',$route,'NOTAUTHORIZED');
			$r = new stdClass();
			$r->result="fail";
			$r->description="missing token";
			$newResponse = $response
			->withHeader('Access-Control-Allow-Origin','*')
			->withJson($r,401); 

			return $newResponse;
		}

    }
}
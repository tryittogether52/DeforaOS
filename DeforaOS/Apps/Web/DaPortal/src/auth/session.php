<?php //$Id$
//Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DeforaOS Web DaPortal
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, version 3 of the License.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.



require_once('./system/auth.php');
require_once('./system/user.php');


//SessionAuth
class SessionAuth extends Auth
{
	//protected
	//methods
	//SessionAuth::match
	protected function match($engine)
	{
		if(!isset($_SERVER['SCRIPT_NAME']))
			return 0;
		//return the result cached if called twice
		if($this->match_score !== FALSE)
			return $this->match_score;
		@ini_set('session.use_only_cookies', 1);
		@ini_set('session.use_trans_sid', 0);
		$params = session_get_cookie_params();
		//XXX probably not always as strict as it could be
		$params['path'] = dirname($_SERVER['SCRIPT_NAME']);
		if(isset($_SERVER['HTTP_HOST']))
		{
			$domain = $_SERVER['HTTP_HOST'];
			if(($pos = strpos($domain, ':', 1)) !== FALSE)
				$domain = substr($domain, 0, $pos);
			$params['domain'] = $domain;
		}
		if(isset($_SERVER['HTTPS']))
			$params['secure'] = 1;
	       	//XXX we may have to set it to 0 later
		$params['httponly'] = 1;
		session_set_cookie_params($params['lifetime'], $params['path'],
				$params['domain'], $params['secure'],
				$params['httponly']);
		$this->match_score = @session_start() ? 100 : 0;
		return $this->match_score;
	}


	//SessionAuth::attach
	protected function attach($engine)
	{
		//attaching depends on the code in match()
		$this->match($engine);
	}


	//public
	//accessors
	//SessionAuth::getCredentials
	public function getCredentials($engine)
	{
		if(($uid = $this->getVariable($engine, 'SessionAuth::uid'))
					=== FALSE
				|| $uid == 0)
			return parent::getCredentials($engine);
		$user = new User($engine, $uid);
		if(!$user->isEnabled())
			return parent::getCredentials($engine);
		$cred = new AuthCredentials($user->getUserId(),
				$user->getUsername(),
				$user->getGroupId(),
				$user->isAdmin());
		parent::setCredentials($engine, $cred);
		return parent::getCredentials($engine);
	}


	//SessionAuth::getVariable
	public function getVariable($engine, $variable)
	{
		if(isset($_SESSION[$variable]))
			return $_SESSION[$variable];
		return FALSE;
	}


	//SessionAuth::setCredentials
	public function setCredentials($engine, $credentials)
	{
		global $config;

		//avoid session-fixation attacks
		$message = 'Could not regenerate the session';
		if($config->getVariable('auth::session', 'regenerate') == 1
				&& session_regenerate_id(TRUE) !== TRUE)
			$engine->log('LOG_WARNING', $message);
		$this->setVariable($engine, 'SessionAuth::uid',
				$credentials->getUserId());
		return parent::setCredentials($engine, $credentials);
	}


	//SessionAuth::setIdempotent
	public function setIdempotent($engine, $request, $idempotent)
	{
		if($idempotent === TRUE)
		{
			$request->setIdempotent(TRUE);
			return;
		}
		//prevent CSRF attacks
		$idempotent = TRUE;
		if(($token = $request->getParameter('_token')) === FALSE
				|| !isset($_SESSION['tokens'])
				|| !is_array($_SESSION['tokens']))
			return TRUE;
		//delete old tokens
		foreach($_SESSION['tokens'] as $k => $v)
			if($v < time())
				unset($_SESSION['tokens'][$k]);
		if(isset($_SESSION['tokens'][$token]))
		{
			//the request is not idempotent
			unset($_SESSION['tokens'][$token]);
			$idempotent = FALSE;
		}
		$request->setIdempotent($idempotent);
	}


	//SessionAuth::setVariable
	public function setVariable($engine, $variable, $value)
	{
		$_SESSION[$variable] = $value;
		return TRUE;
	}


	//private
	//properties
	private $match_score = FALSE;
}

?>

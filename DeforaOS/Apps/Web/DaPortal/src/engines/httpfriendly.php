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



require_once('./engines/http.php');


//HttpFriendlyEngine
class HttpFriendlyEngine extends HttpEngine
{
	//public
	//methods
	//essential
	//HttpFriendlyEngine::match
	public function match()
	{
		if(($score = parent::match()) != 100)
			return $score;
		return $score + 1;
	}


	//accessors
	//HttpFriendlyEngine::getRequest
	public function getRequest()
	{
		return parent::getRequest();
	}

	protected function _getRequestDo()
	{
		if(!isset($_SERVER['PATH_INFO']))
			return parent::_getRequestDo();
		$path = explode('/', $_SERVER['PATH_INFO']);
		if(!is_array($path) || count($path) < 2)
			return parent::_getRequestDo();
		//the first element is empty
		array_shift($path);
		//the second element is the module name
		$module = array_shift($path);
		$action = FALSE;
		$id = FALSE;
		$title = FALSE;
		$extension = FALSE;
		$args = FALSE;
		if(count($path) > 0)
		{
			//the third element is the action (or the ID directly)
			$id = array_shift($path);
			if(!is_numeric($id))
			{
				//there is an action before the ID
				$action = $id;
				$id = FALSE;
				if(count($path) > 0 && is_numeric($path[0]))
					$id = array_shift($path);
			}
		}
		if(count($path) > 0)
			$title = implode('/', $path);
		//arguments
		//XXX is there a function to do this directly?
		$query = explode('&', $_SERVER['QUERY_STRING']);
		foreach($query as $q)
		{
			if($args === FALSE)
				$args = array();
			$q = explode('=', $q);
			if(count($q) != 2)
				continue;
			$q[0] = urldecode($q[0]);
			$q[1] = urldecode($q[1]);
			if($title === FALSE && $q[0] == '_title')
				$title = $q[1];
			else
				$args[$q[0]] = $q[1];
		}
		//analyze the extension
		$this->_getRequestType($extension);
		return new Request($module, $action, $id, $title, $args);
	}

	protected function _getRequestType($extension)
	{
		switch($extension)
		{
			case 'htm':
			case 'html':
				$this->setType('text/html');
				break;
			case 'rss':
				$this->setType('application/rss+xml');
				break;
			default:
				break;
		}
	}


	//HttpFriendlyEngine::getUrl
	public function getUrl($request, $absolute = TRUE)
	{
		global $config;

		//FIXME do not include parameters for a POST request
		if($request === FALSE)
			return FALSE;
		$name = $_SERVER['SCRIPT_NAME'];
		//use the kicker instead if defined
		if(($kicker = $config->getVariable('engine::httpfriendly',
				'kicker')) !== FALSE)
			$name = dirname($name).'/'.$kicker;
		$name = ltrim($name, '/');
		if($absolute)
		{
			//prepare the complete address
			$url = $_SERVER['SERVER_NAME'];
			if(isset($_SERVER['HTTPS']))
			{
				if($_SERVER['SERVER_PORT'] != 443)
					$url .= ':'.$_SERVER['SERVER_PORT'];
				$url = 'https://'.$url;
			}
			else if($_SERVER['SERVER_PORT'] != 80)
				$url = 'http://'.$url.':'
				.$_SERVER['SERVER_PORT'];
			else
				$url = 'http://'.$url;
			$url .= '/'.$name;
		}
		else
			//prepare a relative address
			$url = basename($name);
		//return if already complete
		if(($module = $request->getModule()) === FALSE)
			return $url;
		//handle the main parameters
		$url .= '/'.$module;
		if(($action = $request->getAction()) !== FALSE)
			$url .= '/'.$action;
		if(($id = $request->getId()) !== FALSE)
			$url .= '/'.$id;
		if(($title = $request->getTitle()) !== FALSE)
		{
			if($action === FALSE && $id === FALSE)
				$url .= '/default';
			$title = str_replace(' ', '-', $title);
			$url .= '/'.$title;
		}
		//handle arguments
		if($request->isIdempotent()
				&& ($args = $request->getParameters())
				!== FALSE)
		{
			$sep = '?';
			foreach($args as $key => $value)
			{
				$url .= $sep.urlencode($key)
					.'='.urlencode($value);
				$sep = '&';
			}
		}
		return $url;
	}
}

?>

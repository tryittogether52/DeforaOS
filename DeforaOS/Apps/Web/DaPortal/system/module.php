<?php //$Id$
//Copyright (c) 2004-2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



function _module($module = FALSE, $action = FALSE, $args = FALSE)
{
	global $engine, $module_id, $module_name;

	if($module == '')
		$module = FALSE;
	$id = FALSE;
	$title = FALSE;
	//handle default requests
	if($module === FALSE && $action === FALSE)
	{
		$request = $engine->getRequest();
		$module = $request->getModule();
		$action = $request->getAction();
		if(($args = $request->getParameters()) === FALSE)
			$args = array();
		$args['module'] = $module;
		$args['action'] = $action;
		if(($id = $request->getId()) !== FALSE)
			$args['id'] = $id;
		if(($title = $request->getTitle()) !== FALSE)
			$args['title'] = $title;
	}
	//action was set
	else if($module === FALSE)
	{
		//obtain the default module
		$request = $engine->getRequest();
		$module = $request->getModule();
		if(($args = $request->getParameters()) === FALSE)
			$args = array();
		$args['module'] = $module;
		$args['action'] = $request->getAction();
		if(($id = $request->getId()) !== FALSE)
			$args['id'] = $id;
		if(($title = $request->getTitle()) !== FALSE)
			$args['title'] = $title;
	}
	//create a complete request
	else
	{
		$id = isset($args['id']) ? $args['id'] : FALSE;
		$title = isset($args['title']) ? $args['title'] : FALSE;
	}
	$request = new Request($module, $action, $id, $title, $args);
	$module_name = $module;
	$module_id = _module_id($module);
	return $engine->process($request);
}


function _module_desktop($name)
{
	static $cache = array();

	if(isset($cache[$name]))
		return $cache[$name];
	if(($res = _desktop_include($name)) == FALSE)
		return FALSE;
	$res['name'] = $name;
	$cache[$name] = $res;
	return $res;
}

function _desktop_include($name)
{
	$admin = 0;
	$list = 0;
	$search = 0;
	$title = '';
	$icon = 'admin.png';
	$actions = FALSE;
	$user = 0;
	$filename = './modules/'.$name.'/desktop.php';
	if(is_readable($filename))
		include_once($filename);
	return array('admin' => $admin, 'list' => $list, 'search' => $search,
			'title' => $title, 'icon' => $icon,
			'actions' => $actions, 'user' => $user);
}


function _module_id($name)
{
	global $engine;

	require_once('./src/system/module.php');
	return Module::getId($engine, $name);
}


function _module_id_tag($module, $id, $tag)
{
	$tag = str_replace('-', '_', $tag);
	$sql = 'SELECT content_id FROM daportal_content, daportal_module'
		.' WHERE daportal_content.module_id=daportal_module.module_id'
		." AND name='$module' AND content_id='$id'"
		." AND title LIKE '$tag'";
	if(_sql_single($sql) == $id)
		return TRUE;
	//user module
	$sql = 'SELECT user_id FROM daportal_user'
		." WHERE user_id='$id' AND username LIKE '$tag'";
	if(_sql_single($sql) == $id)
		return TRUE;
	//FIXME some modules have long filenames, but it's ugly to do it here
	return FALSE;
}


//PRE	$module, $action and $params are trusted
function _module_link($module = FALSE, $action = FALSE, $id = FALSE,
		$tag = FALSE, $params = FALSE)
{
	global $friendlylinks, $friendlykicker;

	$link = $_SERVER['SCRIPT_NAME'];
	if(isset($_SERVER['DOCUMENT_ROOT'])
			&& is_readable(dirname($_SERVER['SCRIPT_FILENAME'])
				."/$friendlykicker.php"))
		$link = rtrim(dirname($_SERVER['SCRIPT_NAME']), '/').'/'
			.$friendlykicker;
	if(is_array($params))
	{
		$keys = array_keys($params);
		$p = $params;
		$params = '';
		$and = '';
		$a = array('=', '&amp;');
		$b = array('%3d', '%26');
		foreach($keys as $k)
		{
			$params .= $and.$k.'='.str_replace($a, $b, $p[$k]);
			$and = '&';
		}
	}
	if($friendlylinks == 1)
	{
		if($module === FALSE)
			return $link;
		$link .= '/'.$module;
		if($action !== FALSE && $action != '' && $action != 'default')
			$link .= '/'.$action;
		if($id !== FALSE && is_numeric($id))
		{
			$link .= '/'.$id;
			if($tag !== FALSE && $tag != '')
			{
				$a = array(' ', '/', '?', '&', '%', '#', '<',
						'>', "'", '"', ',', ':');
				$tag = str_replace($a, '-', $tag);
				$link .= '/'.$tag;
			}
		}
		if($params !== FALSE && $params != '')
			$link .= '?'.$params;
	}
	else if($module !== FALSE)
	{
		$link .= '?module='.$module;
		if($action !== FALSE && $action != '' && $action != 'default')
			$link .= '&action='.$action;
		if($id !== FALSE && is_numeric($id))
			$link .= '&id='.$id;
		if($params !== FALSE && $params != '')
			$link .= '&'.$params;
	}
	return $link;
}


function _module_link_full($module = FALSE, $action = FALSE, $id = FALSE,
		$tag = FALSE, $params = FALSE)
{
	$link = $_SERVER['SERVER_NAME'];
	if(isset($_SERVER['HTTPS']))
	{
		$link = 'https://'.$link;
		if($_SERVER['SERVER_PORT'] != '443')
			$link .= ':'.$_SERVER['SERVER_PORT'];
	}
	else
	{
		$link = 'http://'.$link;
		if($_SERVER['SERVER_PORT'] != '80')
			$link .= ':'.$_SERVER['SERVER_PORT'];
	}
	return $link._module_link($module, $action, $id, $tag, $params);
}


function _module_list($enabled = FALSE)
{
	$enabled = ($enabled !== FALSE && $enabled == 0) ? ''
		: " WHERE enabled='1'";
	$sql = 'SELECT module_id AS id, name FROM daportal_module'.$enabled
		.' ORDER BY name ASC';
	$res = _sql_array($sql);
	if(!is_array($res))
		return FALSE;
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		if(($r = _module_desktop($res[$i]['name'])) == FALSE)
			continue;
		foreach($r as $k => $v)
			$res[$i][$k] = $v;
	}
	return $res;
}


function _module_name($id)
{
	static $cache = array();
	global $user_id;

	if(isset($cache[$id]))
		return $cache[$id];
	$cache[$id] = _sql_single('SELECT name FROM daportal_module'
			." WHERE module_id='$id' AND enabled='1'");
	return $cache[$id];
}


function _module_parse_friendly($path)
{
	$path = explode('/', $path);
	if(!is_array($path) || count($path) < 2)
		return FALSE;
	array_shift($path);
	$_GET['module'] = array_shift($path);
	if(count($path) == 0)
		return TRUE;
	$id = array_shift($path);
	if(!is_numeric($id))
	{
		$_GET['action'] = $id;
		if(count($path) == 0)
			return TRUE;
		$id = array_shift($path);
	}
	if(!is_numeric($id))
		return FALSE;
	$_GET['id'] = $id;
	if(count($path) == 0)
		return TRUE;
	$tag = array_shift($path);
	while(count($path))
		$tag .= '/'.array_shift($path);
	return _module_id_tag($_GET['module'], $id, $tag);
}

?>

<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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



//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['ADDRESS'] = 'Address';
$text['CREATE'] = 'Create';
$text['NEW_LINK'] = 'New link';
_lang($text);


//TopModule
class TopModule extends Module
{
	//public
	//methods
	//useful
	//TopModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		$args = $request->getParameters();
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'delete':
			case 'insert':
			case 'modify':
			case 'move':
			case 'update':
				return $this->$action($args);
			case 'new':
				return $this->_new($args);
			case 'system':
				return FALSE;
			default:
				$this->_default($args);
		}
		return FALSE;
	}


//TopModule::admin
protected function admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title top">Top links administration</h1>'."\n");
	$links = _sql_array('SELECT top_id, name, link AS url FROM daportal_top'
			.' ORDER BY top_id ASC');
	if(!is_array($links))
		return _error('Unable to get links');
	$last_id = 0;
	for($i = 0, $cnt = count($links); $i < $cnt; $i++)
	{
		/* $links[$i]['icon'] = ereg('^http://[^/]+/$',
				$links[$i]['url'])
				? _html_safe_link($links[$i]['url'])
						.'favicon.png'
				: 'modules/top/icon.png'; */
		$links[$i]['icon'] = 'icons/16x16/top.png';
		$links[$i]['thumbnail'] = 'icons/48x48/top.png';
		$links[$i]['apply_module'] = 'top';
		$links[$i]['apply_id'] = $links[$i]['top_id'];
		$links[$i]['link'] = _module_link('top', 'modify',
				$links[$i]['top_id']);
		/* FIXME use _html_link() */
		$links[$i]['url'] = '<a href="'
				._html_safe($links[$i]['url'])
				.'">'._html_safe($links[$i]['url']).'</a>';
		$links[$i]['move'] = '';
		if($i+1 < $cnt)
			$links[$i]['move'].='<a href="index.php?module=top'
				.'&action=move&id='.$links[$i]['top_id']
				.'&to='.$links[$i+1]['top_id'].'">'
				._html_icon('down').'</a>';
		if($last_id)
			$links[$i]['move'].= '<a href="index.php?module=top'
				.'&action=move&id='.$links[$i]['top_id']
				.'&to='.$last_id.'">'
				._html_icon('up').'</a>';
		$last_id = $links[$i]['top_id'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_LINK, 'class' => 'new',
			'link' => _module_link('top', 'new'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('module' => 'top',
			'action' => 'admin', 'view' => 'details',
			'class' => array('url' => 'Address', 'move' => 'Move'),
			'toolbar' => $toolbar, 'entries' => $links));
}


//TopModule::_default
protected function _default($args)
{
	$links = _sql_array('SELECT name, link FROM daportal_top'
			.' ORDER BY top_id ASC');
	if(!is_array($links))
		return _error('Unable to get links');
	print("\t\t".'<div class="top">'."\n");
	foreach($links as $l)
		print("\t\t\t".'<a href="'._html_safe($l['link']).'"'
				.' title="'._html_safe($l['name']).'">'
				._html_safe($l['name']).'</a>'."\n");
	print("\t\t".'</div>'."\n");
}


//TopModule::delete
protected function delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	_sql_query("DELETE FROM daportal_top WHERE top_id='".$args['id']."'");
}


//TopModule::insert
protected function insert($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(_sql_query('INSERT INTO daportal_top (name, link) VALUES ('
			."'".$args['name']."', '".$args['link']."')") == FALSE)
		return _error('Unable to insert link');
	$this->admin(array());
}


//TopModule::modify
protected function modify($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$top = _sql_array('SELECT top_id, name, link FROM daportal_top'
			." WHERE top_id='".$args['id']."'");
	if(!is_array($top) || count($top) != 1)
		return _error('Invalid link ID');
	$top = $top[0];
	$title = 'Top link modification';
	$action = 'update';
	include('./modules/top/update.tpl');
}


//TopModule::move
protected function move($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	$from = _sql_array('SELECT name, link FROM daportal_top'
			." WHERE top_id='".$args['id']."'");
	$to = _sql_array('SELECT name, link FROM daportal_top'
			." WHERE top_id='".$args['to']."'");
	if(!is_array($from) || count($from) != 1 || !is_array($to)
			|| count($to) != 1)
		return _error('Unable to move links');
	$from = $from[0];
	$to = $to[0];
	if(!_sql_query("UPDATE daportal_top SET name='".$from['name']."'"
			.", link='".$from['link']."'"
			." WHERE top_id='".$args['to']."'")
			|| !_sql_query('UPDATE daportal_top'
					." SET name='".$to['name']."'"
					.", link='".$to['link']."'"
					." WHERE top_id='".$args['id']."'"))
		_error('Error while moving links');
	$this->admin(array());
}


//TopModule::_new
protected function _new($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$title = 'New top link';
	$action = 'insert';
	include('./modules/top/update.tpl');
}


//TopModule::update
protected function update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(!_sql_query('UPDATE daportal_top SET name='."'".$args['name']."'"
			.", link='".$args['link']."'"
			." WHERE top_id='".$args['id']."'"))
		return _error('Unable to update link');
	$this->admin(array());
}
}

?>

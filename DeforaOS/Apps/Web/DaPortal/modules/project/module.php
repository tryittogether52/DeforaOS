<?php //$Id$
//Copyright (c) 2005-2012 Pierre Pronchery <khorben@defora.org>
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
//FIXME:
//- hide attic option
//- clarify anonymous bug replies etc



//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


//lang
$text = array();
include('./modules/project/lang.php');
global $lang;
if($lang == 'de')
{
	include('./modules/project/lang.de.php');
}
else if($lang == 'fr')
{
	include('./modules/project/lang.fr.php');
}
_lang($text);


//ProjectModule
class ProjectModule extends Module
{
	//public
	//methods
	//useful
	//ProjectModule::call
	public function call(&$engine, $request)
	{
		$args = $request->getParameters();
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'browse':
			case 'disable':
			case 'display':
			case 'download':
			case 'insert':
			case 'modify':
			case 'system':
			case 'timeline':
			case 'update':
				return $this->$action($args);
			case 'bug_delete':
				return $this->bugDelete($args);
			case 'bug_disable':
				return $this->bugDisable($args);
			case 'bug_display':
				return $this->bugDisplay($args);
			case 'bug_enable':
				return $this->bugEnable($args);
			case 'bug_insert':
				return $this->bugInsert($args);
			case 'bug_list':
				return $this->bugList($args);
			case 'bug_new':
				return $this->bugNew($args);
			case 'bug_reply':
				return $this->bugReply($args);
			case 'delete':
				return $this->_delete($args);
			case 'download_insert':
				return $this->downloadInsert($args);
			case 'lastcommits':
				return $this->lastCommits($args);
			case 'list':
				return $this->_list($args);
			case 'new':
				return $this->_new($args);
			case 'screenshot_insert':
				return $this->screenshotInsert($args);
			default:
				return $this->_default($args);
		}
		return FALSE;
	}


//functions
private function _toolbar($id)
{
	global $html;

	if(!$html)
		return;
	$admin = $this->_isAdmin($id);
	$cvsroot = '';
	$enabled = 0;
	$project = _sql_array('SELECT title, cvsroot, enabled'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND project_id='$id'");
	if(is_array($project) && count($project) == 1)
	{
		$title = $project[0]['title'];
		$cvsroot = $project[0]['cvsroot'];
		$enabled = $project[0]['enabled'] == TRUE ? 1 : 0;
	}
	include('./modules/project/toolbar.tpl');
}


//ProjectModule::_getId
private function _getId($name)
{
	return _sql_single('SELECT project_id FROM daportal_project'
			.', daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND title='$name'");
}


//ProjectModule::_isAdmin
private function _isAdmin($id)
{
	global $user_id;

	require_once('./system/user.php');
	if(_user_admin($user_id))
		return 1;
	return _sql_single('SELECT user_id'
			.' FROM daportal_content, daportal_project'
			.' WHERE daportal_content.content_id'
			.'=daportal_project.project_id'
			." AND project_id='$id'") == $user_id;
}


private function _isMember($id, $members = FALSE, $user = FALSE)
{
	global $user_id;

	require_once('./system/user.php');
	if(_user_admin($user_id))
		return 1;
	if($members == FALSE)
		$members = $this->_getMembers($id);
	if($user == FALSE)
		$user = $user_id;
	foreach($members as $m)
		if($m['id'] == $user)
			return 1;
	return 0;
}


private function _getMembers($id)
{
	$admin = _sql_array('SELECT daportal_user.user_id AS id'
			.', username FROM daportal_project, daportal_content'
			.', daportal_user WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND daportal_content.enabled='1'"
			." AND daportal_user.enabled='1'"
			." AND daportal_project.project_id='$id'");
	if(!is_array($admin))
		return array();
	$members = _sql_array('SELECT daportal_user.user_id AS id'
			.', username FROM daportal_project_user, daportal_user'
			.' WHERE daportal_project_user.user_id'
			.'=daportal_user.user_id AND enabled='."'1'"
			." AND daportal_project_user.project_id='$id'");
	if(!is_array($members))
		return $admin;
	return array_merge($admin, $members);
}


private function _getName($id)
{
	return _sql_single('SELECT title FROM daportal_project'
			.', daportal_content WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND project_id='$id'");
}


//ProjectModule::_timeline
private function _timeline($id, $cvsroot, $cpp = FALSE)
{
	require_once('./system/user.php');
	if(($cvsrep = _config_get('project', 'cvsroot')) == FALSE
			|| ($fp = fopen($cvsrep.'/CVSROOT/history', 'r'))
			== FALSE)
	{
		_error('Unable to open history file');
		return array();
	}
	$entries = array();
	$len = strlen($cvsroot);
	for($i = 0; ($line = fgets($fp)) != FALSE;)
	{
		$fields = explode('|', $line);
		if(strcmp('', $fields[4]) == 0)
			continue;
		if(strncmp($fields[3], $cvsroot, $len) != 0)
			continue;
		unset($event);
		unset($icon);
		switch($fields[0][0])
		{
			case 'A': $event = 'Add'; $icon = 'added';	break;
			case 'F': $event = 'Release';			break;
			case 'M': $event = 'Modify'; $icon = 'modified';break;
			case 'R': $event = 'Remove'; $icon = 'removed';	break;
		}
		if(!isset($event))
			continue;
		$icon = isset($icon) ? 'icons/48x48/cvs-'.$icon.'.png' : '';
		$name = substr($fields[3], $len ? $len + 1 : $len).'/'
			.$fields[5];
		$date = base_convert(substr($fields[0], 1, 9), 16, 10);
		$date = date('d/m/Y H:i', $date);
		if(($author = _user_id($fields[1])) != 0)
			$author = '<a href="'._html_link('project', 'list', '',
				'', 'user_id='._html_safe($author)).'">'
					._html_safe($fields[1]).'</a>';
		else
			$author = _html_safe($fields[1]);
		$entry = array('name' => _html_safe($name),
				'icon' => $icon, 'thumbnail' => $icon,
				'date' => _html_safe($date),
				'event' => _html_safe($event),
				'revision' => _html_safe($fields[4]),
				'author' => $author);
		if($id !== FALSE)
		{
			$entry['module'] = 'project';
			$entry['action'] = 'browse';
			$entry['id'] = $id;
			$entry['args'] = 'file='._html_safe($name).',v';
			$entry['revision'] = '<a href="'._html_link('project',
				'browse', $id, FALSE, 'file='._html_safe($name)
					.',v&amp;revision='
					.$entry['revision']).'">'
					.$entry['revision'].'</a>';
		}
		if($cpp !== FALSE && $i++ >= $cpp)
			array_shift($entries);
		$entries[] = $entry;
	}
	fclose($fp);
	$entries = array_reverse($entries);
	return $entries;
}


//functions
//ProjectModule::admin
protected function admin($args)
{
	global $user_id, $module_id;

	if(isset($args['id']))
		return $this->modify($args);
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title project">'._html_safe(PROJECTS_ADMINISTRATION)
			."</h1>\n");
	if(($configs = _config_list('project')))
	{
		print('<h2 class="title settings"/>'._html_safe(SETTINGS)
				."</h2>\n");
		$module = 'project';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title project">'._html_safe(PROJECT_LIST)
			."</h2>\n");
	$sql = 'SELECT content_id AS id, title AS name, username AS admin'
		.', daportal_content.enabled AS enabled'
		.', daportal_content.user_id AS user_id, synopsis, cvsroot'
		.' FROM daportal_content, daportal_user, daportal_project'
		.' WHERE daportal_content.user_id=daportal_user.user_id'
		.' AND daportal_content.content_id=daportal_project.project_id'
		.' ORDER BY name ASC';
	$res = _sql_array($sql);
	if(!is_array($res))
		return _error('Could not list projects');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['name'] = _html_safe($res[$i]['name']);
		$res[$i]['icon'] = 'icons/16x16/project.png';
		$res[$i]['thumbnail'] = 'icons/48x48/project.png';
		$res[$i]['admin'] = '<a href="'._html_link('project', 'list',
			'', '', 'user_id='.$res[$i]['user_id']).'">'
				._html_safe($res[$i]['admin']).'</a>';
		$res[$i]['synopsis'] = _html_safe($res[$i]['synopsis']);
		$res[$i]['module'] = 'project';
		$res[$i]['action'] = 'modify';
		$res[$i]['apply_module'] = 'project';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['enabled'] = ($res[$i]['enabled'] == TRUE)
			? 'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
			.$res[$i]['enabled'].'.png" alt="'
			.$res[$i]['enabled'].'"/>';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_PROJECT, 'class' => 'new',
			'link' => _module_link('project', 'new'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable', 'confirm' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable', 'confirm' => 'enable');
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => array('enabled' => ENABLED,
					'admin' => MANAGER,
					'synopsis' => DESCRIPTION,
					'cvsroot' => CVS_PATH),
				'toolbar' => $toolbar, 'view' => 'details',
				'module' => 'project', 'action' => 'admin'));
	print('<h2 class="title bug">'._html_safe(REPORT_LIST).'</h2>'."\n");
	$sql = 'SELECT daportal_bug.content_id AS id, bug_id, title'
		.', daportal_content.enabled AS enabled, name AS module'
		.', project_id, state'
		.' FROM daportal_bug, daportal_content, daportal_module'
		.' WHERE daportal_bug.content_id=daportal_content.content_id'
		.' AND daportal_content.module_id=daportal_module.module_id'
		.' ORDER BY bug_id DESC';
	$res = _sql_array($sql);
	if(!is_array($res))
		return _error('Could not list reports');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['thumbnail'] = 'icons/48x48/bug';
		switch($res[$i]['state'])
		{
			case 'New': $res[$i]['thumbnail'].='-new'; break;
			case 'Assigned': $res[$i]['thumbnail'].='-assigned';
				 break;
			case 'Closed': $res[$i]['thumbnail'].='-closed';
				break;
			case 'Fixed': case 'Implemented':
			       $res[$i]['thumbnail'].='-fixed'; break;
			default:
				break;
		}
		$res[$i]['thumbnail'].='.png';
		$res[$i]['icon'] = $res[$i]['thumbnail'];
		$res[$i]['action'] = 'bug_modify';
		$res[$i]['apply_module'] = 'project';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['name'] = _html_safe($res[$i]['title']);
		$res[$i]['args'] = 'bug_id='.$res[$i]['bug_id'];
		$res[$i]['enabled'] = ($res[$i]['enabled'] == TRUE)
			? 'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
			.$res[$i]['enabled'].'.png" alt="'
			.$res[$i]['enabled'].'"/>';
		$res[$i]['bug_id'] = '<a href="'._html_link('project',
			'bug_display', $res[$i]['id'], $res[$i]['title'])
				.'">#'._html_safe($res[$i]['bug_id']).'</a>';
		$res[$i]['project'] = $this->_getName($res[$i]['project_id']);
		$res[$i]['project'] = '<a href="'._html_link('module',
			'project', $res[$i]['project_id'], $res[$i]['project'])
				.'">'._html_safe($res[$i]['project']).'</a>';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_REPORT, 'class' => 'new',
			'link' => _module_link('project', 'bug_new'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'bug_disable', 'confirm' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'bug_enable', 'confirm' => 'enable');
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'bug_delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'view' => 'details', 'toolbar' => $toolbar,
				'class' => array('enabled' => ENABLED,
					'bug_id' => 'Bug #',
					'project' => PROJECT),
				'module' => 'project', 'action' => 'admin'));
}


//ProjectModule::browse
protected function browse($args)
{
	if(!isset($args['id']))
		return _error(INVALID_PROJECT);
	$sql = 'SELECT title, cvsroot FROM daportal_content, daportal_project'
		." WHERE project_id='".$args['id']."'"
		.' AND daportal_content.content_id=daportal_project.project_id'
		." AND enabled='1'";
	$project = _sql_array($sql);
	if(!is_array($project) || count($project) != 1
			|| !($cvsrep = _config_get('project', 'cvsroot')))
		return _error(INVALID_PROJECT);
	$cvsrep.='/';
	$project = $project[0];
	$this->_toolbar($args['id']);
	if(strlen($project['cvsroot']) == 0)
	{
		print('<h1 class="title project">'._html_safe($project['title'])
				.' CVS</h1>'."\n");
		return _info(NO_CVS_REPOSITORY, 1);
	}
	if(preg_match('/^[a-zA-Z0-9. \/]+$/', $project['cvsroot']) != 1
			|| strpos($project['cvsroot'], '..') !== FALSE)
		return _error('Invalid CVSROOT', 1);
	require('./modules/project/browse.php');
	if(!isset($args['file']))
		return _browse_dir($args['id'], $project['title'], $cvsrep,
				$project['cvsroot'], '');
	$file = stripslashes($args['file']);
	if(preg_match('/^[a-zA-Z0-9.,-_ \/]+$/', $file) != 1
			|| strpos($file, '..') !== FALSE)
		return _error('Invalid path', 1);
	$filename = $cvsrep.$project['cvsroot'].'/'.$file;
	if(is_dir($filename))
		return _browse_dir($args['id'], $project['title'], $cvsrep,
				$project['cvsroot'], $file);
	else if(file_exists($filename))
	{
		if(isset($args['revision']))
			return _browse_file_revision($args['id'],
					$project['title'], $cvsrep,
					$project['cvsroot'],
					$file, $args['revision'],
					isset($args['download'])
					&& $args['download'] == 1);
		return _browse_file($args['id'], $project['title'], $cvsrep,
				$project['cvsroot'], $file);
	}
	_error('Invalid filename: "'.$filename.'"', 0);
	_browse_dir($args['id'], $project['title'], $cvsrep,
			$project['cvsroot'], '');
}


//ProjectModule::bugDelete
protected function bugDelete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	$id = $args['id'];
	if(($bugid = _sql_single('SELECT bug_id FROM daportal_bug'
					." WHERE content_id='$id'")) === FALSE)
		return _error(INVALID_ARGUMENT);
	_sql_single('DELETE FROM daportal_bug_reply WHERE bug_id='."'$bugid'");
	_sql_single('DELETE FROM daportal_bug WHERE content_id='."'$id'");
}


//ProjectModule::bugDisable
protected function bugDisable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	$id = $args['id'];
	if(($bugid = _sql_single('SELECT bug_id FROM daportal_bug'
					." WHERE content_id='$id'")) === FALSE)
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	_content_disable($id);
}


//ProjectModule::bugDisplay
protected function bugDisplay($args)
{
	global $user_id;

	if(!isset($args['id']) || !is_numeric($args['id'])
			|| !isset($args['bug_id'])
			|| !is_numeric($args['bug_id']))
		return _error(INVALID_ARGUMENT);
	$sql = 'SELECT bug.content_id AS id'
		.', daportal_bug.project_id AS project_id'
		.', daportal_user.user_id AS user_id'
		.', daportal_bug.bug_id AS bug_id, bug.timestamp AS timestamp'
		.', bug.title AS title, bug.content AS content'
		.', prj.title AS project, username, state, type, priority'
		.', assigned AS assigned_id'
		.' FROM daportal_bug, daportal_content bug'
		.', daportal_content prj, daportal_user, daportal_project'
		.' WHERE daportal_bug.content_id=bug.content_id'
		.' AND daportal_bug.project_id=daportal_project.project_id'
		.' AND daportal_project.project_id=prj.content_id'
		.' AND bug.enabled='."'1'".' AND prj.enabled='."'1'"
		.' AND bug.user_id=daportal_user.user_id'
		.' AND daportal_project.project_id=daportal_bug.project_id'
		." AND daportal_bug.content_id='".$args['id']."'"
		." AND bug_id='".$args['bug_id']."'";
	$bug = _sql_array($sql);
	if(!is_array($bug) || count($bug) != 1)
		return _error('Unable to display bug');
	$bug = $bug[0];
	$this->_toolbar($bug['project_id']);
	$title = 'Bug #'.$bug['bug_id'].': '.$bug['title'];
	require_once('./system/user.php');
	$bug['assigned'] = isset($bug['assigned_id']) 
		? _user_name($bug['assigned_id']) : '';
	$admin = _user_admin($user_id) ? 1 : 0;
	$bug['date'] = _sql_date($bug['timestamp']);
	include('./modules/project/bug_display.tpl');
	$sql = 'SELECT bug_reply_id AS id, title, content'
		.', daportal_content.content_id AS content_id'
		.', timestamp AS date, state, type, priority'
		.', daportal_user.user_id AS user_id, username'
		.', daportal_bug_reply.assigned AS assigned_id'
		.' FROM daportal_bug_reply, daportal_content, daportal_user'
		.' WHERE daportal_bug_reply.content_id'
		.'=daportal_content.content_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		." AND daportal_content.enabled='1'"
		." AND (daportal_user.enabled='1' OR daportal_user.user_id='0')"
		." AND daportal_bug_reply.bug_id='".$bug['bug_id']."'"
		.' ORDER BY timestamp ASC';
	$replies = _sql_array($sql);
	if(!is_array($replies))
		return _error('Unable to display bug feedback');
	foreach($replies as $reply)
	{
		$reply['date'] = _sql_date($reply['date']);
		if(isset($reply['assigned_id']))
			$reply['assigned'] = _user_name($reply['assigned_id']);
		include('./modules/project/bug_reply_display.tpl');
	}
}


//ProjectModule::bugEnable
protected function bugEnable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	$id = $args['id'];
	if(($bugid = _sql_single('SELECT bug_id FROM daportal_bug'
					." WHERE content_id='$id'")) === FALSE)
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	_content_enable($id);
}


//ProjectModule::bugInsert
protected function bugInsert($args)
{
	global $user_id, $user_name;

	if(($user_id == 0 && _config_get('project', 'anonymous_bug_reports')
				!= TRUE)
			|| $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(!isset($args['project_id']) || !is_numeric($args['project_id'])
			|| !isset($args['type']) || !isset($args['priority']))
		return _error(INVALID_ARGUMENT); //FIXME return to form
	if(isset($args['preview']))
	{
		$this->_toolbar($args['project_id']);
		$bug = array();
		if(($bug['project'] = $this->_getName($args['project_id']))
				== FALSE)
			return _error(INVALID_ARGUMENT);
		$bug['project_id'] = stripslashes($args['project_id']);
		$project_id = $bug['project_id'];
		$bug['title'] = stripslashes($args['title']);
		$title = PREVIEW.': '.REPORT_BUG_FOR.' '
			._html_safe($bug['project']).': '.$bug['title'];
		$bug['date'] = _sql_date();
		$bug['user_id'] = $user_id;
		$bug['username'] = $user_name;
		$bug['state'] = 'New';
		$bug['type'] = stripslashes($args['type']);
		$bug['priority'] = stripslashes($args['priority']);
		$bug['content'] = stripslashes($args['content']);
		include('./modules/project/bug_display.tpl');
		unset($title);
		include('./modules/project/bug_update.tpl');
		return 0;
	}
	require_once('./system/content.php');
	require_once('./system/user.php');
	$enable = 0;
	if(_user_admin($user_id) || $this->_isMember($args['project_id']))
		$enable = 1;
	if(($id = _content_insert($args['title'], $args['content'], $enable))
			== FALSE)
		return _error('Unable to insert bug content');
	if(_sql_query('INSERT INTO daportal_bug (content_id, project_id'
			.', state, type, priority) VALUES'
			." ('$id'".", '".$args['project_id']."'"
			.", 'New'".", '".$args['type']."'"
			.", '".$args['priority']."'".")") === FALSE)
	{
		_content_delete($id);
		return _error('Unable to insert bug');
	}
	$bugid = _sql_single('SELECT bug_id FROM daportal_bug WHERE content_id='
			."'$id'");
	//send mail
	$to = _sql_array('SELECT username, email, title'
			.' FROM daportal_project, daportal_content'
			.', daportal_user'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id'
			.'=daportal_user.user_id'
			." AND project_id='".$args['project_id']."'");
	$members = _sql_array('SELECT username, email'
			.' FROM daportal_project_user, daportal_user'
			.' WHERE daportal_project_user.user_id'
			.'=daportal_user.user_id'
			." AND project_id='".$args['project_id']."'"
			." AND enabled='1'");
	if(!is_array($to) || !is_array($members))
		_error('Could not list members', 0);
	else
	{
		$project = $to[0]['title'];
		$to = $to[0]['username'].' <'.$to[0]['email'].'>';
		foreach($members as $m)
			$to.=', '.$m['username'].' <'.$m['email'].'>';
		$title = '[Bug submission] '.$project.'/#'.$bugid.': '
			.stripslashes($args['title']);
		$content = 'Project: '.$project."\n".'From: '.$user_name."\n"
			.'State: New'."\n".'Type: '.$args['type']."\n"
			.'Priority: '.$args['priority']."\n\n"
			.stripslashes($args['content']);
		$content = wordwrap($content, 72)."\n\n"
			._module_link_full('project', 'bug_display', $id,
					stripslashes($args['title']),
					array('bug_id' => $bugid));
		require_once('./system/mail.php');
		_mail('Administration Team', $to, $title, $content);
	}
	if($enable)
		return $this->bugDisplay(array('id' => $id,
					'bug_id' => $bugid));
	include('./modules/project/bug_posted.tpl');
}


//ProjectModule::bugList
protected function bugList($args)
{
	$title = BUG_REPORTS;
	if(isset($args['id']))
	{
		if(($args['project'] = $this->_getName($args['id'])) != FALSE)
		{
			$project_id = $args['id'];
			$project = $args['project'];
		}
	}
	else if(isset($args['project_id']))
	{
		if(($args['project'] = $this->_getName($args['project_id']))
				!= FALSE)
		{
			$project_id = $args['project_id'];
			$project = $args['project'];
		}
	}
	else if(isset($args['project']))
	{
		if(($project_id = $this->_getId($args['project'])) != FALSE)
			$project = stripslashes($args['project']);
	}
	if(isset($args['user_id']))
	{
		if(($args['username'] = _sql_single('SELECT username'
				.' FROM daportal_user'
				." WHERE user_id='".$args['user_id']."'"))
				!= FALSE)
			$username = $args['username'];
	}
	else if(isset($args['username']))
	{
		if(($args['username'] = _sql_single('SELECT username'
				.' FROM daportal_user'
				." WHERE username='".$args['username']."'"))
				!= FALSE)
			$username = $args['username'];
	}
	if(isset($project) && isset($project_id))
	{
		$this->_toolbar($project_id);
		$title.=_FOR_.$project;
	}
	if(isset($username))
		$title.=_BY_.$username;
	print('<h1 class="title bug">'._html_safe($title).'</h1>'."\n");
	$where = '';
	if(isset($args['project']) && strlen($args['project']))
		$where.=" AND prj.title='".$args['project']."'";
	if(isset($args['username']) && strlen($args['username']))
		$where.=" AND daportal_user.username='".$args['username']."'";
	if(isset($args['state']) && strlen($args['state']))
		$where.=" AND daportal_bug.state='".$args['state']."'";
	if(isset($args['type']) && strlen($args['type']))
		$where.=" AND daportal_bug.type='".$args['type']."'";
	if(isset($args['priority']) && strlen($args['priority']))
		$where.=" AND daportal_bug.priority='".$args['priority']."'";
	include('./modules/project/bug_list_filter.tpl');
	$order = ' ORDER BY ';
	$sort = isset($args['sort']) ? $args['sort'] : 'nb';
	switch($sort)
	{
		case 'name':	$order.='name DESC';	break;
		case 'project':	$order.='project DESC';	break;
		case 'date':	$order.='date DESC';	break;
		case 'username':$order.='username DESC';break;
		case 'state':	$order.='state DESC';	break;
		case 'type':	$order.='type DESC';	break;
		case 'priority':$order.='priority DESC';break;
		case 'nb':
		default:	$sort = 'nb';
				$order.='bug_id DESC';	break;
	}
	$sql = 'SELECT bug.content_id AS id, bug_id, bug.timestamp AS date'
		.', synopsis, bug.title AS name, username, prj.title AS project'
		.', daportal_bug.project_id AS project_id, state, type'
		.', priority FROM daportal_content bug, daportal_bug'
		.', daportal_user, daportal_content prj, daportal_project'
		.' WHERE bug.content_id=daportal_bug.content_id'
		.' AND prj.content_id=daportal_project.project_id'
		.' AND bug.enabled='."'1'".' AND prj.enabled='."'1'"
		.' AND bug.user_id=daportal_user.user_id'
		.' AND daportal_project.project_id=daportal_bug.project_id';
	$bugs = _sql_array($sql.$where.$order);
	if(!is_array($bugs))
		return _error('Unable to list bugs');
	for($i = 0, $cnt = count($bugs); $i < $cnt; $i++)
	{
		$bugs[$i]['thumbnail'] = 'icons/48x48/bug';
		switch($bugs[$i]['state'])
		{
			case 'New': $bugs[$i]['thumbnail'].='-new'; break;
			case 'Assigned': $bugs[$i]['thumbnail'].='-assigned';
				 break;
			case 'Closed': $bugs[$i]['thumbnail'].='-closed';
				break;
			case 'Fixed': case 'Implemented':
			       $bugs[$i]['thumbnail'].='-fixed'; break;
			default:
				break;
		}
		$bugs[$i]['thumbnail'].='.png';
		$bugs[$i]['icon'] = $bugs[$i]['thumbnail'];
		$bugs[$i]['module'] = 'project';
		$bugs[$i]['action'] = 'bug_display';
		$bugs[$i]['title'] = $bugs[$i]['name'];
		$bugs[$i]['name'] = _html_safe($bugs[$i]['name']);
		$bugs[$i]['args'] = 'bug_id='.$bugs[$i]['bug_id'];
		$bugs[$i]['nb'] = '<a href="'._html_link('project',
			'bug_display', $bugs[$i]['id'], $bugs[$i]['title'],
			'bug_id='.$bugs[$i]['bug_id'])
				.'">#'.$bugs[$i]['bug_id'].'</a>';
		$bugs[$i]['project'] = '<a href="'._html_link('project',
			'bug_list', $bugs[$i]['project_id'],
			$bugs[$i]['project']).'">'
				._html_safe($bugs[$i]['project']).'</a>';
		$bugs[$i]['date'] = date('d/m/Y H:i', strtotime(substr(
						$bugs[$i]['date'], 0, 19)));
	}
	$toolbar = array();
	$link = _module_link('project', 'bug_new', FALSE, FALSE,
			(isset($project_id) ? 'project_id='.$project_id : ''));
	$toolbar[] = array('title' => REPORT_A_BUG, 'class' => 'bug',
			'link' => $link);
	_module('explorer', 'browse_trusted', array('entries' => $bugs,
			'class' => array('nb' => '#', 'project' => PROJECT,
					'date' => DATE, 'state' => STATE,
					'type' => TYPE, 'priority' => PRIORITY),
			'module' => 'project', 'action' => 'bug_list',
			'sort' => $sort,
			//FIXME should set args according to filters
			'view' => 'details', 'toolbar' => $toolbar));
}


//ProjectModule::bugModify
protected function bugModify($args)
{
	/* FIXME we won't use only bug_update.tpl here but propose a series of
	 * logical actions (choose different project, assign to a user, etc) */
	/* FIXME for instance create a user_assign function with current number
	 * of affected bugs etc */
	/* FIXME maybe also restore original project listing function and
	 * create a project_assign one with current number of (open) bugs etc */
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!isset($args['bug_id']))
		return _error(INVALID_ARGUMENT);
	$bug = _sql_array('SELECT daportal_bug.content_id AS id, bug_id, title'
			.', project_id, content, state, type, priority'
			.', assigned AS assigned_id'
			.' FROM daportal_bug, daportal_content'
			.' WHERE daportal_bug.content_id'
			.'=daportal_content.content_id'
			." AND daportal_bug.content_id='".$args['id']."'"
			." AND bug_id='".$args['bug_id']."'");
	if(!is_array($bug) || count($bug) != 1)
		return _error(INVALID_ARGUMENT);
	$bug = $bug[0];
	$title = MODIFICATION_OF_BUG_HASH.$bug['bug_id'].': '.$bug['title'];
	$members = $this->_getMembers($bug['project_id']);
	$project_id = $bug['project_id'];
	include('./modules/project/bug_update.tpl');
}


//ProjectModule::bugNew
protected function bugNew($args)
{
	if(!isset($args['project_id']) || !is_numeric($args['project_id'])
			|| !($project = $this->_getName($args['project_id'])))
		return $this->_list(array('action' => 'bug_new'));
	$title = REPORT_BUG_FOR.' '.$project;
	$project_id = $args['project_id'];
	include('./modules/project/bug_update.tpl');
}


//ProjectModule::bugReply
protected function bugReply($args)
{
	global $user_id, $user_name;

	if($user_id == 0 && _config_get('project', 'anonymous_bug_replies')
			!= TRUE)
		return _error(PERMISSION_DENIED);
	$sql = 'SELECT bug.content_id AS id'
		.', daportal_bug.project_id AS project_id'
		.', daportal_user.user_id AS user_id'
		.', daportal_bug.bug_id AS bug_id'
		.', bug.timestamp AS timestamp, bug.title AS title'
		.', bug.content AS content, prj.title AS project, username'
		.', state, type, priority, assigned AS assigned_id'
		.' FROM daportal_content bug, daportal_bug, daportal_user'
		.', daportal_content prj, daportal_project'
		.' WHERE bug.content_id=daportal_bug.content_id'
		.' AND daportal_bug.project_id=daportal_project.project_id'
		.' AND prj.content_id=daportal_project.project_id'
		." AND bug.enabled='1' AND prj.enabled='1'"
		.' AND bug.user_id=daportal_user.user_id'
		.' AND daportal_project.project_id=daportal_bug.project_id'
		." AND daportal_bug.content_id='".$args['id']."'";
	$bug = _sql_array($sql);
	if(!is_array($bug) || count($bug) != 1)
		return _error('Unable to display bug');
	$bug = $bug[0];
	$bug['date'] = _sql_date($bug['timestamp']);
	require_once('./system/user.php');
	if(isset($bug['assigned_id']))
		$bug['assigned'] = _user_name($bug['assigned_id']);
	$title = REPLY_TO_BUG.' #'.$bug['bug_id'].': '.$bug['title'];
	$admin = _user_admin($user_id) ? 1 : 0;
	$members = $this->_getMembers($bug['project_id']);
	$member = $this->_isMember($bug['project_id'], $members, $user_id);
	if(isset($args['preview']))
	{
		include('./modules/project/bug_display.tpl');
		$reply = array('title' => stripslashes($args['title']),
				'content' => stripslashes($args['content']),
				'date' => strftime(DATE_FORMAT),
				'user_id' => $user_id,
				'username' => $user_name);
		//FIXME check state, type, priority and assigned are coherent
		if(isset($args['state']) && strlen($args['state']))
			$reply['state'] = stripslashes($args['state']);
		if(isset($args['type']) && strlen($args['type']))
			$reply['type'] = stripslashes($args['type']);
		if(isset($args['priority']) && strlen($args['priority']))
			$reply['priority'] = stripslashes($args['priority']);
		if(isset($args['assigned_id'])
				&& is_numeric($args['assigned_id']))
		{
			$reply['assigned_id'] = $args['assigned_id'];
			$reply['assigned'] = _user_name($args['assigned_id']);
		}
		include('./modules/project/bug_reply_display.tpl');
		return include('./modules/project/bug_reply_update.tpl');
	}
	if(!isset($args['submit']))
	{
		include('./modules/project/bug_display.tpl');
		$reply = array('title' => 'Re: '.$bug['title']);
		return include('./modules/project/bug_reply_update.tpl');
	}
	$this->bugReplyInsert($args);
}


//ProjectModule::bugReplyInsert
protected function bugReplyInsert($args)
{
	global $user_id;

	if(($user_id == 0 && _config_get('project', 'anonymous_bug_replies')
				!= TRUE)
			|| $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(!isset($args['id']) || !isset($args['bug_id']))
		return _error(INVALID_ARGUMENT);
	require_once('./system/user.php');
	$enabled = ($admin = _user_admin($user_id)) ? ''
		: " AND enabled='1'";
	$content_id = $args['id'];
	$bug_id = $args['bug_id'];
	if(($project_id = _sql_single('SELECT project_id FROM daportal_bug'
					.', daportal_content'
					.' WHERE daportal_bug.content_id'
					.'=daportal_content.content_id'.$enabled
					.' AND daportal_content.content_id='
					."'$content_id' AND bug_id='$bug_id'"))
			== FALSE)
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], $args['content'], 1))
			== FALSE)
		return _error('Could not insert bug reply');
	$fields = '';
	$values = '';
	$update = '';
	$status = '';
	$from = 'From: '._user_name($user_id)."\n";
	$to = "\n";
	if($admin || $this->_isMember($project_id))
	{
		if(isset($args['assigned_id'])
				&& is_numeric($args['assigned_id'])
				&& $this->_isMember($project_id, FALSE,
					$args['assigned_id']))
		{
			if(!isset($args['state']) || $args['state'] == '')
				$args['state'] = 'Assigned';
			$fields.=', assigned';
			$values.=", '".$args['assigned_id']."'";
			$update.=", assigned='".$args['assigned_id']."'";
			$to = ' (to '._user_name($args['assigned_id']).")\n";
		}
		if(isset($args['state']) && strlen($args['state']))
		{
			$fields.=', state';
			$values.=", '".$args['state']."'";
			$update.=", state='".$args['state']."'";
			$status.='State: '.stripslashes($args['state']).$to;
		}
		if(isset($args['type']) && strlen($args['type']))
		{
			$fields.=', type';
			$values.=", '".$args['type']."'";
			$update.=", type='".$args['type']."'";
			$status.='Type: '.stripslashes($args['type'])."\n";
		}
		if(isset($args['priority']) && strlen($args['priority']))
		{
			$fields.=', priority';
			$values.=", '".$args['priority']."'";
			$update.=", priority='".$args['priority']."'";
			$status.='Priority: '.stripslashes($args['priority'])
				."\n";
		}
	}
	if(_sql_query('INSERT INTO daportal_bug_reply'
			.' (content_id, bug_id'.$fields.') VALUES '
			." ('$id', '".$args['bug_id']."'".$values.')') == FALSE)
		return _error(INVALID_ARGUMENT);
	if(strlen($update)) //should not fail
		_sql_query('UPDATE daportal_bug SET'
				." bug_id='".$args['bug_id']."'".$update
				." WHERE bug_id='".$args['bug_id']."'");
	$this->bugDisplay(array('id' => $args['id'],
				'bug_id' => $args['bug_id']));
	//send mail
	$ba = _sql_array('SELECT username, email' //bug author
			.' FROM daportal_bug, daportal_content, daportal_user'
			.' WHERE daportal_bug.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id'
			.'=daportal_user.user_id'
			." AND bug_id='$bug_id'");
	$pa = _sql_array('SELECT username, email' //project admin
			.' FROM daportal_project, daportal_content'
			.', daportal_user'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND project_id='$project_id'");
	$assigned = _sql_array('SELECT username, email' //assigned member
			.' FROM daportal_bug, daportal_user'
			.' WHERE daportal_bug.assigned=daportal_user.user_id'
			." AND bug_id='$bug_id'");
	$members = _sql_array('SELECT username, email' //all members
			.' FROM daportal_project_user, daportal_user'
			.' WHERE daportal_project_user.user_id'
			.'=daportal_user.user_id'
			." AND project_id='$project_id' AND enabled='1'");
	if(!is_array($ba) || !is_array($pa) || !is_array($assigned)
			|| !is_array($members))
		return _error('Could not list addresses for mailing', 0);
	$array = count($assigned) == 1 ? array_merge($ba, $pa, $assigned)
		: array_merge($ba, $pa, $members);
	if(count($array) == 0)
		return _error('No recipients for mailing', 0);
	$rcpt = array();
	foreach($array as $a)
		$rcpt[$a['username']] = $a['email'];
	$keys = array_keys($rcpt);
	$to = $keys[0].' <'.$rcpt[$keys[0]].'>';
	$cnt = count($keys);
	for($i = 1; $i < $cnt; $i++)
		$to.=', '.$keys[$i].' <'.$rcpt[$keys[$i]].'>';
	$project = $this->_getName($project_id);
	$title = '[Bug reply] '.$project.'/#'.$bug_id.': '
		.stripslashes($args['title']);
	$content = 'Project: '.$project."\n".$from.$status."\n"
		.stripslashes($args['content'])."\n\n"
		._module_link_full('project', 'bug_display', $content_id, FALSE,
				array('bug_id' => $bug_id));
	require_once('./system/mail.php');
	_mail('Administration Team', $to, $title, wordwrap($content, 72));
}


//ProjectModule::bugReplyModify
protected function bugReplyModify($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$admin = 1;
	$sql = 'SELECT bug_reply_id AS id, daportal_bug.bug_id AS bug_id'
		.', title, content, daportal_bug_reply.state AS state'
		.', daportal_bug_reply.type AS type'
		.', daportal_bug_reply.priority AS priority'
		.', daportal_user.user_id AS user_id, username'
		.', daportal_bug_reply.assigned AS assigned_id, project_id'
		.' FROM daportal_bug_reply, daportal_content'
		.', daportal_user, daportal_bug'
		.' WHERE daportal_bug_reply.content_id'
		.'=daportal_content.content_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		.' AND daportal_bug_reply.bug_id=daportal_bug.bug_id'
		." AND daportal_content.enabled='1'"
		." AND (daportal_user.enabled='1'"
		." OR daportal_user.user_id='0')"
		." AND bug_reply_id='".$args['id']."'";
	$reply = _sql_array($sql);
	if(!is_array($reply) || count($reply) != 1)
		return _error('Unable to modify bug feedback');
	$reply = $reply[0];
	print('<h1 class="title bug">'
			._html_safe(MODIFICATION_OF_REPLY_TO_BUG_HASH
				.$reply['bug_id'].': '.$reply['title'])
			.'</h1>'."\n");
	if(isset($reply['assigned_id']))
		$reply['assigned'] = _sql_single('SELECT username'
				.' FROM daportal_user WHERE enabled='."'1'"
				." AND user_id='".$reply['assigned_id']."'");
	$members = $this->_getMembers($reply['project_id']);
	include('./modules/project/bug_reply_update.tpl');
}


//ProjectModule::bugReplyUpdate
protected function bugReplyUpdate($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT content_id FROM daportal_bug_reply'
			." WHERE bug_reply_id='".$args['id']."'")) === FALSE)
		return _error(INVALID_ARGUMENT);
	if(($bug_id = _sql_single('SELECT bug_id FROM daportal_bug_reply'
			." WHERE bug_reply_id='".$args['id']."'")) === FALSE)
		return _error(INVALID_ARGUMENT);
	if(($content_id = _sql_single('SELECT content_id FROM daportal_bug'
			." WHERE bug_id='$bug_id'")) === FALSE)
		return _error(INVALID_ARGUMENT);
	if(($project_id = _sql_single('SELECT project_id FROM daportal_bug'
			." WHERE bug_id='$bug_id'")) === FALSE)
		return _error(INVALID_ARGUMENT);
	//XXX use _content_update()
	_sql_query('UPDATE daportal_content SET title='."'".$args['title']."'"
			.", content='".$args['content']."'"
			." WHERE content_id='$id'");
	$sql = ' state='.(isset($args['state']) && strlen($args['state'])
			? "'".$args['state']."'" : 'NULL');
	$sql.=', type='.(isset($args['type']) && strlen($args['type'])
			? "'".$args['type']."'" : 'NULL');
	$sql.=', priority='.(isset($args['priority'])
			&& strlen($args['priority']) ? "'".$args['priority']."'"
			: 'NULL');
	$sql.=', assigned='.(isset($args['assigned_id'])
			&& is_numeric($args['assigned_id'])
			&& $this->_isMember($project_id, FALSE,
				$args['assigned_id']) ? "'".$args['assigned_id']
			."'" : 'NULL');
	_sql_query('UPDATE daportal_bug_reply SET'.$sql
			." WHERE bug_reply_id='".$args['id']."'");
	$this->bugDisplay(array('id' => $content_id, 'bug_id' => $bug_id));
}


//ProjectModule::bugUpdate
protected function bugUpdate($args)
{
	global $user_id, $module_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$id = $args['id'];
	$bug_id = $args['bug_id'];
	if(($project_id = _sql_single('SELECT project_id FROM daportal_bug'
					." WHERE bug_id='$bug_id'")) === FALSE)
		return _error(INVALID_ARGUMENT);
	if(_sql_single('SELECT content_id FROM daportal_bug'
				." WHERE content_id='$id'"
				." AND bug_id='$bug_id'") != $id)
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	_content_update($id, $args['title'], $args['content']);
	$assigned = ', assigned='.(isset($args['assigned_id'])
			&& is_numeric($args['assigned_id'])
			&& $this->_isMember($project_id, FALSE,
				$args['assigned_id']) ? "'".$args['assigned_id']
			."'" : 'NULL');
	_sql_query('UPDATE daportal_bug SET state='."'".$args['state']."'"
			.", type='".$args['type']."'"
			.", priority='".$args['priority']."'".$assigned
			." WHERE content_id='$id' AND  bug_id='$bug_id'");
	$this->bugDisplay(array('id' => $id, 'bug_id' => $bug_id));
}


//ProjectModule::configUpdate
protected function configUpdate($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return $this->admin(array());
}


//ProjectModule::_default
protected function _default($args)
{
	if(isset($args['id']))
	{
		$id = $args['id'];
		if(_sql_single('SELECT content_id FROM daportal_bug WHERE'
					." content_id='$id'") == $id)
			return $this->bugDisplay($args);
		return $this->display($args);
	}
	if(isset($args['user_id']))
		return $this->_list($args);
	$project_cnt = _sql_single('SELECT COUNT(*) FROM daportal_project'
			.', daportal_content WHERE daportal_project.project_id'
			."=daportal_content.content_id AND enabled='1'");
	$cols = array('state' => STATE, 'type' => TYPE, 'priority' => PRIORITY);
	include('./modules/project/default.tpl');
}


//ProjectModule::_delete
protected function _delete($args)
	/* FIXME should determine if it's a project/bug/bug_reply... */
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT project_id FROM daportal_project'
			." WHERE project_id='".$args['id']."'")) === FALSE)
		return _error(INVALID_PROJECT);
	_sql_query('DELETE FROM daportal_project WHERE project_id='."'$id'");
	require_once('./system/content.php');
	_content_delete($id);
}


//ProjectModule::disable
protected function disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT project_id FROM daportal_project'
			." WHERE project_id='".$args['id']."'")) === FALSE)
		return _error(INVALID_PROJECT);
	require_once('./system/content.php');
	_content_disable($id);
	if($args['display'] != 0)
		$this->display(array('id' => $id));
}


//ProjectModule::display
protected function display($args)
{
	global $user_id;

	if(!isset($args['id']))
		return $this->_default($args);
	require_once('./system/user.php');
	$project = _sql_array('SELECT project_id AS id, title'
			.', content AS description, synopsis'
			.', daportal_content.enabled AS enabled'
			.', daportal_content.user_id AS user_id, username'
			.' FROM daportal_content, daportal_project'
			.', daportal_user'
			." WHERE content_id='".$args['id']."'"
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' AND daportal_content.content_id'
			.'=daportal_project.project_id');
	if(!is_array($project) || count($project) != 1)
		return _error('Unable to display project');
	$project = $project[0];
	//FIXME or the user is the administrator of this project
	$admin = _user_admin($user_id);
	$enabled = ($project['enabled'] == TRUE);
	if($enabled == 0 && !$admin)
	{
		return include('./modules/project/project_submitted.tpl');
	}
	$title = $project['title'];
	$this->_toolbar($args['id']);
	include('./modules/project/project_display.tpl');
	/* list members */
	$members = $this->_getMembers($project['id']);
	for($i = 0, $cnt = count($members); $i < $cnt; $i++)
	{
		$members[$i]['name'] = $members[$i]['username'];
		$members[$i]['icon'] = 'icons/16x16/user.png';
		$members[$i]['thumbnail'] = 'icons/48x48/user.png';
		$members[$i]['module'] = 'user';
		$members[$i]['action'] = 'default';
		$members[$i]['apply_module'] = 'project';
		$members[$i]['apply_id'] = $members[$i]['id'];
		$members[$i]['apply_args'] = 'project_id='.$project['id'];
		/* XXX assumes the first user is admin */
		$members[$i]['admin'] = ($i == 0)
			? '<img src="icons/16x16/enabled.png" alt="yes"/>'
			: '<img src="icons/16x16/disabled.png" alt="no"/>';
	}
	print('<h2 class="title members">'._html_safe(MEMBERS).'</h2>'."\n");
	$explorer = array('view' => 'details', 'entries' => $members,
			'class' => array('admin' => MANAGER),
			'module' => 'project', 'action' => 'display',
			'id' => $project['id']);
	$toolbar = array();
	$toolbar[] = array('title' => 'Add member(s)', 'class' => 'new',
			'link' => _module_link('project', 'member_add',
				$project['id']));
	$toolbar[] = array();
	$toolbar[] = array('title' => 'Delete member(s)', 'class' => 'remove',
			'action' => 'member_delete', 'confirm' => DELETE);
	if($admin)
		$explorer['toolbar'] = $toolbar;
	_module('explorer', 'browse_trusted', $explorer);
}


//ProjectModule::download
protected function download($args)
{
	global $user_id;

	if(!isset($args['id']) || !is_numeric($args['id']))
		return _error(INVALID_PROJECT);
	$category_id = _module_id('category');
	$download_id = _module_id('download');
	if($category_id == 0 || $download_id == 0)
		return _error('Both category and download modules must be'
				.' installed');
	$project = _sql_array('SELECT project_id AS id, title AS name, synopsis'
			.', cvsroot'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND daportal_content.enabled='1'"
			." AND project_id='".$args['id']."'");
	if(!is_array($project) || count($project) != 1)
		return _error(INVALID_PROJECT);
	$project = $project[0];
	$this->_toolbar($project['id']);
	print('<h1 class="title project">'._html_safe($project['name']).': '
		._html_safe(FILES).'</h1>'."\n");
	require_once('./system/user.php');
	if(_user_admin($user_id) || $this->_isMember($project['id']))
		print('<p><a href="'._html_link('project', 'download_insert',
					$project['id'])
				.'"><span class="icon download"></span>'
				.NEW_RELEASE.'</a>'
				.'<span class="middot">&middot;</span>'
				.'<a href="'._html_link('project',
					'screenshot_insert', $project['id'])
				.'"><span class="icon screenshot"></span>'
				.NEW_SCREENSHOT.'</a>'."</p>\n");
	require_once('./system/mime.php');
	/* FIXME factorize code */
	$sql = 'SELECT daportal_content.content_id AS id, mode'
		.', daportal_content.timestamp AS date'
		.', daportal_content.title AS name'
		.', daportal_content.user_id AS user_id, username'
		.' FROM daportal_download, daportal_content, daportal_user'
		.', daportal_category_content cc2, daportal_content dc2'
		.', daportal_category_content cc3, daportal_content dc3'
		.' WHERE daportal_download.content_id'
		.'=daportal_content.content_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		.' AND cc2.content_id=daportal_content.content_id'
		.' AND cc3.content_id=daportal_content.content_id'
		.' AND cc2.category_id=dc2.content_id'
		.' AND cc3.category_id=dc3.content_id'
		." AND dc2.title='".$project['name']."'"
		." AND daportal_content.enabled='1'"
		." AND daportal_user.enabled='1' AND daportal_user.admin='1'"
		." AND dc2.enabled='1' AND dc3.enabled='1'";
	/* screenshots */
	/* FIXME project members should be trusted too */
	$files = _sql_array($sql." AND dc3.title='screenshot'");
	if(is_array($files) && ($cnt = count($files)) > 0)
	{
		print('<h2>'._html_safe(SCREENSHOTS).'</h2>'."\n");
		for($i = 0; $i < $cnt; $i++)
		{
			$files[$i]['module'] = 'download';
			$files[$i]['action'] = 'download';
			$mime = _mime_from_ext($files[$i]['name']);
			if(strncmp($mime, 'image/', 6) == 0)
			{
				$files[$i]['icon'] = 'icons/16x16/mime/'
					.(is_readable('icons/16x16/mime/'
								.$mime.'.png'))
						? $mime.'.png' : 'default.png';
				$files[$i]['thumbnail'] = _html_link('download',
						'download', $files[$i]['id']);
			}
			else if($files[$i]['mode'] & $this->S_IFDIR
					== $this->S_IFDIR)
			{
				$files[$i]['icon'] = 'icons/16x16'
					.'/mime/folder.png';
				$files[$i]['thumbnail'] = 'icons/48x48'
					.'/mime/folder.png';
			}
			else
			{
				if(is_readable('icons/48x48/mime/'.$mime.'.png'))
					$files[$i]['thumbnail'] = 'icons/48x48'
						.'/mime/'.$mime.'.png';
				else
					$files[$i]['thumbnail'] = 'icons/48x48'
						.'/mime/default.png';
				$files[$i]['icon'] = $files[$i]['thumbnail'];
				if(is_readable('icons/16x16/mime/'.$mime
							.'.png'))
					$files[$i]['icon'] = 'icons'
						.'/16x16/mime/'.$mime.'.png';
			}
		}
		_module('explorer', 'browse', array('entries' => $files,
					'view' => 'thumbnails',
					'toolbar' => 0));
	}
	/* releases */
	/* FIXME project members should be trusted too */
	$files = _sql_array($sql." AND dc3.title='release' ORDER BY date DESC");
	if(is_array($files) && ($cnt = count($files)) > 0)
	{
		print('<h2>'._html_safe(RELEASES).'</h2>'."\n");
		for($i = 0; $i < $cnt; $i++)
		{
			$files[$i]['module'] = 'download';
			$files[$i]['action'] = 'default';
			$files[$i]['date'] = strftime('%d/%m/%Y %H:%M',
					strtotime(substr($files[$i]['date'], 0,
							19)));
			$mime = _mime_from_ext($files[$i]['name']);
			if(($files[$i]['mode'] & $this->S_IFDIR)
					== $this->S_IFDIR)
			{
				$files[$i]['icon'] = 'icons/16x16'
					.'/mime/folder.png';
				$files[$i]['thumbnail'] = 'icons/48x48'
					.'/mime/folder.png';
			}
			else
			{
				if(is_readable('icons/48x48/mime/'.$mime.'.png'))
					$files[$i]['thumbnail'] = 'icons/48x48'
						.'/mime/'.$mime.'.png';
				else
					$files[$i]['thumbnail'] = 'icons/48x48'
						.'/mime/default.png';
				$files[$i]['icon'] = $files[$i]['thumbnail'];
				if(is_readable('icons/16x16/mime/'.$mime
							.'.png'))
					$files[$i]['icon'] = 'icons'
						.'/16x16/mime/'.$mime.'.png';
			}
		}
		$toolbar = array();
		$toolbar[] = array('title' => NEW_RELEASE,
				'link' => _module_link('project',
					'download_insert', $project['id']),
				'class' => 'new');
		_module('explorer', 'browse', array('entries' => $files,
					'class' => array('date' => DATE),
					'toolbar' => $toolbar,
					'view' => 'details'));
	}
	/* source code */
	if(strlen($project['cvsroot']) && ($repository = _config_get('project',
					'repository')) != FALSE)
	{
		print('<h2>'._html_safe(SOURCE_CODE).'</h2>'."\n");
		print("<p>The source code is available over anonymous CVS:"
				."</p>\n");
		print('<pre>$ cvs -d:'._html_safe($repository).' co '
				.$project['cvsroot']."</pre>\n");
	}
}


//ProjectModule::downloadInsert
protected function downloadInsert($args)
{
	global $error;

	if(!isset($args['id']))
		return _error(INVALID_PROJECT);
	if(!$this->_isMember($args['id']))
		return _error(PERMISSION_DENIED);
	$project = _sql_array('SELECT project_id AS id, title AS name, synopsis'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND daportal_content.enabled='1'"
			." AND project_id='".$args['id']."'");
	if(!is_array($project) || count($project) != 1)
		return _error(INVALID_PROJECT);
	$project = $project[0];
	$this->_toolbar($project['id']);
	print('<h1 class="title project">'._html_safe($project['name']).': '
		.NEW_RELEASE.'</h1>'."\n");
	if(isset($error) && strlen($error))
		_error($error);
	$directory = isset($args['directory'])
		? stripslashes($args['directory']) : '';
	include('./modules/project/download_update.tpl');
}


//ProjectModule::enable
protected function enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT project_id FROM daportal_project'
			." WHERE project_id='".$args['id']."'")) === FALSE)
		return _error(INVALID_PROJECT);
	require_once('./system/content.php');
	_content_enable($id);
	if(isset($args['display']) && $args['display'] != 0)
		project_display(array('id' => $id));
}


//ProjectModule::insert
protected function insert($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], $args['content'])) === FALSE)
		return _error('Unable to insert project content');
	if(_sql_query('INSERT INTO daportal_project (project_id, synopsis'
					.', cvsroot) VALUES ('."'$id'"
					.", '".$args['synopsis']."'"
					.", '".$args['cvsroot']."')") === FALSE)
	{
		_content_delete($id);
		return _error('Unable to insert project');
	}
	$this->display(array('id' => $id));
}


//ProjectModule::lastCommits
protected function lastCommits($args)
{
	$cpp = 6;
	if(isset($args['cpp']) && is_numeric($args['cpp']))
		$cpp = $args['cpp'];
	if(($entries = $this->_timeline(FALSE, '', $cpp)) === FALSE)
		return;
	return _module('explorer', 'browse_trusted', array(
			'entries' => $entries,
			'class' => array('date' => DATE, 'event' => ACTION,
				'revision' => REVISION,
				'author' => AUTHOR), 'toolbar' => 0,
			'view' => 'details'));
}


//ProjectModule::_list
protected function _list($args)
{
	global $user_id;

	$title = PROJECT_LIST;
	$where = '';
	if(isset($args['action']) && $args['action'] == 'bug_new')
	{
		$title = SELECT_PROJECT_TO_BUG;
		$action = 'bug_new';
	}
	else if(isset($args['user_id'])
			&& ($username = _sql_single('SELECT username FROM'
			." daportal_user WHERE user_id='".$args['user_id']."'"))
			!= FALSE)
	{
		$title = PROJECTS._BY_.$username;
		$where = " AND daportal_content.user_id='".$args['user_id']."'";
	}
	print('<h1 class="title project">'._html_safe($title).'</h1>'."\n");
	$sql = 'SELECT content_id AS id, title, username AS admin'
		.', daportal_content.user_id AS user_id, synopsis'
		.' FROM daportal_content, daportal_user, daportal_project'
		." WHERE daportal_content.enabled='1'"
		.' AND daportal_content.user_id=daportal_user.user_id'
		.' AND daportal_content.content_id=daportal_project.project_id'
		.$where.' ORDER BY title ASC';
	$projects = _sql_array($sql);
	if(!is_array($projects))
		return _error('Could not list projects');
	for($i = 0, $cnt = count($projects); $i < $cnt; $i++)
	{
		$projects[$i]['module'] = 'project';
		$projects[$i]['action'] = 'display';
		if(isset($action))
			$projects[$i]['link'] = _html_link('project', $action,
					FALSE, FALSE,
					'project_id='.$projects[$i]['id']);
		$projects[$i]['icon'] = 'icons/16x16/project.png';
		$projects[$i]['thumbnail'] = 'icons/48x48/project.png';
		$projects[$i]['admin'] = '<a href="'._html_link('user', '',
			$projects[$i]['user_id'], $projects[$i]['admin']).'">'
				._html_safe($projects[$i]['admin']).'</a>';
		$projects[$i]['name'] = $projects[$i]['title'];
		$projects[$i]['tag'] = $projects[$i]['title'];
	}
	$args = array('entries' => $projects, 'view' => 'details',
			'class' => array('admin' => MANAGER,
				'synopsis' => DESCRIPTION));
	require_once('./system/user.php');
	$args['toolbar'] = array();
	if(_user_admin($user_id))
		$args['toolbar'][] = array('title' => NEW_PROJECT,
				'link' => _module_link('project', 'new'),
				'class' => 'new');
	$args['toolbar'][] = array();
	$args['toolbar'][] = array('title' => REFRESH, 'class' => 'refresh',
			'link' => _module_link('project', 'list'),
			'onclick' => 'location.reload(); return false');
	_module('explorer', 'browse_trusted', $args);
}


//ProjectModule::memberAdd
protected function memberAdd($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$project = _sql_array('SELECT project_id AS id, title AS name, user_id'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND project_id='".$args['id']."'");
	if(!is_array($project) || count($project) != 1)
		return _error(INVALID_PROJECT);
	$project = $project[0];
	print('<h1 class="title project"> '._html_safe(ADD_MEMBER_TO_PROJECT)
			.' '.$project['name']."</h1>\n");
	$members = _sql_array('SELECT user_id AS id FROM daportal_project_user'
			." WHERE project_id='".$project['id']."'");
	$where = " WHERE user_id <> '".$project['user_id']."'"
		." AND user_id <> '0'";
	foreach($members as $m)
		$where.=" AND user_id <> '".$m['id']."'";
	$users = _sql_array('SELECT user_id AS id, username AS name'
			.' FROM daportal_user'.$where);
	for($i = 0, $cnt = count($users); $i < $cnt; $i++)
	{
		$users[$i]['icon'] = 'icons/16x16/user.png';
		$users[$i]['thumbnail'] = 'icons/48x48/user.png';
		$users[$i]['module'] = 'user';
		$users[$i]['action'] = 'default';
		$users[$i]['apply_module'] = 'project';
		$users[$i]['apply_id'] = $users[$i]['id'];
		$users[$i]['apply_args'] = 'project_id='.$project['id'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'Add selected users', 'class' => 'add',
			'action' => 'member_insert', 'confirm' => 'add');
	_module('explorer', 'browse', array('toolbar' => $toolbar,
				'entries' => $users, 'module' => 'project',
				'action' => 'display', 'id' => $project['id']));
}


//ProjectModule::memberDelete
protected function memberDelete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED, 1);
	_sql_query('DELETE FROM daportal_project_user WHERE '
			." project_id='".$args['project_id']."'"
			." AND user_id='".$args['id']."'");
}


//ProjectModule::memberInsert
protected function memberInsert($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT user_id FROM daportal_project_user'
			." WHERE project_id='".$args['project_id']."'"
			." AND user_id='".$args['id']."'")) == $args['id'])
		return;
	_sql_query('INSERT INTO daportal_project_user (project_id, user_id)'
			." VALUES ('".$args['project_id']."'"
			.", '".$args['id']."')");
}


//ProjectModule::modify
protected function modify($args)
{
	if(!isset($args['id']))
		return _error(INVALID_PROJECT);
	if(!$this->_isAdmin($args['id']))
		return _error(PERMISSION_DENIED);
	$project = _sql_array('SELECT project_id AS id, title AS name, synopsis'
			.', content, cvsroot'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND project_id='".$args['id']."'");
	if(!is_array($project) || count($project) != 1)
		return _error('Unable to update project');
	$project = $project[0];
	$this->_toolbar($project['id']);
	$title = MODIFICATION_OF.' '.$project['name'];
	include('./modules/project/project_update.tpl');
}


//ProjectModule::_new
protected function _new($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$title = NEW_PROJECT;
	include('./modules/project/project_update.tpl');
}


//ProjectModule::screenshotInsert
protected function screenshotInsert($args)
{
	/* FIXME factorize with download_insert */
	global $error;

	if(!isset($args['id']))
		return _error(INVALID_PROJECT);
	$project = _sql_array('SELECT project_id AS id, title AS name, synopsis'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND daportal_content.enabled='1'"
			." AND project_id='".$args['id']."'");
	if(!is_array($project) || count($project) != 1)
		return _error(INVALID_PROJECT);
	$project = $project[0];
	$this->_toolbar($project['id']);
	print('<h1 class="title project">'._html_safe($project['name']).': '
		.NEW_SCREENSHOT.'</h1>'."\n");
	if(isset($error) && strlen($error))
		_error($error);
	$directory = isset($args['directory'])
		? stripslashes($args['directory']) : '';
	include('./modules/project/screenshot_update.tpl');
}


//ProjectModule::system
protected function system($args)
{
	global $title, $html, $error;

	$title.=' - '.PROJECTS;
	if(!isset($args['action']))
		return;
	if($args['action'] == 'browse' && isset($args['download'])
			&& $args['download'] == 1)
		$html = 0;
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return;
	if($args['action'] == 'config_update')
		$error = $this->_system_config_update($args);
	else if($args['action'] == 'download_insert')
		$error = $this->_system_download_insert($args);
	else if($args['action'] == 'screenshot_insert')
		$error = $this->_system_download_insert($args, 'screenshot');
}

private function _system_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	_config_update('project', $args);
	header('Location: '._module_link('project', 'admin'));
	exit(0);
}

private function _system_download_insert($args, $category = 'release')
{
	if(!isset($args['id']))
		return INVALID_PROJECT;
	if(!$this->_isMember($args['id']))
		return PERMISSION_DENIED;
	if(!isset($args['directory']))
		return INVALID_ARGUMENT;
	$project = _sql_array('SELECT project_id AS id, title AS name, synopsis'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND daportal_content.enabled='1'"
			." AND project_id='".$args['id']."'");
	if(!is_array($project) || count($project) != 1)
		return INVALID_PROJECT;
	$project = $project[0];
	if(!_module_id('download')
			|| !($category_module_id = _module_id('category')))
		return 'The download and category modules are required';
	/* lookup directory */
	/* FIXME this code belongs to the download module */
	$path = stripslashes($args['directory']);
	$path = explode('/', $path);
	$parent = ' IS NULL';
	foreach($path as $p)
	{
		$sql = 'SELECT download_id FROM daportal_download'
			.', daportal_content'
			.' WHERE daportal_download.content_id'
			.'=daportal_content.content_id'
			." AND title='$p' AND parent$parent";
		$download_id = _sql_single($sql);
		if($download_id === FALSE)
			return 'Path not found';
		$parent = "='$download_id'";
		_info("Path \"$p\" found");
	}
	/* upload file */
	/* FIXME will no longer work when downloads are restricted to POST */
	/* FIXME it sets a redirection and this breaks $error */
	/* FIXME currently requires to be an admin */
	_module('download', 'file_insert', array('parent' => $download_id));
	/* FIXME assumes it was successful + race condition? */
	if(($id = _sql_id('daportal_download', 'download_id')) === FALSE)
		return 'Internal server error';
	if(($content_id = _sql_single('SELECT content_id FROM daportal_download'
			." WHERE download_id='$id'")) === FALSE)
		return 'Internal server error';
	/* insert into categories */
	/* FIXME this code belongs to the category module */
	/* FIXME need a nice API here */
	/* FIXME this assumes both categories already exist */
	$category_id = _sql_single('SELECT content_id AS id'
			.' FROM daportal_content'
			." WHERE module_id='$category_module_id'"
			." AND title='".$project['name']."'");
	if($category_id != FALSE)
		_module('category', 'link_insert', array('id' => $category_id,
					'content_id' => $content_id));
	/* FIXME code duplication */
	$category_id = _sql_single('SELECT content_id AS id'
			.' FROM daportal_content'
			." WHERE module_id='$category_module_id'"
			." AND title='$category'");
	if($category_id != FALSE)
		_module('category', 'link_insert', array('id' => $category_id,
					'content_id' => $content_id));
	header('Location: '._module_link('project', 'download',
				$project['id']));
}


//ProjectModule::timeline
protected function timeline($args)
{
	require_once('./system/content.php');
	if(_content_readable($args['id']) == FALSE)
	{
		return include('./modules/project/project_submitted.tpl');
	}
	$project = _sql_array('SELECT project_id AS id, title AS name, cvsroot'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND project_id='".$args['id']."'");
	if(!is_array($project) || count($project) != 1)
		return _error(INVALID_ARGUMENT);
	$project = $project[0];
	$this->_toolbar($project['id']);
	if(strlen($project['cvsroot']) == 0)
	{
		print('<h1 class="title project">'._html_safe($project['name'])
				." CVS</h1>\n");
		return _info(NO_CVS_REPOSITORY, 1);
	}
	print('<h1 class="title project">'._html_safe($project['name'])
			.' '._html_safe(TIMELINE).'</h1>'."\n");
	$entries = $this->_timeline($project['id'], $project['cvsroot']);
	$toolbar = array();
	$toolbar[] = array('title' => BACK, 'class' => 'back',
			'onclick' => 'history.back(); return false');
	$toolbar[] = array('title' => FORWARD, 'class' => 'forward',
			'onclick' => 'history.forward(); return false');
	$toolbar[] = array();
	$toolbar[] = array('title' => REFRESH, 'class' => 'refresh',
			'link' => _module_link('project', 'timeline',
				$project['id'], $project['name']),
			'onclick' => 'location.reload(); return false');
	_module('explorer', 'browse_trusted', array(
			'entries' => $entries,
			'class' => array('date' => DATE, 'event' => ACTION,
					'revision' => REVISION,
					'author' => AUTHOR),
			'toolbar' => $toolbar, 'view' => 'details'));
}


//ProjectModule::update
protected function update($args)
{
	global $user_id;

	if(!isset($args['id']))
		return _error(INVALID_PROJECT);
	if(!$this->_isAdmin($args['id'])
			|| $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	$sql = "UPDATE daportal_project SET synopsis='".$args['synopsis']."'";
	if(isset($args['cvsroot']))
	{
		require_once('./system/user.php');
		if(_user_admin($user_id))
			$sql.=", cvsroot='".$args['cvsroot']."'";
	}
	$sql.=" WHERE project_id='".$args['id']."'";
	require_once('./system/content.php');
	//FIXME check if it is a project
	if(!_content_update($args['id'], $args['title'], $args['content']))
		return _error('Could not update project');
	_sql_query($sql);
	$this->display(array('id' => $args['id']));
}


	//private
	//properties
	private $S_IFDIR = 040000;
}

?>

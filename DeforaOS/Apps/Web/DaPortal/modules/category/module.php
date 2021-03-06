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
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


//lang
$text = array();
$text['ADD'] = 'Add';
$text['ALREADY_LINKED'] = 'Already linked';
$text['ASSOCIATE_TO_CONTENT'] = 'Associate to content';
$text['CATEGORIES_ADMINISTRATION'] = 'Categories administration';
$text['CATEGORIES'] = 'Categories';
$text['CATEGORY'] = 'Category';
$text['CATEGORY_LIST'] = 'Category list';
$text['CHOOSE_CATEGORIES'] = 'Choose categories';
$text['CONTENT_TAGGED'] = 'Content tagged';
$text['DELETE_LINK'] = 'Delete link';
$text['DESCRIPTION'] = 'Description';
$text['MEMBER_OF'] = 'Member of';
$text['MODIFICATION_OF_CATEGORY'] = 'Modification of category';
$text['NEW_CATEGORY'] = 'New category';
$text['SETTINGS'] = 'Settings';
global $lang;
if($lang == 'de')
	$text['DESCRIPTION'] = 'Beschreibung';
else if($lang == 'fr')
{
	$text['ADD'] = 'Ajouter';
	$text['ALREADY_LINKED'] = 'D�j� li�';
	$text['ASSOCIATE_TO_CONTENT'] = 'Lier au contenu';
	$text['CATEGORIES_ADMINISTRATION'] = 'Administration des cat�gories';
	$text['CATEGORY'] = 'Cat�gorie';
	$text['CATEGORY_LIST'] = 'Liste des cat�gories';
	$text['CHOOSE_CATEGORIES'] = 'Choisir parmi les cat�gories';
	$text['DELETE_LINK'] = 'Effacer le lien';
	$text['MEMBER_OF'] = 'Membre de';
	$text['MODIFICATION_OF_CATEGORY'] = 'Modification de la cat�gorie';
	$text['NEW_CATEGORY'] = 'Nouvelle cat�gorie';
}
_lang($text);


//CategoryModule
class CategoryModule extends Module
{
	//public
	//methods
	//useful
	//CategoryModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		$args = $request->getParameters();
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'delete':
			case 'disable':
			case 'enable':
			case 'get':
			case 'insert':
			case 'modify':
			case 'rss':
			case 'set':
			case 'system':
				return $this->$action($args);
			case 'config_update':
				return $this->configUpdate($args);
			case 'list':
				return $this->_list($args);
			case 'new':
				return $this->_new($args);
			default:
				return $this->_default($args);
		}
	}


//CategoryModule::admin
protected function admin($args)
{
	print('<h1 class="title category">'
			._html_safe(CATEGORIES_ADMINISTRATION)."</h1>\n");
	if(($configs = _config_list('category')))
	{
		print('<h2 class="title settings">'._html_safe(SETTINGS)
				."</h2>\n");
		$module = 'category';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title category">'._html_safe(CATEGORIES)."</h2>\n");
	$module_id = _module_id('category');
	$categories = _sql_array('SELECT content_id AS id, title AS name'
			.', enabled, content AS description'
			.' FROM daportal_content'
			." WHERE module_id='$module_id' ORDER BY name ASC");
	if(!is_array($categories))
		return _error('Could not list categories');
	$count = count($categories);
	for($i = 0; $i < $count; $i++)
	{
		$categories[$i]['module'] = 'category';
		$categories[$i]['action'] = 'modify';
		$categories[$i]['icon'] = 'icons/16x16/category.png';
		$categories[$i]['thumbnail'] = 'icons/48x48/category.png';
		$categories[$i]['enabled'] = $categories[$i]['enabled']
			== SQL_TRUE ? 'enabled' : 'disabled';
		$categories[$i]['enabled'] = '<img src="icons/16x16/'
				.$categories[$i]['enabled'].'.png" alt="'
				.$categories[$i]['enabled'].'" title="'
				.($categories[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		/* FIXME do everything in one query */
		$categories[$i]['members'] = _sql_single('SELECT COUNT(*)'
				.' FROM daportal_category_content'
				." WHERE category_id='".$categories[$i]['id']
				."'");
		$categories[$i]['apply_module'] = 'category';
		$categories[$i]['apply_id'] = $categories[$i]['id'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_CATEGORY, 'class' => 'new',
			'link' => _module_link('category', 'new'));
	$toolbar[] = array();
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $categories,
				'class' => array('enabled' => ENABLED,
					'members' => MEMBERS),
				'toolbar' => $toolbar, 'view' => 'details',
				'module' => 'category', 'action' => 'admin'));
}


//CategoryModule::configUpdate
protected function configUpdate($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return $this->admin(array());
}


//CategoryModule::_default
protected function _default($args)
{
	if(isset($args['id']))
		return $this->display($args);
	return $this->_list($args);
}


//CategoryModule::delete
protected function delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	_sql_query('DELETE FROM daportal_category_content'
			." WHERE category_id='".$args['id']."'");
	_sql_query('DELETE FROM daportal_content'
			." WHERE content_id='".$args['id']."'");
}


//CategoryModule::disable
protected function disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	_content_disable($args['id']);
}


//CategoryModule::display
protected function display($args)
{
	$module = _module_id('category');
	$category = _sql_array('SELECT title, content FROM daportal_content'
			." WHERE content_id='".$args['id']."'"
			." AND module_id='$module' AND enabled='1'");
	if(!is_array($category) || count($category) != 1)
		return _error('Unable to display category');
	$category = $category[0];
	$title = CATEGORY.' '.$category['title'];
	include('./modules/category/display.tpl');
	$contents = _sql_array('SELECT daportal_content.content_id AS id'
			.', name AS module, user_id, title'
			.' FROM daportal_category_content, daportal_content'
			.', daportal_module'
			.' WHERE daportal_category_content.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.module_id'
			.'=daportal_module.module_id'
			." AND daportal_content.enabled='1'"
			." AND category_id='".$args['id']."'");
	if(!is_array($contents))
		return _error('Unable to display category');
	$cnt = count($contents);
	for($i = 0; $i < $cnt; $i++)
	{
		$contents[$i]['icon'] = 'icons/16x16/'.$contents[$i]['module']
			.'.png';
		$contents[$i]['thumbnail'] = 'icons/48x48/'
			.$contents[$i]['module'].'.png';
		$contents[$i]['action'] = 'default';
		$contents[$i]['name'] = $contents[$i]['title'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_CATEGORY, 'class' => 'new',
			'link' => _module_link('category', 'new'));
	_module('explorer', 'browse', array('entries' => $contents,
				'toolbar' => $toolbar));
}


//CategoryModule::enable
protected function enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	_content_enable($args['id']);
}


//CategoryModule::get
protected function get($args)
{
	global $user_id;

	if(_sql_single('SELECT name FROM daportal_content, daportal_module'
			.' WHERE daportal_content.module_id'
			.'=daportal_module.module_id'
			." AND content_id='".$args['id']."'") == 'category')
		return _error(INVALID_ARGUMENT);
	print('<h2 class="title category">'.MEMBER_OF.'</h2>'."\n");
	$categories = _sql_array('SELECT category_id AS id, title'
			.' FROM daportal_category_content, daportal_content'
			.' WHERE daportal_category_content.category_id'
			.'=daportal_content.content_id'
			." AND daportal_category_content.content_id='"
			.$args['id']."' AND enabled='1' ORDER BY title ASC");
	if(!is_array($categories))
		return _error('Could not list categories');
	$count = count($categories);
	for($i = 0; $i < $count; $i++)
	{
		$categories[$i]['module'] = 'category';
		$categories[$i]['action'] = 'default';
		$categories[$i]['icon'] = 'icons/16x16/category.png';
		$categories[$i]['thumbnail'] = 'icons/48x48/category.png';
		$categories[$i]['apply_module'] = 'category';
		$categories[$i]['name'] = $categories[$i]['title'];
		$categories[$i]['tag'] = $categories[$i]['title'];
		$categories[$i]['apply_id'] = $categories[$i]['id'];
		$categories[$i]['apply_args'] = 'content_id='.$args['id'];
	}
	$toolbar = 0;
	$module = _sql_single('SELECT name'
			.' FROM daportal_content, daportal_module'
			.' WHERE daportal_content.module_id'
			.'=daportal_module.module_id'
			." AND daportal_content.enabled='1'"
			." AND content_id='".$args['id']."'");
	require_once('./system/user.php');
	if($user_id != 0 && _sql_single('SELECT user_id FROM daportal_content'
			." WHERE content_id='".$args['id']."'") == $user_id
			|| _user_admin($user_id))
	{
		$toolbar = array();
		$toolbar[] = array('title' => NEW_CATEGORY, 'class' => 'add',
				'link' => _module_link('category', 'set',
					$args['id']));
		$toolbar[] = array();
		$toolbar[] = array('title' => DELETE_LINK, 'class' => 'remove',
				'action' => 'link_delete',
				'confirm' => 'delete link');
	}
	_module('explorer', 'browse', array('entries' => $categories,
				'view' => 'list', 'toolbar' => $toolbar,
				'module' => 'category', 'action' => 'set',
				'id' => $args['id']));
}


//CategoryModule::insert
protected function insert($args)
{
	global $user_id;

	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], $args['content'], 1))
			== FALSE)
		return _error('Unable to insert category');
	$this->display(array('id' => $id));
}


function category_link_delete($args)
{
	global $user_id;

	if($args['id'] == 0 || $args['content_id'] == 0)
		return _error(INVALID_ARGUMENT);
	$module = _module_id('category');
	if(_sql_single('SELECT module_id FROM daportal_content'
			." WHERE enabled='1'"
			." AND content_id='".$args['id']."'") != $module
			|| _sql_single('SELECT module_id FROM daportal_content'
				." WHERE enabled='1'"
				." AND content_id='".$args['content_id']."'")
			== $module)
		return _error(INVALID_ARGUMENT);
	if(_sql_single('SELECT user_id FROM daportal_content'
			." WHERE content_id='".$args['content_id']."'")
			!= $user_id || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	_sql_query('DELETE FROM daportal_category_content WHERE'
			." category_id='".$args['id']."'"
			." AND content_id='".$args['content_id']."'");
}


function category_link_insert($args)
{
	global $user_id;

	if($args['id'] == 0 || $args['content_id'] == 0)
		return _error(INVALID_ARGUMENT);
	$module = _module_id('category');
	if(_sql_single('SELECT module_id FROM daportal_content'
			." WHERE enabled='1'"
			." AND content_id='".$args['id']."'") != $module
			|| _sql_single('SELECT module_id FROM daportal_content'
				." WHERE content_id='".$args['content_id']."'")
			== $module)
		return _error(INVALID_ARGUMENT);
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	require_once('./system/user.php');
	if(_sql_single('SELECT user_id FROM daportal_content'
			." WHERE content_id='".$args['content_id']."'")
			!= $user_id && !_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(_sql_single('SELECT content_id FROM daportal_category_content'
			." WHERE category_id='".$args['id']."'"
			." AND content_id='".$args['content_id']."'")
			== $args['content_id'])
		return _warning(ALREADY_LINKED);
	_sql_query('INSERT INTO daportal_category_content'
			.' (category_id, content_id) VALUES ('
			."'".$args['id']."', '".$args['content_id']."')");
}


function category_link_insert_new($args)
{
	global $user_id;

	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	if($args['title'] == '')
		return _error(INVALID_ARGUMENT);
	$module = _module_id('category');
	require_once('./system/user.php');
	if(_sql_single('SELECT user_id FROM daportal_content'
			." WHERE content_id='".$args['content_id']."'")
			!= $user_id && !_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT content_id FROM daportal_content'
			." WHERE module_id='$module'"
			." AND title='".$args['title']."'")) != FALSE)
		category_link_insert(array('id' => $id,
				'content_id' => $args['content_id']));
	else
	{
		require_once('./system/content.php');
		if(($id = _content_insert($args['title'], '', 1)) == FALSE)
			return _error('Unable to insert category');
		category_link_insert(array('id' => $id,
					'content_id' => $args['content_id']));
	}
	category_set(array('id' => $args['content_id']));
}


//CategoryModule::_list
protected function _list($args)
{
	print('<h1 class="title category">'._html_safe(CATEGORY_LIST).'</h1>'
			."\n");
	$categories = _sql_array('SELECT content_id AS id, title'
			.', content AS description, name AS module'
			.' FROM daportal_content, daportal_module'
			.' WHERE daportal_content.module_id'
			.'=daportal_module.module_id'
			." AND daportal_content.enabled='1'"
			." AND name='category' ORDER BY title ASC");
	if(!is_array($categories))
		return _error('Could not list categories');
	$count = count($categories);
	for($i = 0; $i < $count; $i++)
	{
		$categories[$i]['action'] = 'default';
		$categories[$i]['icon'] = 'icons/16x16/category.png';
		$categories[$i]['thumbnail'] = 'icons/48x48/category.png';
		$categories[$i]['name'] = $categories[$i]['title'];
		$categories[$i]['tag'] = $categories[$i]['title'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_CATEGORY, 'class' => 'new',
			'link' => _module_link('category', 'new'));
	_module('explorer', 'browse', array('entries' => $categories,
				'view' => 'list', 'toolbar' => $toolbar));
}


//CategoryModule::modify
protected function modify($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$module = _module_id('category');
	$category = _sql_array('SELECT content_id AS id, title AS name, content'
			.' FROM daportal_content'
			." WHERE content_id='".$args['id']."'"
			." AND module_id='$module'"
			." AND enabled='1'");
	if(!is_array($category) || count($category) != 1)
		return _error('Unable to modify category');
	$category = $category[0];
	$title = MODIFICATION_OF_CATEGORY.' '.$category['name'];
	include('./modules/category/update.tpl');
}


//CategoryModule::_new
protected function _new($args)
{
	global $user_id;

	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	$title = NEW_CATEGORY;
	include('./modules/category/update.tpl');
}


//CategoryModule::rss
protected function rss($args)
{
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	if(($category = _content_select($args['id'])) == FALSE)
		return _error(INVALID_ARGUMENT);
	$sql = 'SELECT daportal_content.content_id AS id, name AS module'
		.', daportal_content.user_id AS user_id, title'
		.', username AS author, email, timestamp AS date, content'
		.' FROM daportal_category_content, daportal_content'
		.', daportal_module, daportal_user'
		.' WHERE daportal_category_content.content_id'
		.'=daportal_content.content_id'
		.' AND daportal_content.module_id=daportal_module.module_id'
		." AND daportal_content.enabled='1'"
		.' AND daportal_content.user_id=daportal_user.user_id'
		." AND category_id='".$args['id']."'"
		.' ORDER BY timestamp DESC '._sql_offset(0, 10);
	$res = _sql_array($sql);
	if(!is_array($res))
		return _error('Could not list content');
	$email = _config_get('category', 'email') ? 1 : 0;
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['author'] = $email ? $res[$i]['email']
			.' ('.$res[$i]['author'].')' : $res[$i]['author'];
		$res[$i]['date'] = date('D, j M Y H:i:s O', strtotime(
					substr($res[$i]['date'], 0, 19)));
		$res[$i]['link'] = _module_link_full($res[$i]['module'], FALSE,
				$res[$i]['id'], $res[$i]['title']);
		/* FIXME forcibly format the content */
		require_once('./system/html.php');
		$res[$i]['content'] = _html_pre($res[$i]['content']);
	}
	require_once('./system/rss.php');
	$link = _module_link_full('category', FALSE, $args['id'],
		$category['title']);
	$atomlink = _module_link_full('category', 'rss', $args['id'],
		$category['title']);
	_rss(CONTENT_TAGGED.' '.$category['title'], $link, $atomlink, '', $res);
}


//CategoryModule::set
protected function set($args)
{
	global $user_id;

	if(!isset($args['id']) || !is_numeric($args['id']))
		return _error(INVALID_ARGUMENT);
	$id = $args['id'];
	require_once('./system/user.php');
	if($user_id == 0 || (_sql_single('SELECT user_id FROM daportal_content'
					." WHERE content_id='$id'") != $user_id
				&& !_user_admin($user_id)))
		return _error(PERMISSION_DENIED);
	$module = _module_id('category');
	if(_sql_single('SELECT module_id FROM daportal_content'
			." WHERE content_id='$id'") == $module)
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	if(!_content_display($id))
		return _error('Could not display content');
	print('<h2 class="title category">'.CHOOSE_CATEGORIES.'</h2>'."\n");
	$categories = _sql_array('SELECT content_id AS id, title'
			." FROM daportal_content WHERE module_id='$module'"
			." AND enabled='1' ORDER BY title ASC");
	if(!is_array($categories))
		return _error('Could not list categories');
	$cnt = count($categories);
	for($i = 0; $i < $cnt; $i++)
	{
		$categories[$i]['module'] = 'category';
		$categories[$i]['action'] = 'display';
		$categories[$i]['icon'] = 'icons/16x16/category.png';
		$categories[$i]['thumbnail'] = 'icons/48x48/category.png';
		$categories[$i]['apply_module'] = 'category';
		$categories[$i]['name'] = $categories[$i]['title'];
		$categories[$i]['apply_id'] = $categories[$i]['id'];
		$categories[$i]['apply_args'] = 'content_id='.$args['id'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => ASSOCIATE_TO_CONTENT, 'class' => 'add',
			'action' => 'link_insert');
	_module('explorer', 'browse', array('entries' => $categories,
				'view' => 'list', 'toolbar' => $toolbar,
				'module' => 'category', 'action' => 'set',
				'id' => $id));
	$module = _content_module($id);
	$title = _content_title($id);
	include('./modules/category/choose.tpl');
}


//CategoryModule::system
protected function system($args)
{
	global $html, $error;

	if($_SERVER['REQUEST_METHOD'] == 'POST')
	{
		if($_POST['action'] == 'config_update')
			$error = $this->_system_config_update($args);
	}
	else if($_SERVER['REQUEST_METHOD'] == 'GET' && isset($args['action'])
			&& $args['action'] == 'rss')
	{
		$html = 0;
		header('Content-Type: text/xml');
	}
}

private function _system_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	$args['category_email'] = isset($args['category_email']) ? TRUE : FALSE;
	_config_update('category', $args);
	header('Location: '._module_link('category', 'admin'));
	exit(0);
}


//CategoryModule::update
protected function update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	$module = _module_id('category');
	if(_sql_query('UPDATE daportal_content SET'
			." title='".$args['title']."'"
			.", content='".$args['content']."'"
			." WHERE content_id='".$args['id']."'"
			." AND module_id='$module'") == FALSE)
		return _error('Unable to update category');
	$this->display(array('id' => $args['id']));
}
}

?>

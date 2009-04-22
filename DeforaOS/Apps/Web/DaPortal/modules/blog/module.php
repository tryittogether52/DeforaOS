<?php //$Id$
//Copyright (c) 2009 Pierre Pronchery <khorben@defora.org>
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
//FIXME
//- create a specific table for blog posts (allow comments, ...)
//- create a table for blogs (name, theme...)
//- allow listing people's blogs



//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


//lang
$text = array();
$text['BLOG'] = 'Blog';
$text['BLOG_ADMINISTRATION'] = 'Blog administration';
$text['BLOG_BY'] = 'Blog by';
$text['BLOG_ON'] = 'on';
$text['BLOG_PLANET'] = 'Planet';
$text['BLOG_POST'] = 'Blog post';
$text['BLOG_PREVIEW'] = 'Blog post preview';
$text['COMMENT_S'] = 'comment(s)';
$text['NEW_BLOG_POST'] = 'New blog post';
$text['NEW_POST'] = 'New post';
global $lang;
if($lang == 'fr')
{
	$text['NEW_BLOG_POST'] = 'Nouveau billet';
	$text['NEW_POST'] = 'Nouveau billet';
}
_lang($text);


//private
//blog_display
function _blog_display($id, $title = BLOG_POST)
{
	require_once('./system/content.php');
	if(($post = _content_select($id, 1)) == FALSE)
	{
		_error(INVALID_ARGUMENT);
		return FALSE;
	}
	$long = 1;
	$post['date'] = _sql_date($post['timestamp']);
	include('./modules/blog/blog_display.tpl');
	return TRUE;
}


//blog_insert
function _blog_insert($post)
{
	global $user_id;

	if($user_id == 0)
		return FALSE;
	require_once('./system/content.php');
	return _content_insert($post['title'], $post['content'], 1);
}


//public
//blog_admin
function blog_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title blog">'._html_safe(BLOG_ADMINISTRATION)
			."</h1>\n");
	$order = 'DESC';
	$sort = 'timestamp';
	if(isset($args['sort']))
	{
		$order = 'ASC';
		switch($args['sort'])
		{
			case 'username':$sort = 'username';	break;
			case 'enabled':	$sort = 'daportal_content.enabled';
								break;
			case 'name':	$sort = 'title';	break;
			default:	$order = 'DESC';	break;
		}
	}
	$res = _sql_array('SELECT content_id AS id, timestamp'
		.', daportal_content.enabled AS enabled, title AS name, content'
		.', daportal_content.user_id AS user_id, username'
		.' FROM daportal_content, daportal_user, daportal_module'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_module.name='blog'"
		.' AND daportal_module.module_id=daportal_content.module_id'
		.' ORDER BY '.$sort.' '.$order);
	if(!is_array($res))
		return _error('Unable to list posts');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'blog';
		$res[$i]['apply_module'] = 'blog';
		$res[$i]['action'] = 'update';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['icon'] = 'icons/16x16/blog.png';
		$res[$i]['thumbnail'] = 'icons/48x48/blog.png';
		$res[$i]['name'] = _html_safe($res[$i]['name']);
		$res[$i]['username'] = '<a href="'._html_link('user', '',
			$res[$i]['user_id'], $res[$i]['username']).'">'
				._html_safe($res[$i]['username']).'</a>';
		$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE ?
			'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
				.$res[$i]['enabled'].'.png" alt="'
				.$res[$i]['enabled'].'" title="'
				.($res[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$res[$i]['date'] = _html_safe(strftime('%d/%m/%y %H:%M',
					strtotime(substr($res[$i]['timestamp'],
							0, 19))));
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_POST, 'class' => 'new',
			'link' => _module_link('blog', 'insert'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => array('enabled' => ENABLED,
					'username' => AUTHOR, 'date' => DATE),
				'module' => 'blog', 'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
						: 'date',
				'toolbar' => $toolbar, 'view' => 'details'));
}


//blog_default
function blog_default($args)
{
	if(!isset($args['id']))
		return blog_planet($args);
	_blog_display($args['id']);
}


//blog_delete
//FIXME allow to delete one's own content
function blog_delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_delete($args['id']))
		return _error('Could not delete post');
}


//blog_disable
//FIXME allow to delete one's own content
function blog_disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_disable($args['id']))
		return _error('Could not disable post');
}


//blog_enable
//FIXME allow to delete one's own content
function blog_enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_enable($args['id']))
		return _error('Could not enable post');
}


//blog_insert
function blog_insert($args)
{
	global $error, $user_id, $user_name;

	if(isset($error) && strlen($error))
		return _error($error);
	$title = NEW_BLOG_POST;
	if(isset($args['preview']))
	{
		$long = 1;
		$title = NEW_BLOG_POST;
		$post = array('user_id' => $user_id,
				'username' => $user_name,
				'title' => stripslashes($args['title']),
				'content' => stripslashes($args['content']),
				'date' => strftime(DATE_FORMAT),
				'preview' => 1);
		include('./modules/blog/blog_display.tpl');
		unset($title);
	}
	include('./modules/blog/blog_update.tpl');
}


//blog_list
function blog_list($args)
{
	$title = BLOG_PLANET;
	$and = '';
	$paging = '';
	if(isset($args['user_id']))
	{
		$username = _sql_single('SELECT username FROM daportal_user'
				." WHERE user_id='".$args['user_id']."'");
		if($username != FALSE)
		{
			$title = BLOG_BY.' '.$username;
			$and = ' AND daportal_user.user_id'
				."='".$args['user_id']."'";
			$paging = 'user_id='._html_safe($args['user_id'])
				.'&amp;';
		}
	}
	print('<h1 class="title blog">'._html_safe($title)."</h1>\n");
	unset($title); //XXX hoping this doesn't affect the global variable
	$sql = ' FROM daportal_module, daportal_content, daportal_user'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_content.enabled='1'"
		." AND daportal_module.name='blog'".$and
		.' AND daportal_module.module_id=daportal_content.module_id';
	$npp = 10;
	$page = isset($args['page']) ? $args['page'] : 1;
	if(($cnt = _sql_single('SELECT COUNT(*)'.$sql)) == 0)
		$cnt = 1;
	$pages = ceil($cnt / $npp);
	$page = min($page, $pages);
	$res = _sql_array('SELECT content_id AS id, timestamp, title, content'
			.', daportal_content.enabled AS enabled'
			.', daportal_content.user_id AS user_id, username'.$sql
			.' ORDER BY timestamp DESC '
			.(_sql_offset(($page - 1) * $npp, $npp)));
	if(!is_array($res))
		return _error('Unable to list posts');
	$long = 0;
	require_once('./system/content.php');
	foreach($res as $post)
	{
		$post['tag'] = $post['title'];
		_content_select_lang($post['id'], $post['title'],
				$post['content']);
		$post['date'] = _sql_date($post['timestamp']);
		include('./modules/blog/blog_display.tpl');
	}
	_html_paging(_html_link('blog', 'list', FALSE, FALSE, $paging.'page='),
			$page, $pages);
}


//blog_planet
function blog_planet($args)
{
	return blog_list($args);
}


//blog_rss
//FIXME share code with the news module (create system/rss.php)
function blog_rss($args)
{
	global $title;

	if(($module_id = _module_id('blog')) == FALSE)
		return;
	$and = '';
	if(isset($args['user_id']))
	{
		$and = " AND user_id='".$args['user_id']."'";
		$link = _module_link_full('blog', FALSE, FALSE,
				array('user_id' => $args['user_id']));
		$link = _module_link_full('blog', 'rss', FALSE,
				array('user_id' => $args['user_id']));
		$title = $title.' - '.BLOG_BY; //FIXME with the username
		$content = $title;
	}
	else
	{
		$link = _module_link_full('blog');
		$atomlink = _module_link_full('blog', 'rss');
		$title = $title.' - '.BLOG_PLANET;
		$content = $title;
	}
	require_once('./system/html.php');
	include('./modules/blog/rss_channel_top.tpl');
	$res = _sql_array('SELECT content_id AS id, timestamp AS date, title'
			.', content, username AS author, email'
			.' FROM daportal_content, daportal_user'
			.' WHERE daportal_content.user_id'
			.'=daportal_user.user_id'
			." AND module_id='$module_id'"
			." AND daportal_content.enabled='1'".$and
			.' ORDER BY timestamp DESC '._sql_offset(0, 10));
	if(is_array($res))
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$post = $res[$i];
			$post['author'] = $post['email']
				.' ('.$post['author'].')';
			$post['date'] = date('D, j M Y H:i:s O', strtotime(
						substr($post['date'], 0, 19)));
			$post['link'] = _html_link_full('post', FALSE,
					$post['id']);
			$post['content'] = _html_pre($post['content']);
			include('./modules/blog/rss_item.tpl');
		}
	include('./modules/blog/rss_channel_bottom.tpl');
}


//blog_system
function blog_system($args)
{
	global $html, $error;

	if($_SERVER['REQUEST_METHOD'] == 'GET')
	{
		if(isset($args['action']) && $args['action'] == 'rss')
		{
			$html = 0;
			header('Content-Type: text/xml');
		}
	}
	else if($_SERVER['REQUEST_METHOD'] == 'POST')
		switch($args['action'])
		{
			case 'insert':
				$error = _system_blog_insert($args);
				return;
		}
}

function _system_blog_insert($args)
{
	global $user_id, $user_name;

	if($user_id == 0)
		return PERMISSION_DENIED;
	if(isset($args['preview']) || !isset($args['send']))
		return;
	$post = array('user_id' => $user_id, 'username' => $user_name,
			'title' => $args['title'],
			'content' => $args['content']);
	if(($post['id'] = _blog_insert($post)) == FALSE)
		return 'Could not insert blog post';
	header('Location: '._module_link('blog', FALSE, $post['id']));
	exit(0);
}

?>

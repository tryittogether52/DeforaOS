<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>
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
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


//lang
$text = array();
$text['ARTICLE_BY'] = 'Article by';
$text['ARTICLE_ON'] = 'on';
$text['ARTICLE_PREVIEW'] = 'Article preview';
$text['ARTICLE_SUBMISSION'] = 'Article submission';
$text['ARTICLES'] = 'Articles';
$text['ARTICLES_ADMINISTRATION'] = 'Articles administration';
$text['ARTICLES_BY'] = 'Articles by';
$text['MODIFICATION_OF_ARTICLE'] = 'Modification of article';
$text['SUBMIT_ARTICLE'] = 'Submit article';
global $lang;
if($lang == 'fr')
{
	$text['ARTICLE_BY'] = 'Article par';
	$text['ARTICLE_ON'] = 'le';
	$text['ARTICLE_PREVIEW'] = "Aper�u de l'article";
	$text['ARTICLES_ADMINISTRATION'] = 'Administration des articles';
	$text['ARTICLES_BY'] = 'Articles par';
}
_lang($text);


function _article_insert($article)
{
	global $user_id;

	if(!$user_id)
		return FALSE;
	require_once('./system/content.php');
	return _content_insert($article['title'], $article['content']);
}


function article_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title article">'.ARTICLES_ADMINISTRATION."</h1>\n");
	$order = 'ASC';
	$sort = 'timestamp';
	if(isset($args['sort']))
		switch($args['sort'])
		{
			case 'username':$sort = 'username';	break;
			case 'enabled':	$sort = 'daportal_content.enabled';
								break;
			case 'name':	$sort = 'title';	break;
			case 'date':
			default:	$order = 'DESC';	break;
		}
	$articles = _sql_array('SELECT content_id AS id, timestamp'
		.', daportal_content.enabled AS enabled, title, content'
		.', daportal_content.user_id AS user_id, username'
		.' FROM daportal_content, daportal_user, daportal_module'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_module.name='article'"
		.' AND daportal_module.module_id=daportal_content.module_id'
		.' ORDER BY '.$sort.' '.$order);
	if(!is_array($articles))
		return _error('Unable to list articles');
	for($i = 0, $cnt = count($articles); $i < $cnt; $i++)
	{
		$articles[$i]['module'] = 'article';
		$articles[$i]['apply_module'] = 'article';
		$articles[$i]['action'] = 'modify';
		$articles[$i]['apply_id'] = $articles[$i]['id'];
		$articles[$i]['icon'] = 'icons/16x16/article.png';
		$articles[$i]['thumbnail'] = 'icons/48x48/article.png';
		$articles[$i]['name'] = $articles[$i]['title'];
		$articles[$i]['username'] = '<a href="'._html_link('user', '',
			$articles[$i]['user_id'], $articles[$i]['username'])
				.'">'._html_safe($articles[$i]['username'])
				.'</a>';
		$articles[$i]['enabled'] = $articles[$i]['enabled']
			== SQL_TRUE ? 'enabled' : 'disabled';
		$articles[$i]['enabled'] = '<img src="icons/16x16/'
				.$articles[$i]['enabled'].'.png" alt="'
				.$articles[$i]['enabled'].'" title="'
				.($articles[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$articles[$i]['date'] = _html_safe(strftime('%d/%m/%y %H:%M',
				strtotime(substr($articles[$i]['timestamp'],
						0, 19))));
	}
	$toolbar = array();
	$toolbar[] = array('title' => SUBMIT_ARTICLE, 'class' => 'new',
			'link' => _module_link('article', 'submit'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $articles,
				'class' => array('enabled' => ENABLED,
					'username' => AUTHOR, 'date' => DATE),
				'module' => 'article', 'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
						: 'date',
				'toolbar' => $toolbar, 'view' => 'details'));
}


function article_default($args)
{
	if(isset($args['id']))
		return article_display(array('id' => $args['id']));
	article_list($args);
}


function article_delete($args)
{
	//FIXME implement
}


function article_disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_disable($args['id']))
		return _error('Could not disable article');
}


function article_display($args)
{
	require_once('./system/content.php');
	if(($article = _content_select($args['id'], 1)) == FALSE)
		return _error(INVALID_ARGUMENT);
	if(($article['username'] = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$article['user_id']."'")) == FALSE)
		return _error('Invalid user');
	$long = 1;
	$title = $article['title'];
	$article['date'] = strftime(DATE_FORMAT,
			strtotime(substr($article['timestamp'], 0, 19)));
	include('./modules/article/display.tpl');
}


function article_enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_enable($args['id']))
		return _error('Could not enable article');
}


function article_list($args)
{
	$title = ARTICLES;
	$where = '';
	if(isset($args['user_id']) && ($username = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['user_id']."'")))
	{
		$title = ARTICLES_BY.' '.$username;
		$where = " AND daportal_content.user_id='".$args['user_id']."'";
	}
	print('<h1 class="title article">'._html_safe($title).'</h1>'."\n");
	$articles = _sql_array('SELECT content_id AS id, timestamp, title'
			.', content, daportal_content.enabled AS enabled'
			.', daportal_content.user_id AS user_id, username'
			.' FROM daportal_content, daportal_user'
			.', daportal_module'
			.' WHERE daportal_user.user_id=daportal_content.user_id'
			." AND daportal_content.enabled='1'"
			." AND daportal_module.name='article'"
			.' AND daportal_module.module_id'
			.'=daportal_content.module_id'.$where
			.' ORDER BY title ASC');
	if(!is_array($articles))
		return _error('Unable to list articles');
	if(!isset($username))
	{
		$long = 0;
		foreach($articles as $article)
		{
			$article['date'] = strftime(DATE_FORMAT,
					strtotime(substr($article['timestamp'],
							0, 19)));
			include('./modules/article/display.tpl');
		}
		return;
	}
	for($i = 0, $cnt = count($articles); $i < $cnt; $i++)
	{
		$articles[$i]['module'] = 'article';
		$articles[$i]['action'] = 'default';
		$articles[$i]['icon'] = 'icons/16x16/article.png';
		$articles[$i]['thumbnail'] = 'icons/48x48/article.png';
		$articles[$i]['name'] = $articles[$i]['title'];
		$articles[$i]['date'] = strftime('%d/%m/%y %H:%M',
				strtotime(substr($articles[$i]['timestamp'],
						0, 19)));
	}
	_module('explorer', 'browse', array('class' => array('date' => 'Date'),
				'view' => 'details', 'entries' => $articles));
}


function article_modify($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!($module_id = _module_id('article')))
		return _error('Could not verify module');
	$article = _sql_array('SELECT content_id AS id, title, content'
			.' FROM daportal_content'
			." WHERE module_id='$module_id'"
			." AND content_id='".$args['id']."'");
	if(!is_array($article) || count($article) != 1)
		return _error('Unable to modify article');
	$article = $article[0];
	$title = MODIFICATION_OF_ARTICLE.' "'.$article['title'].'"';
	include('./modules/article/update.tpl');
}


function article_submit($article)
{
	global $user_id, $user_name;

	//FIXME tweakable?
	if(!$user_id)
		return _error(PERMISSION_DENIED);
	if(isset($article['preview']))
	{
		$long = 1;
		$title = ARTICLE_PREVIEW;
		$article['title'] = stripslashes($article['title']);
		$article['user_id'] = $user_id;
		$article['username'] = $user_name;
		$article['date'] = strftime(DATE_FORMAT);
		$article['content'] = stripslashes($article['content']);
		include('./modules/article/display.tpl');
		unset($title);
		return include('./modules/article/update.tpl');
	}
	if(!isset($article['send']))
	{
		$title = ARTICLE_SUBMISSION;
		return include('./modules/article/update.tpl');
	}
	if(!_article_insert($article))
		return _error('Could not insert article');
	include('./modules/article/posted.tpl');
}


function article_system($args)
{
	global $title;

	$title.=' - Articles';
}


function article_update($article)
{
	global $user_id, $user_name;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($article['preview']))
	{
		$long = 1;
		$title = ARTICLE_PREVIEW;
		$article['id'] = stripslashes($article['id']);
		$article['title'] = stripslashes($article['title']);
		$article['user_id'] = $user_id;
		$article['username'] = $user_name;
		$article['date'] = strftime(DATE_FORMAT);
		$article['content'] = stripslashes($article['content']);
		include('./modules/article/display.tpl');
		unset($title);
		return include('./modules/article/update.tpl');
	}
	require_once('./system/content.php');
	if(!_content_update($article['id'], $article['title'],
				$article['content']))
		return _error('Could not update article');
	article_display(array('id' => $article['id']));
}

?>

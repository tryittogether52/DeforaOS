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
$text['COMPLETE'] = 'Complete';
$text['MISSING'] = 'Missing';
$text['ORIGINAL_CONTENT'] = 'Original content';
$text['TRADUCTION'] = 'Traduction';
$text['TRADUCTION_OF'] = 'Traduction of';
$text['TRANSLATE'] = 'Translate';
$text['TRANSLATE_ADMINISTRATION'] = 'Translation administration';
global $lang;
if($lang == 'fr')
{
	$text['ORIGINAL_CONTENT'] = 'Contenu original';
	$text['TRADUCTION_OF'] = 'Traduction de';
	$text['TRANSLATE'] = 'Traduire';
	$text['TRANSLATE_ADMINISTRATION'] = 'Administration des traductions';
}
_lang($text);


//TranslateModule
class TranslateModule extends Module
{
	//public
	//methods
	//useful
	//TranslateModule::call
	public function call(&$engine, $request)
	{
		$args = $request->getParameters();
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'update':
				return $this->$action($args);
			default:
				//FIXME implement
				return $this->_default($args);
		}
		return FALSE;
	}


//TranslateModule::admin
protected function admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($args['id']))
		return translate_update($args);
	print('<h1 class="title translate">'._html_safe(
				TRANSLATE_ADMINISTRATION)."</h1>\n");
	$sql = 'SELECT content_id AS id, timestamp AS date, name AS module'
		.', daportal_user.user_id AS user_id, username, title AS name'
		.' FROM daportal_content, daportal_module, daportal_user'
		.' WHERE daportal_content.module_id=daportal_module.module_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		.' ORDER BY timestamp DESC';
	$contents = _sql_array($sql);
	$lang = _sql_array('SELECT lang_id, name FROM daportal_lang'
			.' ORDER BY name ASC');
	$classes = array('username' => AUTHOR, 'date' => DATE);
	if(!is_array($contents) || !is_array($lang))
		return _error('Could not list contents');
	foreach($lang as $l)
		$classes[$l['lang_id']] = $l['name'];
	for($i = 0, $cnt = count($contents); $i < $cnt; $i++)
	{
		$contents[$i]['icon'] = '';
		if(($d = _module_desktop($contents[$i]['module'])) != FALSE)
			$contents[$i]['icon'] = $d['icon'];
		$contents[$i]['thumbnail'] = 'icons/48x48/'
			.$contents[$i]['icon'];
		$contents[$i]['icon'] = 'icons/16x16/'.$contents[$i]['icon'];
		$contents[$i]['name'] = _html_safe($contents[$i]['name']);
		$contents[$i]['username'] = '<a href="'._html_link('user',
			FALSE, $contents[$i]['user_id']).'">'
				._html_safe($contents[$i]['username']).'</a>';
		$contents[$i]['module'] = 'content';
		$contents[$i]['action'] = 'update';
		$contents[$i]['date'] = substr($contents[$i]['date'], 0, 19);
		$contents[$i]['date'] = date('d/m/Y H:i',
				strtotime($contents[$i]['date']));
		foreach($lang as $l)
		{
			$id = $contents[$i]['id'];
			$lid = $l['lang_id'];
			$sql = 'SELECT content_id FROM daportal_content_lang'
				." WHERE content_id='$id' AND lang_id='$lid'";
			$contents[$i][$lid] = (_sql_single($sql) != FALSE)
				? '<img src="icons/16x16/enabled.png"'
				.' title="'.COMPLETE.'"/>'
				: '<img src="icons/16x16/disabled.png"'
				.' title="'.MISSING.'"/>';
			$contents[$i][$lid] = '<a href="'._module_link(
				'translate', 'update', $id, FALSE,
				array('lang' => $lid)).'">'
				.$contents[$i][$lid].'</a>';
		}
	}
	_module('explorer', 'browse_trusted', array('entries' => $contents,
				'class' => $classes, 'view' => 'details'));
}


//TranslateModule::update
protected function update($args)
{
	global $error, $user_id;

	if(isset($error) && strlen($error))
		_error($error);
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!isset($args['id']) || !isset($args['lang']))
		return _error(INVALID_ARGUMENT);
	$id = $args['id'];
	$lid = $args['lang'];
	$content = _sql_array('SELECT content_id AS id, timestamp, user_id'
			.', title, content FROM daportal_content'
			." WHERE content_id='$id'");
	if(!is_array($content) || count($content) != 1)
		return _error(INVALID_ARGUMENT);
	$content = $content[0];
	$translate = _sql_array('SELECT title, content'
			.' FROM daportal_content_lang'
			." WHERE content_id='$id' AND lang_id='$lid'");
	$translate = (is_array($translate) && count($translate) == 1)
		? $translate[0] : $content;
	$translate['lang_id'] = $lid;
	$title = TRADUCTION_OF.' '.$content['title'];
	include('./modules/translate/update.tpl');
}


//TranslateModule::system
protected function system($args)
{
	global $title, $error;

	$title.=' - '.TRANSLATE;
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return;
	if($args['action'] == 'update')
		$error = $this->_system_update($args);
}

private function _system_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	if(!isset($args['id']) || !isset($args['lang'])
			|| !isset($args['title'])
			|| !isset($args['content']))
		return INVALID_ARGUMENT;
	$id = $args['id'];
	$lid = $args['lang'];
	if(_sql_single('SELECT lang_id FROM daportal_lang'
				." WHERE lang_id='$lid'") != $lid)
		return INVALID_ARGUMENT;
	if(!_sql_single('SELECT content_id FROM daportal_content_lang'
				." WHERE content_id='$id' AND lang_id='$lid'"))
		$sql = 'INSERT INTO daportal_content_lang (content_id'
				.', lang_id, title, content) VALUES ('
				."'$id', '$lid', '".$args['title']."'"
				.", '".$args['content']."')";
	else
		$sql = 'UPDATE daportal_content_lang SET'
			." title='".$args['title']."'"
			.", content='".$args['content']."'"
			." WHERE content_id='$id' AND lang_id='$lid'";
	if(_sql_query($sql) == FALSE)
		return 'An error occured while updating';
}
}

?>

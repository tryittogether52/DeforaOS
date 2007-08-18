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
	exit(header('Location: ../../index.php'));


function _browse_dir($id, $project, $cvsrep, $cvsroot, $filename)
{
	require_once('./system/html.php');
	print('<h1 class="title project">'._html_safe($project).' CVS: '
			._html_safe($filename).'</h1>'."\n");
	//FIXME un-hardcode locations (invoke the cvs executable instead?)
	$path = $cvsrep.$cvsroot.'/'.$filename;
	if(($dir = opendir($path)) == FALSE)
		return _error('Could not open CVS repository', 1);
	$dirs = array();
	$files = array();
	while($de = readdir($dir))
	{
		if($de == '.' || $de == '..')
			continue;
		if(is_dir($path.'/'.$de))
			$dirs[] = $de;
		else
			$files[] = $de;
	}
	closedir($dir);
	sort($dirs);
	sort($files);
	$entries = array();
	foreach($dirs as $d)
	{
		$name = _html_safe($d);
		$name = '<a href="'._html_link('project', 'browse', $id, '',
			'file='._html_safe($filename).'/'.$name).'">'.$name
				.'</a>';
		$entries[] = array('name' => $name,
				'icon' => 'icons/16x16/mime/folder.png',
				'thumbnail' => 'icons/48x48/mime/folder.png',
				'revision' => '', 'date' => '', 'author' => '',
				'message' => '');
	}
	foreach($files as $f)
	{
		unset($rcs);
		exec('rlog "'.str_replace('"', '\"', $path.'/'.$f).'"', $rcs);
		_info('rlog "'.str_replace('"', '\"', $path.'/'.$f).'"', 0);
		for($i = 0, $count = count($rcs); $i < $count; $i++)
			_info($i.': '.$rcs[$i], 0);
		for($revs = 0; $revs < $count; $revs++)
			if($rcs[$revs] == '----------------------------')
				break;
		$file = _html_safe($filename.'/'.$f);
		$name = _html_safe(substr($rcs[2], 14));
		require_once('./system/mime.php');
		$mime = _mime_from_ext($name);
		$name = '<a href="'._html_link('project', 'browse', $id, '',
			'file='.$file).'">'.$name.'</a>';
		$thumbnail = is_readable('./icons/48x48/mime/'.$mime
				.'.png')
			? 'icons/48x48/mime/'.$mime.'.png'
			: 'icons/48x48/mime/default.png';
		$icon = is_readable('./icons/16x16/mime/'.$mime.'.png')
			? 'icons/16x16/mime/'.$mime.'.png'
			: $thumbnail;
		$revision = _html_safe(substr($rcs[$revs+1], 9));
		$revision = '<a href="'._html_link('project', 'browse', $id, '',
			'file='.$file.'&revision='.$revision).'">'.$revision
				.'</a>';
		$date = _html_safe(substr($rcs[$revs+2], 6, 19));
		$author = substr($rcs[$revs+2], 36);
		$author = substr($author, 0, strspn($author,
				'abcdefghijklmnopqrstuvwxyz'
				.'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'));
		require_once('./system/user.php');
		if(($author_id = _user_id($author)) != FALSE)
			$author = '<a href="'._html_link('user', '', $author_id,
				$author).'">'._html_safe($author).'</a>';
		else
			$author = '';
		//FIXME this is certainly variable (number of lines)
		$message = _html_safe($rcs[$revs+3]);
		//FIXME choose icon depending on the file type
		$entries[] = array('name' => $name,
				'icon' => $icon, 'thumbnail' => $thumbnail,
				'revision' => $revision, 'date' => $date,
				'author' => $author, 'message' => $message);
	}
	$toolbar = array();
	$toolbar[] = array('title' => BACK, 'class' => 'back',
			'link' => 'javascript:history.back()'); /* XXX */
	$toolbar[] = array('title' => PARENT_DIRECTORY,
			'class' => 'parent_directory',
			'link' => _module_link('project', 'browse', $id, '',
				strlen($filename) ? 'file='.dirname($filename)
				: ''));
	$toolbar[] = array('title' => FORWARD, 'class' => 'forward',
			'link' => 'javascript:history.forward()'); /* XXX */
	$toolbar[] = array();
	$toolbar[] = array('title' => REFRESH, 'class' => 'refresh',
			'link' => 'javascript:location.reload()'); /* XXX */
	_module('explorer', 'browse_trusted', array('entries' => $entries,
				'class' => array('revision' => REVISION,
					'date' => DATE, 'author' => AUTHOR,
					'message' => MESSAGE),
				'view' => 'details', 'toolbar' => $toolbar));
}

function _browse_file($id, $project, $cvsrep, $cvsroot, $filename)
	//FIXME
	//- allow diff requests
	//also think about:
	//- creating archives
{
	$path = $cvsrep.$cvsroot.'/'.$filename;
	$path = str_replace('"', '\"', $path);
	$path = str_replace('$', '\$', $path);
	exec('rlog "'.$path.'"', $rcs);
	_info('rlog "'.$path.'"', 0);
	require_once('./system/html.php');
	print('<h1 class="title project">'._html_safe($project).' CVS: '
			._html_safe(dirname($filename)).'/'
			._html_safe(substr($rcs[2], 14)).'</h1>'."\n");
	for($i = 0, $count = count($rcs); $i < $count; $i++)
		_info($i.': '.$rcs[$i], 0);
	for($i = 0; $i < $count;)
		if($rcs[$i++] == '----------------------------')
			break;
	$revisions = array();
	for($count = count($rcs); $i < $count; $i+=3)
	{
		$name = _html_safe(substr($rcs[$i], 9));
		$date = _html_safe(substr($rcs[$i+1], 6, 19));
		$author = substr($rcs[$i+1], 36);
		$author = substr($author, 0, strspn($author,
				'abcdefghijklmnopqrstuvwxyz'
				.'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
				.'0123456789'));
		require_once('./system/user.php');
		if(($author_id = _user_id($author)) != FALSE)
			$author = '<a href="'._html_link('user', '', $author_id,
				$author).'">'._html_safe($author).'</a>';
		else
			$author = '';
		$message = $rcs[$i+2];
		if($message == '----------------------------'
				|| $message ==
'=============================================================================')
			$message = '';
		else
		{
			$apnd = '';
			for($i++; $i < $count
					&& $rcs[$i+2] !=
					'----------------------------'
					&& $rcs[$i+2] !=
'=============================================================================';
					$i++)
				$apnd = '...';
			$message.=$apnd;
			$message = _html_safe($message);
		}
		$icon = 'icons/48x48/cvs-'.((strcmp($name, '1.1') == 0)
			? 'added' : 'modified').'.png';
		$revisions[] = array('module' => 'project',
				'action' => 'browse', 'id' => $id,
				'args' => 'file='.$filename.'&revision='.$name,
				'icon' => $icon, 'thumbnail' => $icon,
				'name' => $name, 'date' => $date,
				'author' => $author, 'message' => $message);
	}
	$toolbar = array();
	$toolbar[] = array('title' => BACK, 'class' => 'back',
			'link' => 'javascript:history.back()'); /* XXX */
	$toolbar[] = array('title' => PARENT_DIRECTORY,
			'class' => 'parent_directory',
			'link' => _module_link('project', 'browse', $id, '',
				'file='.dirname($filename)));
	$toolbar[] = array('title' => FORWARD, 'class' => 'forward',
			'link' => 'javascript:history.forward()'); /* XXX */
	$toolbar[] = array();
	$toolbar[] = array('title' => REFRESH, 'class' => 'refresh',
			'link' => 'javascript:location.reload()'); /* XXX */
	_module('explorer', 'browse_trusted', array('entries' => $revisions,
			'class' => array('date' => DATE, 'author' => AUTHOR,
					'message' => MESSAGE),
			'toolbar' => $toolbar, 'view' => 'details'));
}

function _browse_file_revision($id, $project, $cvsrep, $cvsroot, $filename,
		$revision, $download)
{
	require_once('./system/html.php');
	if(!ereg('^[0-9]+\.[0-9]+$', $revision))
		return _error('Invalid revision');
	$path = $cvsrep.$cvsroot.'/'.$filename;
	$path = str_replace('"', '\"', $path);
	$path = str_replace('$', '\$', $path);
	exec('rlog -h "'.$path.'"', $rcs);
	_info('rlog -h "'.$path.'"', 0);
	for($i = 0, $count = count($rcs); $i < $count; $i++)
		_info($i.': '.$rcs[$i], 0);
	if(!$download)
		print('<h1 class="title project">'._html_safe($project).' CVS: '
				._html_safe(dirname($filename)).'/'
				._html_safe(substr($rcs[2], 14))
				.' '.$revision.'</h1>'."\n");
	$basename = substr($rcs[2], 14);
	$fp = popen('co -p'.$revision.' "'.$path.'"', 'r');
	_info('co -p'.$revision.' "'.$path.'"', 0);
	require_once('./system/mime.php');
	if(($mime = _mime_from_ext($basename)) == 'default')
		$mime = 'text/plain';
	if($download)
	{
		header('Content-Type: '.$mime);
		header('Content-Disposition: inline; filename="'
				._html_safe(basename($basename)).'"');
		while(!feof($fp))
			print(fread($fp, 8192));
		return;
	}
	$link = _html_link('project', 'browse', $id, '', 'file='
			._html_safe($filename));
	print('<div class="toolbar"><a href="'.$link.'"'
			.' title="'._html_safe(BROWSE_REVISIONS).'">'
			.'<div class="icon parent_directory"></div>'
			._html_safe(BROWSE_REVISIONS).'</a>'
			.' &middot; <a href="'.$link."&amp;revision=$revision"
			.'&amp;download=1" title="'._html_safe(DOWNLOAD_FILE)
			.'"><div class="icon download"></div>'
			._html_safe(DOWNLOAD_FILE).'</a></div>'."\n");
	if(strncmp('image/', $mime, 6) == 0)
		return print('<pre><img src="'.$link.' alt=""/></pre>'."\n");
	include('./modules/project/syntax.php');
	print('<pre>'."\n");
	switch($mime)
	{
		case 'application/x-php':
			while(!feof($fp))
				print(_file_php(_html_safe(fgets($fp, 8192))));
			break;
		case 'text/x-chdr':
		case 'text/x-csrc':
			while(!feof($fp))
				print(_file_csrc(_html_safe(fgets($fp, 8192))));
			break;
		case 'text/x-makefile':
			while(!feof($fp))
				print(_file_makefile(_html_safe(fgets($fp, 8192))));
			break;
		default:
			while(!feof($fp))
				print(_html_safe(fgets($fp, 8192)));
			break;
	}
	print('</pre>'."\n");
}

?>

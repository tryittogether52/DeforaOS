<?php
//Copyright 2005 Pierre Pronchery
//Some parts Copyright 2005 FPconcept (used with permission)
//This file is part of PHP Explorer
//
//PHP Explorer is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
//
//PHP Explorer is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with PHP Explorer; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



require('config.php');
require('common.php');


function explorer_delete($args)
{
	global $root;

	$keys = array_keys($args);
	foreach($keys as $k)
	{
		if(!ereg('^entry_[0-9]+$', $k))
			continue;
		$file = filename_safe($args[$k]);
		@unlink($root.'/'.$file);
	}
}


function explorer_download($filename)
{
	global $root, $hidden;

	$filename = $root.'/'.stripslashes($filename);
	if(!is_readable($filename) || is_dir($filename))
		return include('404.tpl');
	if(!$hidden)
	{
		$f = basename($filename);
		if($f[0] == '.')
			return include('403.tpl');
	}
	$mime = mime_from_ext($filename);
	$client_mime = explode(',', $_SERVER['HTTP_ACCEPT']);
	for($i = 0; $i < count($client_mime); $i++)
	{
		if(($pos = strpos($client_mime[$i], ';')) == FALSE)
			continue;
		$client_mime[$i] = substr($client_mime[$i], 0, $pos);
	}
	$attachment = in_array($mime, $client_mime) ? 'inline' : 'attachment';
	header('Content-Type: '.$mime);
	header('Content-Length: '.filesize($filename));
	header('Content-Disposition: '.$attachment.'; filename="'
			.html_safe(basename($filename)).'"');
	readfile($filename);
}

function _sort_permissions($a, $b)
{
	global $root, $path;

	$stata = lstat($root.'/'.$path.'/'.$a);
	$statb = lstat($root.'/'.$path.'/'.$b);
	return $stata['mode'] > $statb['mode'];
}

function _sort_owner($a, $b)
{
	global $root, $path;

	$stata = lstat($root.'/'.$path.'/'.$a);
	$statb = lstat($root.'/'.$path.'/'.$b);
	$ownera = posix_getpwuid($stata['uid']);
	$ownera = $ownera['name'];
	$ownerb = posix_getpwuid($statb['uid']);
	$ownerb = $ownerb['name'];
	return strcmp($ownera, $ownerb);
}

function _sort_group($a, $b)
{
	global $root, $path;

	$stata = lstat($root.'/'.$path.'/'.$a);
	$statb = lstat($root.'/'.$path.'/'.$b);
	$groupa = posix_getgrgid($stata['gid']);
	$groupa = $ownera['name'];
	$groupb = posix_getgrgid($statb['gid']);
	$groupb = $ownerb['name'];
	return strcmp($groupa, $groupb);
}

function _sort_size($a, $b)
{
	global $root, $path;

	$stata = lstat($root.'/'.$path.'/'.$a);
	$statb = lstat($root.'/'.$path.'/'.$b);
	return $stata['size'] > $statb['size'];
}

function _sort_date($a, $b)
{
	global $root, $path;

	$stata = lstat($root.'/'.$path.'/'.$a);
	$statb = lstat($root.'/'.$path.'/'.$b);
	return $stata['mtime'] > $statb['mtime'];
}

function _permissions($mode)
{
	$str = '----------';
	$str[0] = $mode & 040000 ? 'd' : '-';
	$str[1] = $mode & 0400 ? 'r' : '-';
	$str[2] = $mode & 0200 ? 'w' : '-';
	$str[3] = $mode & 0100 ? 'x' : '-';
	$str[4] = $mode & 040 ? 'r' : '-';
	$str[5] = $mode & 020 ? 'w' : '-';
	$str[6] = $mode & 010 ? 'x' : '-';
	$str[7] = $mode & 04 ? 'r' : '-';
	$str[8] = $mode & 02 ? 'w' : '-';
	$str[9] = $mode & 01 ? 'x' : '-';
	return $str;
}

function explorer_folder($folder, $sort, $reverse)
{
	global $root, $hidden, $path, $thumbnails;

	$folder = stripslashes($folder);
	$path = $folder;
	if(($dir = @opendir($root.'/'.$folder)) == FALSE)
		return print("</div>
<h3 style=\"text-align: center\">Could not open directory</h3><div>\n");
	$files = array();
	if(!$hidden)
		while($de = readdir($dir))
		{
			if($de == '.' || $de == '..')
				continue;
			if($de[0] != '.')
				$files[] = $de;
		}
	else
		while($de = readdir($dir))
			if($de != '.' && $de != '..')
				$files[] = $de;
	switch($sort)
	{
		case 'permissions':
			usort($files, _sort_permissions);
			break;
		case 'owner':
			usort($files, _sort_owner);
			break;
		case 'group':
			usort($files, _sort_group);
			break;
		case 'size':
			usort($files, _sort_size);
			break;
		case 'date':
			usort($files, _sort_date);
			break;
		case 'name':
		default:
			sort($files);
			break;
	}
	for($i = 0; count($files); $i++)
	{
		if($reverse)
			$name = array_pop($files);
		else
			$name = array_shift($files);
		if(@is_dir($root.'/'.$folder.'/'.$name))
		{
			$mime = 'folder';
			$icon = 'icons/16x16/mime/folder.png';
			$thumbnail = 'icons/48x48/mime/folder.png';
			$link = 'explorer.php?folder='.$folder.'/'.$name;
		}
		else
		{
			$mime = mime_from_ext($name);
			$icon = 'icons/16x16/mime/'.$mime.'.png';
			if(!is_readable($icon))
				$icon = 'icons/16x16/mime/default.png';
			if($thumbnails && strncmp($mime, 'image/', 6) == 0)
				$thumbnail = 'explorer.php?download='
						.$folder.'/'.$name;
			else if(is_readable('icons/48x48/mime/'.$mime.'.png'))
				$thumbnail = 'icons/48x48/mime/'.$mime.'.png';
			else
				$thumbnail = 'icons/48x48/mime/default.png';
			$link = 'explorer.php?download='.$folder.'/'.$name;
		}
		$permissions = '';
		$owner = '?';
		$group = '?';
		$size = '?';
		$date = '?';
		if(($stat = @lstat($root.'/'.$folder.'/'.$name)))
		{
			$permissions = _permissions($stat['mode']);
			$owner = posix_getpwuid($stat['uid']);
			$owner = $owner['name'];
			$group = posix_getgrgid($stat['gid']);
			$group = $group['name'];
			$size = round($stat['size']/1024);
			$size = $size > 1024 ? round($size/1024).'M'
					: $size.'K';
			$date = date('Y-m-d h:m:s', $stat['mtime']);
		}
		include('entry.tpl');
	}
}


function explorer_sort($folder, $name, $sort, $reverse)
{
	echo '<div class="'.$name.'">';
	if($sort == $name || ($sort == '' && $name == 'name'))
	{
		echo '<a href="explorer.php?folder='.html_safe($folder)
				.'&amp;sort='.$name;
		if(!$reverse)
			echo '&amp;reverse=';
		echo '">'.ucfirst($name).'</a> <img src="icons/16x16/'
				.($reverse ? 'up' : 'down').'.png" alt=""/>';
	}
	else
		echo '<a href="explorer.php?folder='.html_safe($folder)
				.'&amp;sort='.$name.'">'.ucfirst($name).'</a>';
	echo '</div>'."\n";
}


if(isset($_GET['download']))
	return explorer_download(filename_safe($_GET['download']));
if(isset($_POST['action']))
{
	$folder = strlen($_POST['folder']) ? html_safe($_POST['folder'])
		: '/';
	$sort = html_safe($_POST['sort']);
	$reverse = isset($_POST['reverse']) ? '1' : '0';
	if($_POST['action'] == 'delete')
		explorer_delete($_POST);
	return header('Location: explorer.php?folder='.$folder.'&sort='.$sort
			.'&reverse='.$reverse);
}
$folder = strlen($_GET['folder']) ? filename_safe($_GET['folder']) : '/';
$sort = $_GET['sort'];
$reverse = isset($_GET['reverse']);
include('explorer.tpl');

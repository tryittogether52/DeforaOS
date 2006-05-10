<?php //engine.php
//Copyright 2004, 2005 Pierre Pronchery
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
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
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: index.php'));


//global variables
$debug = 1;
$html = 1;
$template = 'DaPortal';
$theme = 'DaPortal';
require_once('./config.php');

//internals
$user_id = 0;
$user_name = 'Anonymous';
$module_id = 0;
$module_name = '';

//mandatory code
//FIXME force magic quotes
session_start();
$vars = array_keys($_SESSION);
foreach($vars as $v)
	$$v = $_SESSION[$v];
require_once('./system/debug.php');
require_once('./system/sql.php');
require_once('./system/config.php');

//configuration variables
if(!isset($title) && ($title = _config_get('admin', 'title')) == FALSE)
	$title = 'DaPortal';

require_once('./system/lang.php');
require_once('./system/module.php');
if($html)
{
	require_once('./system/html.php');
	_html_start();
	_html_template($template);
	_html_stop();
}
else
	_module();

?>

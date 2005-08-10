<?php
//modules/explorer/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['SELECT_ALL'] = 'Select all';
global $lang;
if($lang == 'de')
	$text['SELECT_ALL'] = 'Alles markieren';
else if($lang == 'fr')
	$text['SELECT_ALL'] = 'Tout sélectionner';
lang($text);


function _explorer(&$args)
{
	static $explorer_id = 0;

	$explorer_id++;
	//FIXME default values table
	include('top.tpl');
	if(!isset($args['toolbar']) || is_array($args['toolbar']))
		include('toolbar.tpl');
	$view = isset($args['view']) ? $args['view'] : 'thumbnails';
	include('header.tpl');
	$i = 0;
	foreach($args['entries'] as $entry)
	{
		$i++;
		$link = _explorer_link($entry);
		$link_end = strlen($link) ? '</a>' : '';
		include('entry.tpl');
	}
	include('bottom.tpl');
}

function _explorer_link(&$entry)
{
	if(isset($entry['link']))
		return '<a href="'._html_safe_link($entry['link']).'">';
	else if(!isset($entry['link']) && isset($entry['module'])
			&& isset($entry['action']))
	{
		$link = '<a href="index.php?module='
				._html_safe_link($entry['module'])
				.'&amp;action='
				._html_safe_link($entry['action']);
		if(isset($entry['id']))
			$link.='&amp;id='._html_safe_link($entry['id']);
		if(isset($entry['args']))
			$link.=_html_safe($entry['args']);
		return $link.'">';
	}
	return '';
}

function _explorer_sort($module, $action, $args, $class, $sort, $name)
{
	if($class == $sort)
	{
		print(_html_safe($name));
		print('<img src="modules/explorer/sort.png" alt="sort"/>');
		return;
	}
	print('<a href="index.php?');
	$link = 'module='.$module.'&action='.$action.$args
		.'&sort='.$class;
	print(_html_safe($link).'">'._html_safe($name).'</a>');
}


function explorer_browse(&$args)
{
	for($i = 0, $ic = count($args['entries']); $i < $ic; $i++)
	{
		$args['entries'][$i]['name'] = _html_safe_link(
				$args['entries'][$i]['name']);
		if(!isset($args['class']) || !is_array($args['class']))
			continue;
		$classes = array_keys($args['class']);
		foreach($classes as $c)
			$args['entries'][$i][$c] = _html_safe(
					$args['entries'][$i][$c]);
	}
	return _explorer($args);
}


function explorer_browse_trusted(&$args)
{
	return _explorer($args);
}

?>

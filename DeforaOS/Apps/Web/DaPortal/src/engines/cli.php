<?php //$Id$
//Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DeforaOS Web DaPortal
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, version 3 of the License.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.



require_once('./system/engine.php');
require_once('./system/format.php');
require_once('./system/locale.php');
require_once('./system/page.php');
require_once('./system/template.php');


//CliEngine
class CliEngine extends Engine
{
	//public
	//methods
	//essential
	public function __construct()
	{
		$this->setType('text/plain');
	}


	//accessors
	//CliEngine::getRequest
	public function getRequest()
	{
		if(($options = getopt('DM:fm:a:i:t:')) === FALSE)
			return FALSE;
		$idempotent = TRUE;
		$module = FALSE;
		$action = FALSE;
		$id = FALSE;
		$title = FALSE;
		foreach($options as $key => $value)
			switch($key)
			{
				case 'D':
					$this->setDebug(TRUE);
					break;
				case 'M':
					$this->setType($options['M']);
					break;
				case 'f':
					$idempotent = FALSE;
					break;
				case 'm':
					$module = $options['m'];
					break;
				case 'a':
					$action = $options['a'];
					break;
				case 'i':
					$id = $options['i'];
					break;
				case 't':
					$title = $options['t'];
					break;
			}
		//FIXME also allow parameters to be set
		$ret = new Request($this, $module, $action, $id, $title);
		$ret->setIdempotent($idempotent);
		return $ret;
	}


	//essential
	//CliEngine::match
	public function match()
	{
		return 1;
	}


	//CliEngine::attach
	public function attach()
	{
		Locale::init($this);
	}


	//useful
	//CliEngine::render
	public function render($page)
	{
		$template = Template::attachDefault($this);
		if($template !== FALSE)
			$page = $template->render($this, $page);
		if(($output = Format::attachDefault($this, $this->getType()))
					=== FALSE)
			fprintf(STDERR, "%s\n", "daportal: Could not determine"
					." the proper output format");
		else
			$output->render($this, $page);
	}
}

?>

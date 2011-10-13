<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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



//PageElement
class PageElement
{
	//members
	private $type;
	private $children = array();
	private $properties = array();


	//essential
	public function __construct($type)
	{
		$this->type = $type;
	}


	//accessors
	//PageElement::getChildren
	public function getChildren()
	{
		return $this->children;
	}


	public function getProperties()
	{
		return $this->properties;
	}


	public function getProperty($name)
	{
		if(!isset($this->properties[$name]))
			return FALSE;
		return $this->properties[$name];
	}


	public function getType()
	{
		return $this->type;
	}


	public function setProperty($name, $value)
	{
		$this->properties[$name] = $value;
	}


	public function setContent($content)
	{
		$this->content = $content;
	}


	//useful
	//PageElement::append
	public function append($type)
	{
		$element = new PageElement($type);
		return $this->appendElement($element);
	}


	//PageElement::appendElement
	public function appendElement($element)
	{
		$this->children[] = $element;
		return $element;
	}
}


//Page
class Page extends PageElement
{
	//essential
	public function __construct()
	{
		parent::__construct('page');
	}
}

?>

<?php //$Id$
//Copyright (c) 2007-2012 Pierre Pronchery <khorben@defora.org>
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



function _mail($from, $to, $subject, $content, $headers = array())
{
	//FIXME from should be user-defineable
	$hdr = 'From: '.$from.' <'.$_SERVER['SERVER_ADMIN'].">\n";
	if(is_array($headers))
		foreach($headers as $h)
			$hdr .= $h."\n";
	else if(strlen($headers))
		$hdr.=$headers;
	//FIXME check $from, $to and $subject for newlines
	if(!mail($to, $subject, $content, $hdr))
		_error('Could not send mail to: '.$to, 0);
	else
		_info('Mail sent to: '.$to, 0);
}

?>

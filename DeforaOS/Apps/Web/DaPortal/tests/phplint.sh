#!/bin/sh
#$Id$
#Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, version 3 of the License.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.



#variables
DEBUG="_debug"
FIND="find"
PHPLINT="php -l"


#functions
#debug
_debug()
{
	echo "$@" 1>&2
	"$@"
	res=$?
	#ignore errors when the command is not available
	[ $res -eq 127 ]					&& return 0
	return $res
}


#usage
_usage()
{
	echo "Usage: phplint.sh target" 1>&2
	return 1
}


#main
while getopts "P:" "name"; do
	case "$name" in
		P)
			#XXX ignored for compatibility
			;;
		?)
			_usage
			exit $?
			;;
	esac
done
shift $((OPTIND - 1))
if [ $# -ne 1 ]; then
	_usage
	exit $?
fi
target="$1"

ret=0
> "$target"
for i in $($FIND "../src" -name '*.php'); do
	$DEBUG $PHPLINT -f "$i" 2>&1 >> "$target"		|| ret=2
done
exit $ret

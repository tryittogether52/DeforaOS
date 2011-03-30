#!/usr/bin/env sh
#$Id$
#Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS
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
CFLAGS=
CFLAGSF=
CPATH=
CPPFLAGS=
CPPFLAGSF=
LDFLAGS=
LDFLAGSF=
DESTDIR=
HOST=
IMAGE_FILE=
IMAGE_TYPE=
MACHINE=
PREFIX=
SYSTEM=
TARGET=
VENDOR="DeforaOS"

#executables
CAT="cat"
CC=
CHOWN="chown"
CONFIGURE=
DD="dd bs=1024"
INSTALL="install"
LD=
LN="ln -f"
MAKE="make"
MKDIR="mkdir -p"
MKNOD="mknod"
MV="mv -f"
RMDIR="rmdir -p"

#internals
DEVNULL="/dev/null"
DEVZERO="/dev/zero"
SUBDIRS="System/src/libc \
	System/src/libSystem \
	System/src/libApp \
	System/src/Init \
	System/src/Splasher \
	System/src/VFS/src \
	Apps/Unix/src/sh \
	Apps/Unix/src/utils \
	Apps/Unix/src/devel \
	Apps/Unix/src/others \
	Apps/Servers/src/inetd"


#functions
#check
check()
{
	USAGE="$1"
	EMPTY=

	shift
	for i in $@; do
		VAR=`eval echo "\\\$\$i"`
		[ -z "$VAR" ] && EMPTY="$EMPTY $i"
	done
	[ -z "$EMPTY" ] && return
	USAGE=`echo -e "$USAGE\n\nError:$EMPTY need to be set"`
	usage "$USAGE"
	exit $?
}


#error
error()
{
	echo "build.sh: $1" 1>&2
	exit 2
}


#target
target()
{
	_MAKE="$MAKE"
	[ ! -z "$DESTDIR" ] && _MAKE="$_MAKE DESTDIR=\"$DESTDIR\""
	[ ! -z "$PREFIX" ] && _MAKE="$_MAKE PREFIX=\"$PREFIX\""
	[ ! -z "$CC" ] && _MAKE="$_MAKE CC=\"$CC\""
	[ ! -z "$CPPFLAGS" ] && _MAKE="$_MAKE CPPFLAGS=\"$CPPFLAGS\""
	[ ! -z "$CPPFLAGSF" ] && _MAKE="$_MAKE CPPFLAGSF=\"$CPPFLAGSF\""
	[ ! -z "$CFLAGS" ] && _MAKE="$_MAKE CFLAGS=\"$CFLAGS\""
	[ ! -z "$CFLAGSF" ] && _MAKE="$_MAKE CFLAGSF=\"$CFLAGSF\""
	[ ! -z "$LD" ] && _MAKE="$_MAKE LD=\"$LD\""
	[ ! -z "$LDFLAGS" ] && _MAKE="$_MAKE LDFLAGS=\"$LDFLAGS\""
	[ ! -z "$LDFLAGSF" ] && _MAKE="$_MAKE LDFLAGSF=\"$LDFLAGSF\""
	while [ $# -ge 1 ]; do
		for i in $SUBDIRS; do
			if [ -n "$CONFIGURE" ]; then
				$CONFIGURE "$i"			|| return 2
			fi
			(cd "$i" && eval $_MAKE "$1")		|| return 2
		done
		shift
	done
	return 0
}


#target_all
target_all()
{
	target "install"
}


#target_bootstrap
target_bootstrap()
{
	#reset parameters
	CPPFLAGS=
	CFLAGS=
	LDFLAGS=
	CONFIGURE=
	DESTDIR=
	#build libSystem and configure
	_bootstrap_libsystem					|| return 2
	_bootstrap_configure					|| return 2
	#warn the user
	echo
	echo '================================================================='
	echo 'The source tree is now configured for your environment. Essential'
	echo 'libraries and tools will be installed onto your system, unless'
	echo 'you exit this script now with the CTRL+C key combination.'
	echo 'Press ENTER to proceed with the installation.'
	echo '================================================================='
	echo
	read IGNORE						|| return 0
	#configure, build and install essential libraries and tools
	CONFIGURE="./Apps/Devel/src/configure/src/configure -v -p $PREFIX"
	FAILED=
	_bootstrap_system			|| FAILED="$FAILED System"
	_bootstrap_network			|| FAILED="$FAILED Network"
	_bootstrap_posix			|| FAILED="$FAILED POSIX"
	_bootstrap_devel			|| FAILED="$FAILED Devel"
	_bootstrap_graphics			|| FAILED="$FAILED Graphics"
	_bootstrap_desktop			|| FAILED="$FAILED Desktop"
	[ -z "$FAILED" ]					&& return 0
	echo "Failed to build:$FAILED" 1>&2
	return 2
}

_bootstrap_configure()
{
	C="$CPPFLAGS"
	L="$LDFLAGSF"
	CPPFLAGS="-I ../../../../../System/src/libSystem/include"
	LDFLAGSF="../../../../../System/src/libSystem/src/libSystem.a"
	SUBDIRS="Apps/Devel/src/configure/src"
	TARGETS="clean all"

	[ $# -eq 1 -a "$1" = "install" ] && TARGETS="install"
	target $TARGETS						|| return 2
	./Apps/Devel/src/configure/src/configure -v "System/src" "Apps" \
								|| return 2
	CPPFLAGS="$C"
	LDFLAGSF="$L"
}

_bootstrap_desktop()
{
	RET=0

	#bootstrap libDesktop
	SUBDIRS="Apps/Desktop/src/libDesktop"
	if ! target "clean" "install"; then
		RET=$?
		FAILED="$FAILED Desktop"
		return $RET
	fi
	#build all desktop applications
	SUBDIRS="Apps/Desktop/src Apps/Devel/src/GEDI"
	target "clean all"					|| return 2
}

_bootstrap_devel()
{
	RET=0
	S="Apps/Devel/src/cpp"
	#FIXME we can't install cpp and as because of conflicts with the system
	#	Apps/Devel/src/as \
	#	Apps/Devel/src/c99 \
	#	Apps/Devel/src/strace"

	for i in $S; do
		SUBDIRS="$i"
		target "clean all"				|| RET=$?
	done
	return $RET
}

_bootstrap_graphics()
{
	SUBDIRS="Apps/Graphics/src"

	target "clean all"
}

_bootstrap_libsystem()
{
	SUBDIRS="System/src/libSystem/src"
	target "clean" "libSystem.a"				|| return 2
}

_bootstrap_network()
{
	RET=0
	S="Apps/Network/src/Directory \
		Apps/Network/src/Probe"

	for i in $S; do
		SUBDIRS="$i"
		target "clean all"				|| RET=$?
	done
	return $RET
}

_bootstrap_posix()
{
	RET=0
	S="System/src/libc \
		Apps/Unix/src/sh \
		Apps/Unix/src/utils \
		Apps/Unix/src/devel \
		Apps/Unix/src/others \
		Apps/Servers/src/inetd"

	for i in $S; do
		SUBDIRS="$i"
		target "clean all"				|| RET=$?
	done
	return $RET
}

_bootstrap_system()
{
	RET=0
	S="System/src/Init \
		System/src/VFS"

	#bootstrap libSystem, libApp and libParser
	SUBDIRS="System/src/libSystem System/src/libApp System/src/libParser"
	target "clean" "install"				|| return 2
	for i in $S; do
		SUBDIRS="$i"
		target "clean all"				|| RET=$?
	done
	return $RET
}


#target_clean
target_clean()
{
	target "clean"
}


#target_distclean
target_distclean()
{
	target "distclean"
}


#target_image
target_image()
{
	[ -z "$CPPFLAGS" ] && CPPFLAGS="-D __DeforaOS__"
	_image_pre							&&
	_image_targets							&&
	_image_post
}

_image_pre()
{
	:
}

_image_targets()
{
	#global settings
	CPPFLAGS="$CPPFLAGS -nostdinc -isystem $DESTDIR$PREFIX/include"
	CFLAGS="$CFLAGS -Wall -ffreestanding -g"
	_LDFLAGS="$LDFLAGS -nostdlib -L$DESTDIR$PREFIX/lib -Wl,-rpath-link,$DESTDIR$PREFIX/lib -Wl,-rpath,$PREFIX/lib"
	PKG_CONFIG_LIBDIR="$DESTDIR$PREFIX/lib/pkgconfig"
	PKG_CONFIG_SYSROOT_DIR="$DESTDIR"
	export PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR

	S="$SUBDIRS"
	L="$LDFLAGS"
	for i in $SUBDIRS; do
		SUBDIRS="$i"
		case "$i" in
			System/src/libc)
				LDFLAGS="$_LDFLAGS $L"
				target install			|| return 2
				;;
			System/src/libSystem)
				LDFLAGS="$_LDFLAGS -lc $L"
				SUBDIRS="$i/src $i/include $i/data"
				target install			|| return 2
				;;
			*)
				LDFLAGS="$_LDFLAGS -lc `$CC -print-libgcc-file-name` $DESTDIR$PREFIX/lib/start.o $L"
				target install			|| return 2
				;;
		esac
	done
	SUBDIRS="$S"
	LDFLAGS="$L"
}

_image_post()
{
	:
}


#target_install
target_install()
{
	target "install"
}


#target_uninstall
target_uninstall()
{
	target "uninstall"
}


#usage
usage()
{
	echo "Usage: build.sh [option=value...] target..." 1>&2
	echo "Targets:" 1>&2
	echo "  all		Build and install in a staging directory" 1>&2
	echo "  bootstrap	Bootstrap the system" 1>&2
	echo "  clean		Remove object files" 1>&2
	echo "  distclean	Remove all compiled files" 1>&2
	echo "  image		Create a specific image" 1>&2
	echo "  install	Build and install in the system" 1>&2
	echo "  uninstall	Uninstall everything" 1>&2
	if [ ! -z "$1" ]; then
		echo 1>&2
		echo "$1" 1>&2
	fi
	return 1
}


#main
umask 022
#parse options
while [ $# -gt 0 ]; do
	case "$1" in
		*=*)
			VAR=${1%%=*}
			VALUE=${1#*=}
			eval "$VAR='$VALUE'; export $VAR"
			shift
			;;
		*)
			break
			;;
	esac
done

#initialize target
[ -z "$MACHINE" ] && MACHINE=`uname -m`
[ -z "$SYSTEM" ] && SYSTEM=`uname -s`
[ -z "$TARGET" ] && TARGET="$SYSTEM-$MACHINE"
[ -z "$DESTDIR" ] && DESTDIR="$PWD/destdir-$TARGET"
if [ ! -f "Apps/Devel/src/scripts/targets/$TARGET" ]; then
	case "$MACHINE" in
		arm*b|arm*l)
			MACHINE="arm"
			;;
		i?86)
			MACHINE="i386"
			;;
		sparc64)
			MACHINE="sparc"
			;;
		x86_64)
			MACHINE="amd64"
			;;
	esac
	TARGET="$SYSTEM-$MACHINE"
fi
if [ ! -f "Apps/Devel/src/scripts/targets/$TARGET" ]; then
	echo "$0: warning: $TARGET: Unsupported target" 1>&2
else
	. "Apps/Devel/src/scripts/targets/$TARGET"
fi

#initialize variables
[ -z "$CC" ] && CC="cc"
[ -z "$CONFIGURE" ] && CONFIGURE="configure -O DeforaOS"
[ -z "$IMAGE_TYPE" ] && IMAGE_TYPE="image"
[ -z "$IMAGE_FILE" ] && IMAGE_FILE="$VENDOR-$IMAGE_TYPE.img"
[ -z "$PREFIX" ] && PREFIX="/usr/local"
[ -z "$CC" ] && CC="cc"
[ -z "$UID" ] && UID=`id -u`
[ -z "$SUDO" -a "$UID" -ne 0 ] && SUDO="sudo"

#run targets
if [ $# -lt 1 ]; then
	usage
	exit $?
fi
while [ $# -gt 0 ]; do
	case "$1" in
		all|bootstrap|clean|distclean|image|install|uninstall)
			;;
		*)
			echo "build.sh: $1: Unknown target" 1>&2
			usage
			exit $?
			;;
	esac
	echo "build.sh: Making target $1 on $TARGET" 1>&2
	"target_$1"						|| exit 2
	shift
done

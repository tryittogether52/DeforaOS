#!/usr/bin/env sh
#$Id$
#Copyright (c) 2008-2012 Pierre Pronchery <khorben@defora.org>
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
BOOTSTRAP_CFLAGS=
BOOTSTRAP_CPPFLAGS=
BOOTSTRAP_LDFLAGS=
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
LIBGCC=
MACHINE=
PKG_CONFIG_LIBDIR=
PKG_CONFIG_PATH=
PKG_CONFIG_SYSROOT_DIR=
PREFIX=
SYSTEM=
TARGET=
VENDOR="DeforaOS"
VERBOSE=0

#executables
CAT="cat"
CC=
CHOWN="chown"
CONFIGURE=
DD="dd bs=1024"
DEBUG=
INSTALL="install"
LD=
LN="ln -f"
MAKE="make"
MKDIR="mkdir -m 0755 -p"
MKNOD="mknod"
MV="mv -f"
RMDIR="rmdir -p"

#internals
DEVNULL="/dev/null"
DEVZERO="/dev/zero"
SUBDIRS="System/src/libc \
	System/src/libSystem \
	System/src/libApp \
	System/src/Loader \
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
	_usage "$USAGE"
	exit $?
}


#debug
_debug()
{
	echo "$@" 1>&2
	"$@"
}


#error
error()
{
	echo "build.sh: error: $@" 1>&2
	exit 2
}


#info
_info()
{
	[ "$VERBOSE" -ne 0 ] && echo "build.sh: $@" 1>&2
	return 0
}


#target
_target()
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
	while [ $# -gt 0 ]; do
		for i in $SUBDIRS; do
			if [ -n "$CONFIGURE" ]; then
				$DEBUG $CONFIGURE "$i"		|| return 2
			fi
			_info "Making sub-target $1 in \"$i\""
			(cd "$i" && eval $_MAKE "$1")		|| return 2
		done
		shift
	done
	return 0
}


#target_all
target_all()
{
	target_install
}


#target_bootstrap
target_bootstrap()
{
	#reset parameters
	CPPFLAGS="$BOOTSTRAP_CPPFLAGS"
	CFLAGS="$BOOTSTRAP_CFLAGS"
	LDFLAGS="$BOOTSTRAP_LDFLAGS"
	CONFIGURE=
	DESTDIR=
	#build libSystem and configure
	_bootstrap_libsystem					|| return 2
	_bootstrap_configure					|| return 2
	#warn the user
	echo
	echo '================================================================='
	echo 'The source tree is now configured for your environment. Essential'
	echo 'libraries and tools will now be installed in this folder:'
	echo "\"$PREFIX\""
	echo 'You can still exit this script with the CTRL+C key combination.'
	echo 'Otherwise, press ENTER to proceed.'
	echo '================================================================='
	echo
	read ignore						|| return 0
	#build and install essential libraries and tools
	CONFIGURE=
	FAILED=
	PATH="$PATH:$PREFIX/bin"
	PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
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
	CF="$CFLAGSF"
	L="$LDFLAGSF"
	CPPFLAGS="-I ../../../../../System/src/libSystem/include"
	CFLAGSF="-W"
	LDFLAGSF="../../../../../System/src/libSystem/src/libSystem.a"
	SUBDIRS="Apps/Devel/src/configure/src"
	TARGETS="clean all"

	[ $# -eq 1 -a "$1" = "install" ] && TARGETS="install"
	_target $TARGETS					|| return 2
	$DEBUG ./Apps/Devel/src/configure/src/configure -v -p "$PREFIX" \
			"System/src" "Apps"			|| return 2
	CPPFLAGS="$C"
	CFLAGSF="$CF"
	LDFLAGSF="$L"
}

_bootstrap_desktop()
{
	RET=0

	#bootstrap libDesktop
	SUBDIRS="Apps/Desktop/src/libDesktop"
	if ! _target "clean" "install"; then
		RET=$?
		FAILED="$FAILED Desktop"
		return $RET
	fi
	#build all desktop applications
	SUBDIRS="Apps/Desktop/src"
	_target "clean all"					|| return 2
}

_bootstrap_devel()
{
	RET=0
	S="Apps/Devel/src/cpp"
	#FIXME we can't install cpp because of conflicts with the system
	#	Apps/Devel/src/asm \
	#	Apps/Devel/src/c99 \
	#	Apps/Devel/src/strace"

	for i in $S; do
		SUBDIRS="$i"
		_target "clean all"				|| RET=$?
	done
	return $RET
}

_bootstrap_graphics()
{
	SUBDIRS="Apps/Graphics/src"

	_target "clean all"
}

_bootstrap_libsystem()
{
	SUBDIRS="System/src/libSystem/src"
	_target "clean" "libSystem.a"				|| return 2
}

_bootstrap_network()
{
	RET=0
	S="Apps/Network/src/Directory \
		Apps/Network/src/Probe"

	for i in $S; do
		SUBDIRS="$i"
		_target "clean all"				|| RET=$?
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
		_target "clean all"				|| RET=$?
	done
	return $RET
}

_bootstrap_system()
{
	RET=0
	S="System/src/Init \
		System/src/VFS \
		System/src/VPN"

	#bootstrap libSystem, libApp and libParser
	SUBDIRS="System/src/libSystem System/src/libApp System/src/libParser"
	_target "clean" "install"				|| return 2
	for i in $S; do
		SUBDIRS="$i"
		_target "clean all"				|| RET=$?
	done
	return $RET
}


#target_clean
target_clean()
{
	_target "clean"
}


#target_distclean
target_distclean()
{
	_target "distclean"
}


#target_image
target_image()
{
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
	target_install
}

_image_post()
{
	:
}


#target_install
target_install()
{
	D="$DESTDIR"
	P="$PREFIX"
	S="$SUBDIRS"
	L="$LDFLAGS"
	PKL="$PKG_CONFIG_LIBDIR"
	PKS="$PKG_CONFIG_SYSROOT_DIR"

	[ -z "$PKG_CONFIG_LIBDIR" ] && PKG_CONFIG_LIBDIR="$D$P/lib/pkgconfig"
	[ -z "$PKG_CONFIG_SYSROOT_DIR" ] && PKG_CONFIG_SYSROOT_DIR="$D"
	export PKG_CONFIG_LIBDIR PKG_CONFIG_PATH PKG_CONFIG_SYSROOT_DIR
	for subdir in $SUBDIRS; do
		SUBDIRS="$subdir"
		case "$subdir" in
			System/src/libApp)
				SUBDIRS="$subdir/src"
				LDFLAGS="$L -lc"
				_target "install"		|| return 2
				LDFLAGS="$L -lc $LIBGCC $D$P/lib/start.o"
				SUBDIRS="$subdir"
				_target "install"		|| return 2
				;;
			System/src/libc)
				LDFLAGS="$L"
				_target "install"		|| return 2
				;;
			System/src/libSystem)
				SUBDIRS="$subdir/src $subdir/include \
						$subdir/data"
				LDFLAGS="$L -lc"
				_target "install"		|| return 2
				;;
			*)
				LDFLAGS="$L -lc $LIBGCC $D$P/lib/start.o"
				_target "install"		|| return 2
				;;
		esac
	done
	SUBDIRS="$S"
	LDFLAGS="$L"
	PKG_CONFIG_LIBDIR="$PKL"
	PKG_CONFIG_SYSROOT_DIR="$PKS"
}


#target_uninstall
target_uninstall()
{
	_target "uninstall"
}


#usage
_usage()
{
	echo "Usage: build.sh [-Dv][-O option=value...] target..." 1>&2
	echo "  -D	Run in debugging mode" 1>&2
	echo "  -v	Verbose mode" 1>&2
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


#warning
_warning()
{
	echo "build.sh: warning: $@" 1>&2
	return 2
}


#main
umask 022
#parse options
while getopts "DvO:" name; do
	case "$name" in
		D)
			DEBUG="_debug"
			;;
		v)
			VERBOSE=1
			;;
		O)
			export "${OPTARG%%=*}"="${OPTARG#*=}"
			;;
		*)
			_usage
			exit $?
			;;
	esac
done
shift $((OPTIND - 1))

#initialize target
[ -z "$MACHINE" ] && MACHINE=$(uname -m)
[ -z "$SYSTEM" ] && SYSTEM=$(uname -s)
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
		x86_64)
			MACHINE="amd64"
			;;
	esac
	case "$SYSTEM" in
		MINGW32_NT-?.?)
			SYSTEM="Windows"
			;;
	esac
	TARGET="$SYSTEM-$MACHINE"
fi
if [ ! -f "Apps/Devel/src/scripts/targets/$TARGET" ]; then
	_warning "$TARGET: Unsupported target" 1>&2
else
	. "Apps/Devel/src/scripts/targets/$TARGET"
fi

#initialize variables
[ -z "$CFLAGS" ] && CFLAGS="-Wall -g -O2 -ffreestanding"
if [ -z "$CONFIGURE" ]; then
	if [ -z "$PREFIX" ]; then
		CONFIGURE="configure -O DeforaOS"
	else
		CONFIGURE="configure -O DeforaOS -p $PREFIX"
	fi
fi
[ -z "$PREFIX" ] && PREFIX="/usr/local"
[ -z "$CPPFLAGS" ] && CPPFLAGS="-nostdinc -I $DESTDIR$PREFIX/include -D __DeforaOS__"
[ -z "$IMAGE_TYPE" ] && IMAGE_TYPE="image"
[ -z "$IMAGE_FILE" ] && IMAGE_FILE="$VENDOR-$IMAGE_TYPE.img"
[ -z "$LDFLAGS" ] && LDFLAGS="-nostdlib -L$DESTDIR$PREFIX/lib -Wl,-rpath-link,$DESTDIR$PREFIX/lib -Wl,-rpath,$PREFIX/lib"
[ -z "$LIBGCC" -a -n "$CC" ] && LIBGCC=$($CC -print-libgcc-file-name)
[ -z "$LIBGCC" ] && LIBGCC=$(cc -print-libgcc-file-name)
[ -z "$UID" ] && UID=`id -u`
[ -z "$SUDO" -a "$UID" -ne 0 ] && SUDO="sudo"

#run targets
if [ $# -lt 1 ]; then
	_usage
	exit $?
fi
while [ $# -gt 0 ]; do
	case "$1" in
		all|bootstrap|clean|distclean|image|install|uninstall)
			;;
		*)
			error "$1: Unknown target"
			_usage
			exit $?
			;;
	esac
	_info "Making target $1 on $TARGET"
	"target_$1"						|| exit 2
	shift
done

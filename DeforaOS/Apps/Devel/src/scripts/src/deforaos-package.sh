#!/bin/sh
#$Id$
#Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS Devel scripts
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



#environment
DEVNULL="/dev/null"
EMAIL=
ID="@ID@"
LICENSE=
METHOD=
PACKAGE=
PROJECTCONF="project.conf"
VERSION=
#executables
CKSUM="cksum"
GREP="grep"
MKDIR="mkdir -p"
RM="rm -f"
RMD160="rmd160"
SHA1="sha1"
SIZE="_size"
TR="tr"
WC="wc"
#dependencies
DEPEND_desktop=0
DEPEND_docbook=0
DEPEND_gtkdoc=0
DEPEND_pkgconfig=0


#functions
#deforaos_package
deforaos_package()
{
	revision="$1"

	_package_guess_name
	if [ $? -ne 0 ]; then
		_error "Could not determine the package name or version"
		return $?
	fi
	[ -z "$METHOD" ] && _package_guess_method
	[ -z "$LICENSE" ] && _package_guess_license
	_package_guess_dependencies
	_package_guess_email

	#call the proper packaging function
	case "$METHOD" in
		pkgsrc)
			_package_$METHOD "$revision"
			;;
		*)
			_error "Unsupported packaging method"
			return $?
			;;
	esac
}

_package_guess_dependencies()
{
	#desktop database
	DEPEND_desktop=0
	for i in data/*.desktop; do
		[ ! -f "$i" ] && continue
		DEPEND_desktop=1
	done

	#docbook-xsl
	DEPEND_docbook=0
	[ -f "doc/docbook.sh" ] && DEPEND_docbook=1

	#gtk-doc
	DEPEND_gtkdoc=0
	[ -f "doc/gtkdoc.sh" ] && DEPEND_gtkdoc=1

	#pkg-config
	DEPEND_pkgconfig=0
	$GREP "\`pkg-config " "src/Makefile" > "$DEVNULL" && DEPEND_pkgconfig=1
}

_package_guess_email()
{
	EMAIL="$USERNAME@defora.org"
}

_package_guess_license()
{
	[ ! -f "COPYING" ]					&& return 2

	#guess the license
	sum=$($CKSUM COPYING)
	sum=${sum%% *}
	case "$sum" in
		199341746)
			LICENSE="GNU GPL 3"
			;;
	esac
}

_package_guess_method()
{
	#guess the packaging method
	case $(uname -s) in
		NetBSD)
			METHOD="pkgsrc"
			;;
		*)
			_error "Unsupported platform"
			return $?
			;;
	esac
}

_package_guess_name()
{
	PACKAGE=
	VERSION=

	while read line; do
		case "$line" in
			"package="*)
				PACKAGE="${line#package=}"
				;;
			"version="*)
				VERSION="${line#version=}"
				;;
		esac
	done < "$PROJECTCONF"
	[ -z "$PACKAGE" -o -z "$VERSION" ]			&& return 2
	return 0
}


#package_pkgsrc
_package_pkgsrc()
{
	revision="$1"

	_info "Packaging for pkgsrc"

	#the archive is needed
	_info "Checking the source archive..."
	if [ ! -f "$PACKAGE-$VERSION.tar.gz" ]; then
		_error "The source archive could not be found"
		_error "Have you ran deforaos-release.sh first?"
		return 2
	fi

	distname="$PACKAGE-$VERSION"
	pkgname=$(echo "deforaos-$PACKAGE" | $TR A-Z a-z)

	$MKDIR "$pkgname"					|| return 2

	#check the license
	license=
	case "$LICENSE" in
		"GNU GPL 3")
			license="gnu-gpl-v3"
			;;
	esac
	[ -z "$license" ] && _warning "Unknown license"

	#DESCR
	_info "Creating $pkgname/DESCR..."
	#FIXME really implement
	cat > "$pkgname/DESCR" << EOF
DeforaOS $PACKAGE
EOF

	#Makefile
	_info "Creating $pkgname/Makefile..."
	cat > "$pkgname/Makefile" << EOF
# \$NetBSD\$

DISTNAME=	$distname
PKGNAME=	$pkgname-$VERSION
EOF
	[ $revision -gt 0 ] && cat >> "$pkgname/Makefile" << EOF
PKGREVISION=	$revision
EOF
	cat >> "$pkgname/Makefile" << EOF
CATEGORIES=	wip
MASTER_SITES=	http://www.defora.org/os/download/$ID/

MAINTAINER=	$EMAIL
HOMEPAGE=	http://www.defora.org/
COMMENT=	DeforaOS $PACKAGE
EOF
	[ -n "$license" ] && cat >> "$pkgname/Makefile" << EOF

LICENSE=	$license
EOF

	#tools
	#pkg-config
	[ $DEPEND_pkgconfig -eq 1 ] && cat >> "$pkgname/Makefile" << EOF

USE_TOOLS+=	pkg-config
EOF

	#build dependencies
	#docbook
	[ $DEPEND_docbook -eq 1 ] && cat >> "$pkgname/Makefile" << EOF

BUILD_DEPENDS+=	libxslt-[0-9]*:../../textproc/libxslt
BUILD_DEPENDS+=	docbook-xsl-[0-9]*:../../textproc/docbook-xsl
EOF
	#gtkdoc
	[ $DEPEND_gtkdoc -eq 1 ] && cat >> "$pkgname/Makefile" << EOF

EOF

	cat >> "$pkgname/Makefile" << EOF

PKG_DESTDIR_SUPPORT=	user-destdir
MAKE_FLAGS+=	PREFIX=${PREFIX}
MAKE_FLAGS+=	DESTDIR=${DESTDIR}
AUTO_MKDIRS=	yes
EOF

	[ $DEPEND_desktop -eq 1 ] && cat >> "$pkgname/Makefile" << EOF
.include "../../sysutils/desktop-file-utils/desktopdb.mk"
EOF
	cat >> "$pkgname/Makefile" << EOF

.include "../../mk/bsd.pkg.mk"
EOF

	#PLIST
	_info "Creating $pkgname/PLIST..."
	#FIXME really implement
	cat > "$pkgname/PLIST" << EOF
@comment \$NetBSD\$
EOF

	#distinfo
	_info "Creating $pkgname/distinfo..."
	#FIXME really implement
	cat > "$pkgname/distinfo" << EOF
\$NetBSD\$

EOF
	($SHA1 -- "$PACKAGE-$VERSION.tar.gz" &&
		$RMD160 -- "$PACKAGE-$VERSION.tar.gz" &&
		$SIZE -- "$PACKAGE-$VERSION.tar.gz") >> "$pkgname/distinfo"
	#FIXME also handle existing patches
	if [ $? -ne 0 ]; then
		$RM -- "$pkgname/distinfo"
		_error "Could not create $pkgname/distinfo"
	fi

	#FIXME handle existing options

	_info "The package is complete"

	#FIXME:
	#- run pkglint
	#- build the package
	#- review the differences (if any)
	#- commit
}


#error
_error()
{
	echo "deforaos-package.sh: error: $@" 1>&2
	return 2
}


#info
_info()
{
	echo "deforaos-package.sh: $@"
}


#size
_size()
{
	while getopts "" name; do
	done
	shift $((OPTIND - 1))
	[ $# -ne 1 ]						&& return 2
	size=$($WC -c "$1")
	for i in $size; do
		size="$i"
		break
	done
	echo "Size ($1) = $size bytes"
}


#usage
_usage()
{
	echo "Usage: deforaos-package.sh [-e e-mail][-i id][-l license][-m method] revision" 1>&2
	return 1
}


#warning
_warning()
{
	echo "deforaos-package.sh: warning: $@" 1>&2
}


#main
while getopts "e:i:l:m:" name; do
	case "$name" in
		e)
			EMAIL="$2"
			;;
		i)
			ID="$2"
			;;
		l)
			LICENSE="$2"
			;;
		m)
			METHOD="$2"
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
revision="$1"

deforaos_package "$revision"

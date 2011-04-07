/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <System.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "As/format.h"


/* PE */
/* private */
/* types */
#pragma pack(1)
struct pe_header
{
	char signature[4];
	uint16_t machine;
	uint16_t section_cnt;
	uint32_t timestamp;
	uint32_t symbol_offset;
	uint32_t symbol_cnt;
	uint16_t opthdr_size;
	uint16_t flags;
};

struct pe_msdos
{
	char signature[2];
	char _padding[0x3a];
	uint16_t offset;
};

struct pe_section_header
{
	char name[8];
	union
	{
		uint32_t paddr;
		uint32_t vsize;
	} misc;
	uint32_t vaddr;
	uint32_t raw_size;
	uint32_t raw_offset;
	uint32_t raw_reloc;
	uint32_t lines_offset;
	uint16_t reloc_cnt;
	uint16_t lines_cnt;
	uint32_t flags;
};
#pragma pack()


/* variables */
static const struct
{
	char const * arch;
	uint16_t machine;
} _pe_arch[] =
{
	{ "i386",	0x14c	},
	{ "i486",	0x14c	},
	{ "i586",	0x14c	},
	{ "i686",	0x14c	},
	{ NULL,		0x0	}
};

static char const _pe_msdos_signature[2] = "MZ";
static char const _pe_header_signature[4] = "PE\0\0";


/* prototypes */
/* plug-in */
static int _pe_init(FormatPlugin * format, char const * arch);
static char const * _pe_detect(FormatPlugin * format);

/* useful */
static char const * _pe_get_arch(uint16_t machine);
static int _pe_get_machine(char const * arch);


/* public */
/* variables */
FormatPlugin format_plugin =
{
	NULL,
	_pe_msdos_signature,
	sizeof(_pe_msdos_signature),
	_pe_init,
	NULL,
	NULL,
	NULL,
	_pe_detect,
	NULL,
	NULL
};


/* private */
/* functions */
/* pe_init */
static int _pe_init(FormatPlugin * format, char const * arch)
{
	int machine;
	struct pe_msdos pm;
	struct pe_header ph;

	if((machine = _pe_get_machine(arch)) < 0)
		return -1;
	memset(&pm, 0, sizeof(pm));
	memcpy(pm.signature, _pe_msdos_signature, sizeof(pm.signature));
	pm.offset = sizeof(pm);
	memset(&ph, 0, sizeof(ph));
	memcpy(ph.signature, _pe_header_signature, sizeof(ph.signature));
	ph.machine = _htol16(machine);
	ph.timestamp = _htol32(time(NULL));
	/* FIXME update the section and symbol lists */
	if(fwrite(&pm, sizeof(pm), 1, format->helper->fp) != 1)
		return -error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	if(fwrite(&ph, sizeof(ph), 1, format->helper->fp) != 1)
		return -error_set_code(1, "%s: %s", format->helper->filename,
				strerror(errno));
	return 0;
}


/* pe_detect */
static char const * _detect_error(FormatPlugin * format);

static char const * _pe_detect(FormatPlugin * format)
{
	struct pe_msdos pm;
	struct pe_header ph;

	if(fseek(format->helper->fp, 0, SEEK_SET) != 0)
		return _detect_error(format);
	if(fread(&pm, sizeof(pm), 1, format->helper->fp) != 1)
		return _detect_error(format);
	if((pm.offset = _htol16(pm.offset)) != sizeof(pm)
			&& fseek(format->helper->fp, pm.offset, SEEK_SET) != -1)
		return _detect_error(format);
	if(fread(&ph, sizeof(ph), 1, format->helper->fp) != 1)
		return _detect_error(format);
	ph.machine = _htol16(ph.machine);
	return _pe_get_arch(ph.machine);
}

static char const * _detect_error(FormatPlugin * format)
{
	error_set_code(1, "%s: %s", format->helper->filename, strerror(errno));
	return NULL;
}


/* accessors */
/* pe_get_arch */
static char const * _pe_get_arch(uint16_t machine)
{
	size_t i;

	for(i = 0; _pe_arch[i].arch != NULL; i++)
		if(_pe_arch[i].machine == machine)
			return _pe_arch[i].arch;
	error_set_code(1, "%u: %s", machine, "Unknown machine for PE");
	return NULL;
}


/* pe_get_machine */
static int _pe_get_machine(char const * arch)
{
	size_t i;

	for(i = 0; _pe_arch[i].arch != NULL; i++)
		if(strcmp(_pe_arch[i].arch, arch) == 0)
			return _pe_arch[i].machine;
	return -error_set_code(1, "%s: %s", arch,
			"Unsupported architecture for PE");
}

/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel asm */
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
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "Asm.h"


/* Java */
/* private */
/* types */
#pragma pack(1)
typedef struct _JavaHeader
{
	uint32_t magic;
	uint16_t minor;
	uint16_t major;
	uint16_t cp_cnt;
} JavaHeader;

typedef enum _JavaCpInfoTag
{
	CONSTANT_Utf8 = 1,
	CONSTANT_Integer = 3,
	CONSTANT_Float = 4,
	CONSTANT_Long = 5,
	CONSTANT_Double = 6,
	CONSTANT_Class = 7,
	CONSTANT_String = 8,
	CONSTANT_Fieldref = 9,
	CONSTANT_Methodref = 10,
	CONSTANT_InterfaceMethodref = 11,
	CONSTANT_NameAndType = 12
} JavaCpInfoTag;

typedef struct _JavaCpInfo
{
	uint8_t tag;
	char info[0];
} JavaCpInfo;

typedef struct _JavaHeader2
{
	uint16_t access;
	uint16_t this;
	uint16_t super;
	uint16_t interfaces_cnt;
} JavaHeader2;

typedef struct _JavaFieldInfo
{
	uint16_t access;
	uint16_t name;
	uint16_t descriptor;
	uint16_t attributes_cnt;
} JavaFieldInfo;

typedef struct _JavaAttributeInfo
{
	uint16_t name;
	uint32_t length;
	char info[0];
} JavaAttributeInfo;

typedef struct _JavaMethodInfo
{
	uint16_t access;
	uint16_t name;
	uint16_t descriptor;
	uint16_t attributes_cnt;

} JavaMethodInfo;
#pragma pack()

typedef struct _JavaPlugin
{
	char * class_name;
	char * super_name;
	uint16_t access_flags;
	uint16_t constants_cnt;
	uint16_t interfaces_cnt;
	uint16_t fields_cnt;
	uint16_t methods_cnt;
	uint16_t attributes_cnt;
} JavaPlugin;


/* variables */
/* plug-in */
static char _java_signature[4] = "\xca\xfe\xba\xbe";


/* prototypes */
/* plug-in */
static int _java_init(FormatPlugin * format, char const * arch);
static int _java_exit(FormatPlugin * format);
static char const * _java_detect(FormatPlugin * format);
static int _java_decode(FormatPlugin * format, int raw);
static int _java_decode_section(FormatPlugin * format, AsmSection * section,
		ArchInstructionCall ** calls, size_t * calls_cnt);


/* public */
/* variables */
/* format_plugin */
FormatPlugin format_plugin =
{
	NULL,
	"java",
	_java_signature,
	sizeof(_java_signature),
	_java_init,
	_java_exit,
	NULL,
	NULL,
	_java_detect,
	_java_decode,
	_java_decode_section,
	NULL
};


/* private */
/* functions */
/* java_init */
static int _java_init(FormatPlugin * format, char const * arch)
{
	JavaPlugin * java;

	if(arch != NULL && strcmp(arch, "java") != 0)
		return error_set_code(1, "%s: %s", arch,
				"Unsupported architecture for java");
#if 0 /* FIXME move this where appropriate */
	memcpy(&jh.magic, format->signature, format->signature_len);
	jh.minor = _htob16(0);
	jh.major = _htob16(0x32); /* XXX choose a more appropriate version */
	jh.cp_cnt = _htob16(0);
	if(helper->write(helper->format, &jh, sizeof(jh)) != sizeof(jh))
		return -1;
#endif
	if((java = object_new(sizeof(*java))) == NULL)
		return -1;
	memset(java, 0, sizeof(*java));
	format->priv = java;
	return 0;
}


/* java_exit */
static int _exit_constant_pool(FormatPlugin * format);
static int _exit_access_flags(FormatPlugin * format);
static int _exit_class_name(FormatPlugin * format);
static int _exit_super_name(FormatPlugin * format);
static int _exit_interface_table(FormatPlugin * format);
static int _exit_field_table(FormatPlugin * format);
static int _exit_method_table(FormatPlugin * format);
static int _exit_attribute_table(FormatPlugin * format);

static int _java_exit(FormatPlugin * format)
{
	int ret = 0;

	if(_exit_constant_pool(format) != 0
			|| _exit_access_flags(format) != 0
			|| _exit_class_name(format) != 0
			|| _exit_super_name(format) != 0
			|| _exit_interface_table(format) != 0
			|| _exit_field_table(format) != 0
			|| _exit_method_table(format) != 0
			|| _exit_attribute_table(format) != 0)
		ret = 1;
	free(format->priv);
	return ret;
}

static int _exit_constant_pool(FormatPlugin * format)
{
	FormatPluginHelper * helper = format->helper;
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->constants_cnt + 1);

	if(helper->write(helper->format, &cnt, sizeof(cnt)) != sizeof(cnt))
		return -1;
	/* XXX output the constants */
	return 0;
}

static int _exit_access_flags(FormatPlugin * format)
{
	FormatPluginHelper * helper = format->helper;
	JavaPlugin * java = format->priv;
	uint16_t flags = _htob16(java->access_flags);

	if(helper->write(helper->format, &flags, sizeof(flags))
			!= sizeof(flags))
		return -1;
	return 0;
}

static int _exit_class_name(FormatPlugin * format)
{
	FormatPluginHelper * helper = format->helper;
	uint16_t index = _htob16(0);

	/* FIXME really implement */
	if(helper->write(helper->format, &index, sizeof(index))
			!= sizeof(index))
		return -1;
	return 0;
}

static int _exit_super_name(FormatPlugin * format)
{
	FormatPluginHelper * helper = format->helper;
	uint16_t index = _htob16(0);

	/* FIXME really implement */
	if(helper->write(helper->format, &index, sizeof(index))
			!= sizeof(index))
		return -1;
	return 0;
}

static int _exit_interface_table(FormatPlugin * format)
{
	FormatPluginHelper * helper = format->helper;
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->interfaces_cnt);

	if(helper->write(helper->format, &cnt, sizeof(cnt)) != sizeof(cnt))
		return -1;
	/* XXX output the interfaces */
	return 0;
}

static int _exit_field_table(FormatPlugin * format)
{
	FormatPluginHelper * helper = format->helper;
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->fields_cnt);

	if(helper->write(helper->format, &cnt, sizeof(cnt)) != sizeof(cnt))
		return -1;
	/* XXX output the fields */
	return 0;
}

static int _exit_method_table(FormatPlugin * format)
{
	FormatPluginHelper * helper = format->helper;
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->methods_cnt);

	if(helper->write(helper->format, &cnt, sizeof(cnt)) != sizeof(cnt))
		return -1;
	/* XXX output the methods */
	return 0;
}

static int _exit_attribute_table(FormatPlugin * format)
{
	FormatPluginHelper * helper = format->helper;
	JavaPlugin * java = format->priv;
	uint16_t cnt = _htob16(java->attributes_cnt);

	if(helper->write(helper->format, &cnt, sizeof(cnt)) != sizeof(cnt))
		return -1;
	/* XXX output the attributes */
	return 0;
}


/* java_detect */
static char const * _java_detect(FormatPlugin * format)
{
	FormatPluginHelper * helper = format->helper;
	JavaHeader jh;

	if(helper->seek(helper->format, 0, SEEK_SET) != 0)
		return NULL;
	if(helper->read(helper->format, &jh, sizeof(jh)) != sizeof(jh))
		return NULL;
	if(memcmp(&jh.magic, _java_signature, sizeof(jh.magic)) != 0)
		return NULL;
	jh.minor = _htob16(jh.minor);
	jh.major = _htob16(jh.major);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u.%d\n", __func__, jh.major, jh.minor);
#endif
	return "java";
}


/* java_decode */
static int _java_decode(FormatPlugin * format, int raw)
{
	FormatPluginHelper * helper = format->helper;
	off_t end;

	/* XXX consider the whole file as a section */
	if((end = helper->seek(helper->format, 0, SEEK_END)) < 0)
		return -1;
	return helper->set_section(helper->format, 0, ".text", 0, end, 0);
}


/* java_decode_section */
static int _decode_attributes(FormatPlugin * format, uint16_t cnt,
		ArchInstructionCall ** calls, size_t * calls_cnt);
static int _decode_constants(FormatPlugin * format, uint16_t cnt);
static int _decode_fields(FormatPlugin * format, uint16_t cnt);
static int _decode_interfaces(FormatPlugin * format, uint16_t cnt);
static int _decode_methods(FormatPlugin * format, uint16_t cnt,
		ArchInstructionCall ** calls, size_t * calls_cnt);

static int _java_decode_section(FormatPlugin * format, AsmSection * section,
		ArchInstructionCall ** calls, size_t * calls_cnt)
{
	FormatPluginHelper * helper = format->helper;
	JavaHeader jh;
	JavaHeader2 jh2;
	uint16_t u16;

	/* read header */
	if(helper->seek(helper->format, section->offset, SEEK_SET)
			!= section->offset)
		return -1;
	if(helper->read(helper->format, &jh, sizeof(jh)) != sizeof(jh))
		return -1;
	/* skip constants */
	jh.cp_cnt = _htob16(jh.cp_cnt);
	if(jh.cp_cnt > 1 && _decode_constants(format, jh.cp_cnt) != 0)
		return -1;
	/* skip interfaces */
	if(helper->read(helper->format, &jh2, sizeof(jh2)) != sizeof(jh2))
		return -1;
	jh2.interfaces_cnt = _htob16(jh2.interfaces_cnt);
	if(_decode_interfaces(format, jh2.interfaces_cnt) != 0)
		return -1;
	/* skip fields */
	if(helper->read(helper->format, &u16, sizeof(u16)) != sizeof(u16))
		return -1;
	u16 = _htob16(u16);
	if(_decode_fields(format, u16) != 0)
		return -1;
	/* skip methods */
	if(helper->read(helper->format, &u16, sizeof(u16)) != sizeof(u16))
		return -1;
	u16 = _htob16(u16);
	if(_decode_methods(format, u16, calls, calls_cnt) != 0)
		return -1;
	return 0;
}

static int _decode_attributes(FormatPlugin * format, uint16_t cnt,
		ArchInstructionCall ** calls, size_t * calls_cnt)
{
	FormatPluginHelper * helper = format->helper;
	size_t i;
	JavaAttributeInfo jai;
	off_t offset;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, cnt);
#endif
	for(i = 0; i < cnt; i++)
	{
		if(helper->read(helper->format, &jai, sizeof(jai))
				!= sizeof(jai))
			return -1;
		jai.length = _htob32(jai.length);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() length=%u\n", __func__,
				jai.length);
#endif
		if(calls != NULL)
		{
			if((offset = helper->seek(helper->format, 0, SEEK_CUR))
					< 0
					|| helper->decode(helper->format,
						offset, jai.length, offset,
						calls, calls_cnt) != 0)
				return -1;
		}
		else if(helper->seek(helper->format, jai.length, SEEK_CUR) < 0)
				return -1;
	}
	return 0;
}

static int _decode_constants(FormatPlugin * format, uint16_t cnt)
{
	FormatPluginHelper * helper = format->helper;
	size_t i;
	JavaCpInfo jci;
	size_t size;
	char buf[8];
	uint16_t u16;
	off_t offset;
	AsmString * as;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, cnt);
#endif
	for(i = 1; i < cnt; i++)
	{
		if(helper->read(helper->format, &jci, sizeof(jci))
				!= sizeof(jci))
			return -1;
		switch(jci.tag)
		{
			case CONSTANT_Double:
			case CONSTANT_Long:
				size = 8;
				break;
			case CONSTANT_Fieldref:
			case CONSTANT_Float:
			case CONSTANT_Integer:
			case CONSTANT_InterfaceMethodref:
			case CONSTANT_Methodref:
			case CONSTANT_NameAndType:
				size = 4;
				break;
			case CONSTANT_Class:
				size = 2;
				break;
			case CONSTANT_String:
				size = sizeof(u16);
				if(helper->read(helper->format, &u16, size)
						!= (ssize_t)size)
					return -1;
				u16 = _htob16(u16);
				size = 0;
				as = helper->get_string_by_id(helper->format,
						u16 << 16);
				/* set the proper string ID */
				if(as != NULL && helper->set_string(
							helper->format, i, NULL,
							as->offset, as->size)
						< 0)
					return -1;
				/* XXX abuse set_string() to remember string */
				else if(as == NULL && helper->set_string(
							helper->format,
							u16 << 16, NULL, i, 0)
						< 0)
					return -1;
				break;
			case CONSTANT_Utf8:
				size = sizeof(u16);
				if(helper->read(helper->format, &u16, size)
						!= (ssize_t)size)
					return -1;
				u16 = _htob16(u16);
				if((offset = helper->seek(helper->format, 0,
								SEEK_CUR)) < 0)
					return -1;
				size = 0;
				if(helper->seek(helper->format, u16, SEEK_CUR)
						< 0)
					return -1;
				as = helper->get_string_by_id(helper->format,
						i << 16);
				/* we do not know this string yet */
				if(as == NULL && helper->set_string(
							helper->format, i << 16,
							NULL, offset, u16) < 0)
					return -1;
				/* we do know this string */
				else if(as != NULL && helper->set_string(
							helper->format,
							as->offset, NULL,
							offset, u16) < 0)
					return -1;
				break;
			default:
				return -error_set_code(1, "%s: %s 0x%x",
						helper->get_filename(
							helper->format),
						"Unknown constant tag",
						jci.tag);
		}
		if(size != 0 && helper->read(helper->format, buf, size)
				!= (ssize_t)size)
			return -1;
	}
	return 0;
}

static int _decode_fields(FormatPlugin * format, uint16_t cnt)
{
	FormatPluginHelper * helper = format->helper;
	size_t i;
	JavaFieldInfo jfi;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, cnt);
#endif
	for(i = 0; i < cnt; i++)
	{
		if(helper->read(helper->format, &jfi, sizeof(jfi))
				!= sizeof(jfi))
			return -1;
		jfi.attributes_cnt = _htob16(jfi.attributes_cnt);
		_decode_attributes(format, jfi.attributes_cnt, NULL, NULL);
	}
	return 0;
}

static int _decode_interfaces(FormatPlugin * format, uint16_t cnt)
{
	FormatPluginHelper * helper = format->helper;
	size_t i;
	uint16_t u16;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, cnt);
#endif
	for(i = 0; i < cnt; i++)
		if(helper->read(helper->format, &u16, sizeof(u16))
				!= sizeof(u16))
			return -1;
	return 0;
}

static int _decode_methods(FormatPlugin * format, uint16_t cnt,
		ArchInstructionCall ** calls, size_t * calls_cnt)
{
	FormatPluginHelper * helper = format->helper;
	size_t i;
	JavaMethodInfo jmi;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, cnt);
#endif
	for(i = 0; i < cnt; i++)
	{
		if(helper->read(helper->format, &jmi, sizeof(jmi))
				!= sizeof(jmi))
			return -1;
		jmi.attributes_cnt = _htob16(jmi.attributes_cnt);
		_decode_attributes(format, jmi.attributes_cnt, calls,
				calls_cnt);
	}
	return 0;
}

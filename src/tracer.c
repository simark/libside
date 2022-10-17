// SPDX-License-Identifier: MIT
/*
 * Copyright 2022 Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 */

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include <side/trace.h>

static
void tracer_print_struct(const struct side_type_description *type_desc, const struct side_arg_vec_description *sav_desc);
static
void tracer_print_array(const struct side_type_description *type_desc, const struct side_arg_vec_description *sav_desc);
static
void tracer_print_vla(const struct side_type_description *type_desc, const struct side_arg_vec_description *sav_desc);
static
void tracer_print_vla_visitor(const struct side_type_description *type_desc, void *app_ctx);
static
void tracer_print_array_fixint(const struct side_type_description *type_desc, const struct side_arg_vec *item);
static
void tracer_print_vla_fixint(const struct side_type_description *type_desc, const struct side_arg_vec *item);
static
void tracer_print_dynamic(const struct side_arg_dynamic_vec *dynamic_item);

static
void tracer_print_type(const struct side_type_description *type_desc, const struct side_arg_vec *item)
{
	switch (item->type) {
	case SIDE_TYPE_ARRAY_U8:
	case SIDE_TYPE_ARRAY_U16:
	case SIDE_TYPE_ARRAY_U32:
	case SIDE_TYPE_ARRAY_U64:
	case SIDE_TYPE_ARRAY_S8:
	case SIDE_TYPE_ARRAY_S16:
	case SIDE_TYPE_ARRAY_S32:
	case SIDE_TYPE_ARRAY_S64:
		if (type_desc->type != SIDE_TYPE_ARRAY) {
			printf("ERROR: type mismatch between description and arguments\n");
			abort();
		}
		break;
	case SIDE_TYPE_VLA_U8:
	case SIDE_TYPE_VLA_U16:
	case SIDE_TYPE_VLA_U32:
	case SIDE_TYPE_VLA_U64:
	case SIDE_TYPE_VLA_S8:
	case SIDE_TYPE_VLA_S16:
	case SIDE_TYPE_VLA_S32:
	case SIDE_TYPE_VLA_S64:
		if (type_desc->type != SIDE_TYPE_VLA) {
			printf("ERROR: type mismatch between description and arguments\n");
			abort();
		}
		break;

	default:
		if (type_desc->type != item->type) {
			printf("ERROR: type mismatch between description and arguments\n");
			abort();
		}
		break;
	}
	switch (item->type) {
	case SIDE_TYPE_U8:
		printf("%" PRIu8, item->u.side_u8);
		break;
	case SIDE_TYPE_U16:
		printf("%" PRIu16, item->u.side_u16);
		break;
	case SIDE_TYPE_U32:
		printf("%" PRIu32, item->u.side_u32);
		break;
	case SIDE_TYPE_U64:
		printf("%" PRIu64, item->u.side_u64);
		break;
	case SIDE_TYPE_S8:
		printf("%" PRId8, item->u.side_s8);
		break;
	case SIDE_TYPE_S16:
		printf("%" PRId16, item->u.side_s16);
		break;
	case SIDE_TYPE_S32:
		printf("%" PRId32, item->u.side_s32);
		break;
	case SIDE_TYPE_S64:
		printf("%" PRId64, item->u.side_s64);
		break;
	case SIDE_TYPE_STRING:
		printf("\"%s\"", item->u.string);
		break;
	case SIDE_TYPE_STRUCT:
		tracer_print_struct(type_desc, item->u.side_struct);
		break;
	case SIDE_TYPE_ARRAY:
		tracer_print_array(type_desc, item->u.side_array);
		break;
	case SIDE_TYPE_VLA:
		tracer_print_vla(type_desc, item->u.side_vla);
		break;
	case SIDE_TYPE_VLA_VISITOR:
		tracer_print_vla_visitor(type_desc, item->u.side_vla_app_visitor_ctx);
		break;
	case SIDE_TYPE_ARRAY_U8:
	case SIDE_TYPE_ARRAY_U16:
	case SIDE_TYPE_ARRAY_U32:
	case SIDE_TYPE_ARRAY_U64:
	case SIDE_TYPE_ARRAY_S8:
	case SIDE_TYPE_ARRAY_S16:
	case SIDE_TYPE_ARRAY_S32:
	case SIDE_TYPE_ARRAY_S64:
		tracer_print_array_fixint(type_desc, item);
		break;
	case SIDE_TYPE_VLA_U8:
	case SIDE_TYPE_VLA_U16:
	case SIDE_TYPE_VLA_U32:
	case SIDE_TYPE_VLA_U64:
	case SIDE_TYPE_VLA_S8:
	case SIDE_TYPE_VLA_S16:
	case SIDE_TYPE_VLA_S32:
	case SIDE_TYPE_VLA_S64:
		tracer_print_vla_fixint(type_desc, item);
		break;
	case SIDE_TYPE_DYNAMIC:
		tracer_print_dynamic(&item->u.dynamic);
		break;
	default:
		printf("<UNKNOWN TYPE>");
		abort();
	}
}

static
void tracer_print_field(const struct side_event_field *item_desc, const struct side_arg_vec *item)
{
	printf("(%s: ", item_desc->field_name);
	tracer_print_type(&item_desc->side_type, item);
	printf(")");
}

static
void tracer_print_struct(const struct side_type_description *type_desc, const struct side_arg_vec_description *sav_desc)
{
	const struct side_arg_vec *sav = sav_desc->sav;
	uint32_t side_sav_len = sav_desc->len;
	int i;

	if (type_desc->u.side_struct.nr_fields != side_sav_len) {
		printf("ERROR: number of fields mismatch between description and arguments of structure\n");
		abort();
	}
	printf("{ ");
	for (i = 0; i < side_sav_len; i++) {
		printf("%s", i ? ", " : "");
		tracer_print_field(&type_desc->u.side_struct.fields[i], &sav[i]);
	}
	printf(" }");
}

static
void tracer_print_array(const struct side_type_description *type_desc, const struct side_arg_vec_description *sav_desc)
{
	const struct side_arg_vec *sav = sav_desc->sav;
	uint32_t side_sav_len = sav_desc->len;
	int i;

	if (type_desc->u.side_array.length != side_sav_len) {
		printf("ERROR: length mismatch between description and arguments of array\n");
		abort();
	}
	printf("[ ");
	for (i = 0; i < side_sav_len; i++) {
		printf("%s", i ? ", " : "");
		tracer_print_type(type_desc->u.side_array.elem_type, &sav[i]);
	}
	printf(" ]");
}

static
void tracer_print_vla(const struct side_type_description *type_desc, const struct side_arg_vec_description *sav_desc)
{
	const struct side_arg_vec *sav = sav_desc->sav;
	uint32_t side_sav_len = sav_desc->len;
	int i;

	printf("[ ");
	for (i = 0; i < side_sav_len; i++) {
		printf("%s", i ? ", " : "");
		tracer_print_type(type_desc->u.side_vla.elem_type, &sav[i]);
	}
	printf(" ]");
}

struct tracer_visitor_priv {
	const struct side_type_description *elem_type;
	int i;
};

static
enum side_visitor_status tracer_write_elem_cb(const struct side_tracer_visitor_ctx *tracer_ctx,
			const struct side_arg_vec *elem)
{
	struct tracer_visitor_priv *tracer_priv = tracer_ctx->priv;

	printf("%s", tracer_priv->i++ ? ", " : "");
	tracer_print_type(tracer_priv->elem_type, elem);
	return SIDE_VISITOR_STATUS_OK;
}

static
void tracer_print_vla_visitor(const struct side_type_description *type_desc, void *app_ctx)
{
	enum side_visitor_status status;
	struct tracer_visitor_priv tracer_priv = {
		.elem_type = type_desc->u.side_vla_visitor.elem_type,
		.i = 0,
	};
	const struct side_tracer_visitor_ctx tracer_ctx = {
		.write_elem = tracer_write_elem_cb,
		.priv = &tracer_priv,
	};

	printf("[ ");
	status = type_desc->u.side_vla_visitor.visitor(&tracer_ctx, app_ctx);
	switch (status) {
	case SIDE_VISITOR_STATUS_OK:
		break;
	case SIDE_VISITOR_STATUS_ERROR:
		printf("ERROR: Visitor error\n");
		abort();
	}
	printf(" ]");
}

void tracer_print_array_fixint(const struct side_type_description *type_desc, const struct side_arg_vec *item)
{
	const struct side_type_description *elem_type = type_desc->u.side_array.elem_type;
	uint32_t side_sav_len = type_desc->u.side_array.length;
	void *p = item->u.side_array_fixint;
	enum side_type side_type;
	int i;

	if (elem_type->type != SIDE_TYPE_DYNAMIC) {
		switch (item->type) {
		case SIDE_TYPE_ARRAY_U8:
			if (elem_type->type != SIDE_TYPE_U8)
				goto type_error;
			break;
		case SIDE_TYPE_ARRAY_U16:
			if (elem_type->type != SIDE_TYPE_U16)
				goto type_error;
			break;
		case SIDE_TYPE_ARRAY_U32:
			if (elem_type->type != SIDE_TYPE_U32)
				goto type_error;
			break;
		case SIDE_TYPE_ARRAY_U64:
			if (elem_type->type != SIDE_TYPE_U64)
				goto type_error;
			break;
		case SIDE_TYPE_ARRAY_S8:
			if (elem_type->type != SIDE_TYPE_S8)
				goto type_error;
			break;
		case SIDE_TYPE_ARRAY_S16:
			if (elem_type->type != SIDE_TYPE_S16)
				goto type_error;
			break;
		case SIDE_TYPE_ARRAY_S32:
			if (elem_type->type != SIDE_TYPE_S32)
				goto type_error;
			break;
		case SIDE_TYPE_ARRAY_S64:
			if (elem_type->type != SIDE_TYPE_S64)
				goto type_error;
			break;
		}
		side_type = elem_type->type;
	} else {
		switch (item->type) {
		case SIDE_TYPE_ARRAY_U8:
			side_type = SIDE_TYPE_U8;
			break;
		case SIDE_TYPE_ARRAY_U16:
			side_type = SIDE_TYPE_U16;
			break;
		case SIDE_TYPE_ARRAY_U32:
			side_type = SIDE_TYPE_U32;
			break;
		case SIDE_TYPE_ARRAY_U64:
			side_type = SIDE_TYPE_U64;
			break;
		case SIDE_TYPE_ARRAY_S8:
			side_type = SIDE_TYPE_S8;
			break;
		case SIDE_TYPE_ARRAY_S16:
			side_type = SIDE_TYPE_S16;
			break;
		case SIDE_TYPE_ARRAY_S32:
			side_type = SIDE_TYPE_S32;
			break;
		case SIDE_TYPE_ARRAY_S64:
			side_type = SIDE_TYPE_S64;
			break;
		}
	}

	printf("[ ");
	for (i = 0; i < side_sav_len; i++) {
		struct side_arg_vec sav_elem = {
			.type = side_type,
		};

		switch (side_type) {
		case SIDE_TYPE_U8:
			sav_elem.u.side_u8 = ((const uint8_t *) p)[i];
			break;
		case SIDE_TYPE_S8:
			sav_elem.u.side_s8 = ((const int8_t *) p)[i];
			break;
		case SIDE_TYPE_U16:
			sav_elem.u.side_u16 = ((const uint16_t *) p)[i];
			break;
		case SIDE_TYPE_S16:
			sav_elem.u.side_s16 = ((const int16_t *) p)[i];
			break;
		case SIDE_TYPE_U32:
			sav_elem.u.side_u32 = ((const uint32_t *) p)[i];
			break;
		case SIDE_TYPE_S32:
			sav_elem.u.side_s32 = ((const int32_t *) p)[i];
			break;
		case SIDE_TYPE_U64:
			sav_elem.u.side_u64 = ((const uint64_t *) p)[i];
			break;
		case SIDE_TYPE_S64:
			sav_elem.u.side_s64 = ((const int64_t *) p)[i];
			break;

		default:
			printf("ERROR: Unexpected type\n");
			abort();
		}

		printf("%s", i ? ", " : "");
		tracer_print_type(elem_type, &sav_elem);
	}
	printf(" ]");
	return;

type_error:
	printf("ERROR: type mismatch\n");
	abort();
}

void tracer_print_vla_fixint(const struct side_type_description *type_desc, const struct side_arg_vec *item)
{
	const struct side_type_description *elem_type = type_desc->u.side_vla.elem_type;
	uint32_t side_sav_len = item->u.side_vla_fixint.length;
	void *p = item->u.side_vla_fixint.p;
	enum side_type side_type;
	int i;

	switch (item->type) {
	case SIDE_TYPE_VLA_U8:
		if (elem_type->type != SIDE_TYPE_U8)
			goto type_error;
		break;
	case SIDE_TYPE_VLA_U16:
		if (elem_type->type != SIDE_TYPE_U16)
			goto type_error;
		break;
	case SIDE_TYPE_VLA_U32:
		if (elem_type->type != SIDE_TYPE_U32)
			goto type_error;
		break;
	case SIDE_TYPE_VLA_U64:
		if (elem_type->type != SIDE_TYPE_U64)
			goto type_error;
		break;
	case SIDE_TYPE_VLA_S8:
		if (elem_type->type != SIDE_TYPE_S8)
			goto type_error;
		break;
	case SIDE_TYPE_VLA_S16:
		if (elem_type->type != SIDE_TYPE_S16)
			goto type_error;
		break;
	case SIDE_TYPE_VLA_S32:
		if (elem_type->type != SIDE_TYPE_S32)
			goto type_error;
		break;
	case SIDE_TYPE_VLA_S64:
		if (elem_type->type != SIDE_TYPE_S64)
			goto type_error;
		break;
	default:
		goto type_error;
	}
	side_type = elem_type->type;

	printf("[ ");
	for (i = 0; i < side_sav_len; i++) {
		struct side_arg_vec sav_elem = {
			.type = side_type,
		};

		switch (side_type) {
		case SIDE_TYPE_U8:
			sav_elem.u.side_u8 = ((const uint8_t *) p)[i];
			break;
		case SIDE_TYPE_S8:
			sav_elem.u.side_s8 = ((const int8_t *) p)[i];
			break;
		case SIDE_TYPE_U16:
			sav_elem.u.side_u16 = ((const uint16_t *) p)[i];
			break;
		case SIDE_TYPE_S16:
			sav_elem.u.side_s16 = ((const int16_t *) p)[i];
			break;
		case SIDE_TYPE_U32:
			sav_elem.u.side_u32 = ((const uint32_t *) p)[i];
			break;
		case SIDE_TYPE_S32:
			sav_elem.u.side_s32 = ((const int32_t *) p)[i];
			break;
		case SIDE_TYPE_U64:
			sav_elem.u.side_u64 = ((const uint64_t *) p)[i];
			break;
		case SIDE_TYPE_S64:
			sav_elem.u.side_s64 = ((const int64_t *) p)[i];
			break;

		default:
			printf("ERROR: Unexpected type\n");
			abort();
		}

		printf("%s", i ? ", " : "");
		tracer_print_type(elem_type, &sav_elem);
	}
	printf(" ]");
	return;

type_error:
	printf("ERROR: type mismatch\n");
	abort();
}

static
void tracer_print_dynamic_map(const struct side_arg_dynamic_event_map *map)
{
	const struct side_arg_dynamic_event_field *fields = map->fields;
	uint32_t len = map->len;
	int i;

	printf("[ ");
	for (i = 0; i < len; i++) {
		printf("%s", i ? ", " : "");
		printf("%s:: ", fields[i].field_name);
		tracer_print_dynamic(&fields[i].elem);
	}
	printf(" ]");
}

static
void tracer_print_dynamic_map_visitor(const struct side_arg_dynamic_vec *item)
{
	//TODO
}

static
void tracer_print_dynamic_vla(const struct side_arg_dynamic_vec_vla *vla)
{
	const struct side_arg_dynamic_vec *sav = vla->sav;
	uint32_t side_sav_len = vla->len;
	int i;

	printf("[ ");
	for (i = 0; i < side_sav_len; i++) {
		printf("%s", i ? ", " : "");
		tracer_print_dynamic(&sav[i]);
	}
	printf(" ]");
}

static
void tracer_print_dynamic_vla_visitor(const struct side_arg_dynamic_vec *item)
{
	//TODO
}

static
void tracer_print_dynamic(const struct side_arg_dynamic_vec *item)
{
	switch (item->type) {
	case SIDE_DYNAMIC_TYPE_NULL:
		printf("<NULL TYPE>");
		break;
	case SIDE_DYNAMIC_TYPE_U8:
		printf("%" PRIu8, item->u.side_u8);
		break;
	case SIDE_DYNAMIC_TYPE_U16:
		printf("%" PRIu16, item->u.side_u16);
		break;
	case SIDE_DYNAMIC_TYPE_U32:
		printf("%" PRIu32, item->u.side_u32);
		break;
	case SIDE_DYNAMIC_TYPE_U64:
		printf("%" PRIu64, item->u.side_u64);
		break;
	case SIDE_DYNAMIC_TYPE_S8:
		printf("%" PRId8, item->u.side_s8);
		break;
	case SIDE_DYNAMIC_TYPE_S16:
		printf("%" PRId16, item->u.side_s16);
		break;
	case SIDE_DYNAMIC_TYPE_S32:
		printf("%" PRId32, item->u.side_s32);
		break;
	case SIDE_DYNAMIC_TYPE_S64:
		printf("%" PRId64, item->u.side_s64);
		break;
	case SIDE_DYNAMIC_TYPE_STRING:
		printf("\"%s\"", item->u.string);
		break;
	case SIDE_DYNAMIC_TYPE_MAP:
		tracer_print_dynamic_map(item->u.side_dynamic_map);
		break;
	case SIDE_DYNAMIC_TYPE_MAP_VISITOR:
		tracer_print_dynamic_map_visitor(item);
		break;
	case SIDE_DYNAMIC_TYPE_VLA:
		tracer_print_dynamic_vla(item->u.side_dynamic_vla);
		break;
	case SIDE_DYNAMIC_TYPE_VLA_VISITOR:
		tracer_print_dynamic_vla_visitor(item);
		break;
	default:
		printf("<UNKNOWN TYPE>");
		abort();
	}
}

void tracer_call(const struct side_event_description *desc, const struct side_arg_vec_description *sav_desc)
{
	const struct side_arg_vec *sav = sav_desc->sav;
	uint32_t side_sav_len = sav_desc->len;
	int i;

	printf("provider: %s, event: %s, ", desc->provider_name, desc->event_name);
	if (desc->nr_fields != side_sav_len) {
		printf("ERROR: number of fields mismatch between description and arguments\n");
		abort();
	}
	for (i = 0; i < side_sav_len; i++) {
		printf("%s", i ? ", " : "");
		tracer_print_field(&desc->fields[i], &sav[i]);
	}
	printf("\n");
}

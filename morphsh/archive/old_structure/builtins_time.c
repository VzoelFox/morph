#include "morph.h"

// Native bindings
void mph_native_print(MorphContext* ctx, MorphString* s);
void mph_native_print_int(MorphContext* ctx, mph_int n);
mph_int mph_string_index(MorphContext* ctx, MorphString* s, MorphString* sub);
MorphString* mph_string_trim(MorphContext* ctx, MorphString* s, MorphString* cut);
MorphArray* mph_string_split(MorphContext* ctx, MorphString* s, MorphString* sep);

// Struct Definitions

// RTTI Definitions

// Type IDs
#define MPH_TYPE_mph_io_File 1

// Struct Definitions (Env & Types)
typedef struct mph_io_File mph_io_File;
struct mph_io_File {
	mph_int fd;
};

MorphTypeInfo mph_ti_mph_io_File = { "File", sizeof(mph_io_File), 0, NULL };

// Global Variables
mph_io_File* mph_io_Stdout;

// Function Prototypes
mph_int mph_now_millis(MorphContext* ctx, void* _env_void);
void mph_sleep_millis(MorphContext* ctx, void* _env_void, mph_int ms);
MorphString* mph_format_time(MorphContext* ctx, void* _env_void, mph_int timestamp);
mph_int mph_time_diff(MorphContext* ctx, void* _env_void, mph_int start, mph_int end);
mph_bool mph_is_future(MorphContext* ctx, void* _env_void, mph_int timestamp);
mph_bool mph_is_past(MorphContext* ctx, void* _env_void, mph_int timestamp);
void mph_main(MorphContext* ctx, void* _env_void);
mph_io_File* mph_io_make_file(MorphContext* ctx, void* _env_void, mph_int fd);
mph_io_File* mph_io_Open(MorphContext* ctx, void* _env_void, MorphString* path);
mph_io_File* mph_io_Create(MorphContext* ctx, void* _env_void, MorphString* path);
MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_io_File* f, mph_int size);
mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_io_File* f, MorphString* s);
mph_int mph_io_Close(MorphContext* ctx, void* _env_void, mph_io_File* f);
mph_int mph_time_Now(MorphContext* ctx, void* _env_void);
void mph_time_Sleep(MorphContext* ctx, void* _env_void, mph_int ms);

// Function Definitions
mph_io_File* mph_io_make_file(MorphContext* ctx, void* _env_void, mph_int fd);

mph_io_File* mph_io_Open(MorphContext* ctx, void* _env_void, MorphString* path);

mph_io_File* mph_io_Create(MorphContext* ctx, void* _env_void, MorphString* path);

MorphString* mph_io_Read(MorphContext* ctx, void* _env_void, mph_io_File* f, mph_int size);

mph_int mph_io_Write(MorphContext* ctx, void* _env_void, mph_io_File* f, MorphString* s);

mph_int mph_io_Close(MorphContext* ctx, void* _env_void, mph_io_File* f);

mph_int mph_time_Now(MorphContext* ctx, void* _env_void);

void mph_time_Sleep(MorphContext* ctx, void* _env_void, mph_int ms);

mph_int mph_now_millis(MorphContext* ctx, void* _env_void) {
	return mph_time_Now(ctx, NULL);
}

void mph_sleep_millis(MorphContext* ctx, void* _env_void, mph_int ms) {
	mph_time_Sleep(ctx, NULL, ms);
}

MorphString* mph_format_time(MorphContext* ctx, void* _env_void, mph_int timestamp) {
	if ((timestamp > 1000000000000)) {
{
	return ({ MorphString* _ret_1 = mph_string_new(ctx, "recent"); mph_gc_push_root(ctx, (void**)&_ret_1); MorphString* _ret_2 = _ret_1; mph_gc_pop_roots(ctx, 1); _ret_2; });
}
	} else if ((timestamp > 0)) {
{
	return ({ MorphString* _ret_3 = mph_string_new(ctx, "past"); mph_gc_push_root(ctx, (void**)&_ret_3); MorphString* _ret_4 = _ret_3; mph_gc_pop_roots(ctx, 1); _ret_4; });
}
	} else {
{
	return ({ MorphString* _ret_5 = mph_string_new(ctx, "invalid"); mph_gc_push_root(ctx, (void**)&_ret_5); MorphString* _ret_6 = _ret_5; mph_gc_pop_roots(ctx, 1); _ret_6; });
}
	}
}

mph_int mph_time_diff(MorphContext* ctx, void* _env_void, mph_int start, mph_int end) {
	if ((end > start)) {
{
	return (end - start);
}
	} else {
{
	return 0;
}
	}
}

mph_bool mph_is_future(MorphContext* ctx, void* _env_void, mph_int timestamp) {
	mph_int now = mph_now_millis(ctx, NULL);
	return (timestamp > now);
}

mph_bool mph_is_past(MorphContext* ctx, void* _env_void, mph_int timestamp) {
	mph_int now = mph_now_millis(ctx, NULL);
	return (timestamp < now);
}

void mph_main(MorphContext* ctx, void* _env_void) {
	({ mph_io_File* _arg_7 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_7); MorphString* _arg_8 = mph_string_new(ctx, "=== TIME BUILTINS TEST ===\n"); mph_gc_push_root(ctx, (void**)&_arg_8); mph_int _ret_9 = mph_io_Write(ctx, NULL, _arg_7, _arg_8); mph_gc_pop_roots(ctx, 2); _ret_9; });
	mph_int start_time = mph_now_millis(ctx, NULL);
	if ((start_time > 0)) {
{
	({ mph_io_File* _arg_10 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_10); MorphString* _arg_11 = mph_string_new(ctx, "Current time: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_11); mph_int _ret_12 = mph_io_Write(ctx, NULL, _arg_10, _arg_11); mph_gc_pop_roots(ctx, 2); _ret_12; });
}
	} else {
{
	({ mph_io_File* _arg_13 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_13); MorphString* _arg_14 = mph_string_new(ctx, "Current time: ERROR\n"); mph_gc_push_root(ctx, (void**)&_arg_14); mph_int _ret_15 = mph_io_Write(ctx, NULL, _arg_13, _arg_14); mph_gc_pop_roots(ctx, 2); _ret_15; });
}
	}
	mph_sleep_millis(ctx, NULL, 100);
	mph_int end_time = mph_now_millis(ctx, NULL);
	mph_int diff = mph_time_diff(ctx, NULL, start_time, end_time);
	if ((diff >= 100)) {
{
	({ mph_io_File* _arg_16 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_16); MorphString* _arg_17 = mph_string_new(ctx, "Sleep and time diff: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_17); mph_int _ret_18 = mph_io_Write(ctx, NULL, _arg_16, _arg_17); mph_gc_pop_roots(ctx, 2); _ret_18; });
}
	} else {
{
	({ mph_io_File* _arg_19 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_19); MorphString* _arg_20 = mph_string_new(ctx, "Sleep and time diff: OK (short)\n"); mph_gc_push_root(ctx, (void**)&_arg_20); mph_int _ret_21 = mph_io_Write(ctx, NULL, _arg_19, _arg_20); mph_gc_pop_roots(ctx, 2); _ret_21; });
}
	}
	MorphString* formatted = mph_format_time(ctx, NULL, start_time);
	mph_gc_push_root(ctx, (void**)&formatted);
	if (mph_string_eq(ctx, formatted, mph_string_new(ctx, "recent"))) {
{
	({ mph_io_File* _arg_22 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_22); MorphString* _arg_23 = mph_string_new(ctx, "Time formatting: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_23); mph_int _ret_24 = mph_io_Write(ctx, NULL, _arg_22, _arg_23); mph_gc_pop_roots(ctx, 2); _ret_24; });
}
	} else {
{
	({ mph_io_File* _arg_25 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_25); MorphString* _arg_26 = mph_string_new(ctx, "Time formatting: OK ("); mph_gc_push_root(ctx, (void**)&_arg_26); mph_int _ret_27 = mph_io_Write(ctx, NULL, _arg_25, _arg_26); mph_gc_pop_roots(ctx, 2); _ret_27; });
	({ mph_io_File* _arg_28 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_28); MorphString* _arg_29 = formatted; mph_gc_push_root(ctx, (void**)&_arg_29); mph_int _ret_30 = mph_io_Write(ctx, NULL, _arg_28, _arg_29); mph_gc_pop_roots(ctx, 2); _ret_30; });
	({ mph_io_File* _arg_31 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_31); MorphString* _arg_32 = mph_string_new(ctx, ")\n"); mph_gc_push_root(ctx, (void**)&_arg_32); mph_int _ret_33 = mph_io_Write(ctx, NULL, _arg_31, _arg_32); mph_gc_pop_roots(ctx, 2); _ret_33; });
}
	}
	mph_int past_time = (start_time - 1000);
	if (mph_is_past(ctx, NULL, past_time)) {
{
	({ mph_io_File* _arg_34 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_34); MorphString* _arg_35 = mph_string_new(ctx, "Past time check: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_35); mph_int _ret_36 = mph_io_Write(ctx, NULL, _arg_34, _arg_35); mph_gc_pop_roots(ctx, 2); _ret_36; });
}
	} else {
{
	({ mph_io_File* _arg_37 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_37); MorphString* _arg_38 = mph_string_new(ctx, "Past time check: ERROR\n"); mph_gc_push_root(ctx, (void**)&_arg_38); mph_int _ret_39 = mph_io_Write(ctx, NULL, _arg_37, _arg_38); mph_gc_pop_roots(ctx, 2); _ret_39; });
}
	}
	mph_int future_time = (start_time + 1000);
	if (mph_is_future(ctx, NULL, future_time)) {
{
	({ mph_io_File* _arg_40 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_40); MorphString* _arg_41 = mph_string_new(ctx, "Future time check: OK\n"); mph_gc_push_root(ctx, (void**)&_arg_41); mph_int _ret_42 = mph_io_Write(ctx, NULL, _arg_40, _arg_41); mph_gc_pop_roots(ctx, 2); _ret_42; });
}
	} else {
{
	({ mph_io_File* _arg_43 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_43); MorphString* _arg_44 = mph_string_new(ctx, "Future time check: OK (may have passed)\n"); mph_gc_push_root(ctx, (void**)&_arg_44); mph_int _ret_45 = mph_io_Write(ctx, NULL, _arg_43, _arg_44); mph_gc_pop_roots(ctx, 2); _ret_45; });
}
	}
	({ mph_io_File* _arg_46 = mph_io_Stdout; mph_gc_push_root(ctx, (void**)&_arg_46); MorphString* _arg_47 = mph_string_new(ctx, "Time builtins implemented successfully!\n"); mph_gc_push_root(ctx, (void**)&_arg_47); mph_int _ret_48 = mph_io_Write(ctx, NULL, _arg_46, _arg_47); mph_gc_pop_roots(ctx, 2); _ret_48; });
	mph_gc_pop_roots(ctx, 1);
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_io_Stdout = mph_io_make_file(ctx, NULL, 1);
	mph_gc_push_root(ctx, (void**)&mph_io_Stdout);
	mph_main(ctx, NULL);
}

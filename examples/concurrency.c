#include "morph.h"

// Native bindings
void mph_native_print(MorphContext* ctx, MorphString* s);
void mph_native_print_int(MorphContext* ctx, mph_int n);

void mph_worker(MorphContext* ctx, mph_int arg) {
	mph_native_print(ctx, mph_string_new(ctx, "Worker started with:"));
	mph_native_print_int(ctx, arg);
}

void mph_producer(MorphContext* ctx, MorphChannel* c) {
	mph_int i = 0;
	while ((i < 5)) {
{
	mph_channel_send(ctx, c, i);
	i = (i + 1);
}
	}
	mph_channel_send(ctx, c, (-1));
}

void mph_main(MorphContext* ctx) {
	mph_native_print(ctx, mph_string_new(ctx, "Start Main"));
	mph_thread_spawn((MorphEntryFunction)mph_worker, (void*)999);
	MorphChannel* c = mph_channel_new(ctx);
	mph_thread_spawn((MorphEntryFunction)mph_producer, (void*)c);
	mph_int val = 0;
	while ((val != (-1))) {
{
	val = mph_channel_recv(ctx, c);
	if ((val != (-1))) {
{
	mph_native_print(ctx, mph_string_new(ctx, "Received:"));
	mph_native_print_int(ctx, val);
}
	}
}
	}
	mph_native_print(ctx, mph_string_new(ctx, "Done Main"));
}


// Entry Point
void morph_entry_point(MorphContext* ctx) {
	mph_main(ctx);
}

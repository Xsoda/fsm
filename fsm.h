#ifndef __FSM_H__
#define __FSM_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fsm_state fsm_state_t;
typedef struct _fsm_event fsm_event_t;
typedef struct _fsm_context fsm_context_t;

typedef void(*fsm_state_cb)(fsm_context_t *ctx, fsm_state_t *s);
typedef void(*fsm_event_cb)(fsm_context_t *ctx, fsm_event_t *e);

struct _fsm_state {
   const char *state_name;
   fsm_state_cb enter; /* call when enter this state */
   fsm_state_cb leave; /* call when leave this state */
   struct _fsm_state *next;
   void *data;
};

struct _fsm_event {
   const char *event_name;
   fsm_event_cb before; /* call when before event emit */
   fsm_event_cb after;  /* call when after event emit */
   fsm_state_t *from;
   fsm_state_t *to;
   struct _fsm_event *next;
   void *data;
};

struct _fsm_context {
   fsm_state_t *current;
   fsm_event_cb before; /* 1. call when before any event emit */
   fsm_state_cb leave;  /* 2. call when leave any state */
   fsm_state_cb enter;  /* 3. call when enter any state */
   fsm_event_cb after;  /* 4. call when after any event emit */
   fsm_state_t *states;
   fsm_event_t *events;
   void *data;
};

int fsm_emit_event(fsm_context_t *ctx, const char *event_name);
int fsm_add_event(fsm_context_t *ctx, fsm_event_t *event);
int fsm_add_state(fsm_context_t *ctx, fsm_state_t *event);
void fsm_print(fsm_context_t *ctx);
fsm_context_t *fsm_create(fsm_state_t *initial, void *data);
void fsm_destory(fsm_context_t *ctx);
fsm_state_t *fsm_create_state(const char *state_name, fsm_state_cb enter, fsm_state_cb leave, void *data);
fsm_event_t *fsm_create_event(const char *event_name,
                              fsm_event_cb before, fsm_event_cb after,
                              fsm_state_t *from, fsm_state_t *to,
                              void *data);
bool fsm_is(fsm_context_t *fsm, const char *state_name);
bool fsm_can(fsm_context_t *ctx, const char *event_name);
bool fsm_cannot(fsm_context_t *ctx, const char *event_name);
const char *fsm_current(fsm_context_t *ctx);
fsm_state_t *fsm_find_state(fsm_context_t *ctx, const char *state_name);
fsm_event_t *fsm_find_event(fsm_context_t *ctx, const char *event_name, fsm_state_t *from, fsm_state_t *to);
int fsm_remove_event(fsm_context_t *ctx, fsm_event_t *event);

#ifdef __cplusplus
}
#endif

#endif  /* __FSM_H__ */
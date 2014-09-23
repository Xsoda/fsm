#ifndef __FSM_H__
#define __FSM_H__

typedef struct _fsm_state fsm_state_t;
typedef struct _fsm_event fsm_event_t;
typedef struct _fsm_context fsm_context_t;

typedef void(*fsm_state_cb)(fsm_context_t *ctx, fsm_state_t *s);
typedef void(*fsm_event_cb)(fsm_context_t *ctx, fsm_event_t *e);

struct _fsm_state {
   const char *sname;
   fsm_state_cb enter; /* call when enter this state */
   fsm_state_cb leave; /* call when leave this state */
   struct _fsm_state *next;
   void *data;
};

struct _fsm_event {
   const char *ename;
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

int fsm_emit_event(fsm_context_t *ctx, const char *name);
int fsm_add_event(fsm_context_t *ctx, fsm_event_t *e);
int fsm_add_state(fsm_context_t *ctx, fsm_state_t *e);
void fsm_print(fsm_context_t *ctx);
fsm_context_t *fsm_create(fsm_state_t *initial, void *data);
void fsm_destory(fsm_context_t *ctx);
fsm_state_t *fsm_create_state(const char *name, fsm_state_cb enter, fsm_state_cb leave, void *data);
fsm_event_t *fsm_create_event(const char *name,
                              fsm_event_cb before, fsm_event_cb after,
                              fsm_state_t *from, fsm_state_t *to,
                              void *data);

#endif  /* __FSM_H__ */
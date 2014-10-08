#include "fsm.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

int fsm_emit_event(fsm_context_t *ctx, const char *event_name)
{
   fsm_event_t *event = ctx->events;
   fsm_state_t *state;

   while (event) {
      if (event->from == ctx->current) {
         if (strcmp(event_name, event->event_name) == 0) {
            break;
         }
      }

      event = event->next;
   }

   if (event == NULL) {
      fprintf(stderr, "ERROR: cannot emit event `%s` at current state `%s`\n", event_name, ctx->current->state_name);
      return -1;
   }

   state = ctx->current;
   assert(state);

   if (ctx->before) {
      ctx->before(ctx, event);
   }

   if (event->before) {
      event->before(ctx, event);
   }

   if (ctx->leave) {
      ctx->leave(ctx, state);
   }

   if (state->leave) {
      state->leave(ctx, state);
   }

   ctx->current = event->to;
   state = ctx->current;

   if (ctx->enter) {
      ctx->enter(ctx, state);
   }

   if (state->enter) {
      state->enter(ctx, state);
   }

   if (ctx->after) {
      ctx->after(ctx, event);
   }

   if (event->after) {
      event->after(ctx, event);
   }

   return 0;
}

int fsm_add_event(fsm_context_t *ctx, fsm_event_t *event)
{
   if (event && ctx) {
      assert(event->next == event);
      event->next = ctx->events;
      ctx->events = event;
      return 0;
   }

   return -1;
}

int fsm_add_state(fsm_context_t *ctx, fsm_state_t *state)
{
   if (state && ctx) {
      assert(state->next == state);
      state->next = ctx->states;
      ctx->states = state;
      return 0;
   }

   return -1;
}

void fsm_print(fsm_context_t *ctx)
{
   fsm_event_t *event = ctx->events;
   fsm_state_t *end;
   fprintf(stdout, "digraph fsm {\n");

   while (event) {
      fprintf(stdout, "  \"%s\" -> \"%s\" [label = \"%s\"",
              event->from->state_name, event->to->state_name, event->event_name);

      if (!event->before && event->after) {
         fprintf(stdout, ", color = green, fontcolor = green");
      }

      if (event->before && !event->after) {
         fprintf(stdout, ", color = blue, fontcolor = blue");
      }

      if (event->before && event->after) {
         fprintf(stdout, ", color = red, fontcolor = red");
      }

      fprintf(stdout, "];\n");
      event = event->next;
   }

   fprintf(stdout, "}\n");
}

fsm_context_t *fsm_create(fsm_state_t *initial, void *data)
{
   fsm_context_t *ctx;

   if ((ctx = malloc(sizeof(fsm_context_t))) == NULL) {
      return NULL;
   }

   memset(ctx, 0, sizeof(fsm_context_t));
   ctx->data = data;
   ctx->current = initial;
   return ctx;
}

void fsm_destory(fsm_context_t *ctx)
{
   fsm_event_t *event;
   fsm_state_t *state;

   event = ctx->events;

   while (event) {
      ctx->events = event->next;
      free(event);
      event = ctx->events;
   }

   state = ctx->states;

   while (state) {
      ctx->states = state->next;
      free(state);
      state = ctx->states;
   }

   free(ctx);
}

fsm_state_t * fsm_create_state(const char *state_name, fsm_state_cb enter, fsm_state_cb leave, void *data)
{
   fsm_state_t *state;

   if ((state = malloc(sizeof(fsm_state_t))) == NULL) {
      return NULL;
   }

   memset(state, 0, sizeof(fsm_state_t));
   state->state_name = state_name;
   state->enter = enter;
   state->leave = leave;
   state->data = data;
   state->next = state;
   return state;
}

fsm_event_t * fsm_create_event(const char *event_name,
                               fsm_event_cb before, fsm_event_cb after,
                               fsm_state_t *from, fsm_state_t *to,
                               void *data)
{
   fsm_event_t *event;

   if ((event = malloc(sizeof(fsm_event_t))) == NULL) {
      return NULL;
   }

   event->event_name = event_name;
   event->before = before;
   event->after = after;
   event->from = from;
   event->to = to;
   event->next = event;
   event->data = data;
   return event;
}

bool fsm_is(fsm_context_t *ctx, const char *state_name)
{
   if (ctx && ctx->current) {
      if (strcmp(ctx->current->state_name, state_name) == 0) {
         return true;
      }
   }

   return false;
}

bool fsm_can(fsm_context_t *ctx, const char *event_name)
{
   fsm_event_t *event;

   if (ctx) {
      event = ctx->events;

      while (event) {
         if (ctx->current == event->from)
            if (!strcmp(event->event_name, event_name)) {
               return true;
            }
      }
   }

   return false;
}

bool fsm_cannot(fsm_context_t *ctx, const char *event_name)
{
   return !fsm_can(ctx, event_name);
}

const char *fsm_current(fsm_context_t *ctx)
{
   if (ctx && ctx->current) {
      return ctx->current->state_name;
   }

   return NULL;
}

fsm_state_t *fsm_find_state(fsm_context_t *ctx, const char *state_name)
{
   fsm_state_t *state = ctx->states;

   while (state) {
      if (strcmp(state->state_name, state_name) == 0) {
         break;
      }

      state = state->next;
   }

   return state;
}

fsm_event_t *fsm_find_event(fsm_context_t *ctx, const char *event_name, fsm_state_t *from, fsm_state_t *to)
{
   fsm_event_t *event = ctx->events;

   while (event) {
      if (event->from == from
            && event->to == to
            && !strcmp(event->event_name, event_name)) {
         break;
      }

      event = event->next;
   }

   return event;
}

int fsm_remove_event(fsm_context_t *ctx, fsm_event_t *event)
{
   fsm_event_t *prev = ctx->events;

   if (ctx->events == event && event) {
      ctx->events = event->next;
      event->next = NULL;
      return 0;
   }

   while (prev) {
      if (prev->next == event) {
         break;
      }
   }

   if (event && prev) {
      prev->next = event->next;
      event->next = NULL;
      return 0;
   }

   return -1;
}

/** #define __FSM_TEST__ /**/
#ifdef __FSM_TEST__
#include <crtdbg.h>
void enter(fsm_context_t *ctx, fsm_state_t *state)
{
   fprintf(stdout, "fsm: enter state `%s`\n", state->state_name);
}
void before(fsm_context_t *ctx, fsm_event_t *event)
{
   fprintf(stdout, "fsm: before emit event `%s`\n", event->event_name);
}
void after(fsm_context_t *ctx, fsm_event_t *event)
{
   fprintf(stdout, "fsm: after emit event `%s`\n", event->event_name);
}
void leave(fsm_context_t *ctx, fsm_state_t *state)
{
   fprintf(stdout, "fsm: leave state `%s`\n", state->state_name);
}
void enter_green(fsm_context_t *ctx, fsm_state_t *green)
{
   fprintf(stdout, "%s\n", __FUNCTION__);
}

void enter_yellow(fsm_context_t *ctx, fsm_state_t *yellow)
{
   fprintf(stdout, "%s\n", __FUNCTION__);
}

void enter_red(fsm_context_t *ctx, fsm_state_t *red)
{
   fprintf(stdout, "%s\n", __FUNCTION__);
}

void after_warn(fsm_context_t *ctx, fsm_event_t *warn)
{
   fprintf(stdout, "%s\n", __FUNCTION__);
}

void after_stop(fsm_context_t *ctx, fsm_event_t *stop)
{
   fprintf(stdout, "%s\n", __FUNCTION__);
}

void after_ready(fsm_context_t *ctx, fsm_event_t *ready)
{
   fprintf(stdout, "%s\n", __FUNCTION__);
}

void after_go(fsm_context_t *ctx, fsm_event_t *go)
{
   fprintf(stdout, "%s\n", __FUNCTION__);
}

int main()
{
   _CrtMemState start, end, diff;
   _CrtMemCheckpoint(&start);
   fsm_state_t *green = fsm_create_state("green", enter_green, NULL, NULL);
   fsm_state_t *yellow = fsm_create_state("yellow", enter_yellow, NULL, NULL);
   fsm_state_t *red = fsm_create_state("red", enter_red, NULL, NULL);
   fsm_event_t *warn = fsm_create_event("warn", NULL, after_warn, green, yellow, NULL);
   fsm_event_t *stop = fsm_create_event("stop", NULL, after_stop, yellow, red, NULL);
   fsm_event_t *ready = fsm_create_event("ready", NULL, after_ready, red, yellow, NULL);
   fsm_event_t *go = fsm_create_event("go", NULL, after_go, yellow, green, NULL);
   fsm_context_t *ctx = fsm_create(green, NULL);
   fsm_add_state(ctx, green);
   fsm_add_state(ctx, yellow);
   fsm_add_state(ctx, red);
   fsm_add_event(ctx, warn);
   fsm_add_event(ctx, stop);
   fsm_add_event(ctx, ready);
   fsm_add_event(ctx, go);
   ctx->after = after;
   ctx->before = before;
   ctx->enter = enter;
   ctx->leave = leave;
   fsm_emit_event(ctx, "warn");
   fsm_emit_event(ctx, "stop");
   fsm_emit_event(ctx, "ready");
   fsm_emit_event(ctx, "go");
   fsm_print(ctx);
   fsm_destory(ctx);
   _CrtMemCheckpoint(&end);

   if (_CrtMemDifference(&diff, &start, &end)) {
      _CrtMemDumpStatistics(&diff);
      _CrtMemDumpAllObjectsSince(&diff);
   }

   return 0;
}
#endif /* __FSM_TEST__ */
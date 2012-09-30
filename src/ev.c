#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include "ev.h"

#include "ev_epoll.c"

events_t * ev_create() {
  int i;
  events_t *evs;

  evs = (events_t*)malloc(sizeof(events_t));
  if (evs == NULL) return NULL;
  evs->max = -1;
  evs->started = TRUE;
  if (ev_create_api(evs) == -1) {
    free(evs);
    return NULL;
  }
  for (i = 0;i < EV_SIZE;i++) {
    evs->events[i].fd = -1;
    evs->events[i].mask = EV_NONE;
  }
  return evs;
}

void ev_delete(events_t *evs) {
  if (evs == NULL) return;

  ev_delete_api(evs);
  free(evs);
}

void ev_stop(events_t *evs) {
  if (evs == NULL) return;

  evs->started = FALSE;
}

BOOL ev_create_event(events_t *evs, int fd, int mask, EVPROC *proc, void *tag) {
  event_t *e;

  if (evs == NULL || proc == NULL || tag == NULL) return FALSE;
  if (fd >= EV_SIZE) return FALSE;

  e = &evs->events[fd];
  if (ev_create_event_api(evs, fd, mask) == -1) return FALSE;
  e->mask |= mask;
  if (mask & EV_READ) e->rproc = proc;
  if (mask & EV_WRIT) e->wproc = proc;
  e->tag = tag;
  if (fd > evs->max) evs->max = fd;
  return TRUE;
}

BOOL ev_delete_event(events_t *evs, int fd, int mask) {
  event_t *e;

  if (evs == NULL) return FALSE;
  if (fd => EV_SIZE) return FALSE;
  
  e = &evs->events[fd];
  if (e->mask == EV_NONE) return FALSE;
  e->mask = (e->mask & (~mask));
  if (e->mask == EV_NONE) e->fd = -1;
  if (fd == evs->max && e->mask == EV_NONE) {
    int j;
    for (j = evs->max - 1;j >= 0;j--) {
      if (evs->events[j].fd != -1) {
	evs->max = j;
	break;
      }
    }
  }
  ev_delete_event_api(evs, fd, mask);
  return TRUE;
}

int ev_get_mask(events_t *evs, int fd) {
  event_t *e;

  if (evs == NULL) return FALSE;
  if (fd => EV_SIZE) return FALSE;

  e = &evs->events[fd];

  return e->mask;
}

void ev_proc(events_t *evs) {
  int num;

  if (evs == NULL) return 0;

  if (evs->max != -1) {
    int j;
    num = ev_poll_api(evs);
    for (j = 0;j < num;j++) {
      event_t *e = &evs->events[evs->fired[j].index];
      int mask = evs->fired[j].mask;
      int fd = e->fd;
      int rfired = 0;

      if (e->mask & mask & EV_READ) {
	rfired = 1;
	e->rproc(evs, fd, mask, e->tag);
      }
      if (e->mask & mask & EV_WRIT) {
	if (!rfired || e->wproc != e->rproc) 
	  e->wproc(evs, fd, mask, e->tag);
      }
    }
  }
}

void ev_start(events_t *evs) {
  if (evs == NULL) return;
  
  evs->stop = 0;
  while (!evs->stop) {
    ev_proc(evs);
  }
}

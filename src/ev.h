#ifndef EV_H
#define EV_H

#include "utils.h"

#ifndef EV_SIZE
#ifndef EV_TINY_MODE
#define EV_SIZE (1024 * 5)
#else
#define EV_SIZE 16
#endif /*EV_TINY_MODE*/
#endif /*EV_SiZE*/

#define EV_INTERVAL 1000

#define EV_OK	0
#define EV_ERR	-1

#define EV_READ		1
#define EV_WRIT		2
#define EV_NONE		-1

struct _event_t;
struct _fired_t;
struct _events_t;

typedef void (*EVPROC)(struct _events_t *evs, int fd, int mask, void *tag);

typedef struct _event_t {
  int fd;
  int mask;
  EVPROC rproc;
  EVPROC wproc;
  void *tag;
}event_t;

typedef struct _fired_t {
  int fd;
  int mask;
}fired_t;

typedef struct _events_t {
  int max;
  BOOL started;
  size_t size;
  event_t events[EV_SIZE];
  fired_t fireds[EV_SIZE];
  void *tag;
}events_t;

events_t * ev_create();
void ev_delete(events_t *evs);
void ev_start(events_t *evs);
void ev_stop(events_t *evs);
void ev_proc(events_t *evs);

BOOL ev_create_event(events_t *evs, int fd, int mask, EVPROC proc, void *tag);
BOOL ev_delete_event(events_t *evs, int fd, int mask);

#endif /*EV_H*/

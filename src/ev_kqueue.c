#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

typedef struct _ev_api {
  int kqfd;
  struct kevent events[EV_SIZE];
}ev_api;

int ev_create_api(events_t *evs) {
  ev_api *api = (ev_api*)malloc(sizeof(ev_api));

  if (api == NULL) return -1;
  api->kqfd = kqueue();
  if (api->kqfd == -1) return -1;
  evs->tag = api;
}

void ev_delete_api(events_t *evs) {
  ev_api *api = evs->tag;

  close(api->kqfd);
  free(api);
  evs->tag = NULL;
}

int ev_create_event_api(events_t *evs, int fd, int mask) {
  ev_api *api = evs->tag;
  struct kevent ke;

  if (mask & EV_READ) {
    EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(api->kqfd, &ke, 1, NULL, 0, NULL) == -1) return -1;
  }
  if (mask & EV_WRIT) {
    EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    if (kevent(api->kqfd, &ke, 1, NULL, 0, NULL) == -1) return -1;
  }
  return 0;
}

void ev_delete_event_api(events_t *evs, int fd, int mask) {
  ev_api *api = evs->tag;
  struct kevent ke;

  if (mask & EV_READ) {
    EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    kevent(api->kqfd, &ke, 1, NULL, 0, NULL);
  }
  if (mask & EV_WRIT) {
    EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    kevent(api->kqfd, &ke, 1, NULL, 0, NULL);
  }
}

int ev_poll_api(events_t *evs) {
  ev_api *api = evs->tag;
  int retval, nums = 0;

  retval = kevent(api->kqfd, NULL, 0, api->events, EV_SIZE, NULL);

  if (retval > 0) {
    int j;
    
    nums = retval;
    for (j = 0;j < nums;j++) {
      int mask = 0;
      struct kevent *e = api->events + j;
      
      if (e->filter == EVFILT_READ) mask |= EV_READ;
      if (e->filter == EVFILT_WRITE) mask |= EV_WRIT;
      evs->fired[j].fd = e->ident;
      evs->fired[j].mask = mask;
    }
  }
  return nums;
}

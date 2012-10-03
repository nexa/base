#include <sys/epoll.h>


#define FIND_AND_CHECK(e, i, max, fd) {e = NULL;        \
  for (i = 0;i <= max;i++) {\
    if ((evs->events[i]).fd == fd) {e = &evs->events[i]; break;}}\
  if (e == NULL) return FALSE;}


typedef struct _ev_api {
  int epfd;
  struct epoll_event events[EV_SIZE];
}ev_api;

int ev_create_api(events_t *evs) {
  ev_api *api = (ev_api*)malloc(sizeof(ev_api));

  if (api == NULL) return -1;
  api->epfd = epoll_create(1024);
  if (api->epfd == -1) return -1;
  evs->tag = api;
  return 0;
}

void ev_delete_api(events_t *evs) {
  ev_api *api = evs->tag;

  close(api->epfd);
  free(api);
  evs->tag = NULL;
}

int ev_create_event_api(events_t *evs, int fd, int mask) {
  ev_api *api = evs->tag;
  struct epoll_event ee;
  
  int op = (evs->events[fd].mask == EV_NONE) ?
    EPOLL_CTL_ADD : EPOLL_CTL_MOD;
  ee.events = 0;
  mask |= evs->events[fd].mask;
  if (mask & EV_READ) ee.events |= EPOLLIN;
  if (mask & EV_WRIT) ee.events |= EPOLLOUT;
  ee.data.u64 = 0;
  ee.data.fd = fd;
  if (epoll_ctl(api->epfd, op, fd, &ee) == -1) return -1;
}

void ev_delete_event_api(events_t *evs, int fd, int mask) {
  ev_api *api = evs->tag;
  struct epoll_event ee;
  
  int mask_tmp = evs->events[fd].mask & (~mask);
  ee.events = 0;
  if (mask & EV_READ) ee.events |= EPOLLIN;
  if (mask & EV_WRIT) ee.events |= EPOLLOUT;
  ee.data.u64 = 0;
  ee.data.fd = fd;
  if (mask != EV_NONE) {
    epoll_ctl(api->epfd, EPOLL_CTL_MOD, fd, &ee);
  } else {
    epoll_ctl(api->epfd, EPOLL_CTL_DEL, fd, &ee);
  }
}

int ev_poll_api(events_t *evs) {
  ev_api *api = evs->tag;
  int retval, num = 0;
  
  retval = epoll_wait(api->epfd, api->events, EV_SIZE, EV_INTERVL);
  if (retval > 0) {
    int j;
    num = retval;
    for (j = 0;j < num;j++) {
      int mask = 0;
      struct epoll_event *ee = api->events + j;
      if (ee->events & EPOLLIN) mask |= EV_READ;
      if (ee->events & EPOLLOUT) mask |= EV_WRIT;
      evs->fired[j].fd = ee->data.fd;
      evs->fired[j].mask = mask;
    }
  } 
  return num;
}

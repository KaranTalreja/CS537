#include "cs537.h"
#include "request.h"
// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too

void getargs(int *port, int* num_threads, int argc, char *argv[])
{
    if (argc != 4) {
	fprintf(stderr, "Usage: %s <port> <threads> <buffers>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *num_threads = atoi(argv[2]);
    num_buffers = atoi(argv[3]);
}

void* read_from_buffer (void* arg)
{
  int i;
  int serviced;
  buffer_node_t* connfd = NULL;
  while (1) {
    while (NULL == connfd) {
      pthread_mutex_lock(&lock);
      serviced = 0;
      for (i = 0; i < num_buffers; i++) {
        if (1 == buffer[i].in_use) {
          buffer[i].in_use = 2;
          printf ("Servicing %d\n", buffer[i].connfd);
          fflush(stdout);
          connfd = &buffer[i];
          serviced = 1;
          break;
        }
      }
      if (0 == serviced) pthread_cond_wait(&read_cond, &lock);
      pthread_mutex_unlock(&lock);
    }
    requestHandle(connfd->connfd);
    Close(connfd->connfd);
    connfd->in_use = 0;
    connfd->connfd = 0;
    connfd = NULL;
  }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, num_threads;
    struct sockaddr_in clientaddr;

    getargs(&port, &num_threads, argc, argv);

    // 
    // CS537: Create some threads...
    //
    init();
    int i = 0;
    threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (i = 0; i<num_threads ; i++) {
      pthread_create (&threads[i], NULL, read_from_buffer, NULL);
    }
    listenfd = Open_listenfd(port);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
  printf("Adding %d\n", connfd);
  fflush(stdout);
  add_to_buffer(connfd);
    }
} 

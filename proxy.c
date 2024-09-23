#include "csapp.h"
#include "cache.h"

DLL *dllp;
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void trans(int client_to_proxy_fd);
void *thread(void *vargp);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  int listenfd, *connfdp;
  struct sockaddr_storage clientaddr;
  socklen_t clientlen;
  pthread_t tid;

  dllp = newDll();
  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(struct sockaddr_storage);
    connfdp = Malloc(sizeof(int));
    *connfdp = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
    Pthread_create(&tid, NULL, thread, connfdp);
    
  }
  return 0;
}

void *thread(void *vargp) {
  int connfd = *((int *)vargp);
  Pthread_detach(pthread_self());
  Free(vargp);
  trans(connfd);
  Close(connfd);
  return NULL;
}

void trans(int client_to_proxy_fd) {
  int proxyfd;
  int rc;
  char buf[MAXBUF], header[MAXBUF], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char path[MAXLINE], hostname[100], port[10];
  rio_t rio_client;
  rio_t rio_server;
  
  request_info request_info;
  node* dll_node;

  Rio_readinitb(&rio_client, client_to_proxy_fd);
  Rio_readlineb(&rio_client, buf, MAXBUF);
  strcpy(header, buf);
  printf("client header : %s\n", header);

  // Client header 정보 분석
  sscanf(header, "%s %s %s", method, uri, version);
  // TODO: 유효성 검증 추가 uri, version
  if (strcmp(method, "GET")) {
    return;
  }
  printf("method: %s\n", method);
  printf("uri: %s\n", uri);
  printf("version: %s\n", version);

  // uri 파싱해서 정보 가져오기
  parse_uri(uri, path, hostname, port);
  strcpy(request_info.method, method);
  request_info.path = path;
  printf("Client Requset Info : method - %p, path - %p\n", request_info.method, request_info.path);
  // 만약 클라이언트 요청이 이미 캐싱된 상태라면
  char cached_h[MAXLINE] = "";
  if ((dll_node = search(dllp, request_info)) != NULL) {
    snprintf(cached_h, MAXBUF - strlen(cached_h), "HTTP/1.0 200 OK\r\n");
    snprintf(cached_h + strlen(cached_h), MAXBUF - strlen(cached_h), "Server: Proxy Server\r\n");
    snprintf(cached_h + strlen(cached_h), MAXBUF - strlen(cached_h), "Connection: close\r\n");
    snprintf(cached_h + strlen(cached_h), MAXBUF - strlen(cached_h), "Content-length: %d\r\n\r\n", strlen(dll_node->response));

    printf("Cached Info : method - %p, path - %p\n", dll_node->request_info.method, dll_node->request_info.path);
    moveFront(dllp, dll_node);
    strcpy(buf, dll_node->response);
    Rio_writen(client_to_proxy_fd, cached_h, strlen(cached_h));
    Rio_writen(client_to_proxy_fd, buf, strlen(dll_node->response));
    
    return;
  }

  // proxy 소켓 생성 && proxy-server 연결
  proxyfd = Open_clientfd(hostname, port);
  
  // proxy -> server request 보내기
  sprintf(buf, "%s %s %s", method, path, version);
  Rio_writen(proxyfd, buf, strlen(buf));
  
  // client -> proxy 남은 헤더 요청 읽고 proxy -> server 요청 전송
  while((rc = Rio_readlineb(&rio_client, buf, MAXBUF)) > 0) {
    Rio_writen(proxyfd, buf, rc);
    if (strcmp(buf, "\r\n") == 0) {
      break;
    }
    printf("%s", buf);
  }

  // server -> proxy 응답 받아오고 proxy -> client 응답 보내주기
  printf("---Server Response----");
  Rio_readinitb(&rio_server, proxyfd);

  char response_h[MAXLINE] = "";
  char *data;
  int file_size;
  while ((rc  = Rio_readlineb(&rio_server, buf, MAXBUF)) > 0) {
    Rio_writen(client_to_proxy_fd, buf, strlen(buf));
    printf("buf: %s", buf);
    if (strstr(buf, "Content-length")) {
      sscanf(buf, "%*[^:]:%d", &file_size);
    }
    if (strcmp(buf, "\r\n") == 0) {
      break;
    }
  }
  printf("filesize: %d\n", file_size);
  data = (char *)Malloc(file_size);
  Rio_readnb(&rio_server, data, file_size);
  printf("%s\n", data);
  
  
  if (file_size <= MAX_OBJECT_SIZE) {
    // file_size 가 MAX보다 작다면 캐시에 넣어주기
    dll_node = (node *)Malloc(sizeof(node));
    dll_node->response = data;
    strcpy(dll_node->request_info.method, method);

    dll_node->request_info.path = (char *)Malloc(sizeof(path));
    strcpy(dll_node->request_info.path, path);
    pushFront(dllp, dll_node);
  } else {
    free(data);
  }
  Rio_writen(client_to_proxy_fd, data, file_size);
  printf("---Response End----");
}

void parse_uri(char *uri, char* path, char *hostname, char *port) {
    // 포트가 있을 경우만 포트 추출
    if (strstr(uri, ":")) {
        sscanf(uri, "http://%99[^:]:%9[^/]/", hostname, port);
    } else {
        sscanf(uri, "http://%99[^/]/", hostname);
    }
    sscanf(uri, "http://%*[^/]%s", path);

    printf("\n ####### parse_uri #######\n");
    printf("path: %s\n", path);
    printf("hostname: %s\n", hostname);
    printf("port: %s\n", port);
}
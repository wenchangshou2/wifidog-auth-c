// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved

#include "mongoose.h"
#include "tools.h"
#include <time.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/stat.h> 

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;
static void handle_reply_ping(struct mg_connection *nc, struct http_message *hm) {
  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_printf_http_chunk(nc, "Pong\n");
  mg_send_http_chunk(nc, "", 0);

}
static char* general_32_string(){
  const char CCH[] = "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
  static char ch[33]={0};
  srand(time(NULL));   
  for (int i = 0; i < 32; i++)
  {
    // int x=rand()/(32/(sizeof(CCH)-1));
    ch[i] = CCH[rand() % 36];
  }
  return ch;
}
static void handle_reply_portal(struct mg_connection *nc, struct http_message *hm) {
  mg_http_send_redirect(nc, 302, mg_mk_str("http://www.baidu.com"), mg_mk_str(NULL));

}
static void send_reply(struct mg_connection *nc, int state)
{
  char content[20]={0};
  sprintf(content,"Auth: %d",state);
  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_printf_http_chunk(nc, content);
  mg_send_http_chunk(nc, "", 0);
}
static void handle_reply_message(struct mg_connection *nc, struct http_message *hm) {
  char message[100];
  mg_get_http_var(&hm->query_string, "gw_port",message,sizeof(message));
  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
  mg_printf_http_chunk(nc,message );
  mg_send_http_chunk(nc, "", 0);
}
static void handle_reply_auth(struct mg_connection *nc, struct http_message *hm) {
  char filePath[100]={0};
  char token[100];
  char buffer[256];
  int nowTime=time(NULL);
  mg_get_http_var(&hm->query_string, "token", token, sizeof(token));
  FILE *fp = NULL;
  sprintf(filePath,"/tmp/wifidog-auth/%s",token);
  if(token[0]!='\0'){
    printf("%d\n",is_file_exist(filePath));
    if(is_file_exist(filePath)==-1){
      send_reply(nc,0);
    }else{
      fp = fopen(filePath, "r");
      fscanf(fp, "%s", buffer);
      fclose(fp);
      if((atoi(buffer)-nowTime)>3600*2){
        remove(filePath);
        send_reply(nc, 0);
      }else{
        send_reply(nc, 1);
      }
    }
  }else{
    send_reply(nc,0);
  }
}
static void handle_reply_login(struct mg_connection *nc, struct http_message *hm) {
  printf("login\n");
  char gw_address[100], gw_port[100], url[100];
  int iPort,rtu;
  mg_get_http_var(&hm->query_string, "gw_address", gw_address, sizeof(gw_address));
  mg_get_http_var(&hm->query_string, "gw_port",gw_port,sizeof(gw_port));
  mg_get_http_var(&hm->query_string, "gw_url", url, sizeof(url));
  if (gw_address[0] != '\0' && gw_port[0] != '\0')
  {
    char* token=general_32_string();
    FILE *fp = NULL;
    char filePath[100]={0};
    char redirectUrl[100]={0};
    if (is_dir_exist("/tmp/wifidog-auth")==-1)
    {
      mkdir("/tmp/wifidog-auth",0777);
    }
    sprintf(filePath, "/tmp/wifidog-auth/%s", token);
    fp=fopen(filePath,"w+");
    // fputc(fp, (char*)time(NULL));
    fprintf(fp, "%d",(int)time(NULL));
    fclose(fp);
    sprintf(redirectUrl, "http://%s:%s/wifidog/auth?token=%s", gw_address, gw_port, token);
    // Send response
    mg_http_send_redirect(nc, 302, mg_mk_str(redirectUrl), mg_mk_str(NULL));
  }else{
  }

  mg_printf(nc,"%s","HTTP/1.1 200 OK\r\nTransfer-Encoding:chunked\r\n\r\n");

}
/*http 请求处理*/
static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *) ev_data;
  switch (ev) {
      case MG_EV_HTTP_REQUEST:
        if (mg_vcmp(&hm->uri, "/auth/ping/") == 0) {
          printf("ping\n");
          // handle_sum_call(nc, hm); /* Handle RESTful call */
          handle_reply_ping(nc,hm);
        }
        else if (mg_vcmp(&hm->uri, "/auth/login/") == 0)
        {
          handle_reply_login(nc,hm);
        }else if(mg_vcmp(&hm->uri,"/auth/auth/")==0){
          handle_reply_auth(nc,hm);
        }
        else if (mg_vcmp(&hm->uri, "/auth/portal/") == 0)
        {
          handle_reply_portal(nc, hm);
        }else if(mg_vcmp(&hm->uri,"/auth/gw_message")==0){
          handle_reply_message(nc,hm);
        }
        else
        {
          mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
        }
        break;
      default:
        break;
    }
}

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  printf("Starting web server on port %s\n", s_http_port);
  nc = mg_bind(&mgr, s_http_port, ev_handler);
  if (nc == NULL) {
    printf("Failed to create listener\n");
    return 1;
  }
  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = ".";  // Serve current directory
  s_http_server_opts.enable_directory_listing = "yes";

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}

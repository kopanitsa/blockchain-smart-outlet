//
// Created by Okada, Takahiro on 2017/04/04.
//

#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>
#include <stdint.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define HTTP_SUCCESS 0
#define HTTP_ERROR   1

// will change based on API
typedef enum {
    TYPE_APPLICATION_JSON = 0,
    TYPE_APPLICATION_FORM_URL_ENCODE,
    TYPE_TEXT_HTML,
    TYPE_TEXT_PLAIN,
} mime_type_t;

typedef enum {
    CLOSE = 0,
    KEEP_ALIVE,
    UPGRADE
} connection_type_t;

#define URL_BUF_SIZE      256
#define QUERY_BUF_SIZE    256
#define REQUEST_BUF_SIZE  1024
#define RESPONSE_BUF_SIZE 1024

typedef struct {
    char host[128];
    uint16_t port;
    char url[URL_BUF_SIZE];
    char query[QUERY_BUF_SIZE];
    char body[REQUEST_BUF_SIZE];
    mime_type_t mime_type;
    connection_type_t connection_type;
    int content_length;
} http_request_t;

typedef struct {
    int response_code;
    int content_length;
    char body[RESPONSE_BUF_SIZE];
} http_response_t;


int http_init(char* host, char* path);
int http_secure_init(char* host, char* path);
int http_clean();
int http_secure_clean();

int http_get(http_request_t *request, http_response_t *response, bool is_secure);
int http_post(http_request_t *request, http_response_t *response, bool is_secure);
int http_put(http_request_t *request, http_response_t *response);
int http_simple_parse(char *json_str, int buf_size, char *key, char *value);


#endif //HTTP_H

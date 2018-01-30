//
// Created by Okada, Takahiro on 2017/04/04.
//

#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "log.h"
#if (SSLEAY_VERSION_NUMBER >= 0x0907000L)
# include <openssl/conf.h>
#endif
/**
 * Declaration
 * */
typedef enum {
    GET,
    POST,
    PUT,
    HEAD,
    DELETE,
} method_t;

#define HTTP_GET_STR    "GET"
#define HTTP_POST_STR   "POST"
#define HTTP_PUT_STR    "PUT"
#define HTTP_HEAD_STR   "HEAD"
#define HTTP_DELETE_STR "DELETE"

#define HEADER_BUF_SIZE 1024
#define CONNECTION_TYPE_BUF_SIZE 32
#define MIME_TYPE_BUF_SIZE 50

#define USER_AGENT      "User-Agent: esp"
#define ACCEPT_CHARSET  "Accept-Charset: utf-8"
#define HTTP_VERSION          "HTTP/1.1"
#define HTTP_HOST_KEY         "Host:"
#define CONTENT_LENGTH_LEY    "Content-Length:"

#define TYPE_APPLICATION_JSON_STR             "Content-Type: application/json"
#define TYPE_APPLICATION_FORM_URL_ENCODE_STR  "Content-Type: application/x-www-form-urlencoded"
#define TYPE_TEXT_HTML_STR                    "Content-Type: text/html"
#define TYPE_TEXT_PLAIN_STR                   "Content-Type: text/plain"

#define CONNECTION_CLOSE_STR       "Connection: close"
#define CONNECTION_KEEP_ALIVE_STR  "Connection: keep-alive"
#define CONNECTION_UPGRADE_STR     "Connection: Upgrade"

static int connect_to_server(http_request_t *request);
static void get_connection_type(connection_type_t type, char *str);
static void get_mime_type(mime_type_t type, char *str);
static void get_method_str(method_t method, char *str);

static int generate_request(http_request_t *request, char *str, method_t method);
static int send_request(char *req, int socket_id, bool isssl);

static int receive_json_response(http_response_t *response, int socket_id, bool is_secure);

static void init_openssl_library(void);
static void handleFailure(void);
static int verify_callback(int preverify, X509_STORE_CTX* x509_ctx);

/**
 * Public
 * */

int mysocket;
SSL *ssl;
SSL_CTX *ctx;
int err = 0;
struct hostent *servhost;
struct addrinfo *res;
struct sockaddr_in server;

const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
};

int http_init(char* host, char* path) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    char *service = "https";

    int err = 0;
    if ((err = getaddrinfo(host, service, &hints, &res)) != 0) {
        fprintf(stderr, "Fail to resolve ip address - %d\n", err);
        return EXIT_FAILURE;
    }

    if ((mysocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        fprintf(stderr, "Fail to create a socket.\n");
        return EXIT_FAILURE;
    }

    if (connect(mysocket, res->ai_addr, res->ai_addrlen) != 0) {
        printf("Connection error.\n");
        return EXIT_FAILURE;
    }

    return HTTP_SUCCESS;
}

int http_secure_init(char* host, char* path) {
    // IPアドレスの解決
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    char *service = "https";

    int err = 0;
    if ((err = getaddrinfo(host, service, &hints, &res)) != 0) {
        fprintf(stderr, "Fail to resolve ip address - %d\n", err);
        return EXIT_FAILURE;
    }

    if ((mysocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        fprintf(stderr, "Fail to create a socket.\n");
        return EXIT_FAILURE;
    }

    if (connect(mysocket, res->ai_addr, res->ai_addrlen) != 0) {
        printf("Connection error.\n");
        return EXIT_FAILURE;
    }

    SSL_library_init ();
    SSL_load_error_strings ();
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms ();

    ctx = SSL_CTX_new(SSLv23_client_method());
    ssl = SSL_new(ctx);
    err = SSL_set_fd(ssl, mysocket);
    int connection_ret = SSL_connect(ssl);
    if (connection_ret != 1) {
        unsigned long err = SSL_get_error(ssl, connection_ret);
        LOG("SSL connection error (%d)\r\n\r\n\r\n", err);
    } else {
        LOG("success to conntect to %s\r\n\r\n\r\n", host);
    }


    return EXIT_SUCCESS;
}

int http_clean() {
    close(mysocket);
    return HTTP_SUCCESS;
}

int http_secure_clean() {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ERR_free_strings();

    http_clean();

    return HTTP_SUCCESS;
}

int http_get(http_request_t *request, http_response_t *response, bool is_secure) {
    if (is_secure) {
        request->port = 443;
    }

    int socket_id = connect_to_server(request);
    if (socket_id == 0) {
        return HTTP_ERROR;
    }

    char req[HEADER_BUF_SIZE];
    int success = generate_request(request, req, GET);
    if (success != HTTP_SUCCESS) { return success; }

    success = send_request(req, socket_id, is_secure);
    if (success != HTTP_SUCCESS) { return success; }

    success = receive_json_response(response, socket_id, is_secure);
    if (success != HTTP_SUCCESS) { return success; }

    return HTTP_SUCCESS;
}

int http_post(http_request_t *request, http_response_t *response, bool is_secure) {
    if (is_secure) {
        request->port = 443;
    }

    int socket_id = connect_to_server(request);
    if (socket_id == 0) {
        return HTTP_ERROR;
    }

    char req[HEADER_BUF_SIZE];
    int success = generate_request(request, req, POST);
    if (success != HTTP_SUCCESS) { return success; }

    success = send_request(req, socket_id, is_secure);
    if (success != HTTP_SUCCESS) { return success; }

    success = receive_json_response(response, socket_id, is_secure);
    if (success != HTTP_SUCCESS) { return success; }

    return HTTP_SUCCESS;
}

int http_put(http_request_t *request, http_response_t *response) {
    int socket_id = connect_to_server(request);
    if (socket_id == 0) {
        return HTTP_ERROR;
    }

    char req[HEADER_BUF_SIZE];
    int success = generate_request(request, req, PUT);
    if (success != HTTP_SUCCESS) {
        return success;
    }

    success = send_request(req, socket_id, false);
    if (success != HTTP_SUCCESS) {
        return success;
    }

    success = receive_json_response(response, socket_id, false);
    if (success != HTTP_SUCCESS) {
        return success;
    }

    return HTTP_SUCCESS;
}

int http_simple_parse(char *json_str, int buf_size, char *key, char *value) {
    char *start = strstr(json_str, key);
    start = strstr(start, ":");
    start = strstr(start, "\"");
    char *tmp = strstr(start, "\""); // 1st double quote for value
    char *end = strstr(tmp+1, "\""); // 2nd double quote for value

    if (start > json_str + buf_size) { return HTTP_ERROR; }
    if (end > json_str + buf_size) { return HTTP_ERROR; }
    memcpy(value, start+1, end-start-1);

    return HTTP_SUCCESS;
}


/**
 * Private
 * */

// return socket id
static int connect_to_server(http_request_t *request) {
    struct addrinfo *res;

    char port[5];
    memset(port, 0, 5);
    if (request->port == 0) {
        memcpy(port, "80", 2); // default value
    } else {
        snprintf(port, 5, "%d", request->port);
    }

    LOG("get ip from host : %s", request->host);
    LOG("get ip from port : %s", port);

    int err = getaddrinfo(request->host, port, &hints, &res);
    if (err != 0 || res == NULL) {
        printf("DNS lookup failed err=%d res=%p\r\n", err, res);
        if (res) {
            freeaddrinfo(res);
        }
        return 0;
    }

    int socket_id;
    socket_id = socket(res->ai_family, res->ai_socktype, 0);
    if (socket_id < 0) {
        printf("... Failed to allocate socket.\r\n");
        freeaddrinfo(res);
        return 0;
    }

    if (connect(socket_id, res->ai_addr, res->ai_addrlen) != 0) {
        close(socket_id);
        freeaddrinfo(res);
        printf("... socket connect failed.\r\n");
        return 0;
    }

    freeaddrinfo(res);
    return socket_id;
}

static int generate_request(http_request_t *request, char *req, method_t method) {
    char connection_type[CONNECTION_TYPE_BUF_SIZE];
    get_connection_type(request->connection_type, connection_type);
    char mime_type[MIME_TYPE_BUF_SIZE];
    get_mime_type(request->mime_type, mime_type);

    char method_str[6];
    get_method_str(method, method_str);

    int ret = 0;
    if (method == GET) {
        ret = snprintf(req, HEADER_BUF_SIZE,
                       "%s %s%s %s\r\n%s %s\r\n%s\r\n%s\r\n%s\r\n%s\r\n\r\n",
                       method_str, request->url, request->query, HTTP_VERSION,
                       HTTP_HOST_KEY, request->host,
                       USER_AGENT,
                       ACCEPT_CHARSET,
                       mime_type,
                       connection_type
        );
    } else if (method == POST) {
        ret = snprintf(req, HEADER_BUF_SIZE,
                       "%s %s%s %s\r\n%s %s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s%d\r\n\r\n%s\r\n",
                       HTTP_POST_STR, request->url, request->query, HTTP_VERSION,
                       HTTP_HOST_KEY, request->host,
                       USER_AGENT,
                       ACCEPT_CHARSET,
                       mime_type,
                       connection_type,
                       CONTENT_LENGTH_LEY, request->content_length,
                       request->body
        );
    } else if (method == PUT) {
        ret = snprintf(req, HEADER_BUF_SIZE,
                       "%s %s%s %s\r\n%s %s\r\n%s\r\n%s\r\n%s\r\n%s%d\r\n\r\n%s\r\n",
                       HTTP_PUT_STR, request->url, request->query, HTTP_VERSION,
                       HTTP_HOST_KEY, request->host,
                       USER_AGENT,
                       mime_type,
                       connection_type,
                       CONTENT_LENGTH_LEY, request->content_length,
                       request->body
        );
    }

    LOG("HTTP request :\r\n%s", req);
    if (ret < 0 || ret >= HEADER_BUF_SIZE) {
        LOG("... header is too large. size: %d\r\n", ret);
        return HTTP_ERROR;
    }
    return HTTP_SUCCESS;
}

static int send_request(char *req, int socket_id, bool is_ssl) {
    if (is_ssl) {
        int ret = SSL_write(ssl, req, strlen(req));
        if (ret < 0) {
            int err = SSL_get_error(ssl, ret);
            LOG("... socket send failed in secure (%d)\r\n", err);
            close(socket_id);
            return HTTP_ERROR;
        }
    } else{
        if (write(socket_id, req, strlen(req)) < 0) {
            LOG("... socket send failed\r\n");
            close(socket_id);
            return HTTP_ERROR;
        }
    }
    return HTTP_SUCCESS;
}

static int receive_json_response(http_response_t *response, int socket_id, bool is_secure) {
    size_t TMP_BUF_SIZE = 128;
    int MAX_COUNT = 60;
    char tmp_buf[TMP_BUF_SIZE];
    char receive_buf[TMP_BUF_SIZE * MAX_COUNT];
    int count = 0;
    int r;

#ifdef ESP
    do {
        bzero(tmp_buf, TMP_BUF_SIZE);

        fd_set fdset;
        struct timeval tv;
        int rc = 0;
        FD_ZERO(&fdset);
        FD_SET(socket_id, &fdset);
        // It seems tv_sec actually means FreeRTOS tick
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        rc = select(socket_id + 1, &fdset, 0, 0, &tv);
        LOG("try. rc:%d", rc);
        if ((rc > 0) && (FD_ISSET(socket_id, &fdset))) {
            if (is_secure) {
                LOG("is secure");
                r = SSL_read(ssl, tmp_buf, TMP_BUF_SIZE - 1);
                LOG(tmp_buf);
            } else{
                LOG("is not secure");
                r = read(socket_id, tmp_buf, TMP_BUF_SIZE - 1);
            }
        } else {
            return HTTP_ERROR;
        }

        if (r > 0 && count < MAX_COUNT) {
            memcpy(receive_buf + (TMP_BUF_SIZE - 1) * count, tmp_buf, TMP_BUF_SIZE);
            count++;
        }

    } while (r > 0);
#else
    char buf[TMP_BUF_SIZE];
    int read_size;

    do {
        if (is_secure) {
            read_size = SSL_read(ssl, tmp_buf, TMP_BUF_SIZE - 1);
        } else{
            read_size = read(socket_id, tmp_buf, TMP_BUF_SIZE - 1);
        }
        memcpy(receive_buf + (TMP_BUF_SIZE - 1) * count, tmp_buf, TMP_BUF_SIZE);
        count++;
    } while(read_size > 0 && count < MAX_COUNT);
#endif


    LOG("start parse");
//    LOG("full: %s", receive_buf);
    int status_code = 0;
    status_code = atoi((char *) (receive_buf + (strlen("HTTP/1.1 "))));
    response->response_code = status_code;
    if (status_code == 0) {
        LOG("[Error] status code is zero");
        return HTTP_ERROR;
    }
    LOG("status code: %d", status_code);

    char *data;
    int content_length = 0;
    data = strcasestr((char *) receive_buf, "Content-Length:");
    content_length = atoi(data + (strlen("Content-Length:") + 1));
    response->content_length = content_length;

    // divide head and body
    data = strstr((char *) receive_buf, "{");
    if (data != NULL) {
        char *start = strchr((char *) receive_buf, '{');
        char *end = strrchr((char *) receive_buf, '}');
        if (start == NULL || end == NULL) {
            LOG("parsed data is broken");
            return HTTP_ERROR;
        }
        long content_len = end - start + 1;

        memset(response->body, 0, RESPONSE_BUF_SIZE);
        if (content_len > RESPONSE_BUF_SIZE) {
            LOG("data size is too large!! : %d", content_len);
            return HTTP_ERROR;
        }
        memcpy(response->body, data, (size_t) content_len);
    }

    return HTTP_SUCCESS;
}

static void get_method_str(method_t method, char *str) {
    switch (method) {
        case GET:
            memcpy(str, HTTP_GET_STR, sizeof(HTTP_GET_STR));
            break;
        case POST:
            memcpy(str, HTTP_POST_STR, sizeof(HTTP_POST_STR));
            break;
        case PUT:
            memcpy(str, HTTP_PUT_STR, sizeof(HTTP_PUT_STR));
            break;
        case DELETE:
            memcpy(str, HTTP_DELETE_STR, sizeof(HTTP_DELETE_STR));
            break;
        case HEAD:
            memcpy(str, HTTP_HEAD_STR, sizeof(HTTP_HEAD_STR));
            break;
        default:
            memcpy(str, HTTP_GET_STR, sizeof(HTTP_GET_STR));
    }
}

static void get_connection_type(connection_type_t type, char *str) {
    switch (type) {
        case CLOSE:
            memcpy(str, CONNECTION_CLOSE_STR, sizeof(CONNECTION_CLOSE_STR));
            break;
        case KEEP_ALIVE:
            memcpy(str, CONNECTION_KEEP_ALIVE_STR, sizeof(CONNECTION_KEEP_ALIVE_STR));
            break;
        case UPGRADE:
            memcpy(str, CONNECTION_UPGRADE_STR, sizeof(CONNECTION_UPGRADE_STR));
            break;
        default:
            memcpy(str, CONNECTION_CLOSE_STR, sizeof(CONNECTION_CLOSE_STR));
            break;
    }
}

static void get_mime_type(mime_type_t type, char *str) {
    switch (type) {
        case TYPE_APPLICATION_JSON:
            memcpy(str, TYPE_APPLICATION_JSON_STR, sizeof(TYPE_APPLICATION_JSON_STR));
            break;
        case TYPE_APPLICATION_FORM_URL_ENCODE:
            memcpy(str, TYPE_APPLICATION_FORM_URL_ENCODE_STR, sizeof(TYPE_APPLICATION_FORM_URL_ENCODE_STR));
            break;
        case TYPE_TEXT_HTML:
            memcpy(str, TYPE_TEXT_HTML_STR, sizeof(TYPE_TEXT_HTML_STR));
            break;
        case TYPE_TEXT_PLAIN:
            memcpy(str, TYPE_TEXT_PLAIN_STR, sizeof(TYPE_TEXT_PLAIN_STR));
            break;
        default:
            memcpy(str, TYPE_APPLICATION_JSON_STR, sizeof(TYPE_APPLICATION_JSON_STR));
            break;
    }
}

static void init_openssl_library(void)
{
    (void)SSL_library_init();

    SSL_load_error_strings();

    /* ERR_load_crypto_strings(); */

    OPENSSL_config(NULL);

    /* Include <openssl/opensslconf.h> to get this define */
#if defined (OPENSSL_THREADS)
    fprintf(stdout, "Warning: thread locking is not implemented\n");
#endif
}

static void handleFailure() {
    ERROR("failure");
}

static int verify_callback(int preverify, X509_STORE_CTX* x509_ctx)
{
    int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
    int err = X509_STORE_CTX_get_error(x509_ctx);

    X509* cert = X509_STORE_CTX_get_current_cert(x509_ctx);
    X509_NAME* iname = cert ? X509_get_issuer_name(cert) : NULL;
    X509_NAME* sname = cert ? X509_get_subject_name(cert) : NULL;

    LOG("Issuer (cn)", iname);
    LOG("Subject (cn)", sname);

    if(depth == 0) {
        /* If depth is 0, its the server's certificate. Print the SANs too */
        LOG("Subject (san)", cert);
    }

    return preverify;
}
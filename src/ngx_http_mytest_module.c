#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t   my_str;
    ngx_int_t   my_num;
    ngx_flag_t  my_flag;
    size_t      my_size;
    ngx_array_t*    my_str_array;
    ngx_array_t*    my_keyval;
    off_t           my_off;
    ngx_msec_t      my_msec;
    time_t          my_sec;
    ngx_bufs_t      my_bufs;
    ngx_uint_t      my_enum_seq;
    ngx_uint_t      my_bitmask;
    ngx_uint_t      my_access;
    ngx_path_t*     my_path;
} ngx_http_mytest_conf_t;

static char* ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r);
static void* ngx_http_mytest_create_loc_conf(ngx_conf_t* cf);

static ngx_command_t ngx_http_mytest_commands[] = {
        {
            ngx_string("mytest"),
            NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LMT_CONF|NGX_CONF_NOARGS,
            ngx_http_mytest,
            NGX_HTTP_LOC_CONF_OFFSET,
            0,
            NULL
        },
        {
            ngx_string("test_flag"),
            NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
            ngx_conf_set_flag_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_flag),
            NULL
        },
        {
            ngx_string("test_str"),
            NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_str_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_str),
            NULL
        },
        {
            ngx_string("test_str_array"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
            ngx_conf_set_str_array_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_str_array),
            NULL
        },
        {
            ngx_string("test_keyval"),
            NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
            ngx_conf_set_keyval_slot,
            NGX_HTTP_LOC_CONF_OFFSET,
            offsetof(ngx_http_mytest_conf_t, my_keyval),
            NULL
        },
        ngx_null_command
};

static char* ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    clcf->handler = ngx_http_mytest_handler;

    return NGX_CONF_OK;
}

static ngx_http_module_t ngx_http_mytest_module_ctx = {
        NULL,
        NULL,

        NULL,
        NULL,

        NULL,
        NULL,

        ngx_http_mytest_create_loc_conf,
        NULL
};

ngx_module_t ngx_http_mytest_module = {
        NGX_MODULE_V1,
        &ngx_http_mytest_module_ctx,
        ngx_http_mytest_commands,
        NGX_HTTP_MODULE,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NGX_MODULE_V1_PADDING
};

static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r)
{
    u_char ngx_my_str[1024] = {0};

    ngx_http_mytest_conf_t *my_conf;

    my_conf = ngx_http_get_module_loc_conf(r, ngx_http_mytest_module);

    if (my_conf->my_str.len != 0) {
        ngx_sprintf(ngx_my_str, "%s", my_conf->my_str.data);
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_str: %s", ngx_my_str);
    }

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_flag: %d", my_conf->my_flag);

    for (ngx_uint_t i = 0; i < my_conf->my_str_array->nelts; i++) {
        ngx_str_t* pstr = my_conf->my_str_array->elts;
        memset(ngx_my_str, 0, sizeof(ngx_my_str));
        ngx_sprintf(ngx_my_str, "%s", pstr[i].data);
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_str_array_%d: %s", i, ngx_my_str);
    }

    for (ngx_uint_t i = 0; i < my_conf->my_keyval->nelts; i++) {
        ngx_keyval_t* pkv = my_conf->my_keyval->elts;
        memset(ngx_my_str, 0, sizeof(ngx_my_str));
        ngx_sprintf(ngx_my_str, "%s", pkv[i].key.data);
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_keyval_%d_key: %s", i, ngx_my_str);
        memset(ngx_my_str, 0, sizeof(ngx_my_str));
        ngx_sprintf(ngx_my_str, "%s", pkv[i].value.data);
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_keyval_%d_value: %s", i, ngx_my_str);
    }

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    ngx_int_t rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    ngx_str_t type = ngx_string("text/plain");
    ngx_str_t response = ngx_string("hello world");

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = response.len;
    r->headers_out.content_type = type;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    ngx_buf_t *b;
    b = ngx_create_temp_buf(r->pool, response.len);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_memcpy(b->pos, response.data, response.len);
    b->last = b->pos + response.len;
    b->last_buf = 1;

    ngx_chain_t out;
    out.buf = b;
    out.next = NULL;

    return ngx_http_output_filter(r, &out);
}

static void* ngx_http_mytest_create_loc_conf(ngx_conf_t* cf)
{
    ngx_http_mytest_conf_t *mycf;

    mycf = (ngx_http_mytest_conf_t *) ngx_pcalloc(cf->pool, sizeof(ngx_http_mytest_conf_t));
    if (mycf == NULL) {
        return NULL;
    }

    mycf->my_flag = NGX_CONF_UNSET;
    mycf->my_num = NGX_CONF_UNSET;
    mycf->my_str_array = NGX_CONF_UNSET_PTR;
    mycf->my_keyval = NULL;
    mycf->my_off = NGX_CONF_UNSET;
    mycf->my_msec = NGX_CONF_UNSET_MSEC;
    mycf->my_sec = NGX_CONF_UNSET;
    mycf->my_size = NGX_CONF_UNSET_SIZE;

    return mycf;
}
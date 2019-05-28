#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r);
static char *ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);  
//处理配置项  
static ngx_command_t ngx_http_mytest_commands[] = {  
    {  
        ngx_string("mytest"),  //配置项名称
        NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LMT_CONF|NGX_CONF_NOARGS,  //类型 type
        ngx_http_mytest,  //set 函数 
        NGX_HTTP_LOC_CONF_OFFSET,//偏移  
        0,  
        NULL  
    },  
    ngx_null_command  
};  


//回调函数，出现my_test配置项后调用
static char *
ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
    ngx_http_core_loc_conf_t *clcf;
    //找到mytest所属配置块
    clcf=ngx_http_conf_get_module_loc_conf(cf,ngx_http_core_module);

    //主机域名、URI与mytest所在配置块匹配时，将调用handler
    clcf->handler=ngx_http_mytest_handler;
    return NGX_CONF_OK;
}
//ngx_http_module_t接口，模块上下文
//如果没有什么工作是必须在HTTP框架初始化时完成，则不必实现8个回调
static ngx_http_module_t ngx_http_mytest_module_ctx={
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

//新模块定义
ngx_module_t ngx_http_mytest_module={
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


//处理实际用户请求
static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r)  
{
    //请求方法必须是GET或者HEAD，否则返回405 \NOT ALLOWED
    if(!(r->method &(NGX_HTTP_GET|NGX_HTTP_HEAD)))
    {
        return NGX_HTTP_NOT_ALLOWED;
    }
    //丢弃请求中的包体
    ngx_int_t rc=ngx_http_discard_request_body(r);
    if(NGX_OK!=rc)
    {
        return rc;
    }

    //设置返回的Content-Type ngx_string是一个宏可以初始化data字段和len字段
    ngx_str_t type=ngx_string("text/plain");
    ngx_str_t response=ngx_string("Hello ZhangXiao World");
    //响应包体内容和状态码设置
    r->headers_out.status=NGX_HTTP_OK;
    r->headers_out.content_length_n=response.len;
    r->headers_out.content_type=type;

    //发送http头部
    rc=ngx_http_send_header(r);
    if(rc==NGX_ERROR || rc>NGX_OK || r->header_only)
    {
        return rc;
    }
    //构造ngx_buf_t结构体准备发送报文
    ngx_buf_t *b;
    b=ngx_create_temp_buf(r->pool,response.len);
    if(NULL==b)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    //拷贝响应报文
    ngx_memcpy(b->pos,response.data,response.len);
    b->last=b->pos+response.len;
    //声明这是最后一块缓冲区
    b->last_buf=1;

    //构造发送时的ngx_chain_t结构体
    ngx_chain_t out;
    out.buf=b;
    out.next=NULL;
    //发送响应，结束后HTTP框架会调用ngx_http_finalize_request方法结束请求
    return ngx_http_output_filter(r,&out);

}

#include "sim800.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"

//AT+CSMINS? sim status 0,1 - OK

//https://electronics.stackexchange.com/questions/231224/posting-to-php-server-using-sim900-gprs-with-at-commands
struct sim300_context_t;

typedef enum simx_cmd_t
{
    NO_AT = -1,
    AT_CPINR = 0x0,
    AT_CREG,
    AT_CMGF,
    AT_AT,
    AT_CMGS,
    AT_CSMINS,
    AT_CGATT,
    AT_CIPSHUT,
    AT_CIPSTATUS,
    AT_CIPMUX,
    AT_CSTT,
    AT_CIICR,
    AT_CIFSR,
    AT_CSQ,
    AT_CIPSTART,
    AT_CIPSEND,
    AT_CIPCLOSE,
    AT_CIPHEAD,
    AT_CSCS,
    AT_CMGR,
    AT_CGDCONT,
    ATD,
    ATO,
    PPP,
    AT_CIPCSGP,
}simx_cmd_t;

typedef struct
{
    char *str;
    uint8_t size;
}str_tocken_t;

static inline struct sim300_context_t *_gsm_context(void);

char* str_quotes(const char *src, char *dst, uint16_t maxlen);
void simx_finished(struct sim300_context_t *context);
void _simx_tcp_head_enable_finished(struct sim300_context_t *context);
void _simx_multiple_connection_finished(struct sim300_context_t *context);
void _simx_set_TE_character_finished(struct sim300_context_t *context);
void sim300_send_at_cmd_vp(struct sim300_context_t *context, sim_reply_t *reply, simx_cmd_t cmd, const char * format, ... );
void sim300_send_cmd_vp(struct sim300_context_t *context, sim_reply_t *reply, simx_cmd_t cmd, const char * format, ... );
void sim300_send_no_at_cmd(struct sim300_context_t *context, sim_reply_t *reply, simx_cmd_t cmd);
void sim300_send_at_cmd(struct sim300_context_t *context, sim_reply_t *reply, simx_cmd_t cmd);
uint8_t simx_is_receive(void);
void _simx_current_connection_status_parse(struct sim300_context_t *context, char *buffer, uint16_t length);
void _strchrcpy(char *src, str_tocken_t *tocken, uint8_t size);
uint8_t str_to_ip(char *strip, sim_ip_t *ip);
sim_tcp_mode_t str_to_tcp_mode(uint8_t *str);
uint8_t simx_check_message(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
sim_cip_state_t _simx_mux0_cipstate(uint8_t *buffer, uint16_t length);
sim_mux_cip_state_t _simx_mux1_cipstate(uint8_t *buffer, uint16_t length);
void _simx_context_send_at_init(struct sim300_context_t *context);

void simx_rcv_rframe(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void simx_rcv_wframe(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void simx_rcv_dframe(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void simx_rcv_mframe(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void simx_rcv_ipframe(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void simx_rcv_sdframe(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);

void _simx_pin_is_required_resp(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void _simx_network_registration_resp(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void _simx_sim_inserted_status_resp(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void _simx_is_attach_to_GPRS_resp(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void _simx_current_connection_status_resp(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void _simx_signal_quality_report_resp(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void _simx_sms_read_resp(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);

void _simx_notification_receive(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void _simx_notification_ipd_receive(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void _simx_notification_sms(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);
void _simx_notification(struct sim300_context_t *context, uint8_t *buffer, uint16_t length);

void simx_receive_tcp(struct sim300_context_t *context, uint8_t byte);
void simx_receive_msg(struct sim300_context_t *context, uint8_t byte);

void (*fsimx_receive)(struct sim300_context_t *context, uint8_t byte) = simx_receive_msg;

typedef enum
{
    AT_CMD_ST_ECHO,
    AT_CMD_ST_RN,
    AT_CMD_ST_CMD,
    AT_CMD_ST_MSG,
    AT_CMD_ST_STATUS,
}at_cmd_state_t;

typedef void(*fcmd_frame)(struct sim300_context_t *context, uint8_t *msg, uint16_t length);
typedef void(*fcmd_finished_t)(struct sim300_context_t *context);

typedef struct
{
    simx_cmd_t cmd;
    char *cmdstr;
    uint8_t len;
    fcmd_frame f_frm;
    fcmd_frame f_msg;
}simx_cmd_spec_t;

typedef struct
{
    char *cmdstr;
    uint8_t len;
    fcmd_frame f;
    simx_notification_t ntf;
}simx_notif_spec_t;

typedef struct
{
    char *cmdstr;
    uint8_t len;
    sim_status_t status;
}sim_status_spec_t;

typedef struct sim300_context_t
{
    simx_cmd_t cmd;
    sim_reply_t *reply;
    volatile uint8_t is_receive;
    volatile at_cmd_state_t state;
    char* cmd_data;
    
    uint8_t mux;
    uint8_t ciphead;
    uint8_t cipshowtp;
    sim_TE_chaster_t te_chaster;
    sim_pin_status_t pin_is_required;
    sim_reg_network_t network_reg;
    int8_t sim_is_insert;
    int8_t gprs_is_attach;
    sim_cip_state_t cip_state;
    volatile uint32_t time_ms;
    simx_notification_t simx_notification;
    
    uint8_t sim_cnt;
    uint16_t receive_len;
    uint8_t receive_ncon;
    uint8_t rx_buffer[SIM_RX_BUFFER];
    uint8_t tx_buffer[SIM_TX_BUFFER];
    
    fcmd_finished_t fcmd_finished;
    //debug
    volatile char *buffer2;
}sim300_context_t;

#define CMD_STR(a) .cmdstr = a, .len = sizeof(a) - 1
static const simx_cmd_spec_t simx_cmd_spec[] = 
    {{.cmd = AT_CPINR,     CMD_STR("CPIN?"),     .f_frm = simx_rcv_rframe,  .f_msg = _simx_pin_is_required_resp},
     {.cmd = AT_CREG,      CMD_STR("CREG?"),     .f_frm = simx_rcv_rframe,  .f_msg = _simx_network_registration_resp},
     {.cmd = AT_CMGF,      CMD_STR("CMGF="),     .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = AT_AT,        CMD_STR("AT"),        .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = AT_CMGS,      CMD_STR("CMGS="),     .f_frm = simx_rcv_wframe,  .f_msg = NULL},
     {.cmd = AT_CSMINS,    CMD_STR("CSMINS?"),   .f_frm = simx_rcv_rframe,  .f_msg = _simx_sim_inserted_status_resp},
     {.cmd = AT_CGATT,     CMD_STR("CGATT?"),    .f_frm = simx_rcv_rframe,  .f_msg = _simx_is_attach_to_GPRS_resp},
     {.cmd = AT_CIPSHUT,   CMD_STR("CIPSHUT"),   .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = AT_CIPSTATUS, CMD_STR("CIPSTATUS"), .f_frm = simx_rcv_mframe,  .f_msg = _simx_current_connection_status_resp},
     {.cmd = AT_CIPMUX,    CMD_STR("CIPMUX="),   .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = AT_CSTT,      CMD_STR("CSTT="),     .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = AT_CIICR,     CMD_STR("CIICR"),     .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = AT_CIFSR,     CMD_STR("CIFSR"),     .f_frm = simx_rcv_ipframe, .f_msg = NULL},
     {.cmd = AT_CSQ,       CMD_STR("CSQ"),       .f_frm = simx_rcv_rframe , .f_msg = _simx_signal_quality_report_resp},
     {.cmd = AT_CIPSTART,  CMD_STR("CIPSTART="), .f_frm = simx_rcv_dframe , .f_msg = NULL},
     {.cmd = AT_CIPSEND,   CMD_STR("CIPSEND="),  .f_frm = simx_rcv_sdframe, .f_msg = NULL},
     {.cmd = AT_CIPCLOSE,  CMD_STR("CIPCLOSE="), .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = AT_CIPHEAD,   CMD_STR("CIPHEAD="),  .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = AT_CSCS,      CMD_STR("CSCS="),     .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = AT_CMGR,      CMD_STR("CMGR="),     .f_frm = simx_rcv_rframe,  .f_msg = _simx_sms_read_resp},
     {.cmd = AT_CGDCONT,   CMD_STR("CGDCONT="),  .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = ATD,          CMD_STR("ATD"),       .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = ATO,          CMD_STR("ATO"),       .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = PPP,          CMD_STR("PPP"),       .f_frm = simx_rcv_dframe,  .f_msg = NULL},
     {.cmd = AT_CIPCSGP,   CMD_STR("CIPCSGP="),  .f_frm = simx_rcv_dframe,  .f_msg = NULL}};



simx_notif_spec_t simx_notif_spec[] = 
    {{CMD_STR("+RECEIVE,"),        .ntf = SIMX_NTF_UNKNOWN,    .f = _simx_notification_receive, },
     {CMD_STR("+IPD,"),            .ntf = SIMX_NTF_UNKNOWN,    .f = _simx_notification_ipd_receive, },
     {CMD_STR("+CMTI:"),           .ntf = SIMX_NTF_UNKNOWN,    .f = _simx_notification_sms},
     {CMD_STR("+PDP: DEACT"),      .ntf = SIMX_PDP_DEACT,      .f = _simx_notification},
     {CMD_STR("+CPIN: NOT READY"), .ntf = SIMX_CPIN_NOT_READY, .f = _simx_notification}};

static const sim_status_spec_t sim_status_spec[] = 
    {{CMD_STR("OK\r"), .status = SIM300_OK},
    {CMD_STR("ERROR\r"), .status = SIM300_ERROR},
    {CMD_STR("SHUT OK\r"), .status = SIM300_SHUT_OK},
    {CMD_STR("SEND OK\r"), .status = SIM300_SEND_OK},
    {CMD_STR("SEND FAIL\r"), .status = SIM300_SEND_FAIL},
    {CMD_STR("CLOSE OK\r"), .status = SIM300_CLOSE_OK},
    {CMD_STR("NO DIALTONE\r"), .status = SIM300_NO_DIALTONE},
    {CMD_STR("BUSY\r"), .status = SIM300_BUSY},
    {CMD_STR("NO CARRIER\r"), .status = SIM300_NO_CARRIER},
    {CMD_STR("NO ANSWER\r"), .status = SIM300_NO_ANSWER},
    {CMD_STR("CONNECT\r"), .status = SIM300_CONNECT},};

static inline sim300_context_t *_gsm_context()
{
    static sim300_context_t gsm_context = {
        .pin_is_required = SIM_PIN_UNKNOW, .is_receive = 1, 
        .cmd = NO_AT, .mux = 0,
        .fcmd_finished = NULL,
    };
    return &gsm_context;
}

void simx_receive_frame(sim300_context_t *context, uint8_t *buffer, uint16_t length);

/***********************UTILS********************************/
char* str_quotes(const char *src, char *dst, uint16_t maxlen)
{
    if(src == NULL)
    {
        return NULL;
    }
    char *c1 = strchr(src, '"');
    if(c1 == NULL)
    {
        return NULL;
    }
    c1++;
    char *c2 = strchr(c1, '"');
    if(c2 == NULL)
    {
        return NULL;
    }
    unsigned int len = (unsigned int)c2 - (unsigned int)c1;
    if(len == 1)
    {
        dst[0] = 0;
        return c2;
    }
    if(len < maxlen)
    {
        memcpy(dst, c1, len);
        dst[len] = 0;
        return c1 + len + 1;
    }
    else
    {
        return c2;
    }
}

void _strchrcpy(char *src, str_tocken_t *tocken, uint8_t size)
{
    uint32_t n = strlen(src);
    uint8_t cnt = 0;
    if(src[0] != ',')
    {
        tocken[cnt].str = src;
    }
    
    for(int i = 0; i < n; i++)
    {
        if(src[i] == ',')
        {
            if(++cnt == size)
            {
                break;
            }
            tocken[cnt].str = src + i + 1;
        }
        else
        {
            tocken[cnt].size++;
        }
    }
}



uint8_t str_to_ip(char *strip, sim_ip_t *ip)
{
    int v1, v2, v3, v4;
    int n = sscanf(strip, "%i.%i.%i.%i", &v1, &v2, &v3, &v4);
    if(n == 4)
    {
        ip->addr0 = v1;
        ip->addr1 = v2;
        ip->addr2 = v3;
        ip->addr3 = v4;
        return 0;
    }
    return -1;
}

sim_tcp_mode_t str_to_tcp_mode(uint8_t *str)
{
    sim_tcp_mode_t mode;
    if(memcmp(str, "TCP", 3) == 0)
    {
        mode = SIM_TCP;
    }
    else if(memcmp(str, "UDP", 3) == 0)
    {
        mode = SIM_UDP;
    }
    else
    {
        mode = -1;
    }
    return mode;
}
/*******************************************************/

void simx_tick_1ms(void)
{
    sim300_context_t *context = _gsm_context();
    if(context->is_receive == 0)
    {
        if(++context->time_ms > SIMX_TIMEOUT)
        {
            context->reply->status = SIM300_TIMEOUT;
            context->is_receive = 1;
            simx_callback_timeout();
        }
    }
}

void simx_finished(sim300_context_t *context)
{
     context->is_receive = 1;
     if(context->fcmd_finished != NULL)
     {
         context->fcmd_finished(context);
         context->fcmd_finished = NULL;
     }
}

void simx_rcv_rframe(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    buffer[length] = 0;
    //printf("frame: [%i]\n", length);
    //printf("frame: [%s] state: [%i]\n", buffer, context->state);
    //fflush(stdout);
    volatile char *test_buff = buffer;
    (void)test_buff;

    switch(context->state)
    {
    case AT_CMD_ST_ECHO:
        if(memcmp(buffer, context->tx_buffer, length) == 0)
        {
            context->state = AT_CMD_ST_RN;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_RN:
        if(length == 1)
        {
            context->state = AT_CMD_ST_CMD;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_CMD:
        
        if(memcmp(buffer + 1, simx_cmd_spec[context->cmd].cmdstr, 
                  simx_cmd_spec[context->cmd].len - 1) == 0)
        {
            //printf("frame: %s\n", simx_cmd_spec[context->cmd].cmdstr);
            context->state = AT_CMD_ST_MSG;
            if(simx_cmd_spec[context->cmd].f_msg != NULL)
            {
                simx_cmd_spec[context->cmd].f_msg(context, buffer, length);
            }
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_MSG:
        //printf("msg: %s\n", buffer);
        if(length == 1)
        {
            context->state = AT_CMD_ST_STATUS;
        }
        else
        {
            if(simx_cmd_spec[context->cmd].f_msg != NULL)
            {
                simx_cmd_spec[context->cmd].f_msg(context, buffer, length);
            }
        }
        break;
    case AT_CMD_ST_STATUS:
        //printf("status: %s\n", buffer);
        if(memcmp(buffer, "OK\r", 3) == 0)
        {
            context->reply->status = SIM300_OK;
        }
        else if(memcmp(buffer, "ERROR\r", 6) == 0)
        {
            context->reply->status = SIM300_ERROR;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
        }
        simx_finished(context);
        break;
    }
}

void simx_rcv_dframe(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    
    switch(context->state)
    {
    case AT_CMD_ST_ECHO:
        if(memcmp(buffer, context->tx_buffer, length) == 0)
        {
            context->state = AT_CMD_ST_RN;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_RN:
        if(length == 1 && buffer[0] == '\r')
        {
            context->state = AT_CMD_ST_STATUS;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_STATUS:
        if(length == 1 && buffer[0] == '\r') // TODO move to inline function
        {
            break;
        }
        
        for(uint8_t i = 0; i < sizeof(sim_status_spec) / sizeof(sim_status_spec[0]); i++)
        {
            if(memcmp(buffer, sim_status_spec[i].cmdstr, sim_status_spec[i].len) == 0)
            {
                context->reply->status = sim_status_spec[i].status;
                simx_finished(context);
                return;
            }
        }
        
        uint8_t *tmp = buffer;
        uint16_t len = length;
        if(context->mux)
        {
            tmp += 3;
            len -= 3;
        }
        if(memcmp(tmp, "CLOSE OK\r", len) == 0)
        {
            context->reply->status = SIM300_CLOSE_OK;
            simx_callback_tcp_msg(SIM_TCP_CLOSED, buffer[0] - 0x30);
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
        }
        
        simx_finished(context);
        break;
    }
}

void simx_rcv_wframe(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    switch(context->state)
    {
    case AT_CMD_ST_ECHO:
        if(memcmp(buffer, context->tx_buffer, length) == 0)
        {
            context->state = AT_CMD_ST_RN;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_RN:
        if(length == 1 && buffer[0] == '\r')
        {
            context->state = AT_CMD_ST_MSG;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_CMD:
        
        if(length == 1 && buffer[0] == '\r')
        {
            context->state = AT_CMD_ST_STATUS;
        } 
        else if(memcmp(buffer + 1, simx_cmd_spec[context->cmd].cmdstr, 
                  simx_cmd_spec[context->cmd].len - 1) == 0)
        {
            if(simx_cmd_spec[context->cmd].f_msg != NULL)
            {
                simx_cmd_spec[context->cmd].f_msg(context, buffer, length);
            }
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_MSG:
        if(length == 1 && buffer[0] == '\r')
        {
            context->state = AT_CMD_ST_CMD;
        } 
        else if(buffer[0] == '>')
        {
            sprintf((char*)context->tx_buffer, "%s%c", context->cmd_data, 0x1A); //TODO KOSTIL
            simx_callback_send(context->tx_buffer, strlen((const char*)context->tx_buffer));
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_STATUS:
        //printf("status: %s\n", buffer);
        if(memcmp(buffer, "OK\r", 3) == 0)
        {
            context->reply->status = SIM300_OK;
        }
        else if(memcmp(buffer, "ERROR\r", 6) == 0)
        {
            context->reply->status = SIM300_ERROR;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
        }
        simx_finished(context);
        break;
    }
}

void simx_rcv_mframe(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    switch(context->state)
    {
    case AT_CMD_ST_ECHO:
        if(memcmp(buffer, context->tx_buffer, length) == 0)
        {
            context->state = AT_CMD_ST_RN;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_RN:
        if(length == 1 && buffer[0] == '\r')
        {
            context->state = AT_CMD_ST_STATUS;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_MSG:
        
        if(simx_cmd_spec[context->cmd].f_msg != NULL)
        {
            simx_cmd_spec[context->cmd].f_msg(context, buffer, length);
        }
        
        break;
    case AT_CMD_ST_STATUS:
        if(memcmp(buffer, "OK\r", 3) == 0)
        {
            context->reply->status = SIM300_OK;
            context->state = AT_CMD_ST_MSG;
        }
        else if(memcmp(buffer, "ERROR\r", 6) == 0)
        {
            context->reply->status = SIM300_ERROR;
            simx_finished(context);
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    }
}

void simx_rcv_ipframe(struct sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    int addr0, addr1, addr2, addr3, n;
    switch(context->state)
    {
    case AT_CMD_ST_ECHO:
        if(memcmp(buffer, context->tx_buffer, length) == 0)
        {
            context->state = AT_CMD_ST_RN;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_RN:
        if(length == 1 && buffer[0] == '\r')
        {
            context->state = AT_CMD_ST_MSG;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_MSG:
        
        n = sscanf((char const*)buffer, "%i.%i.%i.%i", &addr0, &addr1, &addr2, &addr3);
        if(n == 4)
        {
            sim_ip_t *ip = (sim_ip_t *)context->reply->user_pointer;
            ip->addr0 = addr0;
            ip->addr1 = addr1;
            ip->addr2 = addr2;
            ip->addr3 = addr3;
            context->reply->status = SIM300_OK;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
        }
        simx_finished(context);
        break;
    }
}

void simx_rcv_sdframe(struct sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    switch(context->state)
    {
    case AT_CMD_ST_ECHO:
        if(memcmp(buffer, context->tx_buffer, length) == 0)
        {
            context->state = AT_CMD_ST_RN;
        }
        else
        {
            context->reply->status = SIM300_ERRFRAME;
            simx_finished(context);
        }
        break;
    case AT_CMD_ST_RN:
        context->state = AT_CMD_ST_MSG;
        break;
    case AT_CMD_ST_MSG:
        if(length == 2)
        {
            if(buffer[0] == '>')
            {
                for(uint32_t i = 0; i < 500000; i++)
                {
                    ___NOP;
                }
                simx_callback_send((uint8_t*)context->reply->user_pointer, context->reply->user_data);
            }
        }
        else if(length == 1 && buffer[0] == '\r')
        {
            
        }
        else 
        {
            uint8_t *tmp = buffer;
            uint16_t len = length;
            if(context->mux)
            {
                tmp += 3;
                len -= 3;
            }
            if(memcmp(tmp, "SEND OK\r", len) == 0)
            {
                context->reply->status = SIM300_SEND_OK;
                simx_finished(context);
            }
            else if(memcmp(tmp, "SEND FAIL\r", len) == 0)
            {
                context->reply->status = SIM300_SEND_FAIL;
                simx_finished(context);
            }
            else
            {
                context->reply->status = SIM300_ERRFRAME;
                simx_finished(context);
            }
        }
       
        break;
    }
}

void _simx_notification_receive(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    if(context->mux && length > 11)
    {
        int size, desc;
        uint8_t n = sscanf((char const*)buffer + 9, "%i,%i", &desc, &size);
        if(n == 2)
        {
            context->receive_ncon = desc;
            context->receive_len = size;
            fsimx_receive = simx_receive_tcp;
        }
    }
}

void _simx_notification_ipd_receive(struct sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    if(context->mux == 0 && context->ciphead)/* && 
            context->is_receive == 1 && context->rx_buffer[context->sim_cnt - 1] == ':')*/
    {
        if(memcmp(context->rx_buffer, "+IPD,", 5) == 0)
        {
            int size;
            int n = sscanf((char*)(context->rx_buffer + 5), "%i", &size);
            if(n == 1)
            {
                context->receive_ncon = 0;
                context->receive_len = size;
                context->sim_cnt = 0;
                fsimx_receive = simx_receive_tcp;
            }
        }
    }
}

void _simx_notification_sms(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    //TODO ADD MEM TYPE
    if(length > 10)
    {
        char mem[10] = {0};
        int n;
        uint8_t k = sscanf((char const *)(buffer + 6), "%*[^\"]\"%[^\"]\",%d", mem, &n);
        if(k == 2)
        {
            simx_callback_sms_received(n);
        }
        else
        {
            //TODO error frame
        }
    }
}

void _simx_notification(struct sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    simx_callback_message(context->simx_notification);
}

uint8_t simx_check_message(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    for(int i = 0; i < sizeof(simx_notif_spec) / sizeof(simx_notif_spec[0]); i++)
    {
        if(memcmp(buffer, simx_notif_spec[i].cmdstr, simx_notif_spec[i].len) == 0)
        {
            if(simx_notif_spec[i].f)
            {
                context->simx_notification = simx_notif_spec[i].ntf;
                simx_notif_spec[i].f(context, buffer, length);
                context->simx_notification = SIMX_NTF_UNKNOWN;
            }
            return 0;
        }
    }
    return 1;
}

void simx_receive_frame(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    uint8_t rescheck = simx_check_message(context, buffer, length);
    if(rescheck == 0)
    {
        if(context->is_receive == 0)
        {
            context->reply->status = SIM300_ERROR;
            simx_finished(context);
        }
        return;
    }
    
    if(context->is_receive == 1)
    {
        if(length > 2)
        {
            uint8_t n = 0;
            uint16_t len = length;
            char *tmpbuff = (char*)buffer;
            
            if(context->mux)
            {
                tmpbuff = (char*)(buffer + 3);
                n = buffer[0] - 0x30;
                len -= 3;
            }
            
            if(memcmp(tmpbuff, "CONNECT OK\r", len) == 0)
            {
                simx_callback_tcp_msg(SIM_TCP_CONNECT_OK, n);
            }
            else if(memcmp(tmpbuff, "CLOSED\r", len) == 0 || 
                    memcmp(tmpbuff, "CLOSE OK\r", len) == 0)
            {
                simx_callback_tcp_msg(SIM_TCP_CLOSED, n);
            }
            else if(memcmp(tmpbuff, "ALREADY CONNECT\r", len) == 0)
            {
                simx_callback_tcp_msg(SIM_TCP_ALREADY_CONNECT, n);
                
            }
            else if(memcmp(tmpbuff, "CONNECT FAIL\r", len) == 0)
            {
                simx_callback_tcp_msg(SIM_TCP_CONNECT_FAIL, n);
            }
        }
        return;
    }
    else
    {
        if(sizeof(simx_cmd_spec) / sizeof(simx_cmd_spec[0]) > context->cmd && context->cmd >= 0)
        {
            if(simx_cmd_spec[context->cmd].f_frm != NULL)
            {
                simx_cmd_spec[context->cmd].f_frm(context, buffer, length);
            }
        }
        else
        {
            //TODO add custom responce
        }
    }
}

void simx_receive_tcp(sim300_context_t *context, uint8_t byte)
{
    context->rx_buffer[context->sim_cnt] = byte;
    if(++context->sim_cnt == context->receive_len + 1) //todo overflow
    {
        context->rx_buffer[context->sim_cnt] = 0;
        simx_callback_tcp_data(context->rx_buffer + 1, context->receive_len, context->receive_ncon);
        context->receive_len = 0;
        context->receive_ncon = 0;
        fsimx_receive = simx_receive_msg;
        context->sim_cnt = 0;
        //callback
    }
}

void simx_receive_msg(sim300_context_t *context, uint8_t byte)
{
    context->rx_buffer[context->sim_cnt] = byte;
    context->sim_cnt++;
    
    if(byte == '\n')
    {
        context->sim_cnt = 0;
    }
    
    
    if(simx_cmd_spec[context->cmd].f_frm == simx_rcv_wframe || 
       simx_cmd_spec[context->cmd].f_frm == simx_rcv_sdframe)
    {
        if(context->rx_buffer[0] == '>' && context->sim_cnt == 2)
        {
            context->rx_buffer[context->sim_cnt] = 0;
            simx_receive_frame(context, context->rx_buffer, context->sim_cnt);
            context->sim_cnt = 0;
        }
    }
    
    if(byte == '\r')
    {
        context->buffer2 = context->rx_buffer;
        context->rx_buffer[context->sim_cnt] = 0;
        //printf("rec: [%s]\n", g_sim_rx_buffer);
        simx_receive_frame(context, context->rx_buffer, context->sim_cnt);
        context->sim_cnt = 0;
    }
}

void simx_receive(uint8_t byte)
{
    sim300_context_t *context = _gsm_context();
    fsimx_receive(context, byte);
}



uint8_t simx_is_receive()
{
    sim300_context_t *context = _gsm_context();
    return context->is_receive;
}

void simx_wait_reply(sim_reply_t *reply)
{
    static uint8_t cnt_timeout = 0;
    sim300_context_t *context = _gsm_context();
    if(reply->status == SIM300_BUSY || reply != context->reply)
    {
        return;
    }
    
    while(context->is_receive == 0)
    {
        if(context->time_ms > SIMX_TIMEOUT)
        {
            context->is_receive = 1;
            context->reply->status = SIM300_TIMEOUT;
            if(++cnt_timeout == 5)
            {
                cnt_timeout = 0;
                simx_callback_reset_modem();
            }
            break;
        }
        simx_callback_update();
    }
    if(reply->status == SIM300_ERRFRAME)
    {
        for(uint32_t i = 0; i < 5000000; i++)
        {
            ___NOP;
        }
        simx_callback_update();
    }
    
    if(reply->status != SIM300_TIMEOUT)
    {
        cnt_timeout = 0;
    }
    
    for(uint32_t i = 0; i < 5000000; i++)
    {
        ___NOP;
    }
}

void _simx_context_send_at_init(sim300_context_t *context)
{
    memset(context->rx_buffer, 0, SIM_RX_BUFFER);
    context->sim_cnt = 0;
    context->time_ms = 0;
    context->is_receive = 0;
}

void sim300_send_no_at_cmd(sim300_context_t *context, sim_reply_t *reply, simx_cmd_t cmd)
{
    if(context->is_receive == 1)
    {
        _simx_context_send_at_init(context);
        fsimx_receive = simx_receive_msg;
        context->state = AT_CMD_ST_ECHO;
        context->reply = reply;
        context->cmd = cmd;
        reply->status = SIM300_NULL;
        sprintf((char*)context->tx_buffer, "%s\r\n", simx_cmd_spec[cmd].cmdstr);
        simx_callback_send(context->tx_buffer, simx_cmd_spec[cmd].len + 2);
    }
    else
    {
        reply->status = SIM300_BUSY;
    }
}

void sim300_send_at_cmd(sim300_context_t *context, sim_reply_t *reply, simx_cmd_t cmd)
{
    if(context->is_receive == 1)
    {
        _simx_context_send_at_init(context);
        fsimx_receive = simx_receive_msg;
        context->state = AT_CMD_ST_ECHO;
        context->reply = reply;
        context->cmd = cmd;
        reply->status = SIM300_NULL;
        sprintf((char*)context->tx_buffer, "AT+%s\r\n", simx_cmd_spec[cmd].cmdstr);
        simx_callback_send(context->tx_buffer, simx_cmd_spec[cmd].len + 5);
    }
    else
    {
        reply->status = SIM300_BUSY;
    }
}

void sim300_send_cmd_vp(sim300_context_t *context, sim_reply_t *reply, simx_cmd_t cmd, const char * format, ... )
{
    if(context->is_receive == 1)
    {
        _simx_context_send_at_init(context);
        fsimx_receive = simx_receive_msg;
        context->state = AT_CMD_ST_ECHO;
        context->reply = reply;
        context->cmd = cmd;
        reply->status = SIM300_NULL;
        sprintf((char*)context->tx_buffer, "%s", simx_cmd_spec[cmd].cmdstr);
        uint16_t len = simx_cmd_spec[cmd].len;
        va_list args;
        va_start(args, format);
        vsprintf((char*)context->tx_buffer + len, format, args);
        va_end(args);
        len = strlen((const char*)context->tx_buffer);
        context->tx_buffer[len++] = '\r';
        context->tx_buffer[len++] = '\n';
        context->tx_buffer[len] = 0;
        
        simx_callback_send(context->tx_buffer, len);
    }
    else
    {
        reply->status = SIM300_BUSY;
    }
}

void sim300_send_at_cmd_vp(sim300_context_t *context, sim_reply_t *reply, simx_cmd_t cmd, const char * format, ... )
{
    if(context->is_receive == 1)
    {
        _simx_context_send_at_init(context);
        fsimx_receive = simx_receive_msg;
        context->state = AT_CMD_ST_ECHO;
        context->reply = reply;
        context->cmd = cmd;
        reply->status = SIM300_NULL;
        sprintf((char*)context->tx_buffer, "AT+%s", simx_cmd_spec[cmd].cmdstr);
        uint16_t len = simx_cmd_spec[cmd].len + 3;
        va_list args;
        va_start(args, format);
        vsprintf((char*)(context->tx_buffer + len), format, args);
        va_end(args);
        len = strlen((char*)context->tx_buffer);
        context->tx_buffer[len++] = '\r';
        context->tx_buffer[len++] = '\n';
        context->tx_buffer[len] = 0;
        
        simx_callback_send(context->tx_buffer, len);
    }
    else
    {
        reply->status = SIM300_BUSY;
    }
}

void simx_test(sim_reply_t *reply)
{
    sim300_context_t *context = _gsm_context();
    sim300_send_no_at_cmd(context, reply, AT_AT);
}

void _simx_pin_is_required_resp(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    static char *msg[] = {"READY", "SIM PIN", "SIM PUK", "PH_SIM PIN", 
                          "PH_SIM PUK", "SIM PIN2", "SIM PUK2"};
    for(int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++)
    {
        int n = strlen(msg[i]);
        //+CPIN: XXXXXX\r\n\0 == XXXXXX\0
        //printf("length %i %i, %s\n", length - 7, n + 2, msg[i]);
        if(length - 7 == n + 1)
        {
            if(memcmp(buffer + 7, msg[i], n) == 0)
            {
                context->pin_is_required = (sim_pin_status_t)i;// TODO cast
                return;
            }
        }
    }
    context->pin_is_required = SIM_PIN_UNKNOW;
    
}

void simx_pin_is_required(sim_reply_t *reply)
{
    sim300_context_t *context = _gsm_context();
    sim300_send_at_cmd(context, reply, AT_CPINR);
}

void _simx_network_registration_resp(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    int n, stat;
    if(length >= 11)
    {
        sscanf((char*)(buffer + 7), "%i,%i", &n, &stat);
        if(stat >= 0 && stat <= 5)
        {
            context->network_reg = (sim_reg_network_t)stat;
        }
        else
        {
            context->network_reg = (sim_reg_network_t)-1;
        }
    }
    else
    {
        context->network_reg = (sim_reg_network_t)-1;
    }
}

void simx_network_registration(sim_reply_t *reply)
{
    sim300_context_t *context = _gsm_context();
    sim300_send_at_cmd(context, reply, AT_CREG);
}

void _simx_sim_inserted_status_resp(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    //printf("rec: %s %i", buffer, length);
    int n, stat;
    if(length >= 13)
    {
        
        sscanf((char*)(buffer + 9), "%i,%i", &n, &stat);
        if(stat >= 0 && stat <= 1)
        {
            context->sim_is_insert = stat;
        }
        else
        {
            context->sim_is_insert = -1;
        }
    }
    else
    {
        context->sim_is_insert = -1;
    }
}

void simx_sim_inserted_status(sim_reply_t *reply)
{
    sim300_context_t *context = _gsm_context();
    sim300_send_at_cmd(context, reply, AT_CSMINS);
}

void _simx_signal_quality_report_resp(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    int q, n, l;
    if(length >= 10)
    {
        
        n = sscanf((char*)(buffer + 6), "%i,%i", &q, &l);
        if(n == 2 && context->reply->user_pointer != NULL)
        {
            *((uint8_t*)context->reply->user_pointer) = q;
        }
    }
}

void simx_signal_quality_report(sim_reply_t *reply, uint8_t *lvl)
{
    sim300_context_t *context = _gsm_context();
    reply->user_pointer = lvl;
    sim300_send_at_cmd(context, reply, AT_CSQ);
}

void _simx_set_TE_character_finished(sim300_context_t *context)
{
    context->te_chaster = (sim_TE_chaster_t)context->reply->user_data;
}

void simx_set_TE_character(sim_reply_t *reply, sim_TE_chaster_t chaster)
{
    static const char* te_chasters[] = {"GSM", "UCS2", "IRA", "HEX", "PCCP", "PCDN", "8859-1"};
    sim300_context_t *context = _gsm_context();
    context->fcmd_finished = _simx_set_TE_character_finished;
    reply->user_data = chaster;
    sim300_send_at_cmd_vp(context, reply, AT_CSCS, "\"%s\"", te_chasters[chaster]);
}

void simx_call_to_dial_number(sim_reply_t *reply, char *number)
{
    sim300_context_t *context = _gsm_context();
    sim300_send_cmd_vp(context, reply, ATD, "%s", number);
}

void simx_switch_mode(sim_reply_t *reply, sim_pdp_mode_t mode)
{
    sim300_context_t *context = _gsm_context();
    if(mode == SIM_DATA_MODE)
    {
        sim300_send_no_at_cmd(context, reply, ATO);
    }
    else
    {
        fsimx_receive = simx_receive_msg;
        context->sim_cnt = 0;
        context->is_receive = 0;
        context->state = AT_CMD_ST_RN;
        context->reply = reply;
        context->cmd = PPP;
        reply->status = SIM300_NULL;
        sprintf((char*)context->tx_buffer, "+++");
        simx_callback_send(context->tx_buffer, 3);
    }
}

void _simx_is_attach_to_GPRS_resp(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    int n;
    if(length >= 8)
    {
        
        sscanf((char*)(buffer + 8), "%i", &n);
        if(n >= 0 && n <= 1)
        {
            context->gprs_is_attach = n;
        }
        else
        {
            context->gprs_is_attach = -1;
        }
    }
    else
    {
        context->gprs_is_attach = -1;
    }
}

void simx_is_attach_to_GPRS(sim_reply_t *reply)
{
    sim300_context_t *context = _gsm_context();
    sim300_send_at_cmd(context, reply, AT_CGATT);
}


void simx_deactivate_gprs_pdp(sim_reply_t *reply)
{
    sim300_context_t *context = _gsm_context();
    sim300_send_at_cmd(context, reply, AT_CIPSHUT);
}

void simx_set_gprs_config(sim_reply_t *reply, const char* apn, const char* username, const char* pass)
{
    sim300_context_t *context = _gsm_context();
    sim300_send_at_cmd_vp(context, reply, AT_CSTT, "\"%s\",\"%s\",\"%s\"", apn, username, pass);
}

void simx_bring_up_wireless_connection(sim_reply_t *reply)
{
    sim300_context_t *context = _gsm_context();
    sim300_send_at_cmd(context, reply, AT_CIICR);
}

void simx_get_local_ip(sim_reply_t *reply, sim_ip_t *ip)
{
    sim300_context_t *context = _gsm_context();
    reply->user_pointer = ip;
    sim300_send_at_cmd(context, reply, AT_CIFSR);
}

void simx_define_pdp_context(sim_reply_t *reply, uint8_t cid, char *pdp_type, char *apn)
{
    sim300_context_t *context = _gsm_context();
    sim300_send_at_cmd_vp(context, reply, AT_CGDCONT, "%i,\"%s\",\"%s\"", cid, pdp_type, apn);
}

void simx_set_connection_mode(sim_reply_t *reply, sim_connection_mode_t mode, char *apn, 
                              char *user_name, char *pass, sim_csd_rate_t rate)
{
    sim300_context_t *context = _gsm_context();
    if(mode == SIM_CSD)
    {
        sim300_send_at_cmd_vp(context, reply, AT_CIPCSGP, 
                              "%i,\"%s\",\"%s\",\"%s\",%i", mode, apn, user_name, pass, rate);
    }
    else if(mode == SIM_GPRS)
    {
        sim300_send_at_cmd_vp(context, reply, AT_CIPCSGP, 
                              "%i,\"%s\",\"%s\",\"%s\"", mode, apn, user_name, pass);
    }
    else
    {
        //TODO ERROR
    }
}

void _simx_multiple_connection_finished(sim300_context_t *context)
{
    if(context->reply->status == SIM300_OK)
    {
        context->mux = context->reply->user_data;
    }
}

void _simx_current_connection_status_parse(sim300_context_t *context, char *buffer, uint16_t length)
{
    if(length < 17)
    {
        return;
    }
    
    sim_cipstatus_t *st = context->reply->user_pointer;
    uint8_t stsize = context->reply->user_data;
    
    char c1 = buffer[0];
    int n1 = buffer[3] - 0x30;
    if(stsize > n1)
    {
    
        st[n1].n = n1;
        st[n1].type = c1;
        
        str_tocken_t mem[8] = {0};
        char *c = buffer + 5;
        
        _strchrcpy(c, mem, 20);
        if(mem[0].size != 0)
        {
            st[n1].bearer = mem[0].str[0] - 0x30;
        }
        if(mem[1].size > 2)
        {
            st[n1].mode = str_to_tcp_mode((uint8_t*)mem[1].str + 1);
        }
        if(mem[2].size > 2)
        {
            str_to_ip(mem[2].str + 1, &st[n1].ip);
        }
        if(mem[3].size > 2)
        {
            int port;
            sscanf(mem[3].str + 1, "%i", &port);
            st[n1].port = port;
        }
        
        /*static const sim_mux_cip_state_t cip_state[] = {
            MUX_CIP_IP_INITIAL, MUX_CIP_IP_START, MUX_CIP_IP_CONFIG, 
            MUX_CIP_IP_GPRSACT, MUX_CIP_IP_STATUS, MUX_CIP_IP_PROCESSUNG, MUX_CIP_PDP_DEACT};
        
        static const char *cipstate[] = {"IP INITIAL",
            "IP START", "IP CONFIG", "IP GPRSACT", "IP STATUS",
            "IP PROCESSUNG", "PDP DEACT"};*/
        static const sim_cip_client_state_t cip_state[] = {
                CCP_INITIAL, CCP_CONNECTING, CCP_CONNECTED, 
                CCP_REMOTE_CLOSING, CCP_CLOSING, CCP_CLOSED};
            
        static const char *cipstate[] = {
            "INITIAL", "CONNECTING", "CONNECTED", "REMOTE CLOSING",
            "CLOSING", "CLOSED"};
          
        if(mem[4].size > 2)
        {
            for(uint8_t i = 0; i < sizeof(cip_state) / sizeof(cip_state[0]); i++)
            {
                if(memcmp(mem[4].str + 1, cipstate[i], strlen(cipstate[i])) == 0)
                {
                    st[n1].state = cip_state[i];
                    break;
                }
            }
        }
    }
    
    if(n1 == SIMX_MUX_SOCKETS)
    {
        context->is_receive = 1;
    }
}

sim_cip_state_t _simx_mux0_cipstate(uint8_t *buffer, uint16_t length)
{
    static const sim_cip_state_t cip_state[] = {CIP_IP_INITIAL,
        CIP_IP_START, CIP_IP_CONFIG, CIP_IP_GPRSACT, CIP_IP_STATUS,
        CIP_CONN_LISTENING, CIP_CONN_LISTENING, CIP_CONN_LISTENING,
        CIP_CONNECT_OK, CIP_CLOSING, CIP_CLOSING, CIP_CLOSED, 
        CIP_CLOSED, CIP_PDP_DEACT};
    static const char *cipstate[] = {"IP INITIAL\0",
        "IP START\0", "IP CONFIG\0", "IP GPRSACT\0", "IP STATUS\0",
        "TCP CONNECTING\0", "UDP CONNECTING\0", "SERVER LISTENING\0", 
        "CONNECT OK\0", "TCP CLOSING\0", "UDP CLOSING\0", "TCP CLOSED\0",
        "UDP CLOSED\0", "PDP DEACT\0"};
    
    for(uint8_t i = 0; i < sizeof(cip_state) / sizeof(cip_state[0]); i++)
    {
        if(memcmp(buffer + 7, cipstate[i], strlen(cipstate[i])) == 0)
        {
            
            return cip_state[i];
        }
    }
    return MUX_CIP_UNKNOWN;
}

sim_mux_cip_state_t _simx_mux1_cipstate(uint8_t *buffer, uint16_t length)
{
    static const sim_mux_cip_state_t cip_state[] = {
        MUX_CIP_IP_INITIAL, MUX_CIP_IP_START, MUX_CIP_IP_CONFIG, 
    MUX_CIP_IP_GPRSACT, MUX_CIP_IP_STATUS, MUX_CIP_IP_PROCESSUNG, MUX_CIP_PDP_DEACT};
    static const char *cipstate[] = {"IP INITIAL",
        "IP START", "IP CONFIG", "IP GPRSACT", "IP STATUS",
        "IP PROCESSING", "PDP DEACT"};
    
    for(uint8_t i = 0; i < sizeof(cip_state) / sizeof(cip_state[0]); i++)
    {
        if(memcmp(buffer + 7, cipstate[i], strlen(cipstate[i])) == 0)
        {
            
            return cip_state[i];
        }
    }
    return CIP_UNKNOWN;
}

void _simx_current_connection_status_resp(sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    if(context->cmd == AT_CIPSTATUS)
    {
        if(memcmp(buffer, "STATE: ", 7) == 0)
        {
            if(context->mux == 0)
            {
                context->cip_state = _simx_mux0_cipstate(buffer, length);
                context->is_receive = 1;
            }
            else
            {
                context->cip_state = _simx_mux1_cipstate(buffer, length);
            }
        }
        else
        {
            _simx_current_connection_status_parse(context, (char*)buffer, length);
        }
    }
}

void simx_current_connection_status(sim_reply_t *reply, sim_cipstatus_t *cipstatus, uint8_t size)
{
    sim300_context_t *context = _gsm_context();
    reply->user_data = size;
    reply->user_pointer = cipstatus;
    sim300_send_at_cmd(context, reply, AT_CIPSTATUS);
}

void simx_multiple_connection(sim_reply_t *reply, uint8_t mode)
{
    sim300_context_t *context = _gsm_context();
    //TODO assert(); mode
    context->fcmd_finished = _simx_multiple_connection_finished;
    reply->user_data = mode;
    sim300_send_at_cmd_vp(context, reply, AT_CIPMUX, "%i", mode);
}

void simx_tcp_connect(sim_reply_t *reply, sim_tcp_mode_t tcp_mode, char *address, uint16_t port, int n)
{
    sim300_context_t *context = _gsm_context();
    char *mode[] = {"TCP", "UDP"};
    uint8_t index = tcp_mode == SIM_TCP ? 0 : 1;
    //TODO assert n = 0..7
    
    if(context->mux)
    {
        sim300_send_at_cmd_vp(context, reply, AT_CIPSTART, "%i,\"%s\",\"%s\",%i", 
                            n, mode[index], address, port);
    }
    else
    {
        sim300_send_at_cmd_vp(context, reply, AT_CIPSTART, "\"%s\",\"%s\",%i", 
                            mode[index], address, port);
    }
}

void simx_tcp_send_data(sim_reply_t *reply, uint8_t *data, uint16_t length, int n)
{
    sim300_context_t *context = _gsm_context();
    reply->user_pointer = data;
    reply->user_data = length;
    if(context->mux)
    {
        sim300_send_at_cmd_vp(context, reply, AT_CIPSEND, "%i,%i", n, length);
    }
    else
    {
        sim300_send_at_cmd_vp(context, reply, AT_CIPSEND, "%i", length);
    }
}

void simx_tcp_close(sim_reply_t *reply, int n)
{
    sim300_context_t *context = _gsm_context();
    context->mux
        ? sim300_send_at_cmd_vp(context, reply, AT_CIPCLOSE, "%i,%i", n, 0)
        : sim300_send_at_cmd_vp(context, reply, AT_CIPCLOSE, "%i", 0);
}

void _simx_tcp_head_enable_finished(sim300_context_t *context)
{
    if(context->reply->status == SIM300_OK)
    {
        context->ciphead = context->reply->user_data;
    }
}

void simx_tcp_head_enable(sim_reply_t *reply, uint8_t is_enable)
{
    sim300_context_t *context = _gsm_context();
    is_enable = is_enable ? 1 : 0;
    context->reply = reply;
    context->reply->user_data = is_enable;
    context->fcmd_finished = _simx_tcp_head_enable_finished;
    sim300_send_at_cmd_vp(context, reply, AT_CIPHEAD, "%i", is_enable);
}

void simx_sms_mode(sim_reply_t *reply, sim_sms_mode_t mode)
{
    sim300_context_t *context = _gsm_context();
    //TODO assert mode
    sim300_send_at_cmd_vp(context, reply, AT_CMGF, "%i", mode);
}

void simx_send_sms(sim_reply_t *reply, const char *number, const char* msg)
{
    sim300_context_t *context = _gsm_context();
    //todo validate number
    context->cmd_data = (char*)msg;
    sim300_send_at_cmd_vp(context, reply, AT_CMGS, "\"%s\"", number);
}

void _simx_sms_read_resp(struct sim300_context_t *context, uint8_t *buffer, uint16_t length)
{
    sim_sms_t *sms = (sim_sms_t*)context->reply->user_pointer;
    if(memcmp(buffer, "+CMGR:", sizeof("+CMGR:") - 1) == 0)
    {
        char strstaus[10];
        char *c = (char*)buffer;
        c = str_quotes(c, strstaus, 10);
        if(c == NULL) return;
        c = str_quotes(c + 1, sms->number, 21);
        if(c == NULL) return;
        c = str_quotes(c + 1, sms->mt, 17);
        if(c == NULL) return;
        c = str_quotes(c + 1, sms->date, 22);
    }
    else
    {
        uint16_t len = sms->msg_length > length ? length - 1 : sms->msg_length;
        memcpy(sms->msg, buffer, len);
        sms->msg[len] = 0;
    }
}

void simx_read_sms(sim_reply_t *reply, sim_sms_t *sim_sms, uint16_t n)
{
    sim300_context_t *context = _gsm_context();
    reply->user_pointer = sim_sms;
    sim300_send_at_cmd_vp(context, reply, AT_CMGR, "%i", n);
}

sim_pin_status_t sim_pin_required()
{
    sim300_context_t *context = _gsm_context();
    return context->pin_is_required;
}

sim_reg_network_t sim_network_registration()
{
    sim300_context_t *context = _gsm_context();
    return context->network_reg;
}

int8_t sim_is_inserted()
{
    sim300_context_t *context = _gsm_context();
    return context->sim_is_insert;
}

int8_t gprs_is_attach()
{
    sim300_context_t *context = _gsm_context();
    return context->gprs_is_attach;
}

sim_cip_state_t current_connection_status()
{
    sim300_context_t *context = _gsm_context();
    return context->cip_state;
}

uint8_t sim_cip_mux_mode()
{
    sim300_context_t *context = _gsm_context();
    return context->mux;
}

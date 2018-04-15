#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "sim800.h"


using namespace testing;
char *g_data;
uint16_t g_length;
sim_con_status_t g_con_status;
uint8_t g_con_n = -1;
char* g_tcp_data;
uint16_t g_tcp_length;
uint8_t g_tcp_con;
uint16_t g_sms_number;
simx_notification_t g_notification;

void set_timeout()
{
    for(int i = 0; i < SIM900_TIMEOUT + 1; i++)
    {
        simx_tick_1ms();
    }
}

void simx_callback_sms_received(uint16_t number)
{
    g_sms_number = number;
}

void simx_callback_send(uint8_t *data, uint16_t length)
{
    g_data = (char*)data;
    g_length = length;
}

void simx_callback_tcp_msg(sim_con_status_t con_status, uint8_t n)
{
    g_con_status = con_status;
    g_con_n = n;
}

void simx_callback_tcp_data(uint8_t *data, uint16_t length, uint8_t n)
{
    g_tcp_data = (char*)data;
    g_tcp_length = length;
    g_tcp_con = n;
}

void simx_callback_message(simx_notification_t notification)
{
    g_notification = notification;
}

void simx_test_send(const char* msg)
{
    for(int i = 0; i < strlen(msg); i++)
    {
        simx_receive(msg[i]);
    }
}

void simx_callback_timeout()
{
    
}

void simx_callback_update()
{
    
}

TEST(AutoTestSim800, SIM_AT)
{
    EXPECT_EQ(1, 1);
    ASSERT_THAT(0, Eq(0));
    
    sim_reply_t reply;
    simx_test(&reply);
    
    EXPECT_EQ(g_length, 4);
    EXPECT_STREQ(g_data, "AT\r\n");
    
    simx_test_send("AT\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_test(&reply);
    simx_test_send("AT\r");
    simx_test_send("\r\nERROR\r\n");
    EXPECT_EQ(reply.status, SIM300_ERROR);
    
    simx_test(&reply);
    simx_test_send("AT\r");
    simx_test_send("\r\nFDSG\r\n");
    EXPECT_EQ(reply.status, SIM300_ERRFRAME);
}

TEST(AutoTestSim800, SIM_AT_CPIN)
{
    sim_reply_t reply;
    simx_pin_is_required(&reply);
    
    EXPECT_EQ(g_length, strlen("AT+CPIN?\r\n"));
    EXPECT_STREQ(g_data, "AT+CPIN?\r\n");
    simx_test_send("AT+CPIN?\r");
    simx_test_send("\r\n+CPIN: READY\r\n\r\n");
    simx_test_send("OK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    EXPECT_EQ(sim_pin_required(), PIN_READY);
    
    
    static char *msg[] = {(char *)"READY", (char *)"SIM PIN", (char *)"SIM PUK", (char *)"PH_SIM PIN", 
                          (char *)"PH_SIM PUK", (char *)"SIM PIN2", (char *)"SIM PUK2"};
    for(int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++)
    {
        simx_pin_is_required(&reply);
        simx_test_send("AT+CPIN?\r");
        simx_test_send("\r\n+CPIN: ");
        simx_test_send(msg[i]);
        simx_test_send("\r\n\r\n");
        simx_test_send("OK\r\n");
        EXPECT_EQ(reply.status, SIM300_OK);
        EXPECT_EQ(sim_pin_required(), i);
        EXPECT_EQ(simx_is_receive(), 1);
    }
    
    simx_pin_is_required(&reply);
    simx_test_send("AT+CPIN?\r");
    simx_test_send("\r\n+CPIN: NO_VALID\r\n\r\n");
    simx_test_send("OK\r\n");
    EXPECT_EQ(simx_is_receive(), 1);
    //EXPECT_EQ(reply.status, SIM300_ERRFRAME);
    //EXPECT_EQ(pin_required(), SIM_PIN_UNKNOW);
    
    simx_pin_is_required(&reply);
    simx_test_send("AT+CPIN?\r");
    simx_test_send("\r\n+CPIN: NO_VALID\r\n\r\n");
    simx_test_send("OK\r\n");
    EXPECT_EQ(simx_is_receive(), 1);
    EXPECT_EQ(reply.status, SIM300_OK);
    EXPECT_EQ(sim_pin_required(), SIM_PIN_UNKNOW);
    
    
    simx_pin_is_required(&reply);
    simx_test_send("AT+CPIN?\r");
    simx_test_send("\r\n+CPIN: NO_VALID\r\n");
    simx_test_send("OK\r\n");
    set_timeout();
    EXPECT_EQ(simx_is_receive(), 1); // timeout
    
    EXPECT_EQ(reply.status, SIM300_TIMEOUT);
}

TEST(AutoTestSim800, SIM_AT_CREG)
{
    sim_reply_t reply;
    simx_network_registration(&reply);
    EXPECT_EQ(g_length, strlen("AT+CREG?\r\n"));
    EXPECT_STREQ(g_data, "AT+CREG?\r\n");
    
    simx_test_send("AT+CREG?\r");
    simx_test_send("\r\n+CREG: 0,1\r\n\r\n");
    simx_test_send("OK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    EXPECT_EQ(sim_network_registration(), NET_REGISTERED_HOME_NETWORK);
    
    for(int i = 0; i < 5; i++)
    {
        simx_network_registration(&reply);
        simx_test_send("AT+CREG?\r");
        simx_test_send("\r\n+CREG: 0,");
        char c[2] = {(char)i + 0x30, 0};
        simx_test_send(c);
        simx_test_send("\r\n\r\n");
        simx_test_send("OK\r\n");
        EXPECT_EQ(reply.status, SIM300_OK);
        EXPECT_EQ(sim_network_registration(), i);
    }
}

TEST(AutoTestSim800, SIM_AT_CMGF)
{
    sim_reply_t reply;
    simx_sms_mode(&reply, SIM_SMS_TEXT);
    EXPECT_EQ(g_length, strlen("AT+CMGF=1\r\n"));
    EXPECT_STREQ(g_data, "AT+CMGF=1\r\n");
    
    simx_test_send("AT+CMGF=1\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_sms_mode(&reply, SIM_SMS_PDU);
    EXPECT_EQ(g_length, strlen("AT+CMGF=0\r\n"));
    EXPECT_STREQ(g_data, "AT+CMGF=0\r\n");
    
    simx_test_send("AT+CMGF=0\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
}

TEST(AutoTestSim800, SIM_AT_CMGS)
{
    sim_reply_t reply;
    simx_send_sms(&reply, "+796400011133", "test");
    EXPECT_EQ(g_length, strlen("AT+CMGS=\"+796400011133\"\r\n"));
    EXPECT_STREQ(g_data, "AT+CMGS=\"+796400011133\"\r\n");
    simx_test_send("AT+CMGS=\"+796400011133\"\r");
    simx_test_send("\r\n> ");
    EXPECT_EQ(g_length, strlen("test\x1A"));
    EXPECT_STREQ(g_data, "test\x1A");
    
    simx_test_send("\r\n+CMGS: 232\r\n");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
}

TEST(AutoTestSim800, SIM_AT_CSMINS)
{
    sim_reply_t reply;
    simx_sim_inserted_status(&reply);
    EXPECT_EQ(g_length, strlen("AT+CSMINS?\r\n"));
    EXPECT_STREQ(g_data, "AT+CSMINS?\r\n");
    simx_test_send("AT+CSMINS?\r");
    simx_test_send("\r\n+CSMINS: 0,1\r\n");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(sim_is_inserted(), 1);
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_sim_inserted_status(&reply);
    simx_test_send("AT+CSMINS?\r");
    simx_test_send("\r\n+CSMINS: 0,0\r\n");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(sim_is_inserted(), 0);
    
    simx_sim_inserted_status(&reply);
    simx_test_send("AT+CSMINS?\r");
    simx_test_send("\r\n+CSMINS: 0,44\r\n");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(sim_is_inserted(), -1);   
}

TEST(AutoTestSim800, SIM_AT_CGATT)
{
    sim_reply_t reply;
    simx_is_attach_to_GPRS(&reply);
    EXPECT_EQ(g_length, strlen("AT+CGATT?\r\n"));
    EXPECT_STREQ(g_data, "AT+CGATT?\r\n");
    simx_test_send("AT+CGATT?\r");
    simx_test_send("\r\n+CGATT: 1\r\n");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(gprs_is_attach(), 1);
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_is_attach_to_GPRS(&reply);
    EXPECT_EQ(g_length, strlen("AT+CGATT?\r\n"));
    EXPECT_STREQ(g_data, "AT+CGATT?\r\n");
    simx_test_send("AT+CGATT?\r");
    simx_test_send("\r\n+CGATT: 0\r\n");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(gprs_is_attach(), 0);
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_is_attach_to_GPRS(&reply);
    EXPECT_EQ(g_length, strlen("AT+CGATT?\r\n"));
    EXPECT_STREQ(g_data, "AT+CGATT?\r\n");
    simx_test_send("AT+CGATT?\r");
    simx_test_send("\r\n+CGATT: 22\r\n");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(gprs_is_attach(), -1);
    EXPECT_EQ(reply.status, SIM300_OK);
}

TEST(AutoTestSim800, SIM_AT_CIPSHUT)
{
    sim_reply_t reply;
    simx_deactivate_gprs_pdp(&reply);
    EXPECT_EQ(g_length, strlen("AT+CIPSHUT\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPSHUT\r\n");
    simx_test_send("AT+CIPSHUT\r");
    simx_test_send("\r\nSHUT OK\r\n");
    EXPECT_EQ(reply.status, SIM300_SHUT_OK);
    
    simx_deactivate_gprs_pdp(&reply);
    simx_test_send("AT+CIPSHUT\r");
    simx_test_send("\r\nERROR\r\n");
    EXPECT_EQ(reply.status, SIM300_ERROR);
    
    simx_deactivate_gprs_pdp(&reply);
    simx_test_send("AT+CIPSHUT\r");
    simx_test_send("\r\ntete\r\n");
    EXPECT_EQ(reply.status, SIM300_ERRFRAME);
    
    simx_deactivate_gprs_pdp(&reply);
    simx_test_send("AT+CIPSHUT\r");
    simx_test_send("tete\r\n");
    EXPECT_EQ(reply.status, SIM300_ERRFRAME);
}

TEST(AutoTestSim800, SIM_AT_CIPSTATUS)
{
    sim_reply_t reply;
    simx_current_connection_status(&reply, NULL, 0);
    EXPECT_EQ(g_length, strlen("AT+CIPSTATUS\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPSTATUS\r\n");
    simx_test_send("AT+CIPSTATUS\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    simx_test_send("\r\nSTATE: IP INITIAL\r\n\r\n");
    EXPECT_EQ(current_connection_status(), CIP_IP_INITIAL);
    
    simx_current_connection_status(&reply, NULL, 0);
    simx_test_send("AT+CIPSTATUS\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    simx_test_send("\r\nSTATE: IP START\r\n\r\n");
    EXPECT_EQ(current_connection_status(), CIP_IP_START);
    
    simx_current_connection_status(&reply, NULL, 0);
    simx_test_send("AT+CIPSTATUS\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    simx_test_send("\r\nSTATE: PDP DEACT\r\n\r\n");
    EXPECT_EQ(current_connection_status(), CIP_PDP_DEACT);
    
    /*******************************************/
    simx_multiple_connection(&reply, 1);
    EXPECT_STREQ(g_data, "AT+CIPMUX=1\r\n");
    simx_test_send("AT+CIPMUX=1\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    EXPECT_EQ(sim_cip_mux_mode(), 1);
    /*******************************************/
    sim_cipstatus_t cipstatus[8];
    
    simx_current_connection_status(&reply, cipstatus, 8);
    simx_test_send("AT+CIPSTATUS\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    simx_test_send("\r\nSTATE: IP STATUS\r\n\r\n");
    simx_test_send("C: 0,0,\"UDP\",\"195.210.189.106\",\"123\",\"CLOSED\"\r\n");
    simx_test_send("C: 1,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    simx_test_send("C: 2,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    simx_test_send("C: 3,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    simx_test_send("C: 4,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    simx_test_send("C: 5,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    simx_test_send("C: 6,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    simx_test_send("C: 7,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    
    
    EXPECT_EQ(cipstatus[0].n, 0);
    EXPECT_EQ(cipstatus[0].bearer, 0);
    EXPECT_EQ(cipstatus[0].mode, SIM_UDP);
    EXPECT_EQ(cipstatus[0].port, 123);
    EXPECT_EQ(cipstatus[0].state, CCP_CLOSED);
    
    for(int i = 1; i < 7; i++)
    {
        EXPECT_EQ(cipstatus[i].n, i);
        EXPECT_EQ(cipstatus[i].state, CCP_INITIAL);
    }
    
    EXPECT_EQ(current_connection_status(), CIP_IP_STATUS);
    
    
    /*******************************************/
    
    
    simx_current_connection_status(&reply, cipstatus, 8);
    simx_test_send("AT+CIPSTATUS\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    simx_test_send("\r\nSTATE: IP PROCESSING\r\n\r\n");
    simx_test_send("C: 0,0,\"UDP\",\"195.210.189.106\",\"123\",\"CONNECTED\"\r\n");
    simx_test_send("C: 1,0,\"TCP\",\"85.153.14.164\",\"80\",\"CLOSED\"\r\n");
    simx_test_send("C: 2,0,\"TCP\",\"85.153.14.164\",\"80\",\"CONNECTED\"\r\n");
    simx_test_send("C: 3,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    simx_test_send("C: 4,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    simx_test_send("C: 5,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    simx_test_send("C: 6,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    simx_test_send("C: 7,,\"\",\"\",\"\",\"INITIAL\"\r\n");
    
    
    EXPECT_EQ(cipstatus[0].n, 0);
    EXPECT_EQ(cipstatus[0].bearer, 0);
    EXPECT_EQ(cipstatus[0].mode, SIM_UDP);
    EXPECT_EQ(cipstatus[0].port, 123);
    EXPECT_EQ(cipstatus[0].state, CCP_CONNECTED);
    
    for(int i = 3; i < 7; i++)
    {
        EXPECT_EQ(cipstatus[i].n, i);
        EXPECT_EQ(cipstatus[i].state, CCP_INITIAL);
    }
    
    EXPECT_EQ(current_connection_status(), MUX_CIP_IP_PROCESSUNG);
    
}

TEST(AutoTestSim800, SIM_AT_CIPMUX)
{
    sim_reply_t reply;
    simx_multiple_connection(&reply, 0);
    EXPECT_EQ(g_length, strlen("AT+CIPMUX=0\r\n"));
    EXPECT_EQ(strlen(g_data), strlen("AT+CIPMUX=0\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPMUX=0\r\n");
    simx_test_send("AT+CIPMUX=0\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    EXPECT_EQ(sim_cip_mux_mode(), 0);
    
    simx_multiple_connection(&reply, 1);
    EXPECT_STREQ(g_data, "AT+CIPMUX=1\r\n");
    simx_test_send("AT+CIPMUX=1\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    EXPECT_EQ(sim_cip_mux_mode(), 1);
}

TEST(AutoTestSim800, SIM_AT_CSTT)
{
    sim_reply_t reply;
    simx_set_gprs_config(&reply, "internet.mts.ru", "mts", "mts");
    EXPECT_EQ(g_length, strlen("AT+CSTT=\"internet.mts.ru\",\"mts\",\"mts\"\r\n"));
    EXPECT_STREQ(g_data, "AT+CSTT=\"internet.mts.ru\",\"mts\",\"mts\"\r\n");
    simx_test_send("AT+CSTT=\"internet.mts.ru\",\"mts\",\"mts\"\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
}

TEST(AutoTestSim800, SIM_AT_CIICR)
{
    sim_reply_t reply;
    simx_bring_up_wireless_connection(&reply);
    EXPECT_EQ(g_length, strlen("AT+CIICR\r\n"));
    EXPECT_STREQ(g_data, "AT+CIICR\r\n");
    simx_test_send("AT+CIICR\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_bring_up_wireless_connection(&reply);
    EXPECT_EQ(g_length, strlen("AT+CIICR\r\n"));
    EXPECT_STREQ(g_data, "AT+CIICR\r\n");
    simx_test_send("AT+CIICR\r");
    simx_test_send("\r\n+PDP: DEACT\r\n\r\n");
    simx_test_send("\r\nERROR\r\n");
    EXPECT_EQ(reply.status, SIM300_ERROR);
}

TEST(AutoTestSim800, SIM_AT_CIFSR)
{
    sim_reply_t reply;
    sim_ip_t ip;
    simx_get_local_ip(&reply, &ip);
    EXPECT_EQ(g_length, strlen("AT+CIFSR\r\n"));
    EXPECT_STREQ(g_data, "AT+CIFSR\r\n");
    simx_test_send("AT+CIFSR\r");
    simx_test_send("\r\n192.168.0.1\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    EXPECT_EQ(ip.addr0, 192);
    EXPECT_EQ(ip.addr1, 168);
    EXPECT_EQ(ip.addr2, 0);
    EXPECT_EQ(ip.addr3, 1);
}

TEST(AutoTestSim800, SIM_AT_CSQ)
{
    sim_reply_t reply;
    uint8_t lvl = -1;
    simx_signal_quality_report(&reply, &lvl);
    EXPECT_EQ(g_length, strlen("AT+CSQ\r\n"));
    EXPECT_STREQ(g_data, "AT+CSQ\r\n");
    simx_test_send("AT+CSQ\r");
    simx_test_send("\r\n+CSQ: 31,0\r\n");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    EXPECT_EQ(lvl, 31);
    
}

TEST(AutoTestSim800, SIM_AT_CIPSTART)
{
    sim_reply_t reply;
    
    simx_tcp_connect(&reply, SIM_TCP, (char *)"www.google.com", 80, 1);
    EXPECT_EQ(g_length, strlen("AT+CIPSTART=1,\"TCP\",\"www.google.com\",80\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPSTART=1,\"TCP\",\"www.google.com\",80\r\n");
    simx_test_send("AT+CIPSTART=1,\"TCP\",\"www.google.com\",80\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    
}

TEST(AutoTestSim800, SIM_AT_CIPSEND)
{
    sim_reply_t reply;
    simx_multiple_connection(&reply, 0);
    simx_test_send("AT+CIPMUX=0\r");
    simx_test_send("\r\nOK\r\n");
    
    char msg[] = "test";
    int n = strlen(msg);
    simx_tcp_send_data(&reply, (uint8_t*)msg, n, 0);
    EXPECT_EQ(g_length, strlen("AT+CIPSEND=4\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPSEND=4\r\n");
    simx_test_send("AT+CIPSEND=4\r");
    simx_test_send("\r\n");
    simx_test_send("> ");
    EXPECT_EQ(g_length, n);
    EXPECT_STREQ(g_data, msg);
    simx_test_send("\r\nSEND OK\r\n");
    EXPECT_EQ(reply.status, SIM300_SEND_OK);
    
    simx_tcp_send_data(&reply, (uint8_t*)msg, n, 0);
    EXPECT_EQ(g_length, strlen("AT+CIPSEND=4\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPSEND=4\r\n");
    simx_test_send("AT+CIPSEND=4\r");
    simx_test_send("\r\n");
    simx_test_send("> ");
    EXPECT_EQ(g_length, n);
    EXPECT_STREQ(g_data, msg);
    simx_test_send("\r\nSEND FAIL\r\n");
    EXPECT_EQ(reply.status, SIM300_SEND_FAIL);
    
    simx_multiple_connection(&reply, 1);
    simx_test_send("AT+CIPMUX=1\r");
    simx_test_send("\r\nOK\r\n");
    
    simx_tcp_send_data(&reply, (uint8_t*)msg, n, 1);
    EXPECT_EQ(g_length, strlen("AT+CIPSEND=1,4\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPSEND=1,4\r\n");
    simx_test_send("AT+CIPSEND=1,4\r");
    simx_test_send("\r\n");
    simx_test_send("> ");
    EXPECT_EQ(g_length, n);
    EXPECT_STREQ(g_data, msg);
    simx_test_send("\r\n1, SEND OK\r\n");
    EXPECT_EQ(reply.status, SIM300_SEND_OK);
    
    simx_tcp_send_data(&reply, (uint8_t*)msg, n, 2);
    EXPECT_EQ(g_length, strlen("AT+CIPSEND=2,4\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPSEND=2,4\r\n");
    simx_test_send("AT+CIPSEND=2,4\r");
    simx_test_send("\r\n");
    simx_test_send("> ");
    EXPECT_EQ(g_length, n);
    EXPECT_STREQ(g_data, msg);
    simx_test_send("\r\n2, SEND FAIL\r\n");
    EXPECT_EQ(reply.status, SIM300_SEND_FAIL);
    
    
    simx_tcp_send_data(&reply, (uint8_t*)msg, n, 2);
    EXPECT_EQ(g_length, strlen("AT+CIPSEND=2,4\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPSEND=2,4\r\n");
    simx_test_send("AT+CIPSEND=2,4\r");
    simx_test_send("\r\n");
    simx_test_send("> ");
    EXPECT_EQ(g_length, n);
    EXPECT_STREQ(g_data, msg);
    simx_test_send("\r\n54354356\r\n");
    EXPECT_EQ(reply.status, SIM300_ERRFRAME);
    
}

TEST(AutoTestSim800, SIM_AT_CIPCLOSE)
{
    sim_reply_t reply;
    simx_multiple_connection(&reply, 0);
    simx_test_send("AT+CIPMUX=0\r");
    simx_test_send("\r\nOK\r\n");
    
    simx_tcp_close(&reply, 0);
    EXPECT_EQ(g_length, strlen("AT+CIPCLOSE=0\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPCLOSE=0\r\n");
    simx_test_send("AT+CIPCLOSE=0\r");
    simx_test_send("\r\nCLOSE OK\r\n");
    EXPECT_EQ(reply.status, SIM300_CLOSE_OK);
    
    simx_tcp_close(&reply, 0);
    EXPECT_EQ(g_length, strlen("AT+CIPCLOSE=0\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPCLOSE=0\r\n");
    simx_test_send("AT+CIPCLOSE=0\r");
    simx_test_send("\r\nERROR\r\n");
    EXPECT_EQ(reply.status, SIM300_ERROR);
    
    simx_multiple_connection(&reply, 1);
    simx_test_send("AT+CIPMUX=1\r");
    simx_test_send("\r\nOK\r\n");
    
    simx_tcp_close(&reply, 1);
    EXPECT_EQ(g_length, strlen("AT+CIPCLOSE=1,0\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPCLOSE=1,0\r\n");
    simx_test_send("AT+CIPCLOSE=1,0\r");
    simx_test_send("\r\n1, CLOSE OK\r\n");
    EXPECT_EQ(reply.status, SIM300_CLOSE_OK);
    
    simx_tcp_close(&reply, 2);
    EXPECT_EQ(g_length, strlen("AT+CIPCLOSE=2,0\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPCLOSE=2,0\r\n");
    simx_test_send("AT+CIPCLOSE=2,0\r");
    simx_test_send("\r\nERROR\r\n");
    EXPECT_EQ(reply.status, SIM300_ERROR);
}

TEST(AutoTestSim800, SIM_AT_CIPHEAD)
{
    sim_reply_t reply;
    simx_tcp_head_enable(&reply, 0);
    EXPECT_EQ(g_length, strlen("AT+CIPHEAD=0\r\n"));
    EXPECT_EQ(strlen(g_data), strlen("AT+CIPHEAD=0\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPHEAD=0\r\n");
    simx_test_send("AT+CIPHEAD=0\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    //EXPECT_EQ(sim_cip_mux_mode(), 0);
    
    simx_tcp_head_enable(&reply, 1);
    EXPECT_STREQ(g_data, "AT+CIPHEAD=1\r\n");
    simx_test_send("AT+CIPHEAD=1\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    //EXPECT_EQ(sim_cip_mux_mode(), 1);
}

TEST(AutoTestSim800, SIM_AT_CSCS)
{
    sim_reply_t reply;
    
    simx_set_TE_character(&reply, SIM_IRA);
    EXPECT_EQ(strlen(g_data), strlen("AT+CSCS=\"IRA\"\r\n"));
    EXPECT_EQ(g_length, strlen("AT+CSCS=\"IRA\"\r\n"));
    simx_test_send("AT+CSCS=\"IRA\"\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
}


TEST(AutoTestSim800, SIM_AT_CMGR)
{
    sim_reply_t reply;
    sim_sms_t sms;
    memset(&sms, 0, sizeof(sim_sms_t));
    char buffer[200];
    sms.msg = buffer;
    sms.msg_length = 200;
    simx_read_sms(&reply, &sms, 4);
    EXPECT_EQ(g_length, strlen("AT+CMGR=4\r\n"));
    EXPECT_STREQ(g_data, "AT+CMGR=4\r\n");
    simx_test_send("AT+CMGR=4\r");
    
    simx_test_send("\r\n+CMGR: \"REC READ\",\"MTC\",\"\",\"18/01/22,19:07:20+12\"\r\n"
                   "test message\r\n\r\nOK\r\n");
    
    simx_test_send("\r\nOK\r\n");
    
    EXPECT_STREQ(sms.number, "MTC");
    EXPECT_STREQ(sms.date, "18/01/22,19:07:20+12");
    EXPECT_STREQ(sms.msg, "test message");
    
    EXPECT_EQ(reply.status, SIM300_OK);
    
    memset(&sms, 0, sizeof(sim_sms_t));
    sms.msg = buffer;
    sms.msg_length = 200;
    simx_read_sms(&reply, &sms, 4);
    EXPECT_EQ(g_length, strlen("AT+CMGR=4\r\n"));
    EXPECT_STREQ(g_data, "AT+CMGR=4\r\n");
    simx_test_send("AT+CMGR=4\r");
    simx_test_send("\r\n+CMGR: \"REC READ\",\"looooooooooooooooooooooooooooooong\",\"\",\"18/01/22,19:07:20+12\"\r\n"
                   "test message\r\n\r\nOK\r\n");
    
    simx_test_send("\r\nOK\r\n");
    
    EXPECT_STREQ(sms.number, "");
    EXPECT_STREQ(sms.date, "18/01/22,19:07:20+12");
    EXPECT_STREQ(sms.msg, "test message");
    
    EXPECT_EQ(reply.status, SIM300_OK);
}


TEST(AutoTestSim800, SIM_AT_CGDCONT)
{
    sim_reply_t reply;
    simx_define_pdp_context(&reply, 1, (char *)"IP", (char *)"internet.mts.ru");
    EXPECT_EQ(g_length, strlen("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r\n"));
    EXPECT_STREQ(g_data, "AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r\n");
    simx_test_send("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_define_pdp_context(&reply, 1, (char *)"IP", (char *)"internet.mts.ru");
    simx_test_send("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r");
    simx_test_send("\r\nERROR\r\n");
    EXPECT_EQ(reply.status, SIM300_ERROR);
}

TEST(AutoTestSim800, SIM_ATD)
{
    sim_reply_t reply;
    simx_call_to_dial_number(&reply, (char *)"*99***1#");
    EXPECT_EQ(g_length, strlen("ATD*99***1#\r\n"));
    EXPECT_STREQ(g_data, "ATD*99***1#\r\n");
    simx_test_send("ATD*99***1#\r");
    simx_test_send("\r\nCONNECT\r\n");
    EXPECT_EQ(reply.status, SIM300_CONNECT);
}

TEST(AutoTestSim800, SIM_ATO_PPP)
{
    sim_reply_t reply;
    simx_switch_mode(&reply, SIM_DATA_MODE);
    EXPECT_EQ(g_length, strlen("ATO\r\n"));
    EXPECT_STREQ(g_data, "ATO\r\n");
    simx_test_send("ATO\r");
    simx_test_send("\r\nCONNECT\r\n");
    EXPECT_EQ(reply.status, SIM300_CONNECT);
    
    simx_switch_mode(&reply, SIM_DATA_MODE);
    simx_test_send("ATO\r");
    simx_test_send("\r\nNO CARRIER\r\n");
    EXPECT_EQ(reply.status, SIM300_NO_CARRIER);
    
    simx_switch_mode(&reply, SIM_COMMAND_MODE);
    EXPECT_EQ(g_length, strlen("+++"));
    EXPECT_STREQ(g_data, "+++");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
}

TEST(AutoTestSim800, SIM_AT_CIPCSGP)
{
    sim_reply_t reply;
    simx_set_connection_mode(&reply, SIM_CSD, (char *)"internet.mts.ru", (char *)"mts", (char *)"mts", SIM_CSD_RATE_14400);
    EXPECT_EQ(g_length, strlen("AT+CIPCSGP=0,\"internet.mts.ru\",\"mts\",\"mts\",14400\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPCSGP=0,\"internet.mts.ru\",\"mts\",\"mts\",14400\r\n");
    simx_test_send("AT+CIPCSGP=0,\"internet.mts.ru\",\"mts\",\"mts\",14400\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_set_connection_mode(&reply, SIM_CSD,(char *)"internet.mts.ru", (char *)"mts", (char *)"mts", SIM_CSD_RATE_14400);
    simx_test_send("AT+CIPCSGP=0,\"internet.mts.ru\",\"mts\",\"mts\",14400\r");
    simx_test_send("\r\nERROR\r\n");
    EXPECT_EQ(reply.status, SIM300_ERROR);
    
    simx_set_connection_mode(&reply, SIM_GPRS, (char *)"internet.mts.ru", (char *)"mts", (char *)"mts", SIM_CSD_RATE_14400);
    EXPECT_EQ(g_length, strlen("AT+CIPCSGP=1,\"internet.mts.ru\",\"mts\",\"mts\"\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPCSGP=1,\"internet.mts.ru\",\"mts\",\"mts\"\r\n");
    simx_test_send("AT+CIPCSGP=1,\"internet.mts.ru\",\"mts\",\"mts\"\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
}

TEST(AutoTestSim800MSG, SIM_MSG_CALLBACK)
{
    sim_reply_t reply;
    
    simx_multiple_connection(&reply, 0);
    simx_test_send("AT+CIPMUX=0\r");
    simx_test_send("\r\nOK\r\n");
    
    simx_test_send("\r\nALREADY CONNECT\r\n");
    EXPECT_EQ(g_con_status, SIM_TCP_ALREADY_CONNECT);
    simx_test_send("\r\nCLOSED\r\n");
    EXPECT_EQ(g_con_status, SIM_TCP_CLOSED);
    simx_test_send("\r\nCONNECT OK\r\n");
    EXPECT_EQ(g_con_status, SIM_TCP_CONNECT_OK);
    simx_test_send("\r\nCONNECT FAIL\r\n");
    EXPECT_EQ(g_con_status, SIM_TCP_CONNECT_FAIL);
    
    simx_multiple_connection(&reply, 1);
    simx_test_send("AT+CIPMUX=1\r");
    EXPECT_STREQ(g_data, "AT+CIPMUX=1\r\n");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    EXPECT_EQ(sim_cip_mux_mode(), 1);
    
    simx_test_send("\r\n1, ALREADY CONNECT\r\n");
    EXPECT_EQ(g_con_status, SIM_TCP_ALREADY_CONNECT);
    EXPECT_EQ(g_con_n, 1);
    simx_test_send("\r\n2, CLOSED\r\n");
    EXPECT_EQ(g_con_status, SIM_TCP_CLOSED);
    EXPECT_EQ(g_con_n, 2);
    simx_test_send("\r\n3, CONNECT OK\r\n");
    EXPECT_EQ(g_con_status, SIM_TCP_CONNECT_OK);
    EXPECT_EQ(g_con_n, 3);
    simx_test_send("\r\n4, CONNECT FAIL\r\n");
    EXPECT_EQ(g_con_status, SIM_TCP_CONNECT_FAIL);
    EXPECT_EQ(g_con_n, 4);
    
}

TEST(AutoTestSim800MSG, SIM_TCP_CALLBACK)
{
    sim_reply_t reply;
    
    simx_multiple_connection(&reply, 1);
    simx_test_send("AT+CIPMUX=1\r");
    simx_test_send("\r\nOK\r\n");
    
    simx_test_send("\r\n+RECEIVE,1,11:\r\nSOMETHING\r\n");
    EXPECT_STREQ(g_tcp_data, "SOMETHING\r\n");
    EXPECT_EQ(g_tcp_length, 11);
    EXPECT_EQ(g_tcp_con, 1);
    
    /***********************************************/
    simx_sms_mode(&reply, SIM_SMS_TEXT);
    EXPECT_EQ(g_length, strlen("AT+CMGF=1\r\n"));
    EXPECT_STREQ(g_data, "AT+CMGF=1\r\n");
    simx_test_send("AT+CMGF=1\r");

    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_sms_mode(&reply, SIM_SMS_PDU);
    EXPECT_EQ(g_length, strlen("AT+CMGF=0\r\n"));
    EXPECT_STREQ(g_data, "AT+CMGF=0\r\n");
    simx_test_send("AT+CMGF=0\r");
   
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    /***********************************************/
    
    simx_multiple_connection(&reply, 0);
    simx_test_send("AT+CIPMUX=0\r");
    simx_test_send("\r\nOK\r\n");
    
    simx_tcp_head_enable(&reply, 1);
    EXPECT_STREQ(g_data, "AT+CIPHEAD=1\r\n");
    simx_test_send("AT+CIPHEAD=1\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_test_send("\r\n+IPD,11:\r\nSOMETHING\r\n");
    EXPECT_STREQ(g_tcp_data, "SOMETHING\r\n");
    EXPECT_EQ(g_tcp_length, 11);
    
    g_tcp_data = NULL;
    g_tcp_length = 0;
    simx_test_send("\r\nSOMETHING\r\n");
    //EXPECT_EQ(g_tcp_data, NULL);
    EXPECT_EQ(g_tcp_length, 0);
    
    /***********************************************/
    simx_sms_mode(&reply, SIM_SMS_TEXT);
    EXPECT_EQ(g_length, strlen("AT+CMGF=1\r\n"));
    EXPECT_STREQ(g_data, "AT+CMGF=1\r\n");
    simx_test_send("AT+CMGF=1\r");

    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    simx_sms_mode(&reply, SIM_SMS_PDU);
    EXPECT_EQ(g_length, strlen("AT+CMGF=0\r\n"));
    EXPECT_STREQ(g_data, "AT+CMGF=0\r\n");
    simx_test_send("AT+CMGF=0\r");
    
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    
    /***********************************************/
    
}

TEST(AutoTestSim800MSG, SIM_SMS_CALLBACK)
{
    sim_reply_t reply;
    simx_test_send("\r\n+CMTI: \"SM\",4\r\n");
    EXPECT_EQ(g_sms_number, 4);
    /**********************************************/
    simx_sms_mode(&reply, SIM_SMS_TEXT);
    EXPECT_EQ(g_length, strlen("AT+CMGF=1\r\n"));
    EXPECT_STREQ(g_data, "AT+CMGF=1\r\n");
    simx_test_send("AT+CMGF=1\r");
    simx_test_send("\r\nOK\r\n");
    EXPECT_EQ(reply.status, SIM300_OK);
    /**********************************************/
    
}

TEST(AutoTestSim800MSG, SIM_PDP_CALLBACK)
{
    
    simx_test_send("\r\n+PDP: DEACT\r\n");
    EXPECT_EQ(g_notification, SIMX_PDP_DEACT);
    
    g_notification = SIMX_NTF_UNKNOWN;
    
    sim_reply_t reply;
    simx_multiple_connection(&reply, 0);
    simx_test_send("\r\nOK\r\n");
    
    char msg[] = "test";
    int n = strlen(msg);
    simx_tcp_send_data(&reply, (uint8_t*)msg, n, 0);
    EXPECT_EQ(g_length, strlen("AT+CIPSEND=4\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPSEND=4\r\n");
    
    simx_test_send("\r\n+PDP: DEACT\r\n");
    EXPECT_EQ(g_notification, SIMX_PDP_DEACT);
    EXPECT_EQ(simx_is_receive(), 1);
    //EXPECT_EQ(reply.status, SIM300_ERROR);
    EXPECT_EQ(reply.status, SIM300_ERRFRAME);
    
}

TEST(AutoTestSim800MSG, SIM_CPIN_NOT_READY_CALLBACK)
{
    
    simx_test_send("\r\n+CPIN: NOT READY\r\n");
    EXPECT_EQ(g_notification, SIMX_CPIN_NOT_READY);
    
    g_notification = SIMX_NTF_UNKNOWN;
    
    sim_reply_t reply;
    simx_multiple_connection(&reply, 0);
    simx_test_send("AT+CIPMUX=0\r");
    simx_test_send("\r\nOK\r\n");
    
    char msg[] = "test";
    int n = strlen(msg);
    simx_tcp_send_data(&reply, (uint8_t*)msg, n, 0);
    EXPECT_EQ(g_length, strlen("AT+CIPSEND=4\r\n"));
    EXPECT_STREQ(g_data, "AT+CIPSEND=4\r\n");
    
    simx_test_send("\r\n+CPIN: NOT READY\r\n");
    EXPECT_EQ(g_notification, SIMX_CPIN_NOT_READY);
    simx_test_send("\r\n+PDP: DEACT\r\n");
    EXPECT_EQ(g_notification, SIMX_PDP_DEACT);
    //EXPECT_EQ(reply.status, SIM300_ERROR);
    EXPECT_EQ(reply.status, SIM300_ERRFRAME);
    EXPECT_EQ(simx_is_receive(), 1);
}

TEST(AutoTestSim800Timeout, SIM_TIMEOUT)
{
    sim_reply_t reply;
    
    simx_test(&reply);
    set_timeout();
    simx_wait_reply(&reply);
    EXPECT_EQ(reply.status, SIM300_TIMEOUT);
}

TEST(AutoTestSim800Timeout, SIM_BUSY)
{
    sim_reply_t reply1;
    sim_reply_t reply2;
    
    simx_test(&reply1);
    simx_test(&reply2);
    EXPECT_EQ(reply2.status, SIM300_BUSY);
    simx_test_send("AT\r");
    simx_test_send("\r\nOK\r\n");
    
    EXPECT_EQ(reply1.status, SIM300_OK);
}

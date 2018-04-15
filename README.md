[![Build Status](https://travis-ci.org/Zaparivanny/sim900.svg?branch=develop)](https://travis-ci.org/Zaparivanny/sim900)
[![Coverage Status](https://coveralls.io/repos/github/Zaparivanny/sim900/badge.svg)](https://coveralls.io/github/Zaparivanny/sim900)

# sim900 library


## How to install this library

```sh
git clone --recursive https://github.com/Zaparivanny/sim900.git

```

## Supported commands

| Command      |  Description                                                       |
|--------------|--------------------------------------------------------------------|
| AT                                                                                |
| AT+CPIN?     | Enter PIN                                                          |
| AT+CREG?     | Network Registration                                               |
| AT+CSMINS?   | Sim inserted status reporting                                      |
| AT+CGATT?    | Attach or detach from gprs service                                 |
| AT+CIPSHUT   | Deactivate gprs pdp context                                        |
| AT+CIPSTATUS | Query Current Connection Status                                    |
| AT+CIPMUX=   | Start Up Multi-IP Connection                                       |
| AT+CSTT=     | Start Task and Set APN, USER NAME, PASSWORD                        |
| AT+CIICR     | Bring Up Wireless Connection with GPRS or CSD                      |
| AT+CIFSR     | Get Local IP Address                                               |
| AT+CSQ       | Signal quality report                                              |
| AT+CIPSTART= | Start up tcp or udp connection                                     |
| AT+CIPSEND=  | Send data through tcp or udp connection                            |
| AT+CIPCLOSE= | Close tcp or udp connection                                        |
| AT+CIPHEAD=  | Add an ip head at the beginning of a package received              |
| AT+CSCS=     | Select TE Character Set                                            |
| AT+CMGF=     | Select SMS Message Format                                          |
| AT+CMGS=     | Send SMS message                                                   |
| CMGR=        | Read SMS Message                                                   |
| ATD          | Originate Call to Phone Number in Memory Which Corresponds to Field|
| ATO          | Switch from Command Mode to Data Mode                              |
| +++          | Switch from Data Mode or PPP Online Mode to Command Mode           |
| AT+CGDCONT   | Define PDP Context                                                 |
| AT+CIPCSGP=  | Set CSD or GPRS for Connection Mode                                |

## Implementation

You must implement 5 callback:

```c
void simx_callback_send(uint8_t *data, uint16_t length);
void simx_callback_tcp_msg(sim_con_status_t con_status, uint8_t n);
void simx_callback_tcp_data(uint8_t *data, uint16_t length, uint8_t n);
void simx_callback_sms_received(uint16_t number);
void simx_callback_pdp_deact();
```
simx_callback_send - called when need to send data to uart  
simx_callback_tcp_msg - called when receive a connection notification  
simx_callback_tcp_data - called when TCPIP data came in  
simx_callback_sms_received - called when the SMS came  
simx_callback_pdp_deact - called when connection lost  

## Examples
* Connect to gprs

```cpp
    sim_reply_t reply;
    simx_is_attach_to_GPRS(&reply);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    
    simx_deactivate_gprs_pdp(&reply);
    simx_wait_reply();
    if(reply.status != SIM300_SHUT_OK){return;}
    
    simx_current_connection_status(&reply);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    
    simx_set_gprs_config(&reply, APN, APN_USER_NAME, APN_PASSWORD);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    
    simx_bring_up_wireless_connection(&reply);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    
    sim_ip_t ip;
    simx_get_local_ip(&reply, &ip);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    std::cout << "IP:" << (int)ip.addr0 << "." << (int)ip.addr1 << "."
                       << (int)ip.addr2 << "."<< (int)ip.addr3 << "." << std::endl;
```

* Send SMS
```cpp
    sim_reply_t reply;
    simx_send_sms(&reply, PHONE_NUMBER, "test");
    simx_wait_reply();
```

* Read SMS
```cpp
    sim_reply_t reply;
    char buffer[200];
    sms.msg = buffer;
    sms.msg_length = 200;
    simx_read_sms(&reply, &sms, 4);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    std::cout << "SMS:" << sms.msg;
```

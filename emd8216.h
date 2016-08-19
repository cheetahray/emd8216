#ifndef __EMD8216_H__
#define __EMD8216_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#define CARD_ID_MAX		1999
#define Password_MAX		8
#define IP_MAX			4
#define	TIME_OUT_MAX			50000
#define	TIME_OUT_MIN			1000
#define	WDT_TIME_OUT_MAX		10000
#define	WDT_TIME_OUT_MIN		10

#define PHY_PORT_MAX		2
#define VIR_PORT_MAX		4
#define POINT_MAX           8
#define HOST_MAX    		4

#define INPORT				1
#define OUTPORT				0

#define REMOTE_PORT_MIN		1024

#define STANDALONE_FUNCTION_MAX			32

#define	_UNUSED     0
#define	_CONNECT	1
#define	_BREAK      2

#define EMD8204     1
#define EMD8208     2
#define EMD8216     3
#define EMC8485     4
#define EMC8432     5
#define EMA8314     6
#define EMA8308     7
#define EMA8308D    8
#define PC          9

////////////// Error Code //////////////
#define NO_ERROR                0
#define INITIAL_SOCKET_ERROR	-1
#define IP_ADDRESS_ERROR	-2
/***** Device Error *****/
#define	UNLOCK_ERROR		-3
#define	LOCK_COUNTER_ERROR	-4
#define	SET_SECURITY_ERROR	-5
/*----------------------*/

#define	DEVICE_RW_ERROR		-100
#define	NO_CARD			-101
#define	DUPLICATE_ID		-102
#define	COMMAND_ERROR		-103

/******** User Parameter Error ********/
#define	ID_ERROR                    -300
#define	PORT_ERROR                  -301
#define	IN_POINT_ERROR              -302
#define	OUT_POINT_ERROR             -303
#define	PARAMETERS_ERROR            -305
#define	CHANGE_SOCKET_ERROR         -306
#define	UNLOCK_SECURITY_ERROR       -307
#define	PASSWORD_ERROR              -308
#define	REBOOT_ERROR                -309
#define TIME_OUT_ERROR              -310
#define	CREATE_SOCKET_ERROR         -311
#define CHANGEIP_ERROR              -312
#define	MASK_CHANNEL_ERROR          -313
#define	COUNTER_ENABLE_ERROR        -314
#define	COUNTER_DISABLE_ERROR       -315
#define	COUNTER_READ_ERROR          -316
#define	COUNTER_CLEAR_ERROR         -317
#define	TIME_ERROR                  -318

#define	CARD_VERSION_ERROR          -320
#define	STANDALONE_ENABLE_ERROR     -321
#define	STANDALONE_DISABLE_ERROR    -322
#define	STANDALONE_CONFIG_ERROR     -323
/*-------------------------------------*/

typedef	char            i8;
typedef	unsigned char   u8;
typedef	short           i16;
typedef	unsigned short  u16;
typedef	long            i32;
typedef	unsigned long   u32;
typedef	float           f32;
typedef	double          f64;

struct _StandaloneData
{
    u16	in_point_bit;
    u16	in_state_bit;
    u16	timer_value;
    u16	out_point_bit;
    u8	timer_mode;
    u8	out_mode;
};

struct _V_StandaloneData
{
    ////u16	in_point_bit;
    ////u16	in_state_bit;
    ////u16	timer_value;
    ////u16	out_point_bit;
    ////u8	timer_mode;
    ////u8	out_mode;
    u32	timer_value;
    u8	in_virtual_point_bit[2];
    u8	in_virtual_state_bit[2];
    u8	out_virtual_point_bit[2];
    u8	in_point_bit[2];
    u8	in_state_bit[2];
    u8	out_point_bit[2];
    u8	control_mode;
    u8	out_mode;
    u8	disconnect_mask[4];
};


#ifdef __cplusplus
    extern "C"{
#endif

#define EMD8216Status   i32
/*************************************************************************************/
//--- Initialization & close
EMD8216Status EMD8216_initial(u32 CardID,u8 IP_Address[4],u16 Host_Port,u16 Remote_port,u16 TimeOut_ms,u8 *CardType);
EMD8216Status EMD8216_close(u32 CardID);
EMD8216Status EMD8216_firmware_version_read(u32 CardID,u8 Version[2]);

//--- Input / Output
EMD8216Status EMD8216_polarity_read(u32 CardID,u8 port,u8 *data);
EMD8216Status EMD8216_polarity_set(u32 CardID,u8 port,u8 data);
EMD8216Status EMD8216_port_read(u32 CardID,u8 port,u8 *data);
EMD8216Status EMD8216_port_set(u32 CardID,u8 port,u8 data);
EMD8216Status EMD8216_port_config_read(u32 CardID ,u8 port, u8 *config);
EMD8216Status EMD8216_port_config_set(u32 CardID ,u8 port, u8 config);

EMD8216Status EMD8216_point_polarity_read(u32 CardID,u8 port,u8 point,u8 *data);
EMD8216Status EMD8216_point_polarity_set(u32 CardID,u8 port,u8 point,u8 data);
EMD8216Status EMD8216_point_read(u32 CardID,u8 port,u8 point,u8 *state);
EMD8216Status EMD8216_point_set(u32 CardID, u8 port, u8 point, u8 state);
EMD8216Status EMD8216_point_config_read(u32 CardID, u8 port, u8 point, u8 *config);
EMD8216Status EMD8216_point_config_set(u32 CardID, u8 port, u8 point, u8 config);

//--- Counter
EMD8216Status EMD8216_counter_mask_set(u32 CardID,u8 port,u8 channel);
EMD8216Status EMD8216_counter_enable(u32 CardID);
EMD8216Status EMD8216_counter_disable(u32 CardID);
EMD8216Status EMD8216_counter_read(u32 CardID,u8 port,u32 counter[8]);
EMD8216Status EMD8216_counter_clear(u32 CardID,u8 port,u8 channel);

//--- Miscellaneous
EMD8216Status EMD8216_socket_port_change(u32 CardID,u16 Remote_port);
EMD8216Status EMD8216_IP_change(u32 CardID,u8 IP[4]);
EMD8216Status EMD8216_reboot(u32 CardID);
EMD8216Status EMD8216_subnet_mask_set(u32 CardID,u8 subnet_mask[4]);
EMD8216Status EMD8216_subnet_mask_read(u32 CardID,u8 subnet_mask[4]);

//--- software key
EMD8216Status EMD8216_security_unlock(u32 CardID,u8 password[8]);
EMD8216Status EMD8216_security_status_read(u32 CardID,u8 *lock_status);
EMD8216Status EMD8216_password_change(u32 CardID,u8 Oldpassword[8],u8 Password[8]);
EMD8216Status EMD8216_password_set_default(u32 CardID);
EMD8216Status EMD8216_write_mac(u32 CardID,u8 mac[6]);

//--- WDT
EMD8216Status EMD8216_WDT_enable(u32 CardID);
EMD8216Status EMD8216_WDT_disable(u32 CardID);
EMD8216Status EMD8216_WDT_set(u32 CardID,u16 time,u8 state[2]);
EMD8216Status EMD8216_WDT_read(u32 CardID,u16 *time,u8 state[2],u8 *enable);

//--- standalone
EMD8216Status EMD8216_standalone_enable(u32 CardID);
EMD8216Status EMD8216_standalone_disable(u32 CardID);
EMD8216Status EMD8216_standalone_config_set(u32 CardID,u8 function_number,_StandaloneData data[32],u8 standalone_state);
EMD8216Status EMD8216_standalone_config_read(u32 CardID,u8 *function_number,_StandaloneData data[32],u8 *enable,u8 *power_on_enable);
EMD8216Status EMD8216_standalone_V_config_set(u32 CardID,u8 number,_V_StandaloneData data[32],u8 standalone_state);
EMD8216Status EMD8216_standalone_V_config_read(u32 CardID,u8 *number,_V_StandaloneData data[32],u8 *enable,u8 *standalone_state);
EMD8216Status EMD8216_standalone_config_clear(u32 CardID);

//--- host config
EMD8216Status EMD8216_host_config_set(u32 CardID, u8 host_ID, u8 IP_address[4], u8 password[8], u16 host_port, u8 card_type);
EMD8216Status EMD8216_host_config_read(u32 CardID, u8 host_ID, u8 IP_address[4], u16 *host_port, u8 *card_type);
EMD8216Status EMD8216_host_connection_clear(u32 CardID, u8 state[4]);
EMD8216Status EMD8216_host_connection_read(u32 CardID, u8 state[4], u8 error_state[4]);

/*************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /*- __EMD820x_H__ -*/

#ifndef __CONST8216_H
#define __CONST8216_H


typedef	char		i8;
typedef	unsigned char	u8;
typedef	short		i16;
typedef	unsigned short	u16;
typedef	long		i32;
typedef	unsigned long	u32;
typedef	float		f32;
typedef	double		f64;

#define	SUCCESS_FLAG			99

#define	COMMAND_GET_CARD_TYPE           0x1
#define	COMMAND_REBOOT                  0x2
#define	COMMAND_CHANGESOCKETPORT    	0x3
#define	COMMAND_CHANGE_PASSWORD         0x4
#define	COMMAND_RESTORE_PASSWORD        0x5
#define	COMMAND_CHANGEIP        		0x6
#define	COMMAND_GET_FIRMWARE_VERSION	0x7
#define COMMAND_CHANGE_SUBNET_MASK		0x8
#define COMMAND_READ_SUBNET_MASK 		0x9

#define	COMMAND_SET_COUNTER_MASK        0x20
#define	COMMAND_ENABLE_COUNTER_MODE     0x21
#define	COMMAND_DISABLE_COUNTER_MODE	0x22
#define	COMMAND_READ_COUNTER            0x23
#define	COMMAND_CLEAR_COUNTER           0x24

#define COMMAND_PORT_CONFIG_SET         0x30
#define	COMMAND_PORT_CONFIG_READ        0x31
#define COMMAND_PORT_SET                0x32
#define COMMAND_PORT_READ               0x33
#define COMMAND_POLARITY_SET            0x34
#define COMMAND_POLARITY_READ           0x35

#define	COMMAND_POINT_CONFIG_SET	0x36
#define	COMMAND_POINT_CONFIG_READ	0x37
#define	COMMAND_POINT_SET		0x38
#define	COMMAND_POINT_READ		0x39
#define	COMMAND_POINT_POLARITY_SET	0x3A
#define	COMMAND_POINT_POLARITY_READ 0x3B

#define	COMMAND_STANDALONE_ENABLE	0x50
#define	COMMAND_STANDALONE_DISABLE	0x51
#define	COMMAND_STANDALONE_CONFIG_SET	0x52
#define	COMMAND_STANDALONE_CONFIG_READ	0x53
#define	COMMAND_STANDALONE_CONFIG_SET_NEW	0x54
#define	COMMAND_STANDALONE_CONFIG_READ_NEW	0x55
#define	COMMAND_STANDALONE_CONFIG_CLEAR	0x56

#define	COMMAND_WDT_ENABLE		0x60
#define	COMMAND_WDT_DISABLE		0x61
#define	COMMAND_WDT_SET			0x62
#define	COMMAND_WDT_READ		0x63

#define	COMMAND_HOST_CONFIG_SET						0x70
#define	COMMAND_HOST_CONFIG_READ					0x71
#define	COMMAND_HOST_CONNECTION_CLEAR				0x74
#define	COMMAND_HOST_CONNECTION_READ				0x75

#define	COMMAND_WRITE_MAC		0xfa

#define	_COMMAND_ERROR		100
#define	_PASSWORD_ERROR		101
#define	_CHANGE_IP_ERROR	102
#define	_CHANGE_SOCKET_ERROR	103
#define	_CHANGE_MAC_ERROR	104

#define	_PORT_ERROR		120
#define	_POINT_ERROR		121
#define	_STATE_ERROR		122
#define	_VALUE_ERROR		123
#define	_MODE_ERROR		124

#define	_VOLTAGE_ERROR		140
#define	_CONVERTER_ERROR	141

#define	_UNUSED			0
#define	_CONNECTED		1
#define _RE_CONNECTION	2
#define	_DISCONNECTION	3
#define	_WAIT_RESPOND	4
#define	_WAIT_TO_SEND	5
#define	_SEND_TO_HOST	6

// -- structure
struct _StandaloneDataUdp
{
    u8  function_index;     // start function index
    u8  function_number;    // be used max function number
    u8  timer_mode[2];      // set timer mode
    u16 timer_value[2];     // set timer value
    u16 input_point[2];     // choose input point00 ~ 17
    u16 input_state[2];     // set input state
    u16 output_point[2];    // choose output point00 ~ 17
    u8  out_mode[2];        // set out mode
    u8  standalone_state;   // 1: standalone Enable; 0: standalone Disable
};

struct _WDTData
{
    u16 Timer_value;    // WDT timer value
    u8  output[2];      // WDT timer out, output data
    u8  state;          // 1: WDT Enable, 0: WDT Disable
};

struct _FUNCTION_DATA
{
    u8	control_mode;			// set control mode
    u8	out_mode;			// set out mode
    u8	timer_value[4];			// set timer value

    //u16	input_point[2];		// choose input point00 ~ 17
    u8	in_point[2];		// choose input point00 ~ 17
    u8	in_virtual_point[2];		// choose input point00 ~ 17

    //u16	input_state[2];			// set input state
    u8	in_state[2];			// set input state
    u8	in_virtual_state[2];			// set input state

    //u16	output_point[2];		// choose output point00 ~ 17
    u8	output_point[2];		// choose output point00 ~ 17
    u8	out_virtual_point[2];		// choose output point00 ~ 17

    u8	disconect_mask[4];		// set disconect mask
};

struct _NEW_ST_DATA
{
    u8	function_index;		// start function index
    u8	function_number;	// be used max function number
    u8	standalone_state;
    u8	power_on_state;
    _FUNCTION_DATA	config_data;
};


struct _HOST_CONFIG
{
    u8	IP[4];			// host IP address
    u8	password[8];		// host device password
    u16	host_port;		// host device socket port
    u8	card_type;		// host device type, other: error
                            // 1: EMD8204, 2: EMD8208, 3: EMD8216
                            // 4: EMC8432, 5: EMC8485, 6: EMA8314R
                            // 7: EMA8308, 8: EMA8308D,9: PC
    u8	index;			// host device ID, 0~3
};

struct _HOST_STATE
{
    u8	connection_state[4];	//Data (connection state and mask)
    u8	connection_ERROR_code[4];	//Data (connection state and mask)
};


struct _MULTI
{
    u8 point[4];
    u8 stat[4];
};

typedef struct _EMD8216_Data
{
    u8  card_name[7];   // CardName={'E','M','D','8','2','0','1'}
    u8  password[8];    // Password,8 words
    u8  command;        // 0:No command
                        // 1:Read port
                        // 2:Write port
                        // 3:Change socket port
                        // 4:Reboot
                        // 5:Change password
                        // 6:restore password
                        // 7:ReadCardType
    union
    {
        u8  IP[4];              // Data (IP Address)
        u16 socket_port;        // Data (Socket Port)
        u8  new_password[8];    // Data (New password,8 words)
        u8  MAC[6];             // Data (MAC Address)
        u8  Port_value[2];      // Data (Port Value)
        u8  Point_value[3];     // Data (Point Select)
        u8  subnet_mask[4];
        _StandaloneDataUdp standalone_data; // maximum 82byte
        _WDTData WDT;           // Data (WDT wait time & output state)

        _NEW_ST_DATA		new_st_data;	// maximum24byte
        _MULTI				multi;			// multi point R/W
        _HOST_CONFIG		host_config;	// host device config data
        _HOST_STATE         host_state;		// host device connect state and error code
    };
}EMD8216_Data;

typedef struct _EMD8216_Receive
{
    union
    {
        u8  counter[8][4];      //Data (Read back Counte value)
        u8  Port_value[2];      //Data (Port Value)
        u8  Point_value[3];     //Data (Point Select)
        u8  Version[2];         //Data (firmware version)
        u8  subnet_mask[4];
        _StandaloneDataUdp standalone_data; // maximum 82byte
        _WDTData WDT;           //Data (WDT wait time & output state)
        _NEW_ST_DATA		new_st_data;	// maximum24byte
        _MULTI				multi;			// multi point R/W
        _HOST_CONFIG		host_config;	// host device config data
        _HOST_STATE         host_state;		// host device connect state and error code
    };
    u8  success_flag;   // 0:Send command Failed
                        // 99:Send command successfully
    u8  command;        // 0:No command
                        // 1:Read port
                        // 2:Write port
                        // 3: Change socket port
                        // 4:Reboot
                        // 5:Change password
                        // 6:restore password
                        // 7:ReadCardType
}EMD8216_Receive;

union _u_type
{
    u32 u32_data;
    u8 u8_data[4];
};

struct _Stack
{
    int index;
    _Stack *pre;
    _Stack *next;
    union
    {
        u8 IP[4];
        u32 u32data;
    };
};

#endif	/*- EMD8216_H -*/

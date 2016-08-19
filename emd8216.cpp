// EMD8208.cpp : Defines the entry point for the DLL application.

#include "emd8216.h"
#include "const8216.h"

//-----------------------
int m_pBroadSock[CARD_ID_MAX+1]         = {0};            // socket fd

bool    socket_flag[CARD_ID_MAX+1]          = {false};
bool    mblnUnlockFlag[CARD_ID_MAX+1]       = {false};
u16     mu8SocketPort_Local[CARD_ID_MAX+1]  = {0};		// communication port of host PC
u16     mu8SocketPort_Remote[CARD_ID_MAX+1] = {0};		// communication port of EMD8216
u32     cintTimeOut                          = 1000;		// End time(mini-second)
char    dstIP[CARD_ID_MAX+1][16]            = {{0}};
u8	Card_Type[CARD_ID_MAX+1]            = {0};
u8	Point_Max[CARD_ID_MAX+1]            = {0};

EMD8216_Data *pmaudtEMD8216[CARD_ID_MAX+1];

u8 Send_Size = sizeof(_EMD8216_Data);
u8 Receive_Size = sizeof(EMD8216_Receive);


/*------------------- sub function ------------------*/
EMD8216Status EMD8216_init(u32 CardID)
{
//    printf("EMD8216_initial00002\n");
    i32 ErrorCode = 0;

    pmaudtEMD8216[CardID] = new EMD8216_Data();

    if( NULL != pmaudtEMD8216[CardID] )
    {
            pmaudtEMD8216[CardID]->card_name[0] = 'E';
            pmaudtEMD8216[CardID]->card_name[1] = 'M';
            pmaudtEMD8216[CardID]->card_name[2] = 'D';
            pmaudtEMD8216[CardID]->card_name[3] = '8';
            pmaudtEMD8216[CardID]->card_name[4] = '2';
            pmaudtEMD8216[CardID]->card_name[5] = '1';
            pmaudtEMD8216[CardID]->card_name[6] = '6';

            mblnUnlockFlag[CardID] = false;
            socket_flag[CardID] = false;

            ErrorCode = NO_ERROR;
    }
    else
    {
            ErrorCode = INITIAL_SOCKET_ERROR;
    }

    return ErrorCode;
}

i16 check_success_flag( u8 success_flag, i16 ErrorCode )
{
        i16 error_code = ErrorCode;
        switch(success_flag)
        {
        case _COMMAND_ERROR:
                break;
        case _PASSWORD_ERROR:
                error_code = PASSWORD_ERROR;
                break;
        case _CHANGE_IP_ERROR:
                error_code = CHANGEIP_ERROR;
                break;
        case _CHANGE_SOCKET_ERROR:
                error_code = CHANGE_SOCKET_ERROR;
                break;
        case _CHANGE_MAC_ERROR:
                error_code = (-1) * _CHANGE_MAC_ERROR;
                break;

        case _PORT_ERROR:
                error_code = PORT_ERROR;
                break;
        case _POINT_ERROR:
                //error_code = PARAMETERS_ERROR;
                //break;
        case _STATE_ERROR:
                //error_code = PARAMETERS_ERROR;
                //break;
        case _VALUE_ERROR:
                //error_code = PARAMETERS_ERROR;
                //break;
        case _MODE_ERROR:
                //error_code = PARAMETERS_ERROR;
                //break;
        case _VOLTAGE_ERROR:
                //error_code = PARAMETERS_ERROR;
                //break;
        case _CONVERTER_ERROR:
                error_code = PARAMETERS_ERROR;
                break;
        case SUCCESS_FLAG:
                error_code = 0;
                break;
        default:
                error_code = ErrorCode;
                break;
        }
        return error_code;
}

// Calculate time
unsigned long long calculate_time()
{
  struct timeval tv;
  gettimeofday(&tv , NULL);
  
  unsigned long long total_time = (unsigned long long)tv.tv_sec * 1000 + tv.tv_usec/1000;
  return total_time;
}

i16 Receive_data(u32 CardID,i16 ErrorCode,EMD8216_Receive *Receive8216)
{
    int send_counter = 0;
    i16 error_code = 0;
    unsigned long long lngEndTime = 0;


    sockaddr_in m_saBroadClient;          // remote
    socklen_t ClientAddrSize;

    Receive8216->command = 0;

    m_saBroadClient.sin_family = AF_INET;
    m_saBroadClient.sin_addr.s_addr = inet_addr(dstIP[CardID]);
    m_saBroadClient.sin_port = htons(mu8SocketPort_Remote[CardID]);
    memset(m_saBroadClient.sin_zero, '\0', sizeof(m_saBroadClient.sin_zero));

    ClientAddrSize = sizeof(struct sockaddr);

    do
    {
        error_code = 0;

        /*int ret  =*/ sendto( m_pBroadSock[CardID], (char *)pmaudtEMD8216[CardID], Send_Size, 0, (struct sockaddr *) &m_saBroadClient, sizeof(struct sockaddr) );
        // 	printf("ret = %d\n",ret);

        lngEndTime = 0;
        lngEndTime = calculate_time() + cintTimeOut;
        // printf("timeout = %lld\n",lngEndTime);
	
        Receive8216->success_flag = 0;
        do
        {
            /*ret =*/ recvfrom( m_pBroadSock[CardID], (char *)Receive8216, Receive_Size, 0, (struct sockaddr *) &m_saBroadClient, &ClientAddrSize);
        }while( ((Receive8216->command != pmaudtEMD8216[CardID]->command)) && (calculate_time() < lngEndTime) );
        // printf("now = %lld\n\n",calculate_time());

        send_counter++;
    }while((Receive8216->command != pmaudtEMD8216[CardID]->command) && (send_counter < 2));

    if( Receive8216->command == pmaudtEMD8216[CardID]->command )
    {
        // printf("receive data,command fail\n");
        error_code = check_success_flag(Receive8216->success_flag,ErrorCode);
    }
    else
    {
//        printf("time out\n");
        error_code = TIME_OUT_ERROR;
    }
    return error_code;
}


/*------------  EMD8216 main function  -----------------*/
//--- Initialization & close 
EMD8216Status EMD8216_initial(u32 CardID,u8 IP_Address[4],u16 Host_Port,u16 Remote_port,u16 TimeOut_ms,u8 *CardType)
{
        int index = 0;
        i16 ErrorCode = 0;

        if ( CardID > CARD_ID_MAX)
        {
                return ID_ERROR;
        }
        if ( (TimeOut_ms < TIME_OUT_MIN) || (TimeOut_ms > TIME_OUT_MAX) )
        {
                return TIME_OUT_ERROR;
        }
        if ( socket_flag[CardID] )
        {
                return CREATE_SOCKET_ERROR;
        }
        if(EMD8216_init(CardID) != 0)
        {
                return INITIAL_SOCKET_ERROR;
        }

        //-- Socket setting
        sockaddr_in m_saBroadServer;        // local
        socklen_t ServerAddrSize = sizeof(struct sockaddr);

        socket_flag[CardID] = true;
        mblnUnlockFlag[CardID] = false;

        //-- declare & initialize structrue
        EMD8216_Receive Receive8216;
        char *clear_Receive8216 = (char *)&Receive8216;
        for ( index = 0; index < (int)sizeof(Receive8216); index ++ )
        {
            *(clear_Receive8216 + index) = 0;
        }

        //-- To judge whether the IP address is acceptable or not
        if ( 0 == IP_Address[0] )
        {
            return IP_ADDRESS_ERROR;
        }

        //-- Get Card Type
        cintTimeOut = TimeOut_ms;
        pmaudtEMD8216[CardID]->command = COMMAND_GET_CARD_TYPE;
        sprintf(dstIP[CardID],"%d.%d.%d.%d",IP_Address[0],IP_Address[1],IP_Address[2],IP_Address[3]);
        mu8SocketPort_Remote[CardID] = Remote_port;


        if( (m_pBroadSock[CardID] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 ) // Create UDP socket.
        {
            EMD8216_close(CardID);
            return CREATE_SOCKET_ERROR;
        }
        else
        {
            mu8SocketPort_Local[CardID] = Host_Port;

            // bzero(&m_saBroadServer, sizeof(m_saBroadServer));
            m_saBroadServer.sin_family = AF_INET;
            m_saBroadServer.sin_addr.s_addr = htons(INADDR_ANY);
            m_saBroadServer.sin_port = htons(mu8SocketPort_Local[CardID]);
            memset(m_saBroadServer.sin_zero, '\0', sizeof(m_saBroadServer.sin_zero));

            // bind address and port to socket
            if( bind( m_pBroadSock[CardID], (struct sockaddr *)&m_saBroadServer, ServerAddrSize ) == -1 )
            {
                EMD8216_close(CardID);
                return CREATE_SOCKET_ERROR;
            }
	    
            // Setting Non-Blocking mode.
            int flags = fcntl( m_pBroadSock[CardID], F_GETFL, 0 );
            if ( fcntl( m_pBroadSock[CardID], F_SETFL, flags|O_NONBLOCK ) == -1 )
            {
//                printf("Error: Set CardID %ld NonBlocking!\n", CardID);
            }
        }

        ErrorCode = Receive_data(CardID,IP_ADDRESS_ERROR,&Receive8216);
//	printf("Error: ErrorCode = %d\n", ErrorCode);
        if ( 0 > ErrorCode )
        {
            EMD8216_close(CardID);
            return ErrorCode;
        }

    return NO_ERROR;
}

EMD8216Status EMD8216_close(u32 CardID)
{
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }

    delete pmaudtEMD8216[CardID];
//    printf("EMD8216_close\n");

    mblnUnlockFlag[CardID] = false;

    close(m_pBroadSock[CardID]);
    socket_flag[CardID] = false;

    return NO_ERROR;
}

EMD8216Status EMD8216_firmware_version_read(u32 CardID,u8 Version[2])
{
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    EMD8216_Receive Receive8216;

//    printf("EMD8216_read_version\n");

    pmaudtEMD8216[CardID]->command = COMMAND_GET_FIRMWARE_VERSION;

    i16 ErrorCode = Receive_data(CardID,DEVICE_RW_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
            return ErrorCode;
    }

//    printf("EMD8216_read_port loop end\n");

    Version[0] = Receive8216.Version[0];
    Version[1] = Receive8216.Version[1];

    return NO_ERROR;
}


//--- Input / Output
EMD8216Status EMD8216_polarity_set(u32 CardID,u8 port,u8 data)
{
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if( port >= PHY_PORT_MAX )
    {
        return PORT_ERROR;
    }       

//    printf("EMD8216_set_port\n");

    pmaudtEMD8216[CardID]->command = COMMAND_POLARITY_SET;
    pmaudtEMD8216[CardID]->Port_value[0] = port;
    pmaudtEMD8216[CardID]->Port_value[1] = data;

    EMD8216_Receive Receive8216;

    i16 ErrorCode = Receive_data(CardID,DEVICE_RW_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_polarity_read(u32 CardID,u8 port,u8 *data)
{
    if( CardID > CARD_ID_MAX )
    {
        return ID_ERROR;
    }
    if( port >= PHY_PORT_MAX )
    {
        return PORT_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if( !mblnUnlockFlag[CardID] )
    {
        return	LOCK_COUNTER_ERROR;
    }

    EMD8216_Receive Receive8216;

//    printf("EMD8216_read_port\n");

    pmaudtEMD8216[CardID]->command = COMMAND_POLARITY_READ;
    pmaudtEMD8216[CardID]->Port_value[0] = port;

    i16 ErrorCode = Receive_data(CardID,DEVICE_RW_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

//    printf("EMD8216_read_port loop end\n");

    *data = Receive8216.Port_value[1];

    return NO_ERROR;
}

EMD8216Status EMD8216_port_set(u32 CardID,u8 port,u8 data)
{
    if( CardID > CARD_ID_MAX )
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if( port >= VIR_PORT_MAX )
    {
        return PORT_ERROR;
    }

//    printf("EMD8216_set_port\n");

    pmaudtEMD8216[CardID]->command = COMMAND_PORT_SET;
    pmaudtEMD8216[CardID]->Port_value[0] = port;
    pmaudtEMD8216[CardID]->Port_value[1] = data;

    EMD8216_Receive Receive8216;

    i16 ErrorCode = Receive_data(CardID,PORT_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_port_read(u32 CardID,u8 port,u8 *data)
{
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if( port >= VIR_PORT_MAX )
    {
        return PORT_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    EMD8216_Receive Receive8216;

//    printf("EMD8216_port_read\n");

    pmaudtEMD8216[CardID]->command=COMMAND_PORT_READ;
    pmaudtEMD8216[CardID]->Port_value[0] = port;

    i16 ErrorCode = Receive_data(CardID,DEVICE_RW_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    *data = Receive8216.Port_value[1];

    return NO_ERROR;
}

EMD8216Status EMD8216_port_config_set(u32 CardID ,u8 port, u8 config)
{
//    printf("EMD8216_set_port");

    if( CardID > CARD_ID_MAX )
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if( !mblnUnlockFlag[CardID] )
    {
        return	LOCK_COUNTER_ERROR;
    }
    if( port >= PHY_PORT_MAX )
    {
        return PORT_ERROR;
    }

    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_PORT_CONFIG_SET;
    pmaudtEMD8216[CardID]->Port_value[0] = port;
    pmaudtEMD8216[CardID]->Port_value[1] = config;


    i16 ErrorCode=Receive_data(CardID,PORT_ERROR,&Receive8216);

    if( 0 > ErrorCode )
    {
            return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_port_config_read(u32 CardID ,u8 port, u8 *config)
{
//    printf("EMD8216_set_port");

    if( CardID > CARD_ID_MAX )
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if( !mblnUnlockFlag[CardID] )
    {
        return	LOCK_COUNTER_ERROR;
    }
    if( port >= PHY_PORT_MAX )
    {
        return PORT_ERROR;
    }

    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_PORT_CONFIG_READ;
    pmaudtEMD8216[CardID]->Port_value[0] = port;

    i16 ErrorCode = Receive_data(CardID,PORT_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
            return ErrorCode;
    }
    *config = Receive8216.Port_value[1];

    return NO_ERROR;
}

EMD8216Status EMD8216_point_polarity_set(u32 CardID,u8 port,u8 point,u8 data)
{
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if(port >= PHY_PORT_MAX)
    {
        return PORT_ERROR;
    }
    if(point >= POINT_MAX)
    {
        return PARAMETERS_ERROR;
    }

//    printf("EMD8216_point_polarity_set");

    pmaudtEMD8216[CardID]->command=COMMAND_POINT_POLARITY_SET;
    pmaudtEMD8216[CardID]->Point_value[0] = port;
    pmaudtEMD8216[CardID]->Point_value[1] = point;
    pmaudtEMD8216[CardID]->Point_value[2] = data;

    EMD8216_Receive Receive8216;

    i16 ErrorCode = Receive_data(CardID,DEVICE_RW_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_point_polarity_read(u32 CardID,u8 port,u8 point,u8 *data)
{
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if( port >= PHY_PORT_MAX )
    {
        return PORT_ERROR;
    }
    if(point >= POINT_MAX)
    {
        return PARAMETERS_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    EMD8216_Receive Receive8216;

//    printf("EMD8216_read_port\n");

    pmaudtEMD8216[CardID]->command = COMMAND_POINT_POLARITY_READ;
    pmaudtEMD8216[CardID]->Point_value[0] = port;
    pmaudtEMD8216[CardID]->Point_value[1] = point;

    i16 ErrorCode = Receive_data(CardID,DEVICE_RW_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

//    printf("EMD8216_read_port loop end\n");

    *data = Receive8216.Point_value[2];

    return NO_ERROR;
}

EMD8216Status EMD8216_point_set(u32 CardID, u8 port, u8 point, u8 state)
{
//    printf("EMD8216_set_port");
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if( port >= VIR_PORT_MAX )
    {
        return PORT_ERROR;
    }
    if(state > 1)
    {
        return	PARAMETERS_ERROR;
    }
    if(point & 0xe8)
    {
        return	PARAMETERS_ERROR;
    }

    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_POINT_SET;
    pmaudtEMD8216[CardID]->Point_value[0] = port;
    pmaudtEMD8216[CardID]->Point_value[1] = point;
    pmaudtEMD8216[CardID]->Point_value[2] = state;

    i16 ErrorCode = Receive_data(CardID,PORT_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_point_read(u32 CardID,u8 port,u8 point,u8 *state)
{
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if( point >= POINT_MAX )
    {
        return PARAMETERS_ERROR;
    }
    if( port >= VIR_PORT_MAX )
    {
        return PORT_ERROR;
    }
    if(point >= POINT_MAX)
    {
        return PARAMETERS_ERROR;
    }

//    printf("EMD8216_point_read\n");
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command=COMMAND_POINT_READ;
    pmaudtEMD8216[CardID]->Point_value[0] = port;
    pmaudtEMD8216[CardID]->Point_value[1] = point;

    i16 ErrorCode=Receive_data(CardID,PORT_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }
//    printf("EMD8216_read_port loop end\n");

    *state=Receive8216.Point_value[2];

    return NO_ERROR;
}

EMD8216Status EMD8216_point_config_set(u32 CardID, u8 port, u8 point, u8 config)
{
//    printf("EMD8216_set_port\n");

    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if(port >= PHY_PORT_MAX)
    {
        return PORT_ERROR;
    }
    if(point >= POINT_MAX)
    {
        return PARAMETERS_ERROR;
    }

    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_POINT_CONFIG_SET;
    pmaudtEMD8216[CardID]->Point_value[0] = port;
    pmaudtEMD8216[CardID]->Point_value[1] = point;
    pmaudtEMD8216[CardID]->Point_value[2] = config;


    i16 ErrorCode = Receive_data(CardID,PORT_ERROR,&Receive8216);

    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_point_config_read(u32 CardID, u8 port, u8 point, u8 *config)
{
//    printf("EMD8216_set_port\n");

    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if(port >= PHY_PORT_MAX)
    {
        return PORT_ERROR;
    }
    if(point >= POINT_MAX)
    {
        return PARAMETERS_ERROR;
    }

    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_POINT_CONFIG_READ;
    pmaudtEMD8216[CardID]->Point_value[0] = port;
    pmaudtEMD8216[CardID]->Point_value[1] = point;

    i16 ErrorCode = Receive_data(CardID,PORT_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    *config = Receive8216.Point_value[2];

    return NO_ERROR;
}

//--- Counter 
EMD8216Status EMD8216_counter_mask_set(u32 CardID,u8 port,u8 channel)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if(port >= PHY_PORT_MAX)
    {
        return	PORT_ERROR;
    }

    EMD8216_Receive Receive8216;
    i16 ErrorCode;

    pmaudtEMD8216[CardID]->command = COMMAND_SET_COUNTER_MASK;
    pmaudtEMD8216[CardID]->Port_value[0] = port;
    pmaudtEMD8216[CardID]->Port_value[1] = channel;

    ErrorCode = Receive_data(CardID, MASK_CHANNEL_ERROR, &Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_counter_enable(u32 CardID)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    i16 ErrorCode;
    EMD8216_Receive Receive8216;
    pmaudtEMD8216[CardID]->command = COMMAND_ENABLE_COUNTER_MODE;

    ErrorCode = Receive_data(CardID,COUNTER_ENABLE_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_counter_disable(u32 CardID)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    i16 ErrorCode;
    EMD8216_Receive Receive8216;
    pmaudtEMD8216[CardID]->command = COMMAND_DISABLE_COUNTER_MODE;

    ErrorCode = Receive_data(CardID,COUNTER_DISABLE_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_counter_read(u32 CardID,u8 port,u32 counter[8])
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if(port >= PHY_PORT_MAX)
    {
        return	PORT_ERROR;
    }

    i16 ErrorCode;
    u32 *temp;
    int i;
    EMD8216_Receive Receive8216;
    pmaudtEMD8216[CardID]->command = COMMAND_READ_COUNTER;
    pmaudtEMD8216[CardID]->Port_value[0] = port;

    ErrorCode = Receive_data(CardID,COUNTER_READ_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    for( i = 0; i < POINT_MAX; i++ )
    {
        temp = (u32 *)(&Receive8216.counter[i][0]);
        counter[i]=*temp;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_counter_clear(u32 CardID,u8 port,u8 channel)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if( channel >= POINT_MAX )
    {
        return	IN_POINT_ERROR;
    }
    if( port >= PHY_PORT_MAX )
    {
        return	PORT_ERROR;
    }

    i16 ErrorCode;
    EMD8216_Receive Receive8216;
    pmaudtEMD8216[CardID]->command = COMMAND_CLEAR_COUNTER;
    pmaudtEMD8216[CardID]->Port_value[0] = port;
    pmaudtEMD8216[CardID]->Port_value[1] = channel;

    ErrorCode = Receive_data(CardID,COUNTER_CLEAR_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    return NO_ERROR;
}


//--- Miscellaneous
EMD8216Status EMD8216_socket_port_change(u32 CardID,u16 Remote_port)
{
    EMD8216_Receive Receive8216;
//    printf("EMD8216_change_socket_port\n");

    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if(Remote_port < REMOTE_PORT_MIN)
    {
        return PARAMETERS_ERROR;
    }

    //Change socket number
    pmaudtEMD8216[CardID]->command = COMMAND_CHANGESOCKETPORT;
    pmaudtEMD8216[CardID]->socket_port = Remote_port;

    i16 ErrorCode = Receive_data(CardID,CHANGE_SOCKET_ERROR,&Receive8216);
    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    EMD8216_close(CardID);

    return NO_ERROR;
}

EMD8216Status EMD8216_IP_change(u32 CardID,u8 IP[4])
{
    u8 boardcase = 0xff;
    u8 domain = 0x0;
    //Reboot EMD8216 to refresh new socket
    int i;
    EMD8216_Receive Receive8216;
    i16 ErrorCode;
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if(IP[3] == 0 || IP[3] == 255)
    {
        return	IP_ADDRESS_ERROR;
    }

    pmaudtEMD8216[CardID]->command = COMMAND_CHANGEIP;
    for( i = 0; i < 4; i++)
    {
        pmaudtEMD8216[CardID]->IP[i] = IP[i];
        boardcase = boardcase & IP[i];
        domain = domain | IP[i];
    }

    if((0xff == boardcase) || (0x0 == domain))
    {
        ErrorCode = CHANGEIP_ERROR;
    }
    else
    {
        ErrorCode=Receive_data(CardID,REBOOT_ERROR,&Receive8216);
    }

    if( 0 > ErrorCode )
    {
        return ErrorCode;
    }

    EMD8216_close(CardID);
    return NO_ERROR;
}

EMD8216Status EMD8216_reboot(u32 CardID)
{
    //Reboot EMD8216 to refresh new socket
    EMD8216_Receive Receive8216;
    i16 ErrorCode;
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    pmaudtEMD8216[CardID]->command = COMMAND_REBOOT;
    ErrorCode = Receive_data(CardID,REBOOT_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        if(ErrorCode == PASSWORD_ERROR)
        {
            return REBOOT_ERROR;
        }
        else
        {
            return ErrorCode;
        }
    }

    EMD8216_close(CardID);
    return NO_ERROR;
}


//--- software key
EMD8216Status EMD8216_security_unlock(u32 CardID,u8 password[8])
{
    int i = 0;
    EMD8216_Receive Receive8216;
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if( false == socket_flag[CardID] )
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }

    for( i = 0; i < 8; i++ )
    {
        pmaudtEMD8216[CardID]->password[i] = password[i];	// to ascii
    }

    pmaudtEMD8216[CardID]->command = COMMAND_POLARITY_READ;
    pmaudtEMD8216[CardID]->Port_value[0] = 0x0;

    i16 ErrorCode = Receive_data(CardID,UNLOCK_SECURITY_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    mblnUnlockFlag[CardID] = true;

    return NO_ERROR;
}

EMD8216Status EMD8216_security_status_read(u32 CardID,u8 *lock_status)
{
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }

    *lock_status = !mblnUnlockFlag[CardID];		// The variable mblnUnlockFlag[CardID]=true means unlock, so the lock_status equals to inverse mblnUnlockFlag[CardID]

//    printf("EMD8216_read_security_status\n");
    return NO_ERROR;
}

EMD8216Status EMD8216_password_change(u32 CardID,u8 Oldpassword[8],u8 Password[8])
{
    int i = 0 ;
    EMD8216_Receive Receive8216;
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }    
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        for (i=0;i<Password_MAX;i++)
        {
            pmaudtEMD8216[CardID]->password[i] = Oldpassword[i];
        }
    }
    else
    {
        for (i=0;i<Password_MAX && (pmaudtEMD8216[CardID]->password[i] == Oldpassword[i]);i++);
        if(i!=Password_MAX)
        {
            return PASSWORD_ERROR;
        }
    }
    for (i=0;i<Password_MAX;i++)
    {
        pmaudtEMD8216[CardID]->new_password[i] = Password[i];
    }

    pmaudtEMD8216[CardID]->command=COMMAND_CHANGE_PASSWORD;

    i16 ErrorCode=Receive_data(CardID,PASSWORD_ERROR,&Receive8216);
    if( 0 == ErrorCode )
    {
        //return ErrorCode;
        EMD8216_close(CardID);

//        printf("EMD8216_change_password\n");
    }

    return ErrorCode;
}

EMD8216Status EMD8216_password_set_default(u32 CardID)
{
    EMD8216_Receive Receive8216;

    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }

    pmaudtEMD8216[CardID]->command = COMMAND_RESTORE_PASSWORD;

    i16 ErrorCode = Receive_data(CardID,SET_SECURITY_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    EMD8216_close(CardID);

    return NO_ERROR;
}

EMD8216Status EMD8216_write_mac(u32 CardID,u8 mac[6])
{
    int i;
    EMD8216_Receive Receive8216;
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }

    pmaudtEMD8216[CardID]->command = COMMAND_WRITE_MAC;
    for(i=0;i<6;i++)
    {
        pmaudtEMD8216[CardID]->MAC[i] = mac[i];
    }
    i16 ErrorCode = Receive_data(CardID,PASSWORD_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    return NO_ERROR;
}


//--- WDT
EMD8216Status EMD8216_WDT_enable(u32 CardID)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    i16 ErrorCode;
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_WDT_ENABLE;

    ErrorCode = Receive_data(CardID,PASSWORD_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_WDT_disable(u32 CardID)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    i16 ErrorCode;
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_WDT_DISABLE;

    ErrorCode=Receive_data(CardID,PASSWORD_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_WDT_set(u32 CardID,u16 time,u8 state[2])
{
    if(CardID > CARD_ID_MAX)
    {
        return ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return LOCK_COUNTER_ERROR;
    }
    if((time < WDT_TIME_OUT_MIN) || (time > WDT_TIME_OUT_MAX))
    {
        return	TIME_ERROR;
    }

    i16 ErrorCode;
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_WDT_SET;
    pmaudtEMD8216[CardID]->WDT.output[0] = state[0];
    pmaudtEMD8216[CardID]->WDT.output[1] = state[1];
    pmaudtEMD8216[CardID]->WDT.Timer_value = time;

    ErrorCode = Receive_data(CardID,PASSWORD_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_WDT_read(u32 CardID,u16 *time,u8 state[2],u8 *enable)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    i16 ErrorCode;

    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_WDT_READ;

    ErrorCode = Receive_data(CardID,PASSWORD_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    state[0] = Receive8216.WDT.output[0];
    state[1] = Receive8216.WDT.output[1];

    *time = Receive8216.WDT.Timer_value;
    *enable = Receive8216.WDT.state;

    return NO_ERROR;
}


//--- standalone
EMD8216Status EMD8216_standalone_enable(u32 CardID)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    i16 ErrorCode;
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_STANDALONE_ENABLE;
    ErrorCode = Receive_data(CardID,STANDALONE_ENABLE_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_standalone_disable(u32 CardID)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    i16 ErrorCode;
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_STANDALONE_DISABLE;
    ErrorCode=Receive_data(CardID,STANDALONE_DISABLE_ERROR,&Receive8216);
    if(ErrorCode < 0)
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_standalone_config_set(u32 CardID,u8 function_number,_StandaloneData data[32],u8 standalone_state)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    i16 ErrorCode;
    int number,index;
    EMD8216_Receive Receive8216;
    
    number = function_number;

    if( 0 == number )
    {
        return	PARAMETERS_ERROR;
    }

    pmaudtEMD8216[CardID]->command = COMMAND_STANDALONE_CONFIG_SET;
    pmaudtEMD8216[CardID]->standalone_data.standalone_state = standalone_state;

    pmaudtEMD8216[CardID]->standalone_data.function_number = number;
    for(index=0; index<number; index+=2)
    {
        pmaudtEMD8216[CardID]->standalone_data.function_index = index;
        pmaudtEMD8216[CardID]->standalone_data.input_point[0]  = data[index].in_point_bit;
        pmaudtEMD8216[CardID]->standalone_data.input_state[0]  = data[index].in_state_bit;
        pmaudtEMD8216[CardID]->standalone_data.out_mode[0]     = data[index].out_mode;
        pmaudtEMD8216[CardID]->standalone_data.output_point[0] = data[index].out_point_bit;
        pmaudtEMD8216[CardID]->standalone_data.timer_mode[0]   = data[index].timer_mode;
        pmaudtEMD8216[CardID]->standalone_data.timer_value[0]  = data[index].timer_value;

        pmaudtEMD8216[CardID]->standalone_data.input_point[1]  = data[index+1].in_point_bit;
        pmaudtEMD8216[CardID]->standalone_data.input_state[1]  = data[index+1].in_state_bit;
        pmaudtEMD8216[CardID]->standalone_data.out_mode[1]     = data[index+1].out_mode;
        pmaudtEMD8216[CardID]->standalone_data.output_point[1] = data[index+1].out_point_bit;
        pmaudtEMD8216[CardID]->standalone_data.timer_mode[1]   = data[index+1].timer_mode;
        pmaudtEMD8216[CardID]->standalone_data.timer_value[1]  = data[index+1].timer_value;

        ErrorCode=Receive_data(CardID,STANDALONE_CONFIG_ERROR,&Receive8216);

        if(ErrorCode < 0)
        {
            return ErrorCode;
        }
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_standalone_config_read(u32 CardID,u8 *function_number,_StandaloneData data[32],u8 *enable,u8 *power_on_enable)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }

    i16 ErrorCode;
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_STANDALONE_CONFIG_READ;

    int index;
    int total_function_num = 32;

    for(index = 0; index < total_function_num ; index += 2)
    {
        pmaudtEMD8216[CardID]->standalone_data.function_index = index;
        ErrorCode=Receive_data(CardID,STANDALONE_CONFIG_ERROR,&Receive8216);
        if(ErrorCode < 0)
        {
            return ErrorCode;
        }

        if( index == 0 )
        {
	  total_function_num = Receive8216.standalone_data.function_number;
	  *function_number = total_function_num;
	  *enable = Receive8216.standalone_data.standalone_state & 0x01;
	  *power_on_enable = (Receive8216.standalone_data.standalone_state & 0x02)>>1;	  
	}
        data[index].in_point_bit  = Receive8216.standalone_data.input_point[0];
        data[index].in_state_bit  = Receive8216.standalone_data.input_state[0];
        data[index].out_mode      = Receive8216.standalone_data.out_mode[0];
        data[index].out_point_bit = Receive8216.standalone_data.output_point[0];
        data[index].timer_mode    = Receive8216.standalone_data.timer_mode[0];
        data[index].timer_value   = Receive8216.standalone_data.timer_value[0];

        data[index+1].in_point_bit  = Receive8216.standalone_data.input_point[1];
        data[index+1].in_state_bit  = Receive8216.standalone_data.input_state[1];
        data[index+1].out_mode      = Receive8216.standalone_data.out_mode[1];
        data[index+1].out_point_bit = Receive8216.standalone_data.output_point[1];
        data[index+1].timer_mode    = Receive8216.standalone_data.timer_mode[1];
        data[index+1].timer_value   = Receive8216.standalone_data.timer_value[1];
    }
    
    return NO_ERROR;
}

EMD8216Status EMD8216_standalone_V_config_set(u32 CardID,u8 number,_V_StandaloneData data[32],u8 standalone_state)
{
    if(CardID > CARD_ID_MAX)
    {
        return	ID_ERROR;
    }
    if(!socket_flag[CardID])
    {
        return INITIAL_SOCKET_ERROR;
    }
    if(!mblnUnlockFlag[CardID])
    {
        return	LOCK_COUNTER_ERROR;
    }
    if(standalone_state > 1)
    {
        return	PARAMETERS_ERROR;
    }
    if( (number > STANDALONE_FUNCTION_MAX)
     || (number == 0) )
    {
        return PARAMETERS_ERROR;
    }

    u16 ErrorCode;
    int index;
    EMD8216_Receive Receive8216;
    _u_type u_temp;

    pmaudtEMD8216[CardID]->command = COMMAND_STANDALONE_CONFIG_SET_NEW;
    pmaudtEMD8216[CardID]->new_st_data.power_on_state = standalone_state;
    pmaudtEMD8216[CardID]->new_st_data.function_number = number;
    for(index=0; index<number; index++)
    {
        pmaudtEMD8216[CardID]->new_st_data.function_index = index;
        pmaudtEMD8216[CardID]->new_st_data.config_data.in_point[0] = data[index].in_point_bit[0];
        pmaudtEMD8216[CardID]->new_st_data.config_data.in_point[1] = data[index].in_point_bit[1];
        pmaudtEMD8216[CardID]->new_st_data.config_data.in_state[0] = data[index].in_state_bit[0];
        pmaudtEMD8216[CardID]->new_st_data.config_data.in_state[1] = data[index].in_state_bit[1];
        pmaudtEMD8216[CardID]->new_st_data.config_data.control_mode = data[index].control_mode;

        u_temp.u32_data = data[index].timer_value;
        pmaudtEMD8216[CardID]->new_st_data.config_data.timer_value[0] = u_temp.u8_data[0];
        pmaudtEMD8216[CardID]->new_st_data.config_data.timer_value[1] = u_temp.u8_data[1];
        pmaudtEMD8216[CardID]->new_st_data.config_data.timer_value[2] = u_temp.u8_data[2];
        pmaudtEMD8216[CardID]->new_st_data.config_data.timer_value[3] = u_temp.u8_data[3];

        pmaudtEMD8216[CardID]->new_st_data.config_data.in_virtual_point[0] = data[index].in_virtual_point_bit[0];
        pmaudtEMD8216[CardID]->new_st_data.config_data.in_virtual_point[1] = data[index].in_virtual_point_bit[1];
        pmaudtEMD8216[CardID]->new_st_data.config_data.in_virtual_state[0] = data[index].in_virtual_state_bit[0];
        pmaudtEMD8216[CardID]->new_st_data.config_data.in_virtual_state[1] = data[index].in_virtual_state_bit[1];
        pmaudtEMD8216[CardID]->new_st_data.config_data.output_point[0]			= data[index].out_point_bit[0];
        pmaudtEMD8216[CardID]->new_st_data.config_data.output_point[1]			= data[index].out_point_bit[1];
        pmaudtEMD8216[CardID]->new_st_data.config_data.out_virtual_point[0]		= data[index].out_virtual_point_bit[0];
        pmaudtEMD8216[CardID]->new_st_data.config_data.out_virtual_point[1]		= data[index].out_virtual_point_bit[1];
        pmaudtEMD8216[CardID]->new_st_data.config_data.out_mode					= data[index].out_mode;


        pmaudtEMD8216[CardID]->new_st_data.config_data.disconect_mask[0]		= data[index].disconnect_mask[0];
        pmaudtEMD8216[CardID]->new_st_data.config_data.disconect_mask[1]		= data[index].disconnect_mask[1];
        pmaudtEMD8216[CardID]->new_st_data.config_data.disconect_mask[2]		= data[index].disconnect_mask[2];
        pmaudtEMD8216[CardID]->new_st_data.config_data.disconect_mask[3]		= data[index].disconnect_mask[3];
        ErrorCode=Receive_data(CardID,STANDALONE_CONFIG_ERROR,&Receive8216);
        if(ErrorCode>0)
        {
            return ErrorCode;
        }
        //_V_StandaloneData
        //delete Receive8216;
        //destory Receive8216;
    }

    return NO_ERROR;
}


EMD8216Status EMD8216_standalone_V_config_read(u32 CardID,u8 *number,_V_StandaloneData data[32],u8 *enable,u8 *standalone_state)
{
    if(CardID > CARD_ID_MAX)													return	ID_ERROR;
    if(!socket_flag[CardID])													return ID_ERROR;
    if(!mblnUnlockFlag[CardID])													return	UNLOCK_ERROR;

    u16 ErrorCode;
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_STANDALONE_CONFIG_READ_NEW;

    _u_type u_temp;
    int index;
    int ct=32;

    for(index=0; index<STANDALONE_FUNCTION_MAX; index++)
    {
        if(index<ct)
        {
            pmaudtEMD8216[CardID]->standalone_data.function_index = index;
            ErrorCode=Receive_data(CardID,STANDALONE_CONFIG_ERROR,&Receive8216);
            if(ErrorCode>0)
            {
                return ErrorCode;
            }
            if(index==0)
            {
                ct = Receive8216.new_st_data.function_number;
                *enable = Receive8216.new_st_data.standalone_state & 0x01;
                *standalone_state = Receive8216.new_st_data.power_on_state;
            }
            data[index].in_point_bit[0]		= Receive8216.new_st_data.config_data.in_point[0];
            data[index].in_point_bit[1]		= Receive8216.new_st_data.config_data.in_point[1];
            data[index].in_state_bit[0]		= Receive8216.new_st_data.config_data.in_state[0];
            data[index].in_state_bit[1]		= Receive8216.new_st_data.config_data.in_state[1];
            data[index].out_mode			= Receive8216.new_st_data.config_data.out_mode;
            data[index].out_point_bit[0]	= Receive8216.new_st_data.config_data.output_point[0];
            data[index].out_point_bit[1]	= Receive8216.new_st_data.config_data.output_point[1];
            data[index].control_mode		= Receive8216.new_st_data.config_data.control_mode;


            //data[index].timer_value			= Receive8216.config_data.timer_value;
            //u_temp.u32_data = data[index].timer_value;
            u_temp.u8_data[0]=Receive8216.new_st_data.config_data.timer_value[0];
            u_temp.u8_data[1]=Receive8216.new_st_data.config_data.timer_value[1];
            u_temp.u8_data[2]=Receive8216.new_st_data.config_data.timer_value[2];
            u_temp.u8_data[3]=Receive8216.new_st_data.config_data.timer_value[3];
            data[index].timer_value			= u_temp.u32_data;

            data[index].in_virtual_point_bit[0] = Receive8216.new_st_data.config_data.in_virtual_point[0];
            data[index].in_virtual_point_bit[1] = Receive8216.new_st_data.config_data.in_virtual_point[1];
            data[index].in_virtual_state_bit[0] = Receive8216.new_st_data.config_data.in_virtual_state[0];
            data[index].in_virtual_state_bit[1] = Receive8216.new_st_data.config_data.in_virtual_state[1];
            data[index].out_virtual_point_bit[0] = Receive8216.new_st_data.config_data.out_virtual_point[0];
            data[index].out_virtual_point_bit[1] = Receive8216.new_st_data.config_data.out_virtual_point[1];

            data[index].disconnect_mask[0]		= Receive8216.new_st_data.config_data.disconect_mask[0];
            data[index].disconnect_mask[1]		= Receive8216.new_st_data.config_data.disconect_mask[1];
            data[index].disconnect_mask[2]		= Receive8216.new_st_data.config_data.disconect_mask[2];
            data[index].disconnect_mask[3]		= Receive8216.new_st_data.config_data.disconect_mask[3];
        }
        else
        {
            data[index].in_point_bit[0]	= 0;
            data[index].in_point_bit[1]	= 0;
            data[index].in_state_bit[0]	= 0;
            data[index].in_state_bit[1]	= 0;
            data[index].out_mode		= 0;
            data[index].out_point_bit[0]	= 0;
            data[index].out_point_bit[1]	= 0;
            data[index].control_mode		= 0;
            data[index].timer_value		= 0;
            data[index].in_virtual_point_bit[0] = 0;
            data[index].in_virtual_point_bit[1] = 0;
            data[index].in_virtual_state_bit[0] = 0;
            data[index].in_virtual_state_bit[1] = 0;
            data[index].out_virtual_point_bit[0] = 0;
            data[index].out_virtual_point_bit[1] = 0;

            data[index].disconnect_mask[0]		= 0;
            data[index].disconnect_mask[1]		= 0;
            data[index].disconnect_mask[2]		= 0;
            data[index].disconnect_mask[3]		= 0;
        }
    }
    *number=ct;
    return NO_ERROR;
}

EMD8216Status EMD8216_standalone_config_clear(u32 CardID)
{
    if(CardID > CARD_ID_MAX)													return	ID_ERROR;
    if(!socket_flag[CardID])													return ID_ERROR;
    if(!mblnUnlockFlag[CardID])													return	UNLOCK_ERROR;


    u16 ErrorCode;
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command = COMMAND_STANDALONE_CONFIG_CLEAR;
    ErrorCode=Receive_data(CardID,STANDALONE_ENABLE_ERROR,&Receive8216);
    if(ErrorCode>0)
    {
        return ErrorCode;
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_host_config_set(u32 CardID, u8 host_ID, u8 IP_address[4], u8 password[8], u16 host_port, u8 card_type)
{
    if(CardID > CARD_ID_MAX)													return	ID_ERROR;
    if(!socket_flag[CardID])													return ID_ERROR;
    if(!mblnUnlockFlag[CardID])													return	UNLOCK_ERROR;

    u32 error_code;
    EMD8216_Receive Receive8216;

    int i;

    pmaudtEMD8216[CardID]->command = COMMAND_HOST_CONFIG_SET;
    pmaudtEMD8216[CardID]->host_config.index = host_ID;
    pmaudtEMD8216[CardID]->host_config.host_port = host_port;
    pmaudtEMD8216[CardID]->host_config.card_type = card_type;
    for(i=0;i<4;i++)
    {
        pmaudtEMD8216[CardID]->host_config.IP[i] = IP_address[i];
    }
    for(i=0;i<8;i++)
    {
        pmaudtEMD8216[CardID]->host_config.password[i] = password[i];
    }

    error_code = Receive_data(CardID,PARAMETERS_ERROR,&Receive8216);
    if(error_code != 0)
    {
        return error_code;
    }
    //OutputDebugString("EMA8314 set MAC\n");
    return NO_ERROR;
}

EMD8216Status EMD8216_host_config_read(u32 CardID, u8 host_ID, u8 IP_address[4], u16 *host_port, u8 *card_type)
{
    if(CardID > CARD_ID_MAX)													return	ID_ERROR;
    if(!socket_flag[CardID])													return ID_ERROR;
    if(!mblnUnlockFlag[CardID])													return	UNLOCK_ERROR;

    u32 error_code;
    EMD8216_Receive Receive8216;
    int i;

    pmaudtEMD8216[CardID]->command = COMMAND_HOST_CONFIG_READ;

    pmaudtEMD8216[CardID]->host_config.index = host_ID;

    error_code = Receive_data(CardID,PARAMETERS_ERROR,&Receive8216);
    if(error_code != 0)
    {
        return error_code;
    }
    //OutputDebugString("EMA8314 set MAC\n");

    *host_port = Receive8216.host_config.host_port;
    *card_type = Receive8216.host_config.card_type;
    for(i=0;i<4;i++)
    {
        IP_address[i] = Receive8216.host_config.IP[i];
    }

    return NO_ERROR;
}


EMD8216Status EMD8216_host_connection_clear(u32 CardID, u8 state[4])
{
    if(CardID > CARD_ID_MAX)													return	ID_ERROR;
    if(!socket_flag[CardID])													return ID_ERROR;
    if(!mblnUnlockFlag[CardID])													return	UNLOCK_ERROR;

    u32 error_code;
    EMD8216_Receive Receive8216;

    int i;

    pmaudtEMD8216[CardID]->command = COMMAND_HOST_CONNECTION_CLEAR;

    for(i=0;i<4;i++)
    {
        pmaudtEMD8216[CardID]->host_state.connection_state[i] = state[i];
    }

    error_code = Receive_data(CardID,PARAMETERS_ERROR,&Receive8216);
    if(error_code != 0)
    {
        return error_code;
    }
    //OutputDebugString("EMA8314 set MAC\n");
    return NO_ERROR;
}


EMD8216Status EMD8216_host_connection_read(u32 CardID, u8 state[4], u8 error_state[4])
{
    if(CardID > CARD_ID_MAX)													return	ID_ERROR;
    if(!socket_flag[CardID])													return ID_ERROR;
    if(!mblnUnlockFlag[CardID])													return	UNLOCK_ERROR;

    u32 error_code;
    EMD8216_Receive Receive8216;

    int i;

    pmaudtEMD8216[CardID]->command = COMMAND_HOST_CONNECTION_READ;


    error_code = Receive_data(CardID,PARAMETERS_ERROR,&Receive8216);
    if(error_code != 0)
    {
        return error_code;
    }
    //OutputDebugString("EMA8314 set MAC\n");
    for(i=0;i<4;i++)
    {
        state[i] = Receive8216.host_state.connection_state[i];
        switch(state[i])
        {
        case _UNUSED:
            state[i] = _UNUSED;
            break;
        case _CONNECTED:
            //state[i] = _
            state[i] = _CONNECT;
            break;
        case _RE_CONNECTION:
           state[i] = _CONNECT;
            break;
        case _DISCONNECTION:
           state[i] = _BREAK;
            break;
        case _WAIT_RESPOND:
           state[i] = _BREAK;
            break;
        case _WAIT_TO_SEND:
           state[i] = _CONNECT;
            break;
        case _SEND_TO_HOST:
           state[i] = _CONNECT;
            break;

        }
        error_state[i] = Receive8216.host_state.connection_ERROR_code[i];
    }

    return NO_ERROR;
}

EMD8216Status EMD8216_subnet_mask_set(u32 CardID,u8 subnet_mask[4])
{
    u32 error_code = NO_ERROR;

    if(CardID > CARD_ID_MAX)					return	ID_ERROR;
    if(!socket_flag[CardID])					return  ID_ERROR;
    if(!mblnUnlockFlag[CardID])					return	UNLOCK_ERROR;


    u8 i;
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command=COMMAND_CHANGE_SUBNET_MASK;
    for(i=0;i<4;i++)
    {
        pmaudtEMD8216[CardID]->subnet_mask[i] = subnet_mask[i];
    }


    u16 ErrorCode=Receive_data(CardID,PARAMETERS_ERROR,&Receive8216);

    if(ErrorCode>0)
    {
        error_code = (u32)ErrorCode;
    }
    else
    {
        EMD8216_close(CardID);
    }

    return error_code;
}

EMD8216Status EMD8216_subnet_mask_read(u32 CardID,u8 subnet_mask[4])
{
    u32 error_code = NO_ERROR;

    if(CardID > CARD_ID_MAX)					return	ID_ERROR;
    if(!socket_flag[CardID])													return ID_ERROR;
    if(!mblnUnlockFlag[CardID])													return	UNLOCK_ERROR;

    u8 i;
    EMD8216_Receive Receive8216;

    pmaudtEMD8216[CardID]->command=COMMAND_READ_SUBNET_MASK;



    u16 ErrorCode=Receive_data(CardID,PARAMETERS_ERROR,&Receive8216);

    for(i=0;i<4;i++)
    {
        subnet_mask[i] = Receive8216.subnet_mask[i];
    }
    if(ErrorCode>0)
    {
        error_code = (u32)ErrorCode;
    }

    return error_code;
}


//-------------------------------------------------------------------------------------------


#pragma once 

#include "mbed.h"
#include <stdarg.h>

#include "Pipe.h"
#include "SerialPipe.h"

#ifdef TARGET_UBLOX_C027
 #define MDM_IF(onboard,shield) onboard
#else
 #define MDM_IF(onboard,shield) shield
#endif

//! include debug capabilty on more powerful targets with a dedicated debug port 
#if defined(TARGET_LPC1768) || defined(TARGET_LPC4088) || defined(TARGET_K64F)
 #define MDM_DEBUG 
#endif 
 
/** basic modem parser class 
*/
class MDMParser
{
public:
    //! Constructor 
    MDMParser(void);
    //! get static instance
    static MDMParser* getInstance() { return inst; };
    
    // ----------------------------------------------------------------
    // Types 
    // ----------------------------------------------------------------
    //! MT Device Types 
    typedef enum { DEV_UNKNOWN, DEV_SARA_G350, DEV_LISA_U200, DEV_LISA_C200, 
                   DEV_SARA_U260, DEV_SARA_U270, DEV_LEON_G200 } Dev; 
    //! SIM Status
    typedef enum { SIM_UNKNOWN, SIM_MISSING, SIM_PIN, SIM_READY } Sim;
    //! SIM Status
    typedef enum { LPM_DISABLED, LPM_ENABLED, LPM_ACTIVE } Lpm; 
    //! Device status
    typedef struct { 
        Dev dev;            //!< Device Type
        Lpm lpm;            //!< Power Saving 
        Sim sim;            //!< SIM Card Status
        char ccid[20+1];    //!< Integrated Circuit Card ID
        char imsi[15+1];    //!< International Mobile Station Identity
        char imei[15+1];    //!< International Mobile Equipment Identity
        char meid[18+1];    //!< Mobile Equipment IDentifier
        char manu[16];      //!< Manufacturer (u-blox)
        char model[16];     //!< Model Name (LISA-U200, LISA-C200 or SARA-G350)
        char ver[16];       //!< Software Version
    } DevStatus;
    //! Registration Status
    typedef enum { REG_UNKNOWN, REG_DENIED, REG_NONE, REG_HOME, REG_ROAMING } Reg; 
    //! Access Technology
    typedef enum { ACT_UNKNOWN, ACT_GSM, ACT_EDGE, ACT_UTRAN, ACT_CDMA } AcT; 
    //! Network Status
    typedef struct { 
        Reg csd;        //!< CSD Registration Status (Circuit Switched Data)
        Reg psd;        //!< PSD Registration status (Packet Switched Data)
        AcT act;        //!< Access Technology
        int rssi;       //!< Received Signal Strength Indication (in dBm, range -113..-53)
        int ber;        //!< Bit Error Rate (BER), see 3GPP TS 45.008 [20] subclause 8.2.4
        char opr[16+1]; //!< Operator Name
        char num[32];   //!< Mobile Directory Number
        unsigned short lac;  //!< location area code in hexadecimal format (2 bytes in hex)
        unsigned int ci;     //!< Cell ID in hexadecimal format (2 to 4 bytes in hex)
    } NetStatus;
    //! An IP v4 address
    typedef uint32_t IP;
    #define NOIP ((MDMParser::IP)0) //!< No IP address
    // ip number formating and conversion
    #define IPSTR           "%d.%d.%d.%d"
    #define IPNUM(ip)       ((ip)>>24)&0xff, \
                            ((ip)>>16)&0xff, \
                            ((ip)>> 8)&0xff, \
                            ((ip)>> 0)&0xff
    #define IPADR(a,b,c,d) ((((IP)(a))<<24) | \
                            (((IP)(b))<<16) | \
                            (((IP)(c))<< 8) | \
                            (((IP)(d))<< 0))

    
    // ----------------------------------------------------------------
    // Device 
    // ----------------------------------------------------------------
    
    typedef enum { AUTH_NONE, AUTH_PAP, AUTH_CHAP, AUTH_DETECT } Auth; 
    
    /** Combined Init, checkNetStatus, join suitable for simple applications
        \param simpin a optional pin of the SIM card
        \param apn  the of the network provider e.g. "internet" or "apn.provider.com"
        \param username is the user name text string for the authentication phase
        \param password is the password text string for the authentication phase
        \param auth is the authentication mode (CHAP,PAP,NONE or DETECT)
        \return true if successful, false otherwise
    */
    bool connect(const char* simpin = NULL, 
            const char* apn = NULL, const char* username = NULL, 
            const char* password = NULL, Auth auth = AUTH_DETECT,
            PinName pn MDM_IF( = MDMPWRON, = PD_1), PinName r_pn MDM_IF( = MDMRESET, = PD_2));

    /** register (Attach) the MT to the GPRS service. 
        \param simpin a optional pin of the SIM card
        \param status an optional struture to with device information 
        \return true if successful, false otherwise
    */
    bool init(const char* simpin = NULL, DevStatus* status = NULL, 
                PinName pn MDM_IF( = MDMPWRON, = PD_1), PinName r_pn MDM_IF( = MDMRESET, = PD_2));

    /** register to the network 
        \param status an optional structure to with network information 
        \param timeout_ms -1 blocking, else non blocking timeout in ms
        \return true if successful and connected to network, false otherwise
    */
    bool registerNet(NetStatus* status = NULL, int timeout_ms = 180000);
    
    /** check if the network is available 
        \param status an optional structure to with network information 
        \return true if successful and connected to network, false otherwise
    */
    bool checkNetStatus(NetStatus* status = NULL);
    
    /** Power off the MT, This function has to be called prior to 
        switching off the supply. 
        \return true if successfully, false otherwise
    */ 
    bool powerOff(void);
    
    // ----------------------------------------------------------------
    // Data Connection (GPRS)
    // ----------------------------------------------------------------
    
    /** register (Attach) the MT to the GPRS service. 
        \param apn  the of the network provider e.g. "internet" or "apn.provider.com"
        \param username is the user name text string for the authentication phase
        \param password is the password text string for the authentication phase
        \param auth is the authentication mode (CHAP,PAP,NONE or DETECT)
        \return the ip that is assigned 
    */
    MDMParser::IP join(const char* apn = NULL, const char* username = NULL, 
                       const char* password = NULL, Auth auth = AUTH_DETECT);
    
    /** deregister (detach) the MT from the GPRS service.
        \return true if successful, false otherwise
    */
    bool disconnect(void);
    
    /** Translates a domain name to an IP address
        \param host the domain name to translate e.g. "u-blox.com"
        \return the IP if successful, 0 otherwise
    */
    MDMParser::IP gethostbyname(const char* host);
    
    // ----------------------------------------------------------------
    // Sockets
    // ----------------------------------------------------------------
    
    //! Type of IP protocol 
    typedef enum { IPPROTO_TCP, IPPROTO_UDP } IpProtocol; 
    
    //! Socket error return codes
    #define SOCKET_ERROR -1
    
    /** Create a socket for a ip protocol (and optionaly bind)
        \param ipproto the protocol (UDP or TCP) 
        \param port in case of UDP, this optional port where it is bind
        \return the socket handle if successful or SOCKET_ERROR on failure 
    */
    int socketSocket(IpProtocol ipproto, int port = -1);
    
    /** make a socket connection
        \param socket the socket handle
        \param host the domain name to connect e.g. "u-blox.com"
        \param port the port to connect
        \return true if successfully, false otherwise
    */
    bool socketConnect(int socket, const char* host, int port);
        
    /** make a socket connection
        \param socket the socket handle
        \return true if connected, false otherwise
    */
    bool socketIsConnected(int socket);
     
    /** Get the number of bytes pending for reading for this socket
        \param socket the socket handle
        \param timeout_ms -1 blocking, else non blocking timeout in ms
        \return 0 if successful or SOCKET_ERROR on failure 
    */
    bool socketSetBlocking(int socket, int timeout_ms);
    
    /** Write socket data 
        \param socket the socket handle
        \param buf the buffer to write
        \param len the size of the buffer to write
        \return the size written or SOCKET_ERROR on failure 
    */
    int socketSend(int socket, const char * buf, int len);
    
    /** Write socket data to a IP
        \param socket the socket handle
        \param ip the ip to send to
        \param port the port to send to
        \param buf the buffer to write
        \param len the size of the buffer to write
        \return the size written or SOCKET_ERROR on failure 
    */
    int socketSendTo(int socket, IP ip, int port, const char * buf, int len);
    
    /** Get the number of bytes pending for reading for this socket
        \param socket the socket handle
        \return the number of bytes pending or SOCKET_ERROR on failure 
    */
    int socketReadable(int socket);
    
    /** Read this socket
        \param socket the socket handle
        \param buf the buffer to read into
        \param len the size of the buffer to read into
        \return the number of bytes read or SOCKET_ERROR on failure 
    */
    int socketRecv(int socket, char* buf, int len);
    
    /** Read from this socket
        \param socket the socket handle
        \param ip the ip of host where the data originates from
        \param port the port where the data originates from
        \param buf the buffer to read into
        \param len the size of the buffer to read into
        \return the number of bytes read or SOCKET_ERROR on failure 
    */
    int socketRecvFrom(int socket, IP* ip, int* port, char* buf, int len);
    
    /** Close a connectied socket (that was connected with #socketConnect)
        \param socket the socket handle
        \return true if successfully, false otherwise
    */    
    bool socketClose(int socket);
    
    /** Free the socket (that was allocated before by #socketSocket)
        \param socket the socket handle
        \return true if successfully, false otherwise
    */    
    bool socketFree(int socket);
        
    // ----------------------------------------------------------------
    // SMS Short Message Service
    // ----------------------------------------------------------------
    
    /** count the number of sms in the device and optionally return a 
        list with indexes from the storage locations in the device.
        \param stat what type of messages you can use use 
                    "REC UNREAD", "REC READ", "STO UNSENT", "STO SENT", "ALL"
        \param ix   list where to save the storage positions
        \param num  number of elements in the list 
        \return the number of messages, this can be bigger than num, -1 on failure
    */
    int smsList(const char* stat = "ALL", int* ix = NULL, int num = 0);
    
    /** Read a Message from a storage position
        \param ix the storage position to read
        \param num the originator address (~16 chars)
        \param buf a buffer where to save the sm
        \param len the length of the sm
        \return true if successful, false otherwise
    */
    bool smsRead(int ix, char* num, char* buf, int len);
    
    /** Send a message to a recipient 
        \param ix the storage position to delete
        \return true if successful, false otherwise
    */
    bool smsDelete(int ix);
    
    /** Send a message to a recipient 
        \param num the phone number of the recipient
        \param buf the content of the message to sent
        \return true if successful, false otherwise
    */
    bool smsSend(const char* num, const char* buf);
    
    // ----------------------------------------------------------------
    // USSD Unstructured Supplementary Service Data
    // ----------------------------------------------------------------
    
    /** Read a Message from a storage position
        \param cmd the ussd command to send e.g "*#06#"
        \param buf a buffer where to save the reply
        \return true if successful, false otherwise
    */
    bool ussdCommand(const char* cmd, char* buf);
    
    // ----------------------------------------------------------------
    // FILE 
    // ----------------------------------------------------------------
    
    /** Delete a file in the local file system
        \param filename the name of the file 
        \return true if successful, false otherwise
    */
    bool delFile(const char* filename);
    
    /** Write some data to a file in the local file system
        \param filename the name of the file 
        \param buf the data to write 
        \param len the size of the data to write
        \return the number of bytes written
    */
    int writeFile(const char* filename, const char* buf, int len);
    
    /** REad a file from the local file system
        \param filename the name of the file 
        \param buf a buffer to hold the data 
        \param len the size to read
        \return the number of bytes read
    */
    int readFile(const char* filename, char* buf, int len);
    
    // ----------------------------------------------------------------
    // DEBUG/DUMP status to standard out (printf)
    // ----------------------------------------------------------------
    
    /*! Set the debug level 
        \param level 0 = OFF, 1 = INFO(default), 2 = TRACE, 3 = ATCMD
        \return true if successful, false not possible
    */ 
    bool setDebug(int level);

    //! helper type for DPRINT
    typedef int (*_DPRINT)(void* param, char const * format, ...);
    
    //! helper to declate templates and void versions
#define _DUMP_TEMPLATE(func, type, arg) \
    template<class T> \
    inline void func(type arg, \
                int (*dprint)( T* param, char const * format, ...), \
                T* param) { func(arg, (_DPRINT)dprint, (void*)param); } \
    static void func(type arg, \
                _DPRINT dprint = (_DPRINT)fprintf, \
                void* param = (void*)stdout);

    /** dump the device status to stdout using printf
        \param status the status to convert to textual form, 
               unavailable fields are ommited (not printed)
        \param dprint a function pointer
        \param param  the irst argument passed to dprint
    */
    _DUMP_TEMPLATE(dumpDevStatus, MDMParser::DevStatus*, status)

    /** dump the network status to stdout using printf
        \param status the status to convert to textual form, 
               unavailable fields are ommited (not printed)
        \param dprint a function pointer
        \param param  the irst argument passed to dprint
    */
    _DUMP_TEMPLATE(dumpNetStatus, MDMParser::NetStatus*, status)
    
    /** dump the ip address to stdout using printf
        \param ip the ip to convert to textual form, 
               unavailable fields are ommited (not printed)
        \param dprint a function pointer
        \param param  the irst argument passed to dprint
    */
    _DUMP_TEMPLATE(dumpIp, MDMParser::IP, ip)
   
    // ----------------------------------------------------------------
    // Parseing
    // ----------------------------------------------------------------
    
    enum { 
        // waitFinalResp Responses
        NOT_FOUND     =  0,
        WAIT          = -1, // TIMEOUT
        RESP_OK       = -2, 
        RESP_ERROR    = -3,
        RESP_PROMPT   = -4,
    
        // getLine Responses
        #define LENGTH(x)  (x & 0x00FFFF) //!< extract/mask the length
        #define TYPE(x)    (x & 0xFF0000) //!< extract/mask the type
        
        TYPE_UNKNOWN    = 0x000000,
        TYPE_OK         = 0x110000,
        TYPE_ERROR      = 0x120000,
        TYPE_RING       = 0x210000,
        TYPE_CONNECT    = 0x220000,
        TYPE_NOCARRIER  = 0x230000,
        TYPE_NODIALTONE = 0x240000,
        TYPE_BUSY       = 0x250000,
        TYPE_NOANSWER   = 0x260000,
        TYPE_PROMPT     = 0x300000,
        TYPE_PLUS       = 0x400000,
        TYPE_TEXT       = 0x500000,
        
        // special timout constant
        TIMEOUT_BLOCKING = -1
    };
    
    /** Get a line from the physical interface. This function need 
        to be implemented in a inherited class. Usually just calls 
        #_getLine on the rx buffer pipe. 
            
        \param buf the buffer to store it
        \param buf size of the buffer
        \return type and length if something was found, 
                WAIT if not enough data is available
                NOT_FOUND if nothing was found
    */ 
    virtual int getLine(char* buf, int len) = 0; 
    
    /* clear the pending input data
    */
    virtual void purge(void) = 0;
    
    /** Write data to the device 
        \param buf the buffer to write
        \param buf size of the buffer to write
        \return bytes written
    */
    virtual int send(const char* buf, int len);
    
    /** Write formated date to the physical interface (printf style)
        \param fmt the format string
        \param .. variable arguments to be formated
        \return bytes written
    */
    int sendFormated(const char* format, ...);
    
    /** callback function for #waitFinalResp with void* as argument
        \param type the #getLine response
        \param buf the parsed line
        \param len the size of the parsed line
        \param param the optional argument passed to #waitFinalResp
        \return WAIT if processing should continue, 
                any other value aborts #waitFinalResp and this retunr value retuned
    */
    typedef int (*_CALLBACKPTR)(int type, const char* buf, int len, void* param);
    
    /** Wait for a final respons
        \param cb the optional callback function
        \param param the optional callback function parameter
        \param timeout_ms the timeout to wait (See Estimated command 
               response time of AT manual)
    */
    int waitFinalResp(_CALLBACKPTR cb = NULL, 
                      void* param = NULL, 
                      int timeout_ms = 10000);

    /** template version of #waitFinalResp when using callbacks, 
        This template will allow the compiler to do type cheking but 
        internally symply casts the arguments and call the (void*) 
        version of #waitFinalResp.
        \sa waitFinalResp
    */ 
    template<class T>
    inline int waitFinalResp(int (*cb)(int type, const char* buf, int len, T* param), 
                    T* param, 
                    int timeout_ms = 10000) 
    {
        return waitFinalResp((_CALLBACKPTR)cb, (void*)param, timeout_ms);
    }
    
protected:
    /** Write bytes to the physical interface. This function should be 
        implemented in a inherited class.
        \param buf the buffer to write
        \param buf size of the buffer to write
        \return bytes written
    */
    virtual int _send(const void* buf, int len) = 0;

    /** Helper: Parse a line from the receiving buffered pipe
        \param pipe the receiving buffer pipe 
        \param buf the parsed line
        \param len the size of the parsed line
        \return type and length if something was found, 
                WAIT if not enough data is available
                NOT_FOUND if nothing was found
    */
    static int _getLine(Pipe<char>* pipe, char* buffer, int length);
    
    /** Helper: Parse a match from the pipe
        \param pipe the buffered pipe
        \param number of bytes to parse at maximum, 
        \param sta the starting string, NULL if none
        \param end the terminating string, NULL if none
        \return size of parsed match 
    */   
    static int _parseMatch(Pipe<char>* pipe, int len, const char* sta, const char* end);
    
    /** Helper: Parse a match from the pipe
        \param pipe the buffered pipe
        \param number of bytes to parse at maximum, 
        \param fmt the formating string (%d any number, %c any char of last %d len)
        \return size of parsed match
    */   
    static int _parseFormated(Pipe<char>* pipe, int len, const char* fmt);

protected:
    // for rtos over riding by useing Rtos<MDMxx> 
    /** override in a rtos system, you us the wait function of a Thread
        \param ms the number of milliseconds to wait
    */
    virtual void wait_ms(int ms)   { if (ms) ::wait_ms(ms); }
    //! override the lock in a rtos system
    virtual void lock(void)        { } 
    //! override the unlock in a rtos system
    virtual void unlock(void)      { } 
protected:
    // parsing callbacks for different AT commands and their parameter arguments
    static int _cbString(int type, const char* buf, int len, char* str);
    static int _cbInt(int type, const char* buf, int len, int* val);
    // device
    static int _cbATI(int type, const char* buf, int len, Dev* dev);
    static int _cbCPIN(int type, const char* buf, int len, Sim* sim);
    static int _cbCCID(int type, const char* buf, int len, char* ccid);
    // network 
    static int _cbCSQ(int type, const char* buf, int len, NetStatus* status);
    static int _cbCOPS(int type, const char* buf, int len, NetStatus* status);
    static int _cbCNUM(int type, const char* buf, int len, char* num);
    static int _cbUACTIND(int type, const char* buf, int len, int* i);
    static int _cbUDOPN(int type, const char* buf, int len, char* mccmnc);
    // sockets
    static int _cbCMIP(int type, const char* buf, int len, IP* ip);
    static int _cbUPSND(int type, const char* buf, int len, int* act);
    static int _cbUPSND(int type, const char* buf, int len, IP* ip);
    static int _cbUDNSRN(int type, const char* buf, int len, IP* ip);
    static int _cbUSOCR(int type, const char* buf, int len, int* socket);
    static int _cbUSORD(int type, const char* buf, int len, char* out);
    typedef struct { char* buf; IP ip; int port; } USORFparam;
    static int _cbUSORF(int type, const char* buf, int len, USORFparam* param);
    typedef struct { char* buf; char* num; } CMGRparam;
    static int _cbCUSD(int type, const char* buf, int len, char* resp);
    // sms
    typedef struct { int* ix; int num; } CMGLparam;
    static int _cbCMGL(int type, const char* buf, int len, CMGLparam* param);
    static int _cbCMGR(int type, const char* buf, int len, CMGRparam* param);
    // file
    typedef struct { const char* filename; char* buf; int sz; int len; } URDFILEparam;
    static int _cbURDFILE(int type, const char* buf, int len, URDFILEparam* param);
    // variables
    DevStatus   _dev; //!< collected device information
    NetStatus   _net; //!< collected network information 
    IP          _ip;  //!< assigned ip address
    // management struture for sockets
    typedef enum { SOCK_FREE, SOCK_CREATED, SOCK_CONNECTED } SockState;
    typedef struct { volatile SockState state; volatile int pending; int timeout_ms; } SockCtrl;
    // LISA-C has 6 TCP and 6 UDP sockets starting at index 18
    // LISA-U and SARA-G have 7 sockets starting at index 1
    SockCtrl _sockets[32];
    static MDMParser* inst;
    bool _init;
#ifdef TARGET_UBLOX_C027
    bool _onboard;
#endif
#ifdef MDM_DEBUG
    int _debugLevel;
    Timer _debugTime;
#endif
};

// -----------------------------------------------------------------------

/** modem class which uses a serial port 
    as physical interface. 
*/
class MDMSerial :  public SerialPipe, public MDMParser
{
public: 
    /** Constructor
    
        \param tx is the serial ports transmit pin (modem to CPU)
        \param rx is the serial ports receive pin (CPU to modem)
        \param baudrate the baudrate of the modem use 115200
        \param rts is the serial ports ready to send pin (CPU to modem) 
               this pin is optional 
        \param cts is the serial ports clear to send pin (modem to CPU) 
               this pin is optional, but required for power saving to be enabled
        \param rxSize the size of the serial rx buffer
        \param txSize the size of the serial tx buffer
    */
    MDMSerial(PinName tx    MDM_IF( = MDMTXD,  = PD_5 ), 
              PinName rx    MDM_IF( = MDMRXD,  = PD_6 ), 
              int baudrate  MDM_IF( = MDMBAUD, = 115200 ),
 #if DEVICE_SERIAL_FC
              PinName rts   MDM_IF( = MDMRTS,  = NC /* D2 resistor R62 on shield not mounted */ ), 
              PinName cts   MDM_IF( = MDMCTS,  = NC /* D3 resistor R63 on shield not mounted */ ),
 #endif
              int rxSize    = 256 , 
              int txSize    = 128 );
    //! Destructor          
    virtual ~MDMSerial(void);
    
    /** Get a line from the physical interface. 
        \param buf the buffer to store it
        \param buf size of the buffer
        \return type and length if something was found, 
                WAIT if not enough data is available
                NOT_FOUND if nothing was found
    */ 
    virtual int getLine(char* buffer, int length);
    
    /* clear the pending input data */
    virtual void purge(void) 
    { 
        while (readable())
            getc();
    }
protected:
    /** Write bytes to the physical interface.
        \param buf the buffer to write
        \param buf size of the buffer to write
        \return bytes written
    */
    virtual int _send(const void* buf, int len);
};

// -----------------------------------------------------------------------

//#define HAVE_MDMUSB
#ifdef HAVE_MDMUSB
class MDMUsb :  /*public UsbSerial,*/ public MDMParser
{
public: 
    //! Constructor          
    MDMUsb(void);
    //! Destructor          
    virtual ~MDMUsb(void);
    virtual int getLine(char* buffer, int length);
    virtual void purge(void) { }
protected:
    virtual int _send(const void* buf, int len);
};
#endif

// -----------------------------------------------------------------------

#ifdef RTOS_H
/** Use this template to override the lock and wait functions of the 
    modem driver in a Rtos system. For example declare it the modem 
    object as MDMRtos<MDMSerial> instead of MDMSerial.
*/
template <class T>
class MDMRtos :  public T
{
protected:
    //! we assume that the modem runs in a thread so we yield when waiting
    virtual void wait_ms(int ms)   {
        if (ms) Thread::wait(ms);
        else    Thread::yield();
    }
    //! lock a mutex when accessing the modem
    virtual void lock(void)     { _mtx.lock(); }  
    //! unlock the modem when done accessing it
    virtual void unlock(void)   { _mtx.unlock(); }
    // the mutex resource
    Mutex _mtx;
};
#endif

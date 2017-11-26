#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <eXosip2/eXosip.h>
#include <pthread.h>

#define FENG_DEBUG 1

#ifdef FENG_DEBUG
#endif

struct SipUA{
	pthread_mutex_t mutex; 
	pthread_t sip_thread;
	pthread_t rtp_thread;
	eXosip_event_t *je;
	osip_message_t *reg ;
	osip_message_t *invite ;
	osip_message_t *ack ;
	osip_message_t *info ;
	osip_message_t *message ;
	osip_message_t *answer ;
	sdp_message_t *remote_sdp ;

	sdp_connection_t * speech_con_req ;
	sdp_media_t * speech_md_req ;

	sdp_connection_t * video_con_req ;
	sdp_media_t * video_md_req ;

	int call_id;
	int dialog_id;
	int flag;
	int flag2;
	int flag3;
	int flag1;

	int rtp_start;
	int rtp_port;

	int speech_start;
	int speech_port;

	int video_start;
	int video_port;

	char *src_name;
	char *dst_name;
	char *proxy_ip;
	char *identity;
	char *registerer;
	char *source_call;
	char *dest_call;
	char *payload_str;

	char ansCall;
	char localip[128];
	char event_type[55][100];
};

extern struct SipUA mySipUA={
	.reg = NULL,
	.invite = NULL,
	.ack = NULL,
	.info = NULL,
	.message = NULL,
	.answer = NULL,
	.remote_sdp = NULL,

	.con_req = NULL,
	.md_req = NULL,

	.flag1 = 1,

	.rtp_start=0,
	.rtp_port=5058,

	.speech_start=0,
	.speech_port=5056,

	.video_start=0,
	.video_port=5054,

	.src_name="1000",
	.dst_name="1001",
	.proxy_ip="192.168.2.210",
	.identity=NULL,
	.registerer=NULL,
	.source_call=NULL,
	.dest_call=NULL,
	.payload_str=NULL,

	.ansCall='0',
	.event_type={
	"EXOSIP_REGISTRATION_NEW,           /**< announce new registration.       */",
	"EXOSIP_REGISTRATION_SUCCESS,       /**< user is successfully registred.  */",
	"EXOSIP_REGISTRATION_FAILURE,       /**< user is not registred.           */",
	"EXOSIP_REGISTRATION_REFRESHED,     /**< registration has been refreshed. */",
	"EXOSIP_REGISTRATION_TERMINATED,    /**< UA is not registred any more.    */",
	"EXOSIP_CALL_INVITE,            /**< announce a new call                   */",
	"EXOSIP_CALL_REINVITE,          /**< announce a new INVITE within call     */",
	"EXOSIP_CALL_NOANSWER,          /**< announce no answer within the timeout */",
	"EXOSIP_CALL_PROCEEDING,        /**< announce processing by a remote app   */",
	"EXOSIP_CALL_RINGING,           /**< announce ringback                     */",
	"EXOSIP_CALL_ANSWERED,          /**< announce start of call                */",
	"EXOSIP_CALL_REDIRECTED,        /**< announce a redirection                */",
	"EXOSIP_CALL_REQUESTFAILURE,    /**< announce a request failure            */",
	"EXOSIP_CALL_SERVERFAILURE,     /**< announce a server failure             */",
	"EXOSIP_CALL_GLOBALFAILURE,     /**< announce a global failure             */",
	"EXOSIP_CALL_ACK,               /**< ACK received for 200ok to INVITE      */",
	"EXOSIP_CALL_CANCELLED,         /**< announce that call has been cancelled */",
	"EXOSIP_CALL_TIMEOUT,           /**< announce that call has failed         */",
	"EXOSIP_CALL_MESSAGE_NEW,              /**< announce new incoming request. */",
	"EXOSIP_CALL_MESSAGE_PROCEEDING,       /**< announce a 1xx for request. */",
	"EXOSIP_CALL_MESSAGE_ANSWERED,         /**< announce a 200ok  29*/",
	"EXOSIP_CALL_MESSAGE_REDIRECTED,       /**< announce a failure. */",
	"EXOSIP_CALL_MESSAGE_REQUESTFAILURE,   /**< announce a failure. */",
	"EXOSIP_CALL_MESSAGE_SERVERFAILURE,    /**< announce a failure. */",
	"EXOSIP_CALL_MESSAGE_GLOBALFAILURE,    /**< announce a failure. */",
	"EXOSIP_CALL_CLOSED,            /**< a BYE was received for this call      */",
	"EXOSIP_CALL_RELEASED,             /**< call context is cleared.            */",
	"EXOSIP_MESSAGE_NEW,              /**< announce new incoming request. */",
	"EXOSIP_MESSAGE_PROCEEDING,       /**< announce a 1xx for request. */",
	"EXOSIP_MESSAGE_ANSWERED,         /**< announce a 200ok  */",
	"EXOSIP_MESSAGE_REDIRECTED,       /**< announce a failure. */",
	"EXOSIP_MESSAGE_REQUESTFAILURE,   /**< announce a failure. */",
	"EXOSIP_MESSAGE_SERVERFAILURE,    /**< announce a failure. */",
	"EXOSIP_MESSAGE_GLOBALFAILURE,    /**< announce a failure. */",
	"EXOSIP_SUBSCRIPTION_UPDATE,         /**< announce incoming SUBSCRIBE.      */",
	"EXOSIP_SUBSCRIPTION_CLOSED,         /**< announce end of subscription.     */",
	"EXOSIP_SUBSCRIPTION_NOANSWER,          /**< announce no answer              */",
	"EXOSIP_SUBSCRIPTION_PROCEEDING,        /**< announce a 1xx                  */",
	"EXOSIP_SUBSCRIPTION_ANSWERED,          /**< announce a 200ok                */",
	"EXOSIP_SUBSCRIPTION_REDIRECTED,        /**< announce a redirection          */",
	"EXOSIP_SUBSCRIPTION_REQUESTFAILURE,    /**< announce a request failure      */",
	"EXOSIP_SUBSCRIPTION_SERVERFAILURE,     /**< announce a server failure       */",
	"EXOSIP_SUBSCRIPTION_GLOBALFAILURE,     /**< announce a global failure       */",
	"EXOSIP_SUBSCRIPTION_NOTIFY,            /**< announce new NOTIFY request     */",
	"EXOSIP_SUBSCRIPTION_RELEASED,          /**< call context is cleared.        */",
	"EXOSIP_IN_SUBSCRIPTION_NEW,            /**< announce new incoming SUBSCRIBE.*/",
	"EXOSIP_IN_SUBSCRIPTION_RELEASED,       /**< announce end of subscription.   */",
	"EXOSIP_NOTIFICATION_NOANSWER,          /**< announce no answer              */",
	"EXOSIP_NOTIFICATION_PROCEEDING,        /**< announce a 1xx                  */",
	"EXOSIP_NOTIFICATION_ANSWERED,          /**< announce a 200ok                */",
	"EXOSIP_NOTIFICATION_REDIRECTED,        /**< announce a redirection          */",
	"EXOSIP_NOTIFICATION_REQUESTFAILURE,    /**< announce a request failure      */",
	"EXOSIP_NOTIFICATION_SERVERFAILURE,     /**< announce a server failure       */",
	"EXOSIP_NOTIFICATION_GLOBALFAILURE,     /**< announce a global failure       */",
	"EXOSIP_EVENT_COUNT                  /**< MAX number of events              */"
	},
};



void myInit(void);

/////////////////////////////////////////
//
/////////////////////////////////////////
void myRegister(int expire1);

/////////////////////////////////////////
//
/////////////////////////////////////////
void myInvite(void);


/////////////////////////////////////////
//
/////////////////////////////////////////
void myHoldUp(void);


/////////////////////////////////////////
//
/////////////////////////////////////////
void myInfo(void)

/////////////////////////////////////////
//
/////////////////////////////////////////
void myMsg(void);


/////////////////////////////////////////
//
/////////////////////////////////////////
void myQuit(void);

	
/////////////////////////////////////////
//
/////////////////////////////////////////
int build_media(int ifSend);

/////////////////////////////////////////
//
/////////////////////////////////////////
void reNewMsg(void);

void reInviteCall();

void reAckCall(void);

void reCloseCall(void);

void reNewMsgCall(void);

/////////////////////////////////////////
//
/////////////////////////////////////////
void reRegOK();


/////////////////////////////////////////
//
/////////////////////////////////////////
void reRegFail();


/////////////////////////////////////////
//
/////////////////////////////////////////
void reRegRefrsh();

/////////////////////////////////////////
//
/////////////////////////////////////////
void reProcCall();

/////////////////////////////////////////
//
/////////////////////////////////////////
void reRingCall();


/////////////////////////////////////////
//
/////////////////////////////////////////
void reAnswerCall();

/////////////////////////////////////////
//
/////////////////////////////////////////
void *SipThread();

 //////////////////////////////////////
 //	
 //////////////////////////////////////
 void *RtpThread();
 
 static const char *MENU =
    "|----------------------------------------|\n"
    "|        a-----Clear Screen             |\n"
    "|        r-----Register to server        |\n"
    "|        c-----Cancel registration       |\n"
    "|        i-----mySipUA.invite Call               |\n"
    "|        h-----Hang up                   |\n"
    "|        q-----Quit                      |\n"
    "|        s-----send mySipUA.info	            |\n"
    "|        m-----send mySipUA.message     	  |\n"
    "|        d------Display Menu           |\n"
    "|----------------------------------------|\n\n";
/////////////////////////////////////////
//
/////////////////////////////////////////
int sipMainInit(void);



void getSipCommand(void);


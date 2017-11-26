/*******************UA*****************************************************
Auther:	fengjx
Env:	eXosip3.6.0/redhat AS 5
compile:
gcc feng_uas.c -o feng_uas -L/usr/local/lib/ -I/usr/local/include/ -losip2 -losipparser2 -leXosip2 -lpthread

Date:	2012.07.23
UpDate:	2012.08.10
***************************************************************************/

#include "feng_ua.h"

void myInit(void)
{
	int i;
   // set names
   mySipUA.identity=(char *)malloc( (strlen(mySipUA.src_name)+strlen(mySipUA.proxy_ip)+5+1)* sizeof(char) );
   snprintf(mySipUA.identity, strlen(mySipUA.src_name)+strlen(mySipUA.proxy_ip)+5+1, "sip:%s@%s", mySipUA.src_name, mySipUA.proxy_ip);
   
   mySipUA.registerer=(char *)malloc( (strlen(mySipUA.proxy_ip)+9+1)* sizeof(char) );
   snprintf(mySipUA.registerer, strlen(mySipUA.proxy_ip)+9+1, "sip:%s:5060", mySipUA.proxy_ip);
   
   mySipUA.source_call=(char *)malloc( (strlen(mySipUA.src_name)+strlen(mySipUA.proxy_ip)+5+1)* sizeof(char) );
   snprintf(mySipUA.source_call, strlen(mySipUA.src_name)+strlen(mySipUA.proxy_ip)+5+1, "sip:%s@%s", mySipUA.src_name, mySipUA.proxy_ip);
   
   mySipUA.dest_call=(char *)malloc( (strlen(mySipUA.dst_name)+strlen(mySipUA.proxy_ip)+5+1)* sizeof(char) );
   snprintf(mySipUA.dest_call, strlen(mySipUA.dst_name)+strlen(mySipUA.proxy_ip)+5+1, "sip:%s@%s", mySipUA.dst_name, mySipUA.proxy_ip);
   
   // eXosip Initial
    i = eXosip_init ();
    if (i != 0)
    {
        printf ("Couldn't initialize eXosip!\n");
        exit(0);
    }
    printf ("eXosip_init successfully!\n");
	
    // Listen initial
    i = eXosip_listen_addr (IPPROTO_UDP, NULL, 0, AF_INET, 0);
    if (i != 0)
    {
        eXosip_quit ();
        fprintf (stderr, "Couldn't initialize transport layer!\n");
        exit(0);
    }
    i = pthread_mutex_init(&mySipUA.mutex, NULL); 
    if (i != 0) 
    { 
        perror("mySipUA.mutex init failed!"); 
        exit(EXIT_FAILURE); 
    } 
    printf("\nYour Proxy server is %s\n", mySipUA.proxy_ip);
}

/////////////////////////////////////////
//
/////////////////////////////////////////
void myRegister(int expire1)
{
    int i, expire2, id;

    if(expire1==0)
    	expire2=expire1;
    else
    	expire2=1800
    eXosip_lock();
    id = eXosip_register_build_initial_register (mySipUA.identity, mySipUA.registerer, NULL, expire2, &mySipUA.reg);

    if (id < 0)
    {
        eXosip_unlock();
        fprintf (stderr, "eXosip_register_build_initial_register failed:(bad arguments?)\n");
        return;
    }

    i = eXosip_register_send_register(id, mySipUA.reg);
    if (i != 0)
    {
        fprintf (stderr, "eXosip_register_send_register failed: (bad arguments?)\n");
        return;
    }
    eXosip_unlock ();

    printf("eXosip_register_send_register OK\n");
}

/////////////////////////////////////////
//
/////////////////////////////////////////
void myInvite(void)
{
    char *name1;
    char inbuffer[20];
	int i;
    
    printf("\nPlease input the hostname you want to call: ");
    // Clear inbuffer
    for(i=0; i<20-1;i++)
    	inbuffer[i]='0';
    inbuffer[i]='\0';
    pthread_mutex_lock(&mySipUA.mutex);
    fflush(stdout); 
    if (fgets(inbuffer, strlen(inbuffer), stdin) == NULL) 
     {
	    puts("EOF while reading stdin, will quit now..");
	    return;
      }
      pthread_mutex_unlock(&mySipUA.mutex);
      name1=(char *)malloc( (strlen(inbuffer))* sizeof(char) );
      snprintf(name1, strlen(inbuffer), "%s", inbuffer);
      if (strlen(name1)!=0) 
     {
    	free(mySipUA.dest_call);
    	mySipUA.dest_call=(char *)malloc( (strlen(name1)+strlen(mySipUA.proxy_ip)+5+1)* sizeof(char) );
    	
    	snprintf(mySipUA.dest_call, strlen(name1)+strlen(mySipUA.proxy_ip)+5+1, "sip:%s@%s", name1, mySipUA.proxy_ip);
    	
     }
     printf("You choose to call %s\n", mySipUA.dest_call);
    i = eXosip_call_build_initial_invite (&mySipUA.invite, mySipUA.dest_call, mySipUA.source_call, NULL, "This si a call for a conversation");
    if (i != 0)
    {
        printf ("Intial mySipUA.invite failed!\n");
        return;
    }

    snprintf (tmp, 4096,
        "v=0\r\n"   //version
        "o=%s 0 2 IN IP4 %s\r\n"    // owner
        "s=feng_ua\r\n" // session name
        "c=IN IP4 %s\r\n"
        "t=0 0\r\n"
		"m=audio %d RTP/AVP 0 8 101\r\n"
		"a=rtpmap:0 PCMU/8000\r\n"
		"a=rtpmap:8 PCMA/8000\r\n"
		"a=rtpmap:101 telephone-event/8000\r\n"
		"a=fmtp:101 0-15\r\n"
        "a=username:%s\r\n"   //attribute
        "a=password:%s\r\n", mySipUA.src_name, mySipUA.localip, mySipUA.localip, mySipUA.speech_port, mySipUA.src_name, mySipUA.src_name);
    osip_message_set_body (mySipUA.invite, tmp, strlen(tmp));
    osip_message_set_content_type (mySipUA.invite, "application/sdp");

    eXosip_lock ();
    i = eXosip_call_send_initial_invite (mySipUA.invite);
    eXosip_unlock ();
}

/////////////////////////////////////////
//
/////////////////////////////////////////
void myHoldUp(void)
{
    printf ("Holded !\n");
    eXosip_lock ();
    eXosip_call_terminate (mySipUA.call_id, mySipUA.dialog_id);
    eXosip_unlock ();
    mySipUA.rtp_start=0;
}

/////////////////////////////////////////
//
/////////////////////////////////////////
void myInfo(void)
{
	char tmp[4096];
	
	eXosip_call_build_info (mySipUA.dialog_id, &mySipUA.info);
	snprintf (tmp , 4096, "hello,rainfish");
	osip_message_set_body (mySipUA.info, tmp, strlen(tmp));
	osip_message_set_content_type (mySipUA.info, "text/plain");
	eXosip_call_send_request (mySipUA.dialog_id, mySipUA.info);
}

/////////////////////////////////////////
//
/////////////////////////////////////////
void myMsg(void)
{
	char tmp[4096];
	printf ("the mothed :mySipUA.message\n");
	eXosip_message_build_request (&mySipUA.message, "mySipUA.message", mySipUA.dest_call, mySipUA.source_call, NULL);
	snprintf (tmp, 4096, "hello rainfish");
	osip_message_set_body (mySipUA.message, tmp, strlen(tmp));
	osip_message_set_content_type (mySipUA.message, "text/xml");
    eXosip_message_send_request (mySipUA.message);
}

/////////////////////////////////////////
//
/////////////////////////////////////////
void myQuit(void)
{
	myRegister(0);
	printf ("Exit the setup!\n");
    mySipUA.flag = 0;
   	mySipUA.flag2=0;
    mySipUA.flag3=0;
	gblSetQuit();
}
	
/////////////////////////////////////////
//
/////////////////////////////////////////
int build_media(int ifSend)
{
	printf("His IP address is %s\n",  mySipUA.speech_con_req->c_addr);
	printf("His m_port is %s\n", mySipUA.speech_md_req->m_port);
	printf("His mySipUA.payload_str is %s\n", atoi(mySipUA.payload_str));
	printf("Media builded successfully!\n");
	
	if(ifSend)
	{
		mySipUA.speech_start=1;
		mySipUA.video_start=1;
	}
	return(1);
}

void stop_media(void)
{
	mySipUA.speech_start=0;
	mySipUA.video_start=0;
}
/////////////////////////////////////////
//
/////////////////////////////////////////
void reNewMsg(void)
{
	#ifdef FENG_DEBUG
	printf("reNewMsg()\n");
	#endif
	printf (" EXOSIP_MESSAGE_NEW!\n");
	if (MSG_IS_MESSAGE (mySipUA.je->request))// 
	{
		{
			osip_body_t *body;
			osip_message_get_body (mySipUA.je->request, 0, &body);
			printf ("I get the msg is: %s\n", body->body);
			printf ("the cid is %s, did is %s\n", mySipUA.je->did, mySipUA.je->cid);
		}
		eXosip_message_build_answer (mySipUA.je->tid, 200,&mySipUA.answer);
		eXosip_message_send_answer (mySipUA.je->tid, 200,mySipUA.answer);
    }
}

void reInviteCall()
{
	int pos=0;	

#ifdef FENG_DEBUG
	printf("reInviteCall()\n");
#endif
	printf("mySipUA.answer the call? [y/n]:\n");
	printf ("mySipUA.invite msg from %s:%s\n\tUserName is %s\n\tpassword is %s\n",
		mySipUA.je->request->req_uri->host, mySipUA.je->request->req_uri->port,
		mySipUA.je->request->req_uri->username, mySipUA.je->request->req_uri->password);
	//
	eXosip_call_send_answer (mySipUA.je->tid, 180, NULL);// Ringing
	mySipUA.ansCall='0';
	while(mySipUA.ansCall=='0')
	{
		sleep(1);
	}
	mySipUA.remote_sdp = eXosip_get_remote_sdp (mySipUA.je->did);
	if(mySipUA.remote_sdp==NULL)
		printf("mySipUA.remote_sdp==Null\n");
	else
	{
		printf("SDP mySipUA.message is:\nv-%s\no-%s\n", mySipUA.remote_sdp->v_version, mySipUA.remote_sdp->o_username);
	}
	mySipUA.call_id = mySipUA.je->cid;
	mySipUA.dialog_id = mySipUA.je->did;
	mySipUA.speech_con_req = eXosip_get_audio_connection(mySipUA.remote_sdp);
	if(!mySipUA.speech_con_req)
	{
		printf("Can't get audio connection!\n");
		myQuit();
		return;
	}
	mySipUA.speech_md_req = eXosip_get_audio_media(mySipUA.remote_sdp);
	if(!mySipUA.speech_md_req)
	{
		printf("Can't get media!\n");
		myQuit();
		return;
	}
	mySipUA.payload_str = (char *)osip_list_get(&mySipUA.speech_md_req->m_payloads, 0);	//取主叫媒体能力编�?	if(!mySipUA.payload_str)
	if(!mySipUA.payload_str)
	{
		printf("Can't get mySipUA.payload_str!\n");
		myQuit();
		return;
	}
	if(mySipUA.ansCall=='y')
	{
		eXosip_lock ();
		i = eXosip_call_build_answer (mySipUA.je->tid, 200, &mySipUA.answer);
		if (i != 0)
		{
			printf ("This request msg is invalid!Cann't response!\n");
			eXosip_call_send_answer (mySipUA.je->tid, 400, NULL);
		}
		else
		{
			snprintf (tmp, 4096,
       				"v=0\r\n"   //version
        			"o=%s 0 2 IN IP4 %s\r\n"    // owner
        			"s=feng_ua\r\n" // session name
        			"c=IN IP4 %s\r\n"
        			"t=0 0\r\n"
					"m=audio %d RTP/AVP 0 8 101\r\n"
					"a=rtpmap:0 PCMU/8000\r\n"
					"a=rtpmap:8 PCMA/8000\r\n"
					"a=rtpmap:101 telephone-event/8000\r\n"
					"a=fmtp:101 0-15\r\n"
        			"a=username:%s\r\n"   //attribute
        			"a=password:%s\r\n", mySipUA.src_name, mySipUA.localip, mySipUA.localip, mySipUA.speech_port, mySipUA.src_name, mySipUA.src_name);
			osip_message_set_body (mySipUA.answer, tmp, strlen(tmp));
			osip_message_set_content_type (mySipUA.answer, "application/sdp");
		
			eXosip_call_send_answer (mySipUA.je->tid, 200, mySipUA.answer);
			printf ("send 200 over!\n");
		}
		
		printf("\nRTP thread is starting...\n");
		eXosip_unlock ();
	}
	else
	{
		printf("\nYou choose not to mySipUA.answer the phone!\n");
		eXosip_call_send_answer (mySipUA.je->tid, 400, NULL);
		return;
	}
	// 
	printf ("the mySipUA.info is :\n");
	while (!osip_list_eol (&mySipUA.remote_sdp->a_attributes, pos))
	{
		sdp_attribute_t *at;
		
		at = (sdp_attribute_t *) osip_list_get (&mySipUA.remote_sdp->a_attributes, pos);
		printf ("%s : %s\n", at->a_att_field, at->a_att_value);// 
		pos ++;
    }
}

void reAckCall(void)
{
	int ret;
	
	#ifdef FENG_DEBUG
	printf("reAckCall()\n");
	#endif
	
	mySipUA.rtp_start=1;// Start RTP thread
	ret = build_media(0);
	if(!ret)
	{
		printf("Media Build Failure!\n");
		return;
	}
}

void reCloseCall(void)
{
	int i;

	#ifdef FENG_DEBUG
	printf("reCloseCall()\n");
	#endif
	mySipUA.rtp_start=0;
	printf ("the remote hold the session!\n");
	// eXosip_call_build_ack(mySipUA.dialog_id, &mySipUA.ack);
	//eXosip_call_send_ack(mySipUA.dialog_id, mySipUA.ack);
	i = eXosip_call_build_answer (mySipUA.je->tid, 200, &mySipUA.answer);
	if (i != 0)
	{
		printf ("This request msg is invalid!Cann't response!\n");
		eXosip_call_send_answer (mySipUA.je->tid, 400, NULL);
	}
	else
	{
		eXosip_call_send_answer (mySipUA.je->tid, 200, mySipUA.answer);
		printf ("bye send 200 over!\n");
	}
}

void reNewMsgCall(void)
{
	int i;

	#ifdef FENG_DEBUG
	printf("reNewMsgCall()\n");
	#endif
    /* request related events within calls (except mySipUA.invite) */
    EXOSIP_CALL_MESSAGE_NEW,            /**< announce new incoming request. */
    /* response received for request outside calls */
    EXOSIP_MESSAGE_NEW,            /**< announce new incoming request. */
    /*   */
    printf(" EXOSIP_CALL_MESSAGE_NEW\n");
    if (MSG_IS_INFO(mySipUA.je->request))// 
    {
        eXosip_lock ();
        i = eXosip_call_build_answer (mySipUA.je->tid, 200, &mySipUA.answer);
        if (i == 0)
        {
            eXosip_call_send_answer (mySipUA.je->tid, 200, mySipUA.answer);
        }
        eXosip_unlock ();
        {
            osip_body_t *body;
            osip_message_get_body (mySipUA.je->request, 0, &body);
            printf ("the body is %s\n", body->body);
        }
    }
}

/////////////////////////////////////////
//
/////////////////////////////////////////
void reRegOK()
{
	#ifdef FENG_DEBUG
	printf("reRegOK()\n");
	#endif
	if(0 < mySipUA.je->response->contacts.nb_elt)	/*bind 数大�?表示注册，否则为注销*/
	{
		printf("Registered!\n");
	}
	else
	{
		printf("Registeration Canceled!\n");
	}
	printf("textinfo is: %s\n", mySipUA.je->textinfo);
}

/////////////////////////////////////////
//
/////////////////////////////////////////
void reRegFail()
{
	#ifdef FENG_DEBUG
	printf("reRegFail()\n");
	#endif
	eXosip_add_authentication_info("77001234", "77001234","654987", NULL, NULL);
}

/////////////////////////////////////////
//
/////////////////////////////////////////
void reRegRefrsh()
{
	printf("reRegRefrsh()\n");
}
/////////////////////////////////////////
//
/////////////////////////////////////////
void reProcCall()
{
	#ifdef FENG_DEBUG
	printf("reProcCall()\n");
	#endif
	printf ("proceeding!\n");
}
/////////////////////////////////////////
//
/////////////////////////////////////////
void reRingCall()
{
	#ifdef FENG_DEBUG
	printf("reRingCall()\n");
	#endif
	printf ("ringing!\n");
	printf ("mySipUA.call_id is %d, mySipUA.dialog_id is %d \n", mySipUA.je->cid, mySipUA.je->did);
}

/////////////////////////////////////////
//
/////////////////////////////////////////
void reAnswerCall()
{
	int ret;
	
	#ifdef FENG_DEBUG
	printf("reAnswerCall()\n");
	#endif
	
	mySipUA.call_id = mySipUA.je->cid;
	mySipUA.dialog_id = mySipUA.je->did;
	printf ("mySipUA.call_id is %d, mySipUA.dialog_id is %d \n", mySipUA.je->cid, mySipUA.je->did);
	eXosip_lock();
	eXosip_call_build_ack (mySipUA.je->did, &mySipUA.ack);
	eXosip_call_send_ack (mySipUA.je->did, mySipUA.ack);
	
	mySipUA.remote_sdp = eXosip_get_remote_sdp (mySipUA.je->did);
	mySipUA.speech_con_req = eXosip_get_audio_connection(mySipUA.remote_sdp);
	if(!mySipUA.speech_con_req)
	{
		printf("Can't get audio connection!\n");
		myQuit();
		return;
	}
	mySipUA.speech_md_req = eXosip_get_audio_media(mySipUA.remote_sdp);
	if(!mySipUA.speech_md_req)
	{
		printf("Can't get media!\n");
		myQuit();
		return;
	}
	mySipUA.payload_str = (char *)osip_list_get(&mySipUA.speech_md_req->m_payloads, 0);
	if(!mySipUA.payload_str)
	{
		printf("Can't get mySipUA.payload_str!\n");
		myQuit();
		return;
	}
	eXosip_unlock();
	ret = build_media(0);
	if(!ret)
	{
		printf("Media Build Failure!\n");
		return;
	}
	printf ("ok! connected!\n");
	mySipUA.rtp_start=1;
}
/////////////////////////////////////////
//
/////////////////////////////////////////
void *SipThread()
{
        mySipUA.flag2=1;
        printf("\nSipThread starts OK!\n");
        while(mySipUA.flag2)
        {
			mySipUA.je = eXosip_event_wait (0,50);

            eXosip_lock ();
            eXosip_default_action (mySipUA.je);
            eXosip_automatic_refresh ();
            eXosip_unlock ();

            if (mySipUA.je == NULL)
                continue;
            printf("\n--------------------------------------------------");
            printf("\nSipThread: New Event!\n%s\n", mySipUA.event_type[mySipUA.je->type]);
            switch (mySipUA.je->type)
            {
                case EXOSIP_MESSAGE_NEW:		reNewMsg();		break;
                case EXOSIP_CALL_INVITE:		reInviteCall(); break;
                case EXOSIP_CALL_PROCEEDING:	reProcCall();	break;
				case EXOSIP_CALL_RINGING:		reRingCall();	break;
				case EXOSIP_CALL_ANSWERED:		reAnswerCall();	break;
                case EXOSIP_CALL_ACK:			reAckCall();	break;
                case EXOSIP_CALL_CLOSED:		reCloseCall();	break;
                case EXOSIP_CALL_MESSAGE_NEW:	reNewMsgCall(); break;
				case EXOSIP_REGISTRATION_SUCCESS: reRegOK();	break;
				case EXOSIP_REGISTRATION_FAILURE: reRegFail();	break;
				case EXOSIP_REGISTRATION_REFRESHED: reRegRefrsh();break;
				case EXOSIP_CALL_RELEASED:;break;
                default: 
			printf ("Could not parse the msg!\n");
            }
	}
	
	pthread_exit("Thank you for your CPU time!\n"); 
 }
 
 //////////////////////////////////////
 //	
 //////////////////////////////////////
 void *RtpThread()
 {
 	mySipUA.flag3=1;
 	printf("RTP thread starts!\n");
 	while(mySipUA.flag3)
 	{
 		while( (!mySipUA.rtp_start) && mySipUA.flag3)
 			sleep(1);
		build_media(1);
 		while(mySipUA.rtp_start && mySipUA.flag3)
 		{
 			sleep(1);
 		}
		destroy_media(1);
 	}
 	printf("RTP thread quit successfully!\n");
 }
 	
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
int sipMainInit(void)
{
    int res; 
    char input1[10];
    
    myInit();
    printf("%s", MENU);

	// Create SIP receive thread
    res = pthread_create(&mySipUA.sip_thread, NULL, SipThread, NULL); 
    if (res != 0) 
    { 
        perror("SipThread creation failed!"); 
        exit(EXIT_FAILURE); 
    } 
    res = pthread_create(&mySipUA.rtp_thread, NULL, RtpThread, NULL); 
    if (res != 0) 
    { 
        perror("RtpThread creation failed!"); 
        exit(EXIT_FAILURE); 
    } 
    eXosip_guess_localip (AF_INET, mySipUA.localip, 128);
    printf("local IP address is %s\n", mySipUA.localip);
    myRegister(1);
    mySipUA.flag = 1;
}

void getSipCommand(void)
{
    while (mySipUA.flag&&(!gblGetQuit()))
     {
     	    printf("(fengUAC) ");fflush(stdout);
            if (fgets(input1, sizeof(input1), stdin) == NULL) 
            {
	    		puts("EOF while reading stdin, will quit now..");
	    		break;
			 }
            switch (input1[0]) 
            {
				case 'r': myRegister(1); 	break;
				case 'i': myInvite();	break;
				case 'h': myHoldUp();	break;
				case 'c': myRegister(0);break;
				case 's': myInfo();		break;
				case 'm': myMsg();	break;
				case 'q': myQuit();		break;
				case 'a': system("clear"); break;
				case 'd': printf("%s", MENU);break;
				case 'y': mySipUA.ansCall='y'; sleep(2);break;
				default: break;
            }
            mySipUA.ansCall='n';
    }
}
void sipQuit(void)
{
	void *sip_thread_result;
	void *rtp_thread_result;
	int res;

     res= pthread_join(mySipUA.sip_thread, &sip_thread_result); 
	 if (res != 0) 
    { 
        perror("SipThread join failed!\n"); 
        exit(EXIT_FAILURE); 
    } 
	 res= pthread_join(mySipUA.rtp_thread, &rtp_thread_result); 
     if (res != 0) 
    { 
        perror("RtpThread join failed!\n"); 
        exit(EXIT_FAILURE); 
    } 
    printf("SIP and RTP Thread Exit Successfully!\n");
    pthread_mutex_destroy(&mySipUA.mutex);
    eXosip_quit ();
    return (0);
}

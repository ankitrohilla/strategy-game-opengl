#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <arpa/inet.h>
//#include "variables.h"

#define SERVER "255.255.255.255"
#define BUFLEN 512
#define PORT 6767
#define PORT2 6868

int n = 0;

void die(char *s)
{
    perror(s);
    exit(1);
}

char* get_my_ip()
{
//    strcpy(remote_ip , my_ip);

    struct ifaddrs *addrs, *tmp;
    getifaddrs(&tmp);
    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET && (!strcmp(tmp->ifa_name, "eth0") || !strcmp(tmp->ifa_name, "wlan0") || !strcmp(tmp->ifa_name , "p3p1")) )
        {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            return inet_ntoa(pAddr->sin_addr);
        }
        tmp = tmp->ifa_next;
    }
    printf("Unable to obtain IP");
}

bool checkAllSet()
{
    int playerCount = 0 , heroCount = 0 ;
    for(int i=0 ; i<4 ;i++)
    {
        if(isPlayerChosen[i])
            playerCount++;
        if(isHeroChosen[i])
            heroCount++;
    }

    if(playerCount < players || heroCount < players )
        return false;
    else return true;

}


void setForBots()
{
    for(int i = 0 ; i<4 ; i++)
    {
        if( !isPlayerChosen[i] )
        {
            isPlayerChosen[i] = true;
            yesBot[i] = true;
            for(int j = 0 ; j<4 ; j++ )
            {
                if( !isHeroChosen[j] )
                {
                    isHeroChosen[j] = true;
                    heroSelected[i] = char(j+65);
                    break;
                }
            }
        }
    }
}


void initializeClient()
{

    std::cout<<"initiaklize client started\n";fflush(stdout);
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other),yes = 1;
    char buf[BUFLEN];
    char message[BUFLEN];
 
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
   
    int true1 = 1;
    setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &true1, sizeof(int));
     
    if(setsockopt(s,SOL_SOCKET,SO_BROADCAST,&yes,sizeof(int))==-1)
    {
    die("setsockopt");
    //return 0;
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
 
    while(1)
    {
        printf(" I am client \n ");
        fflush(stdout);
        strcpy(message , "255.255.255.255");
         
        //send the message
        if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        {
            die("sendto()");
        }
         

         std::cout<<"sent hello\n";
        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFLEN);
        //try to receive some data, this is a blocking call
        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other,(socklen_t*) &slen) == -1)
        {
            die("recvfrom()");
        }
        std::cout<<"received message from server :  "<<buf<<"\n";
        
        puts(buf);
        strcpy(remoteIp,buf);
        break;
    }
 
    close(s);
    //return 0;

}


void* threadClient( void* )
{
    struct hostent *HostEntPtr;
    char Hostname[100];
    char chosenHeroId[2];
    int j_holder = 0 , c=0;
    char adjustChar[2];

    memset(adjustChar , 0 , sizeof(adjustChar));

    struct sockaddr_in serv_addr;

    struct in_addr in;

    int sockfd;
    std::cout<<"thread client stared\n"; fflush(stdout);


    if(!isServer)
        initializeClient();


    while(1)
    {
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
        {
            printf("\n-- Error : Could not create socket \n");
            //return 1;
        }
  //    else printf("NO RROR");

         memset(&serv_addr, 0, sizeof(serv_addr));

//        // these 2 lines are for testing
//        strcpy(myIp , get_my_ip());
//        strcpy( remoteIp , myIp );

  //    std::cout << remoteIp << " " << myIp;
    //    fflush(stdout);

        serv_addr.sin_family = AF_INET;
        int p = PORT;
        serv_addr.sin_port = htons(p);
        serv_addr.sin_addr.s_addr = inet_addr( remoteIp );

        while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0);

  //    printf("CONNECTD");
        fflush(stdout);

      // I need to send my data to the server
        if( !isServer )
        {
            memset(clientMessage,0,sizeof(clientMessage));
            strcpy(clientMessage , "ready");
        }
   //   printf("CONNECTD");
        fflush(stdout);

        if( send(sockfd , clientMessage , sizeof(clientMessage) , 0) < 0)
        {
            puts("Send failed");
        }
        else
        {
            printf("SENT MESSAGE %s\n\n", clientMessage);fflush(stdout);
        }

        memset(clientReceived, 0, sizeof(clientReceived));

        n = read(sockfd, clientReceived, sizeof(clientReceived));

        printf("GOT REPLY %s\n\n",clientReceived);

//      std::cout << clientReceived << "\n";

      // update the states of each hero including myself ( I don't know who is attacking me )
        
        if( !strcmp(clientReceived , "yes1") || !strcmp(clientReceived , "yes2") || !strcmp(clientReceived , "yes3") || !strcmp(clientReceived , "yes4") )
        {
            players = clientReceived[3] - 48;
            close(sockfd);
            screen = 2;
            init();
            glutTimerFunc(5,animate,5);
            break;
        }

        close(sockfd); // at the end
        usleep(5000);
    }

    // ///////////////////////////////changes done here //////////////////////////

    while(1)
    {
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
        {
            printf("\n-- Error : Could not create socket \n");
            //return 1;
        }
  //    else printf("NO RROR");

         memset(&serv_addr, 0, sizeof(serv_addr));

//        // these 2 lines are for testing
//        strcpy(myIp , get_my_ip());
//        strcpy( remoteIp , myIp );

  //    std::cout << remoteIp << " " << myIp;
    //    fflush(stdout);

        serv_addr.sin_family = AF_INET;
        int p = PORT;
        serv_addr.sin_port = htons(p);
        serv_addr.sin_addr.s_addr = inet_addr( remoteIp );

        while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0);

  //    printf("CONNECTD");
        fflush(stdout);

      // I need to send my data to the server
        if( !isServer )
        {
            memset(clientMessage,0,sizeof(clientMessage));

            if(myHeroId == -1)
                strcpy(clientMessage , "N");
            else
                sprintf(clientMessage , "%d" , myHeroId);

            if(myChosenHero == NULL)
                strcat(clientMessage , "N");
            else
            {
                adjustChar[0] = myChosenHero->heroName;
                strcat(clientMessage , adjustChar);
                //strcat(clientMesaage , "  "); //to take care of last space
            }
        }
   //   printf("CONNECTD");
        fflush(stdout);

        if( send(sockfd , clientMessage , sizeof(clientMessage) , 0) < 0)
        {
            puts("Send failed");
        }
        else
        {
            printf("SENT MESSAGE %s\n\n", clientMessage);fflush(stdout);
        }

        memset(clientReceived, 0, sizeof(clientReceived));

        n = read(sockfd, clientReceived, sizeof(clientReceived));

        printf("GOT REPLY %s\n\n",clientReceived);

        std::cout << clientReceived << "\n";

      // update the states of each hero including myself ( I don't know who is attacking me )

        // process clientReceived

  //        if(!strcmp(clientReceived,"allSet"))
    //    {
      //      close(sockfd);
        //    break;
        //}

        j_holder = 0 ; c = 0;


        for( int i = 0 ; clientReceived[i] != 32; i += j_holder )
        {                                                         // abc bhr jhsdf
            for( int j = 0; clientReceived[i+j] != 32; j++ )
            {
                switch (c)
                {
                    case 0:
                        if(clientReceived[i+j] != 'N')
                            isPlayerChosen[0] = true;
                        break;
                    case 1:
                        if(clientReceived[i+j] != 'N')
                            isPlayerChosen[1] = true;
                //        std::cout << c << " " << j << " " << isDead << "\n";
                        break;
                    case 2:
                        if(clientReceived[i+j] != 'N')
                            isPlayerChosen[2] = true;
              //          std::cout << c << " " << j << " " << heroX[j] << "\n";
                        break;
                    case 3:
                        if(clientReceived[i+j] != 'N')
                            isPlayerChosen[3] = true;
            //            std::cout << c << " " << j << " " << heroY[j] << "\n";
                        break;
                    case 4:
                        if(clientReceived[i+j] != 'N')
                        {
                            heroSelected[0] = clientReceived[i+j];
                            isHeroChosen[(int)(clientReceived[i+j]-65)] = true;
                        }
            //            std::cout << c << " " << j << " " << heroY[j] << "\n";
                        break;
                    case 5:
                        if(clientReceived[i+j] != 'N')
                        {
                            heroSelected[1] = clientReceived[i+j];
                            isHeroChosen[(int)(clientReceived[i+j]-65)] = true;

                        }
            //            std::cout << c << " " << j << " " << heroY[j] << "\n";
                        break;
                    case 6:
                        if(clientReceived[i+j] != 'N')
                        {
                            heroSelected[2] = clientReceived[i+j];
                            isHeroChosen[(int)(clientReceived[i+j]-65)] = true;
                        }
                        break;
                    case 7:
                        if(clientReceived[i+j] != 'N')
                        {
                            heroSelected[3] = clientReceived[i+j];
                            isHeroChosen[(int)(clientReceived[i+j]-65)] = true;
                        }
                        break;
                }
                j_holder = j;
            }
           // counter++;
            c++;
            j_holder += 2;
       }

        if(checkAllSet())
        {
            close(sockfd);
            break;
        }

        // ///////////////////////////////////////////////////

        /*    if(myHeroId != -1 && myChosenHero != NULL)
            {
                close(sockfd);
                break;
            }
        */
        close(sockfd); // at the end
        usleep(5000);
    }

    if(players < 4)
        setForBots();
    screen = 3;
    init();
    glutTimerFunc( 5, animate, 5);

    // //////////////////////////////////////////////////////////////////////////


    while(!isGamePlaySet);


    while(isGamePlaySet)
    {
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
        {
            printf("\n-- Error : Could not create socket \n");
            //return 1;
        }
  //    else printf("NO RROR");

         memset(&serv_addr, 0, sizeof(serv_addr));

//        // these 2 lines are for testing
//        strcpy(myIp , get_my_ip());
//        strcpy( remoteIp , myIp );

  //    std::cout << remoteIp << " " << myIp;
    //    fflush(stdout);

        serv_addr.sin_family = AF_INET;
        int p = PORT;
        serv_addr.sin_port = htons(p);
        serv_addr.sin_addr.s_addr = inet_addr( remoteIp );

        while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0);

  //    printf("CONNECTD");
        fflush(stdout);

      // I need to send my data to the server
        if( !isServer )
        {
            memset(clientMessage,0,sizeof(clientMessage));
            strcpy(clientMessage , "gamePlaySet");
        }
   //   printf("CONNECTD");
        fflush(stdout);

        if( send(sockfd , clientMessage , sizeof(clientMessage) , 0) < 0)
        {
            puts("Send failed");
        }
        else
        {
            printf("SENT MESSAGE %s\n\n", clientMessage);
        }

        memset(clientReceived, 0, sizeof(clientReceived));

        n = read(sockfd, clientReceived, sizeof(clientReceived));

        printf("GOT REPLY %s\n\n",clientReceived); fflush(stdout);

//      std::cout << clientReceived << "\n";

      // update the states of each hero including myself ( I don't know who is attacking me )
        
        if(!strcmp(clientReceived , "gamePlaySet"))
        {
            close(sockfd);
            break;
        }

        close(sockfd); // at the end
        usleep(5000);

    }


   std::cout<<"client came out of gamePlayset loop\n";




    while(1)
    {
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
        {
            printf("\n-- Error : Could not create socket \n");
            //return 1;
        }
  //    else printf("NO RROR");

         memset(&serv_addr, 0, sizeof(serv_addr));

//        // these 2 lines are for testing
//        strcpy(myIp , get_my_ip());
//        strcpy( remoteIp , myIp );

  //    std::cout << remoteIp << " " << myIp;
        fflush(stdout);

        serv_addr.sin_family = AF_INET;
        int p = PORT;
        serv_addr.sin_port = htons(p);
        serv_addr.sin_addr.s_addr = inet_addr( remoteIp );

        while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0);

  //    printf("CONNECTD");
        fflush(stdout);

      // I need to send my data to the server
        if( !isServer )
        {
            memset(clientMessage,0,sizeof(clientMessage));
            setClientMessage();
        }
   //   printf("CONNECTD");
        fflush(stdout);

        if( send(sockfd , clientMessage , sizeof(clientMessage) , 0) < 0)
        {
            puts("Send failed");
        }
        else
        {
            if( heroes[myHeroId]->isInvincible )
                printf("SENT MESSAGE %s\n\n", clientMessage);
        }

        memset(clientReceived, 0, sizeof(clientReceived));

        n = read(sockfd, clientReceived, sizeof(clientReceived));

  //    printf("GOT REPLY %s\n\n",clientReceived);

//      std::cout << clientReceived << "\n";

      // update the states of each hero including myself ( I don't know who is attacking me )
        for( int i = 0 ; i < 4; i++ )
        {
            // Format is <Id><Is Dead><X><Y><Row><Col><State><Health>
            clientProcessReply( i );
        }

//    std::cout<<"templehealth 0  nd 1 ----   "<<teams[0].currentHealth<<"    "<<teams[1].currentHealth<<"\n";
        fflush(stdout);

        close(sockfd); // at the end
        usleep(5000);
    }

    return NULL;
}

void* initializeServer(void*) 
{
    initializeStarted = true;
    struct sockaddr_in si_me, si_other;
     
    int i, slen = sizeof(si_other) , recv_len;
    char buf[BUFLEN];
    //int players = 1 , timer = 100;

    memset(buf , 0 , sizeof(buf));
   
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    int true1 = 1;
    setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &true1, sizeof(int));
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    memset(myIp , 0 , sizeof(myIp));
    strcpy(myIp , get_my_ip());

    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    while(1)
    {
        printf("Waiting for connections...");
        fflush(stdout);
        memset(buf , 0 , sizeof(buf));
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other,(socklen_t*) &slen)) == -1)
        {
            die("recvfrom()");
        }
        
        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Data: %s\n" , buf);
         
        if(!strcmp(buf,"255.255.255.255"))
        {
            memset(remoteIp , 0 , sizeof(remoteIp));
            sprintf(remoteIp , "%s" , inet_ntoa(si_other.sin_addr));
            memset(buf , 0 , sizeof(buf));
            strcpy(buf , myIp);
        //strcpy(buf , "vaishali gupta");
            std::cout<<"i sent my ip as  : " <<buf<<"\n";
            //now reply the client with the your IP address
                
            if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
            {
                die("sendto()");
            }
            else players++;
            if(players == 4) return NULL;
        }
    }
}


void* threadServer( void* )  //  start here.........................................
{

    struct hostent *HostEntPtr;
    struct in_addr in;
    char Hostname[100];
    char temple0Health[5] , temple1Health[5];
    char itemId[4][2] ;
    char itemRow[4][3] ;
    char itemCol[4][3] ;
    char timeToDisplayItem[4][3];

    char adjustChar[2];

    memset(adjustChar, 0 , sizeof(adjustChar));

    int timer = 20;

    int listenfd = 0 , n = 0, connfd = 0 ;

    struct sockaddr_in serv_addr;

    pthread_create(&initializeServerThread , NULL , initializeServer , NULL);

    int port = PORT;

    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        printf("Error, unable to create socket");
    else{;}
   //     printf("Server socket created");

    int true1 = 1;
    setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &true1, sizeof(int));

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_addr.sin_port = htons(port);



    while(!initializeStarted);
    while(timer--) { usleep(500000) ; }
    pthread_cancel(initializeServerThread);
    std::cout<<"pthread cancelled \n";

    if(players == 1)
    {
         whichGameMode = SINGLE_PLAYER;
         screen = 2;
         init();
         glutTimerFunc(5,animate,5);
         pthread_cancel(serverThreadId);

    }

    std::cout<<"players : "<<players<<" \n";

    screen = 2;
    init();
    glutTimerFunc(5,animate,5);
    close(s);

    if( bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0 )
    {
        puts("\n-- Trying to connect . . . . . .\n-- Unable to bind, please try another port");
        fflush(stdout);
        //deleteThread(getID());
    }
    else
    {
    //    puts("SERVER _ BIND DONE");
    }

    if(listen(listenfd, 10) == -1){
        printf("Failed to listen\n");
        return NULL;
    }


    while(1)
    {
        int clientId , itemCount = 0;

  //      puts("before  connfd");
        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
   //     puts("after  connfd");
  //      puts("ready to receive data");
        n=recv(connfd, serverReceived, sizeof(serverReceived), 0);
   //     std::cout << "SERVER received this " << serverReceived << "\n\n";

        // ASCII code of '0' is 48
        // Sending the ID of the sender hero
        // Updates the server values to the values provided by the client

        if(!strcmp(serverReceived , "ready"));
        else if(serverReceived[1] == 'A' || serverReceived[1] == 'B' || serverReceived[1] == 'C' || serverReceived[1] == 'D' || serverReceived[1] == 'N')
        {
            if(serverReceived[1] != 'N')
            {
                isPlayerChosen[ (int)(serverReceived[0] - 48) ] = true;
                heroSelected[ (int)(serverReceived[0] - 48) ] = serverReceived[1];
                isHeroChosen[ (int)(serverReceived[1] - 65) ] = true;
            }
        }
        else if(!strcmp(serverReceived , "gamePlaySet") && !isGamePlaySet);
        else if(!strcmp(serverReceived , "gamePlaySet") && isGamePlaySet );
        else if(isGamePlaySet)
        {
            clientId = serverReceived[0] - 48;
            serverProcessReceived( clientId );
        }
        memset(serverReply,0,sizeof(serverReply));


        if(!strcmp(serverReceived , "ready"))
        {
            strcpy(serverReply , "yes");
            adjustChar[0] = players+48;
            strcat(serverReply ,adjustChar);
        }
        else if(serverReceived[1] == 'A' || serverReceived[1] == 'B' || serverReceived[1] == 'C' || serverReceived[1] == 'D' || serverReceived[1] == 'N')
        {
            isPlayerChosen[ serverReceived[0]-48 ] = true;
            heroSelected[ serverReceived[0]-48 ] = serverReceived[1];
            isHeroChosen[serverReceived[1]-65] = true;

            for(int i = 0 ; i < 4 ; i++)
            {
                if(isPlayerChosen[i] == true )
                {
                    memset(adjustChar , 0 , sizeof(adjustChar));
                    adjustChar[0] = i+48;
                    strcat(serverReply, adjustChar);
                }
                else
                    strcat(serverReply, "N");
                strcat(serverReply , " ");
            }

            for(int j = 0 ; j < 4 ; j++)
            {
                if(heroSelected[j] != 'N')
                {
                    memset(adjustChar , 0 , sizeof(adjustChar));
                    adjustChar[0] = heroSelected[j];
                    strcat(serverReply , adjustChar);
                }
                else
                    strcat(serverReply , "N");
                strcat(serverReply , " ");
            }

            strcat(serverReply , " ");

            if(checkAllSet() && !enteredCheckAllSet)
            {
                std::cout<<"came here in check all set condition \n\n";fflush(stdout);

                if(players < 4)
                    setForBots();
                enteredCheckAllSet = true;
                screen = 3;
                init();
                glutTimerFunc( 5 ,animate, 5);
            }

        }
        else if(!strcmp(serverReceived ,"gamePlaySet") && !isGamePlaySet)
        {
            strcpy(serverReply , "gamePlayNotSet");
        }
        else if(!strcmp(serverReceived , "gamePlaySet") && isGamePlaySet)
        {
            strcpy(serverReply , "gamePlaySet");
        }
        else if( isGamePlaySet)
        {   
            for( int i = 0; i < 4; i++)
            {
                // Format is <Id><Is Dead><X><Y><Row><Col><State><Health>
                // Send stats of each hero as a reply
                setServerReply( i );
            }

            // send <S/N><D/N><B/N><V/N><(0-999)> after general information, client can be victim of all 4 together

            if( heroes[clientId]->isSlowed )     strcat(serverReply,"S ");
            else                                 strcat(serverReply,"N ");

            if( heroes[clientId]->isDisabled )   strcat(serverReply,"D ");
            else                                 strcat(serverReply,"N ");

            if( heroes[clientId]->isBlackouted ) strcat(serverReply,"B ");
            else                                 strcat(serverReply,"N ");

            // send info to all those except the one who induced vandalism
            if( heroes[clientId]->isVandalized )
            {
                // to make sure to send the message only once
                heroes[clientId]->isVandalized = false;
                char vandalizedTileId[4];
                sprintf(vandalizedTileId, "%d", heroes[clientId]->vandalizedTileId);

                strcat(serverReply,"V ");
                strcat(serverReply,vandalizedTileId);
                strcat(serverReply," ");
                std::cout<<" \n server replied   "<<serverReply<<"\n";

            }
            else strcat(serverReply,"N 0 ");

            //items info sent

            memset(itemId            ,0,sizeof(itemId));
            memset(itemRow           ,0,sizeof(itemRow));
            memset(itemCol           ,0,sizeof(itemCol));
            memset(timeToDisplayItem ,0,sizeof(timeToDisplayItem));

            for( itemMap::const_iterator it = teams[clientId/2].itemSet.begin(); it != teams[clientId/2].itemSet.end(); it++)
            {
                if(it->second->timeToDisplay <=0)
                {
                   sprintf(itemId[itemCount] ,"%d" , (int)it->second->whichItem);
                   sprintf(itemRow[itemCount],"%d" , (int)it->second->row);
                   sprintf(itemCol[itemCount],"%d" , (int)it->second->col);
                   sprintf(timeToDisplayItem[itemCount] ,"%d" , (int)it->second->timeToDisplay);
                   itemCount++;
                }
             }

             while( itemCount<=4 )
             {
                   sprintf(itemId[itemCount] ,"%d" , 9); // invalid item id
                   sprintf(itemRow[itemCount],"%d" , 99);
                   sprintf(itemCol[itemCount],"%d" , 99);
                   sprintf(timeToDisplayItem[itemCount] ,"%d" , 11);
                   itemCount++;
             }

             for(int k = 0; k<4 ; k++)
             {
                 strcat(serverReply, itemId[k]);
                 strcat(serverReply," ");
                 strcat(serverReply, itemRow[k]);
                 strcat(serverReply," ");
                 strcat(serverReply, itemCol[k]);
                 strcat(serverReply," ");
                 strcat(serverReply, timeToDisplayItem[k]);
                 strcat(serverReply," ");
             }


            // send temple's health at the end
            sprintf(temple0Health , "%d" , teams[0].currentHealth);
            sprintf(temple1Health , "%d" , teams[1].currentHealth);

            strcat(serverReply, temple0Health);
            strcat(serverReply," ");
            strcat(serverReply, temple1Health);
            strcat(serverReply," ");
            if( !Exit )
                strcat(serverReply,"F");
            else
                strcat(serverReply,"T");
            strcat(serverReply , "  ");
        }

        write(connfd, serverReply, strlen(serverReply));
        if(Exit) exit(1);
        close(connfd);
    }
    return NULL;
}


void setClientMessage()
{
    char myId[2];   // 0,1,2,3
    char isDead[2]; // 0/1
    char myX[6];    // -13.28 will be multiplied by 10 -> -132 etc
    char myY[5];    // -8.22  will be multiplied by 10 -> -82  etc
    char myRow[3];  // 0 - 31
    char myCol[3];  // 0 - 17
    char isAttackingTemple[2];
    char isAttackingHero[2];
    char gettingAttackedHeroId[2];
    char attackPower[2]; // attackPower
    char myState[3];
    char health[6];
    char stunVictimId[2];
    char enemy1Id[2];
    char enemy2Id[2];
    char myTileId[4];

    memset(myId                 ,0,sizeof(myId));
    memset(isDead               ,0,sizeof(isDead));
    memset(myX                  ,0,sizeof(myX));
    memset(myY                  ,0,sizeof(myY));
    memset(myRow                ,0,sizeof(myRow));
    memset(myCol                ,0,sizeof(myCol));
    memset(isAttackingTemple    ,0,sizeof(isAttackingTemple));
    memset(isAttackingHero      ,0,sizeof(isAttackingHero));
    memset(gettingAttackedHeroId,0,sizeof(gettingAttackedHeroId));
    memset(attackPower          ,0,sizeof(attackPower));
    memset(myState              ,0,sizeof(myState));
    memset(health               ,0,sizeof(health));
    memset(stunVictimId         ,0,sizeof(stunVictimId));
    memset(enemy1Id             ,0,sizeof(enemy1Id));
    memset(enemy2Id             ,0,sizeof(enemy2Id));
    memset(myTileId             ,0,sizeof(myTileId));

    sprintf( enemy1Id , "%d", enemyHero1Id );
    sprintf( enemy2Id , "%d", enemyHero2Id );

    if( heroes[myHeroId]->whichState == ATTACKING_TEMPLE )
    {
        strcpy(isAttackingTemple,"1");
        strcpy(isAttackingHero,"0");
        strcpy(gettingAttackedHeroId,"4");
        //sprintf( gettingAttackedHealth , "%d", teams[enemyTeamId].currentHealth );
    }
    else if( heroes[myHeroId]->whichState == ATTACKING_ENEMY_1 )
    {
        strcpy(isAttackingTemple,"0");
        strcpy(isAttackingHero,"1");
        sprintf( gettingAttackedHeroId , "%d", enemyHero1Id );
        //sprintf( gettingAttackedHealth , "%d", heroes[enemyHero1Id]->current_health );
    }
    else if( heroes[myHeroId]->whichState == ATTACKING_ENEMY_2 )
    {
        strcpy(isAttackingTemple,"0");
        strcpy(isAttackingHero,"1");
        sprintf( gettingAttackedHeroId , "%d", enemyHero2Id );
        //sprintf( gettingAttackedHealth , "%d", heroes[enemyHero2Id]->current_health );
    }
    else
    {
        strcpy(isAttackingTemple,"0");
        strcpy(isAttackingHero,"0");
        strcpy(gettingAttackedHeroId,"4");        // 4 means NA
        //strcpy(gettingAttackedHealth,"9999");     // 9999 means NA
    }

    sprintf( myId                  , "%d", (int)heroes[myHeroId]->id                );
    sprintf( isDead                , "%d", (int)heroes[myHeroId]->isDead            );
    sprintf( myX                   , "%d", (int)(heroes[myHeroId]->x*100)           );
    sprintf( myY                   , "%d", (int)(heroes[myHeroId]->y*100)           );
    sprintf( myRow                 , "%d", (int)(heroes[myHeroId]->row)             );
    sprintf( myCol                 , "%d", (int)(heroes[myHeroId]->col)             );
    sprintf( myState               , "%d", (int)(heroes[myHeroId]->whichState)      );
    sprintf( myTileId              , "%d", (int)(heroes[myHeroId]->vandalizedTileId));
    sprintf( attackPower           , "%d", (int)(heroes[myHeroId]->attack)          );

    strcat(clientMessage, myId);
    strcat(clientMessage," ");
    strcat(clientMessage, isDead);
    strcat(clientMessage," ");
    strcat(clientMessage, myX);
    strcat(clientMessage," ");
    strcat(clientMessage, myY);
    strcat(clientMessage," ");
    strcat(clientMessage, myRow);
    strcat(clientMessage," ");
    strcat(clientMessage, myCol);
    strcat(clientMessage," ");
    strcat(clientMessage, isAttackingTemple);
    strcat(clientMessage," ");
    strcat(clientMessage, isAttackingHero);
    strcat(clientMessage," ");
    strcat(clientMessage, gettingAttackedHeroId);
    strcat(clientMessage," ");
    strcat(clientMessage, attackPower);
    strcat(clientMessage," ");
    strcat(clientMessage, myState);
    strcat(clientMessage," ");

    if( heroes[myHeroId]->isInvincible )  strcat(clientMessage,"I ");
    else                                  strcat(clientMessage,"N ");

    // send message if I am Slowing/Disabling/Blackouting/Vandalizing others or not affecting
    // (S/D/B/V/N) (0-3)/(0-3)/(4)/(0-999)/(0) to be sent where N means not doing anything
    if( heroes[myHeroId]->isSlowing )
    {
        strcat(clientMessage, "S ");

        if( heroes[enemyHero1Id]->isSlowed )
            strcat(clientMessage, enemy1Id);
        else if( heroes[enemyHero2Id]->isSlowed )
            strcat(clientMessage, enemy2Id);

        strcat(clientMessage, " ");
    }
    else if( heroes[myHeroId]->isDisabling )
    {
        strcat(clientMessage, "D ");

        if( heroes[enemyHero1Id]->isDisabled )
            strcat(clientMessage, enemy1Id);
        else if( heroes[enemyHero2Id]->isSlowed )
            strcat(clientMessage, enemy2Id);

        strcat(clientMessage, " ");
    }
    else if( heroes[myHeroId]->isBlackouting )
    {
        // blackout both enemies
        strcat(clientMessage, "B 4 ");
    }
    else if( heroes[myHeroId]->hasVandalized )
    {
        // send tile id from where I gave burst damage
        heroes[myHeroId]->hasVandalized = false;
        strcat(clientMessage, "V "    );
        strcat(clientMessage, myTileId);
        strcat(clientMessage, " "     );
    }
    else
    {
        // no power in use
        strcat(clientMessage, "N 0 ");
    }

    // send my health if locked, send enemy's state if he is stunned
    if( heroes[myHeroId]->healthLock )
    {
        sprintf( health, "%d", (int)(heroes[myHeroId]->current_health));
        strcat(clientMessage, "H ");
        strcat(clientMessage, health);
        strcat(clientMessage," ");
//        std::cout << "Updated health sent " << clientMessage << "\n";
    }
    else if( teams[myTeamId].healthLock )
    {
        sprintf( health, "%d", (int)(teams[myTeamId].currentHealth));
        strcat(clientMessage, "T ");
        strcat(clientMessage, health);
        strcat(clientMessage," ");
    }
    else if( heroes[enemyHero1Id]->whichState == STUNNED )
    {
        strcat(clientMessage, "S ");
        sprintf( stunVictimId, "%d", (int)enemyHero1Id);
        strcat(clientMessage, stunVictimId);
        strcat(clientMessage," ");
    }
    else if( heroes[enemyHero2Id]->whichState == STUNNED )
    {
        strcat(clientMessage, "S ");
        sprintf( stunVictimId, "%d", (int)enemyHero2Id);
        strcat(clientMessage, stunVictimId);
        strcat(clientMessage," ");
    }

    strcat(clientMessage," ");

}


void setServerReply( int currentHeroId )
{
    char heroId[2];   // 0,1,2,3
    char isDead[2]; // 0/1
    char heroX[6];    // -13.28 will be multiplied by 10 -> -132 etc
    char heroY[5];    // -8.22  will be multiplied by 10 -> -82  etc
    char heroRow[3];  // 0 - 31
    char heroCol[3];  // 0 - 17
    char heroState[3];// STANDING / ATTACKING_TEMPLE etc
    char heroHealth[5]; // health of hero

    memset(heroId        ,0,sizeof(heroId));
    memset(isDead        ,0,sizeof(isDead));
    memset(heroX         ,0,sizeof(heroX));
    memset(heroY         ,0,sizeof(heroY));
    memset(heroRow       ,0,sizeof(heroRow));
    memset(heroCol       ,0,sizeof(heroCol));
    memset(heroState     ,0,sizeof(heroState));
    memset(heroHealth    ,0,sizeof(heroHealth));

    sprintf( heroId                , "%d", (int)currentHeroId                       );
    sprintf( isDead                , "%d", (int)heroes[currentHeroId]->isDead       );
    sprintf( heroX                 , "%d", (int)(heroes[currentHeroId]->x*100)      );
    sprintf( heroY                 , "%d", (int)(heroes[currentHeroId]->y*100)      );
    sprintf( heroRow               , "%d", (int)(heroes[currentHeroId]->row)        );
    sprintf( heroCol               , "%d", (int)(heroes[currentHeroId]->col)        );
    sprintf( heroState             , "%d", (int)(heroes[currentHeroId]->whichState) );
    sprintf( heroHealth            , "%d", (int)(heroes[currentHeroId]->current_health) );

    strcat(serverReply, heroId);
    strcat(serverReply," ");
    strcat(serverReply, isDead);
    strcat(serverReply," ");
    strcat(serverReply, heroX);
    strcat(serverReply," ");
    strcat(serverReply, heroY);
    strcat(serverReply," ");
    strcat(serverReply, heroRow);
    strcat(serverReply," ");
    strcat(serverReply, heroCol);
    strcat(serverReply," ");
    strcat(serverReply, heroState);
    strcat(serverReply," ");
    strcat(serverReply, heroHealth);
    strcat(serverReply," ");

    // tell client about currentHeroId
    if( heroes[currentHeroId]->isInvincible ) strcat(serverReply,"I ");
    else                                      strcat(serverReply,"N "); 

    // append an extra space at the end

}

void clientProcessReply( int currentHeroId )
{
    int c = 0;
    int j_holder = 0;

    static int i = 0;


   // int counter = 0;
   // counter = (counter + 1)%50;

    int heroTeamId = currentHeroId/2;
    int heroEnemyTeamId = (1+heroTeamId)%2;

    char heroId[2] , heroHealth[5] , temple0Health[5] , temple1Health[5];
    char  heroX[6], heroY[5], heroRow[3], heroCol[3], vandalizedTileId[4];
    char itemId[4][2],itemRow[4][3] , itemCol[4][3] , timeToDisplayItem[4][3];

    float _heroX, _heroY;

    int _heroRow, _heroCol;
    itemDesc _itemId ;
    int _itemRow , _itemCol , _timeToDisplayItem ;

    int _hero_State , _heroHealth ;

    bool isVandalized = false;

    memset(heroId        , 0 , sizeof(heroId));
    memset(heroHealth    , 0 , sizeof(heroHealth));
    memset(temple0Health , 0 , sizeof(temple0Health));
    memset(temple1Health , 0 , sizeof(temple1Health));
    memset(heroX         , 0 , sizeof(heroX));
    memset(heroY         , 0 , sizeof(heroY));
    memset(heroRow       , 0 , sizeof(heroRow));
    memset(heroCol       , 0 , sizeof(heroCol));

    // Format is <Id><Is Dead><X><Y><Row><Col><State><Health><I/N>

    for(  ; clientReceived[i] != 32; i += j_holder )
    {                                                         // abc bhr jhsdf
        for( int j = 0; clientReceived[i+j] != 32; j++ )
        {
            switch (c)
            {
                case 0:
     //               std::cout << c << " " << j << " " << serverReceived[i+j] << "\n";
                    break;
                case 1:
                    heroes[ currentHeroId ]->isDead = (bool)(clientReceived[i+j] - 48);
            //        std::cout << c << " " << j << " " << isDead << "\n";
                    break;
                case 2:
                    heroX[j] = clientReceived[i+j];
          //          std::cout << c << " " << j << " " << heroX[j] << "\n";
                    break;
                case 3:
                    heroY[j] = clientReceived[i+j];
        //            std::cout << c << " " << j << " " << heroY[j] << "\n";
                    break;
                case 4:
                    heroRow[j] = clientReceived[i+j];
        //            std::cout << c << " " << j << " " << heroY[j] << "\n";
                    break;
                case 5:
                    heroCol[j] = clientReceived[i+j];
        //            std::cout << c << " " << j << " " << heroY[j] << "\n";
                    break;
                case 6:
                    _hero_State = clientReceived[i+j] - 48;
                    break;
                case 7:
                    heroHealth[j] = clientReceived[i+j];
                    break;
                case 8:
                    // don't listen to server if he is telling me about my invincibility
                    if( clientReceived[i+j] == 'I' && currentHeroId != myHeroId ) heroes[currentHeroId]->isInvincible = true;
                    if( clientReceived[i+j] == 'N' && currentHeroId != myHeroId ) heroes[currentHeroId]->isInvincible = false;
                    break;
            }
            j_holder = j;
        }
       // counter++;

        switch(c)
        {
            case 2:
                heroX[j_holder + 1] = 0;
                break;
            case 3:
                heroY[j_holder + 1] = 0;
                break;
            case 4:
                heroRow[j_holder + 1] = 0;
                break;
            case 5:
                heroCol[j_holder + 1] = 0;
                break;
            case 7:
                heroHealth[j_holder + 1] = 0;
                break;
        }
        c++;
        j_holder += 2;
        if(c == 9 )  { i += j_holder; break; }
    }
    _heroX = atof(heroX)/100.0;
    _heroY = atof(heroY)/100.0;
    _heroRow = atoi(heroRow);
    _heroCol = atoi(heroCol);
    _heroHealth = atoi(heroHealth);

   // heroes[ currentHeroId ]->isDead = isDead;

    if((currentHeroId != myHeroId) || (currentHeroId == myHeroId && heroes[myHeroId]->isDead) )
    {
        heroes[ currentHeroId ]->x = _heroX;
        heroes[ currentHeroId ]->y = _heroY;
        heroes[ currentHeroId ]->row = _heroRow;
        heroes[ currentHeroId ]->col = _heroCol;
    }

    if( heroes[myHeroId]->isDead )
    {
        switch(myHeroId)
        {
            case 0 :
                heroes[myHeroId]->x = hero0SpawnX;
                heroes[myHeroId]->y = hero0SpawnY;
                break;
            case 1 :
                heroes[myHeroId]->x = hero1SpawnX;
                heroes[myHeroId]->y = hero1SpawnY;
                break;
            case 2 :
                heroes[myHeroId]->x = hero2SpawnX;
                heroes[myHeroId]->y = hero2SpawnY;
                break;
            case 3 :
                heroes[myHeroId]->x = hero3SpawnX;
                heroes[myHeroId]->y = hero3SpawnY;
                break;
        }
    }

    heroes[ currentHeroId ]->whichState = (heroState)_hero_State;

    // if healths unlocked, modify, else don't modify
    // 99.99%, currentHeroId is myHeroId because client can only modify its own locks
    if( !heroes[currentHeroId]->healthLock && !heroes[currentHeroId]->isInvincible ) heroes[ currentHeroId ]->current_health = _heroHealth;

    // after general information has been processed
    if( currentHeroId == 3)
    {
        for(; clientReceived[i] != 32; i += j_holder )
        {                                                         // abc bhr jhsdf
            for( int j = 0; clientReceived[i+j] != 32; j++ )
            {
                switch (c)
                {
                    case 9:
                        // Am I slowed
                        if( clientReceived[i+j] == 'S' ) heroes[myHeroId]->isSlowed = true;
                        else                             heroes[myHeroId]->isSlowed = false;
                        break;
                    case 10:
                        // Am I Disabled
                        if( clientReceived[i+j] == 'D' ) heroes[myHeroId]->isDisabled = true;
                        else                             heroes[myHeroId]->isDisabled = false;
                        break;
                    case 11:
                        // Am I Blackouted
                        if( clientReceived[i+j] == 'B' ) heroes[myHeroId]->isBlackouted = true;
                        else                             heroes[myHeroId]->isBlackouted = false;
                        break;
                    case 12:
                        // Am I Vandalized or has my friend vandalized
                        // after I obtained information of whether I am vandalized or not
                        // I will do the processing in my thread
                        if( clientReceived[i+j] == 'V')
                        {
                            isVandalized = true;
                            heroes[myHeroId]->isVandalized = true;
                        }
                        break;
                    case 13:
                        // Tile Id of vandalized tile, if any
                        if( isVandalized ) vandalizedTileId[j] = clientReceived[i+j];
                        break;
                    case 14:
                        itemId[0][j] = clientReceived[i+j];
                        break;
                    case 15:
                        itemRow[0][j] = clientReceived[i+j];
                        break;
                    case 16:
                        itemCol[0][j] = clientReceived[i+j];
                        break;
                    case 17:
                        timeToDisplayItem[0][j] = clientReceived[i+j];
                        break;
                    case 18:
                        itemId[1][j] = clientReceived[i+j];
                        break;
                    case 19:
                        itemRow[1][j] = clientReceived[i+j];
                        break;
                    case 20:
                        itemCol[1][j] = clientReceived[i+j];
                        break;
                    case 21:
                        timeToDisplayItem[1][j] = clientReceived[i+j];
                        break;
                    case 22:
                        itemId[2][j] = clientReceived[i+j];
                        break;
                    case 23:
                        itemRow[2][j] = clientReceived[i+j];
                        break;
                    case 24:
                        itemCol[2][j] = clientReceived[i+j];
                        break;
                    case 25:
                        timeToDisplayItem[2][j] = clientReceived[i+j];
                        break;
                    case 26:
                        itemId[3][j] = clientReceived[i+j];
                        break;
                    case 27:
                        itemRow[3][j] = clientReceived[i+j];
                        break;
                    case 28:
                        itemCol[3][j] = clientReceived[i+j];
                        break;
                    case 29:
                        timeToDisplayItem[3][j] = clientReceived[i+j];
                        break;
                    case 30:
                        temple0Health[j] = clientReceived[i+j];
                        break;
                    case 31:
                        temple1Health[j] = clientReceived[i+j];
                        break;
                case 32:
                        if(clientReceived[i+j] == 'T')
                            exit(1);
                        break;

                }
                j_holder = j;
            }
           // counter++;

            switch(c)
            {
                case 13:
                    if( isVandalized ) vandalizedTileId[j_holder + 1] = 0;
                    break;
                case 14:
                    itemId[0][j_holder+1] = 0;
                    break;
                case 15:
                    itemRow[0][j_holder+1] = 0;
                    break;
                case 16:
                    itemCol[0][j_holder+1] = 0;
                    break;
                case 17:
                    timeToDisplayItem[0][j_holder+1] = 0;
                    break;
                case 18:
                    itemId[1][j_holder+1] = 0;
                    break;
                case 19:
                    itemRow[1][j_holder+1] = 0;
                    break;
                case 20:
                    itemCol[1][j_holder+1] = 0;
                    break;
                case 21:
                    timeToDisplayItem[1][j_holder+1] = 0;
                    break;
                case 22:
                    itemId[2][j_holder+1] = 0;
                    break;
                case 23:
                    itemRow[2][j_holder+1] = 0;
                    break;
                case 24:
                    itemCol[2][j_holder+1] = 0;
                    break;
                case 25:
                    timeToDisplayItem[2][j_holder+1] = 0;
                    break;
                case 26:
                    itemId[3][j_holder+1] = 0;
                    break;
                case 27:
                    itemRow[3][j_holder+1] = 0;
                    break;
                case 28:
                    itemCol[3][j_holder+1] = 0;
                    break;
                case 29:
                    timeToDisplayItem[3][j_holder+1] = 0;
                    break;
                case 30:
                    temple0Health[j_holder + 1] = 0;
                    break;
                case 31:
                    temple1Health[j_holder + 1] = 0;
                    break;
            }
            c++;
            j_holder += 2;
         }

        if( isVandalized ) heroes[myHeroId]->vandalizedTileId = atoi(vandalizedTileId);
//        std::cout << "VANDALIZED TIEL IS - " << heroes[myHeroId]->vandalizedTileId << "\n\n";
    

        if( atoi(itemId[0]) != 9 )
        {
            _itemId = (itemDesc)atoi(itemId[0]);
            _itemRow = atoi(itemRow[0]);
            _itemCol = atoi(itemCol[0]);
            _timeToDisplayItem = atoi(timeToDisplayItem[0]);
            heroes[myHeroId]->map[ _itemRow ][ _itemCol ].isItemPresent = true;
            teams[myTeamId].itemSet[(int)_itemId] = new item( _itemId , _itemRow , _itemCol , itemsTexture[_itemId] );
            teams[myTeamId].itemSet[(int)_itemId]->isPresent = true;
            teams[myTeamId].itemSet[(int)_itemId]->timeToDisplay = _timeToDisplayItem;
            heroes[myHeroId]->map[ _itemRow ][ _itemCol ].whichItem = *teams[myTeamId].itemSet[_itemId];

        }

        if( atoi(itemId[1]) != 9 )
        {
            _itemId = (itemDesc)atoi(itemId[1]);
            _itemRow = atoi(itemRow[1]);
            _itemCol = atoi(itemCol[1]);
            _timeToDisplayItem = atoi(timeToDisplayItem[1]);
            heroes[myHeroId]->map[ _itemRow ][ _itemCol ].isItemPresent = true;
            teams[myTeamId].itemSet[(int)_itemId] = new item( _itemId , _itemRow , _itemCol , itemsTexture[_itemId] );
            teams[myTeamId].itemSet[(int)_itemId]->isPresent = true;
            teams[myTeamId].itemSet[(int)_itemId]->timeToDisplay = _timeToDisplayItem;
            heroes[myHeroId]->map[ _itemRow ][ _itemCol ].whichItem = *teams[myTeamId].itemSet[_itemId];
        }

        if( atoi(itemId[2]) != 9 )
        {
            _itemId = (itemDesc)atoi(itemId[2]);
            _itemRow = atoi(itemRow[2]);
            _itemCol = atoi(itemCol[2]);
            _timeToDisplayItem = atoi(timeToDisplayItem[2]);
            heroes[myHeroId]->map[ _itemRow ][ _itemCol ].isItemPresent = true;
            teams[myTeamId].itemSet[(int)_itemId] = new item( _itemId , _itemRow , _itemCol , itemsTexture[_itemId] );
            teams[myTeamId].itemSet[(int)_itemId]->isPresent = true;
            teams[myTeamId].itemSet[(int)_itemId]->timeToDisplay = _timeToDisplayItem;
            heroes[myHeroId]->map[ _itemRow ][ _itemCol ].whichItem = *teams[myTeamId].itemSet[_itemId];
        }

        if( atoi(itemId[3]) != 9 )
        {
            _itemId = (itemDesc)atoi(itemId[3]);
            _itemRow = atoi(itemRow[3]);
            _itemCol = atoi(itemCol[3]);
            _timeToDisplayItem = atoi(timeToDisplayItem[3]);
            heroes[myHeroId]->map[ _itemRow ][ _itemCol ].isItemPresent = true;
            teams[myTeamId].itemSet[(int)_itemId] = new item( _itemId , _itemRow , _itemCol , itemsTexture[_itemId] );
            teams[myTeamId].itemSet[(int)_itemId]->isPresent = true;
            teams[myTeamId].itemSet[(int)_itemId]->timeToDisplay = _timeToDisplayItem;
            heroes[myHeroId]->map[ _itemRow ][ _itemCol ].whichItem = *teams[myTeamId].itemSet[_itemId];
        }

         for( itemMap::const_iterator it = teams[myTeamId].itemSet.begin(); it != teams[myTeamId].itemSet.end();  )
         {
             if(heroes[friendHeroId]->row == it->second->row  &&  heroes[friendHeroId]->col == it->second->col)
             {
                 teams[myTeamId].itemSet.erase(it++);
             }
             else
             {
                 ++it;
             }
         }



        // if unlocked, modify, else leave it
        if(!teams[0].healthLock) teams[0].currentHealth = atoi( temple0Health );
        if(!teams[1].healthLock) teams[1].currentHealth = atoi( temple1Health );

        i = 0;

    }

}

void serverProcessReceived( int heroId )
{
    int c = 0;
    int j_holder = 0;
    static int noOfTimesClientPinged = 0;

    static int counter = 0;
    counter = (counter + 1)%50;

    int heroTeamId = heroId/2;
    int heroEnemyTeamId = (1+heroTeamId)%2;

    int heroFriendId = (heroTeamId*2)      + (heroId+1)%2;
    int heroEnemy1Id = (heroEnemyTeamId*2) + (heroId  )%2;
    int heroEnemy2Id = (heroEnemyTeamId*2) + (heroId+1)%2;

    bool isDead, isAttackingTemple, isAttackingHero;
    char  heroX[6], heroY[5], heroRow[3], heroCol[3], health[6];
    char attackPower[2], vandalizedTileId[4];
    float _heroX, _heroY;
    int _heroRow, _heroCol;
    int gettingAttackedHeroId, _attackPower;
    int _hero_State;
    int _health;
    int _stunVictimId, _magicVictimId;

    bool _isSlowing     = false;
    bool _isDisabling   = false;
    bool _isBlackouting = false;
    bool _hasVandalized = false;

    bool flag1 = false; // false means NO   power raised, true means SOMEONE'S power raised
    bool flag2 = false; // false means hero power raised, true means temple power raised
    bool flag3 = false; // true means someone is stunned

    // Format is <Hero Id><Is Dead = 1>.......<Hero State><I/N><S/D/B/V/N><0-9><H/T/S Health/Stun victim Id>
    // Format is <Hero Id><Is Dead = 0><Hero X><Hero Y><Hero Row><Hero Col><Is Attacking Temple = 0><Is Attacking Hero = 0>.......<Hero State><I/N><S/D/B/V/N><0-9><H/T/S Health/Stun victim Id>
    // Format is <Hero Id><Is Dead = 0><Hero X><Hero Y><Hero Row><Hero Col><Is Attacking Temple = 1><Is Attacking Hero = 0><Getting Attacked Hero Id><Getting Attacked Health><Hero State><I/N><S/D/B/V/N><0-9><H/T/S Health/Stun victim Id>
    // Format is <Hero Id><Is Dead = 0><Hero X><Hero Y><Hero Row><Hero Col><Is Attacking Temple = 0><Is Attacking Hero = 1><Getting Attacked Hero Id><Getting Attacked Health><Hero State><I/N><S/D/B/V/N><0-9><H/T/S Health/Stun victim Id>

    // leave the sender hero Id, start from i = 2
    for( int i = 0; serverReceived[i] != 32; i += j_holder )
    {                                                         // abc bhr jhsdf
        for( int j = 0; serverReceived[i+j] != 32; j++ )
        {
            switch (c)
            {
                case 0:
     //               std::cout << c << " " << j << " " << serverReceived[i+j] << "\n";
                    break;
                case 1:
                    isDead = (bool)(serverReceived[i+j] - 48);
            //        std::cout << c << " " << j << " " << isDead << "\n";
                    break;
                case 2:
                    heroX[j] = serverReceived[i+j];
          //          std::cout << c << " " << j << " " << heroX[j] << "\n";
                    break;
                case 3:
                    heroY[j] = serverReceived[i+j];
        //            std::cout << c << " " << j << " " << heroY[j] << "\n";
                    break;
                case 4:
                    heroRow[j] = serverReceived[i+j];
        //            std::cout << c << " " << j << " " << heroY[j] << "\n";
                    break;
                case 5:
                    heroCol[j] = serverReceived[i+j];
        //            std::cout << c << " " << j << " " << heroY[j] << "\n";
                    break;
                case 6:
                    isAttackingTemple = (bool)(serverReceived[i+j] - 48);
      //              std::cout << c << " " << j << " " << isAttackingTemple << "\n";
                    break;
                case 7:
                    isAttackingHero = (bool)(serverReceived[i+j] - 48);
    //                std::cout << c << " " << j << " " << isAttackingHero << "\n";
                    break;
                case 8:
                    gettingAttackedHeroId = serverReceived[i+j] - 48;
     //               std::cout << c << " " << j << " " << gettingAttackedHeroId << "\n";
                    break;
                case 9:
                    attackPower[j] = serverReceived[i+j];
     //               std::cout << c << " " << j << " " << gettingAttackedHeroId << "\n";
                    break;
                case 10:
                    _hero_State = serverReceived[i+j] - 48;
                    break;
                case 11:
                    // <I/N>
                    if( serverReceived[i+j] == 'I' ){ heroes[heroId]->isInvincible = true; }
                    else                            { heroes[heroId]->isInvincible = false; }
                    break;
                case 12:
                    // S/D/B/V/N to be managed
                    if( serverReceived[i+j] == 'S' ) _isSlowing     = true;
                    if( serverReceived[i+j] == 'D' ) _isDisabling   = true;
                    if( serverReceived[i+j] == 'B' ) _isBlackouting = true;
                    if( serverReceived[i+j] == 'V' ) _hasVandalized = true;
                    if( serverReceived[i+j] == 'N' ) ;// do nothing
                    break;
                case 13:
                    if( _isSlowing || _isDisabling ) _magicVictimId = serverReceived[i+j]-48;
                    if( _hasVandalized ) vandalizedTileId[j] = serverReceived[i+j];
                    break;
                case 14:
                    if( serverReceived[i+j] == 'H' ){ flag1 = true; flag2 = false; } // update power of hero
                    if( serverReceived[i+j] == 'T' ){ flag1 = true; flag2 = true;  } // update power of temple
                    if( serverReceived[i+j] == 'S' ){ flag3 = true; }                // someone is stunned
                    break;
                case 15:
                    if( flag1 ) { health[j] = serverReceived[i+j]; }
                    if( flag3 ) { _stunVictimId = serverReceived[i+j] - 48; }
//                    std::cout << "SERVER received this " << serverReceived << "\n\n";
//                    std::cout << "case 12 says - " << _stunVictimId << "\n\n";
                    break;
            }
            j_holder = j;
        }
        switch(c)
        {
            case 2:
                heroX[j_holder + 1] = 0;
                break;
            case 3:
                heroY[j_holder + 1] = 0;
                break;
            case 4:
                heroRow[j_holder + 1] = 0;
                break;
            case 5:
                heroCol[j_holder + 1] = 0;
                break;
            case 9:
                attackPower[j_holder + 1] = 0;
                break;
            case 13:
                if( _hasVandalized ) vandalizedTileId[j_holder + 1] = 0;
                break;
            case 15:
                if( flag1 )health[j_holder + 1] = 0;
                break;
        }
        c++;
        j_holder += 2;
    }
    _heroX = atof(heroX)/100.0;
    _heroY = atof(heroY)/100.0;
    _heroRow = atoi(heroRow);
    _heroCol = atoi(heroCol);
    _attackPower = atoi(attackPower);
    if( flag1 ) _health = atoi(health);

    if( heroes[ heroId ]->isDead )
    {
       return;
    }

    // set/reset the victim's states
    if( heroes[heroId]->whichMagicPower == SLOWER )
    {
        if( _isSlowing )
        {//std::cout << "Slowed  " <<_magicVictimId << "\n\n";
            heroes[ _magicVictimId ]->isSlowed   = true;
        }
        else
        {//std::cout << "Slowed  over" << "\n\n";
            heroes[ heroEnemy1Id ]->isSlowed = false;
            heroes[ heroEnemy2Id ]->isSlowed = false;
        }
    }

    if( heroes[heroId]->whichMagicPower == DISABLER )
    {
        if( _isDisabling )
        {//std::cout << "Disabled  " <<_magicVictimId << "\n\n";
            heroes[ _magicVictimId ]->isDisabled   = true;
        }
        else
        {//std::cout << "Disabled  over" << "\n\n";
            heroes[ heroEnemy1Id ]->isDisabled = false;
            heroes[ heroEnemy2Id ]->isDisabled = false;
        }
    }

    if( heroes[heroId]->whichMagicPower == BLACKOUT )
    {
        if( _isBlackouting )
        {
            heroes[ heroEnemy1Id ]->isBlackouted = true;
            heroes[ heroEnemy2Id ]->isBlackouted = true;
        }
        else
        {
            heroes[ heroEnemy1Id ]->isBlackouted = false;
            heroes[ heroEnemy2Id ]->isBlackouted = false;
        }
    }

    if( heroes[heroId]->whichMagicPower == VANDALIZE )
    {
        if( _hasVandalized )
        {
            heroes[heroId]->hasVandalized = true;

            // they all will be informed of vandalism for display()
            heroes[heroFriendId]->isVandalized = true;
            heroes[heroEnemy1Id]->isVandalized = true;
            heroes[heroEnemy2Id]->isVandalized = true;

            heroes[heroEnemy1Id]  ->vandalizedTileId = atoi(vandalizedTileId);
            heroes[heroEnemy2Id]  ->vandalizedTileId = atoi(vandalizedTileId);
            heroes[heroFriendId]  ->vandalizedTileId = atoi(vandalizedTileId);
            heroes[heroId]        ->vandalizedTileId = atoi(vandalizedTileId);
        }
        else
        {
            heroes[heroId]->hasVandalized = false;
        }
    }

    heroes[ heroId ]->x = _heroX;
    heroes[ heroId ]->y = _heroY;
    heroes[ heroId ]->row = _heroRow;
    heroes[ heroId ]->col = _heroCol;

    for( itemMap::const_iterator it = teams[heroId/2].itemSet.begin(); it != teams[heroId/2].itemSet.end(); )
    {
        if(_heroRow == it->second->row && _heroCol == it->second->col)
        {
            teams[heroId/2].itemSet.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    if( heroes[heroId]->whichState != STUNNED && !heroes[heroId]->stateLock ) heroes[ heroId ]->whichState = (heroState)_hero_State;

    // set the locks if health of heroId raised
    if( flag1 && !flag2 )
    {//std::cout << "Updated client health to " << _health << "\n";
        heroes[ heroId ] ->current_health = _health;
        heroes[ heroId ] ->healthLock=true;
    }
    if( flag1 && flag2 )
    {
        teams[ heroTeamId ]. currentHealth = _health;
        teams[ heroTeamId].healthLock=true;
    }

    // to avoid repeated stunning
    if( flag3 && heroes[_stunVictimId]->whichState != STUNNED && !heroes[_stunVictimId]->stateLock ){
        heroes[_stunVictimId]->whichState = STUNNED;std::cout << "\n\n" << _stunVictimId << " stunned";fflush(stdout);}

    // reduce healths if locks are false
    if( isAttackingTemple && !teams[heroEnemyTeamId].healthLock)
    {
        if( noOfTimesClientPinged == 0 )
            teams[ heroEnemyTeamId ].currentHealth -= _attackPower;
        noOfTimesClientPinged++;

        if(noOfTimesClientPinged == 20)  noOfTimesClientPinged = 0;
    }
    if( isAttackingHero && !heroes[ gettingAttackedHeroId ]->healthLock && !heroes[ gettingAttackedHeroId ]->isInvincible && !heroes[ gettingAttackedHeroId ]->isDead )
    {
        if( noOfTimesClientPinged == 0 )
            heroes[ gettingAttackedHeroId ]->current_health -= _attackPower;
        noOfTimesClientPinged++;

        if( heroes[ gettingAttackedHeroId ]->current_health <= 0 )
        {
            heroes[ gettingAttackedHeroId ]->whichState = DEAD;
            heroes[ gettingAttackedHeroId ]->isDead = true;
        }

        if(noOfTimesClientPinged == 20)  noOfTimesClientPinged = 0;
    }
}

#include "relay.h"

void
EpollEvent::runEpoll()
{

    epfd = epoll_create(20);

    e_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset( &servaddr, 0, sizeof(servaddr) );
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(e_port);

    e_msg = new ( struct Message );
    e_msg->fd = e_sockfd;

    TransAgent *listen_agent = new TransAgent;
    listen_agent->set_fd( e_sockfd );
    ev.data.ptr = ( void* )listen_agent;
    ev.events   = EPOLLIN;
    epoll_ctl( epfd, EPOLL_CTL_ADD, e_sockfd, &ev );

    int tag = 0;

    if ( (tag = bind( e_sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr) )) < 0 )
    {
        cerr << "bind error" << endl;
    }

    if ( (tag = listen( e_sockfd, 10000 )) < 0 )
    {
        cerr << "listen error" << endl;
    }

    int flag = 0;
    flag = fcntl( e_sockfd, F_GETFL, 0 );
    fcntl( e_sockfd, F_SETFL, flag | O_NONBLOCK );

    int i, nready;
    struct epoll_event events[MAXHANDLE];
    struct sockaddr_in cliaddr;

    for ( ; ; )
    {
        nready = epoll_wait( epfd, events, MAXHANDLE, -1 );

        for ( i = 0; i < nready; i++ )
        {
            TransAgent *agent = (TransAgent *)events[i].data.ptr;

            if ( events[i].events & EPOLLIN )
            {
                if ( agent->get_fd() == e_sockfd )
                {

                    socklen_t clilen;
                    struct Message *new_msg = new struct Message;

                    clilen = sizeof(cliaddr);

                    new_msg->fd = accept(e_sockfd, (struct sockaddr*) &cliaddr, &clilen);

                    int val = fcntl( new_msg->fd, F_GETFL, 0 );
                    fcntl( new_msg->fd, F_SETFL, val | O_NONBLOCK );

                    TransAgent *new_agent = new TransAgent( new_msg, manager );
                    new_agent->set_fd( new_msg->fd );

                    ev.data.ptr = ( void* )new_agent;
                    ev.events   = EPOLLIN;

                    epoll_ctl( epfd, EPOLL_CTL_ADD, new_msg->fd, &ev );

                    //cout << "new client come in, accepted!" << endl;
            
                }    
                else
                {
                    agent->Recv();
                    if ( agent->getSTATE() == WRITE )
                    {
                        ev.events = EPOLLOUT;
                        ev.data.ptr = (void*) agent;
                        epoll_ctl( epfd, EPOLL_CTL_MOD, agent->get_fd(), &ev );
                    }
                }
            }    
            if (events[i].events & EPOLLOUT)
            {
                agent->Send();
                if ( agent->getSTATE() == READ )
                {
                    ev.events = EPOLLIN;
                    ev.data.ptr = (void*) agent;
                    epoll_ctl( epfd, EPOLL_CTL_MOD, agent->get_fd(), &ev );
                }
            }
        }
    }
}

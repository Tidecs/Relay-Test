#include <iostream>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <utility>
#include <map>
#include <queue>
#include <errno.h>
#include <signal.h>
#include <sstream>
#include <string.h>

using namespace std;

const int FALSE     = 1;
const int FAIL      = -1;
const int MAXCLINUM = 10000;
const int TIMEOUT   = 500;
const int HEADLEN   = 12;
const int MAXHANDLE = 10000;

const int LOGIN     = -1;
const int LOGOUT    = -2;
const int ERROR     = -3;
const int READ      = -4;
const int WRITE     = -5;
const int END       = -6;
const int READFIN   = -7;
const int WRITEFIN  = -8;
const int DEFAULT   = -100;

class TransAgent;
class AgentManager;
class EpollEvent;

struct head
{
    int srcID;
    int destID;
    int   len;
};

struct Message
{
    struct head msg_head;
    struct head opp_head;
    char *fr, *friptr, *froptr;
    char *to, *toiptr, *tooptr;
    int fd;
    int state;
    int wt_state;
    int kept;
};


class TransAgent
{
friend class AgentManager;

public:
    TransAgent() : m_msg(nullptr), opp_fd(DEFAULT), manager(nullptr) {}
    TransAgent( struct Message* msg, AgentManager *agent_manager )
        : m_msg(msg), fr_queue(), to_queue(), opp_fd(m_msg->fd), manager(agent_manager) {}
    ~TransAgent();

    void read_head();
    void write_init();
    void rd_buf_rst();
    void wt_buf_rst();
    void Read();
    void Write();
    void close_agent();
    void set_fd( int );
    int get_fd() const;
    int check_fd();
    int getSTATE() const;

    int Recv();
    int Send();


private:
    struct Message *m_msg;
    queue< pair<char*, char*> > fr_queue;
    queue< pair<char*, char*> > to_queue;
    int       opp_fd;
    AgentManager *manager;
    int STATE;

};

class EpollEvent
{
public:
    EpollEvent() {}
    EpollEvent( int p, AgentManager *agent_manager ) : manager(agent_manager), e_port(p) {}
    ~EpollEvent() {}

    void initEpoll();
    void initListen();
    void runEpoll();

private:
    AgentManager    *manager;
    struct sockaddr_in servaddr;
    struct Message* e_msg;
    struct epoll_event ev;
    int             e_port;
    int             e_sockfd;
    int             epfd;

};

class AgentManager
{
public:
    AgentManager() {}
    ~AgentManager() {}

    void AgentInsert( int agent_id, TransAgent *trans_ptr );
    void AgentErase( TransAgent *trans_ptr );
    void AgentTaskCTL( queue< pair<char*, char*> >& , TransAgent*);

private:
    map<int, TransAgent*> id_to_agent;
};

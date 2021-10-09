#include "relay.h"

const int SUCCESS = 0;

TransAgent::~TransAgent()
{
}

int
TransAgent::Recv()
{
    if ( m_msg->state != READ )
    {
        read_head();
    }

    if ( m_msg->msg_head.len == LOGIN )
    {
        m_msg->state = DEFAULT;
        manager->AgentInsert( m_msg->msg_head.srcID, this );
        //cout << "login:" << m_msg->msg_head.srcID << endl;
	delete [] m_msg->fr;
        return LOGIN;
    }

    if ( m_msg->msg_head.len != 0 )
    {
        Read();
        if ( m_msg->state == READFIN )
        {
            rd_buf_rst();
            //
            manager->AgentTaskCTL( fr_queue, this );
            STATE = WRITE;
            
        }
    }
    else
    {
        rd_buf_rst();
        //
        manager->AgentTaskCTL( fr_queue, this );
        Send();
    }

    return SUCCESS;
}

int
TransAgent::Send()
{
    while ( to_queue.size() != 0 )
    {
        if ( m_msg->wt_state != WRITE )
            write_init();

        if ( m_msg->opp_head.len != 0 )
        {
            Write();
            if ( m_msg->wt_state == WRITEFIN )
            {
                wt_buf_rst();
            }
        }
        else
        {
            close_agent();
            wt_buf_rst();
        }
    }
    STATE = READ;

    return SUCCESS;
}

void
TransAgent::read_head()
{
    int nread = 0;

    if ( (nread = read(opp_fd, &m_msg->msg_head, HEADLEN)) < 0 )
    {
        if ( errno != EWOULDBLOCK )
        {
            if ( errno != ECONNRESET )
            {
                cerr << "read head error" <<endl;
            }
        }
    }
    else if ( m_msg->msg_head.len == LOGIN)
    {
	return;
    }
    else
    {
        m_msg->fr     = new char[m_msg->msg_head.len + HEADLEN];
        m_msg->friptr = m_msg->fr;
        m_msg->froptr = m_msg->fr;
        memcpy(m_msg->friptr, &(m_msg->msg_head), HEADLEN);
        m_msg->friptr += HEADLEN;
        m_msg->state = READ;
    }
}

void
TransAgent::Read()
{
    int    nread;
    int &len = m_msg->msg_head.len;

    if ( (nread = read(opp_fd, m_msg->friptr, len)) < 0)
    {
        if ( errno != EWOULDBLOCK )
        {
            if ( errno != ECONNRESET )
            {
                cerr << "read data error" << endl;
            }
        }
    }
    else
    {
        m_msg->friptr += nread;
        len           -= nread;

        if ( len == 0 )
        {
            if ( m_msg->friptr != m_msg->froptr )
                fr_queue.push( make_pair( m_msg->froptr, m_msg->friptr ) );
            m_msg->state = READFIN;
        }
    }

}

void
TransAgent::write_init()
{
    m_msg->toiptr   = to_queue.front().second;
    m_msg->tooptr   = to_queue.front().first;
    m_msg->to       = m_msg->tooptr;
    to_queue.pop();
    m_msg->wt_state = WRITE;
    
    memcpy( &(m_msg->opp_head), m_msg->tooptr, HEADLEN );
}

void
TransAgent::Write()
{
    int nwrite;

            if ( (nwrite = write(opp_fd, m_msg->tooptr, m_msg->toiptr - m_msg->tooptr)) < 0)
            {
                if ( errno != EWOULDBLOCK )
                {
                    if ( errno != ECONNRESET )
                    {
                        cerr << "write data error" << endl;
                    }
                }
            }
            else
            {
                m_msg->tooptr += nwrite;

                if ( m_msg->tooptr == m_msg->toiptr )
                {
                    delete [] m_msg->to;
                    m_msg->wt_state = WRITEFIN;
                }
            }
}

void 
TransAgent::close_agent()
{
    close( opp_fd );

    while ( to_queue.size() != 0 )
    {
        m_msg->tooptr = to_queue.front().first;
        delete [] m_msg->tooptr;
        m_msg->tooptr = nullptr;
        to_queue.pop();
    }

}

void 
TransAgent::rd_buf_rst()
{
    m_msg->state            = DEFAULT;
    m_msg->fr               = nullptr;
    m_msg->friptr           = nullptr;
    m_msg->froptr           = nullptr;

}

void 
TransAgent::wt_buf_rst()
{
    m_msg->to         = nullptr;
    m_msg->toiptr     = nullptr;
    m_msg->tooptr     = nullptr;
    m_msg->wt_state   = DEFAULT;
}

void
TransAgent::set_fd( int fd )
{
    opp_fd = fd;
}

int
TransAgent::get_fd() const
{
    return opp_fd;
}

int
TransAgent::check_fd()
{
    if ( opp_fd == DEFAULT || opp_fd == 0 )
        return FAIL;
    else
        return SUCCESS;
}

int
TransAgent::getSTATE() const
{
    return STATE;
}

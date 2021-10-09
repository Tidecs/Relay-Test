#include "relay.h"

void
AgentManager::AgentInsert( int agent_id, TransAgent *trans_ptr )
{
    id_to_agent.insert(make_pair(agent_id, trans_ptr));
}

void
AgentManager::AgentErase( TransAgent *trans_ptr )
{
    id_to_agent.erase( trans_ptr->m_msg->msg_head.srcID );
}

void
AgentManager::AgentTaskCTL( queue< pair<char*, char*> > &trans_queue, TransAgent *trans_agent )
{
    char *optr, *iptr;
    TransAgent *opp_agent;

    map<int, TransAgent*>::iterator it = 
        id_to_agent.find(trans_agent->m_msg->msg_head.destID);

    if ( it == id_to_agent.end() )
        return;

    opp_agent = it->second;

    while ( trans_queue.size() != 0 )
    {
        optr = trans_queue.front().first;
        iptr = trans_queue.front().second;
        opp_agent->to_queue.push( make_pair(optr, iptr) );
        trans_queue.pop();
    }
}

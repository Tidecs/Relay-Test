#include "relay.h"

const int MAXLISTEN = 1024;
const int PORT      = 10001;

int main()
{
    AgentManager *manager = new AgentManager;
    EpollEvent ee(PORT, manager);
    ee.runEpoll();
}

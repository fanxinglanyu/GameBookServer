#include "robot_state_login.h"
#include "robot.h"
#include "libserver/message_system_help.h"
#include "libserver/component_help.h"

void RobotStateLoginConnectting::OnEnterState()
{
    _pParentObj->ConnectToLogin();
}

RobotStateType RobotStateLoginConnectting::OnUpdate()
{
    auto socketKey = _pParentObj->GetSocketKey();
    if (socketKey.Socket != INVALID_SOCKET && socketKey.NetType == NetworkType::TcpConnector)
    {
        return RobotStateType::Login_Connected;
    }

    return GetState();
}

void RobotStateLoginConnectted::OnEnterState()
{
    Proto::AccountCheck accountCheck;
    accountCheck.set_account(_pParentObj->GetAccount().c_str());
    accountCheck.set_password("e10adc3949ba59abbe56e057f20f883e");
    MessageSystemHelp::SendPacket(Proto::MsgId::C2L_AccountCheck, _pParentObj, accountCheck );

#ifdef LOG_TRACE_COMPONENT_OPEN
    std::stringstream traceMsg;
    traceMsg << "send check account.";
    traceMsg << " account:" << _pParentObj->GetAccount();
    traceMsg << " socket:" << _pParentObj->GetSocketKey().Socket;

    ComponentHelp::GetTraceComponent()->TraceAccount(_pParentObj->GetAccount(), _pParentObj->GetSocketKey().Socket);
    ComponentHelp::GetTraceComponent()->Trace(TraceType::Player, _pParentObj->GetSocketKey().Socket, traceMsg.str());
#endif
}

RobotStateType RobotStateLoginConnectted::OnUpdate()
{
    return GetState();
}

RobotStateType RobotStateLoginLogined::OnUpdate()
{
    return GetState();
}

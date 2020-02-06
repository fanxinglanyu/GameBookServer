#include "account.h"
#include "libserver/common.h"
#include "libserver/packet.h"
#include "libserver/thread_mgr.h"

#include "http_request_account.h"

bool Account::Init()
{
    return true;
}

void Account::RegisterMsgFuntion()
{
    auto pMsgCallBack = new MessageCallBackFunction();
    AttachCallBackHander(pMsgCallBack);

    pMsgCallBack->RegisterFuntion(Proto::MsgId::C2L_AccountCheck, BindFunP1(this, &Account::HandleAccountCheck));
    pMsgCallBack->RegisterFuntion(Proto::MsgId::MI_AccountCheckToHttpRs, BindFunP1(this, &Account::HandleAccountCheckToHttpRs));

    // ��������
    pMsgCallBack->RegisterFuntion(Proto::MsgId::MI_NetworkDisconnect, BindFunP1(this, &Account::HandleNetworkDisconnect));
}

void Account::Update()
{

}

void Account::HandleNetworkDisconnect(Packet* pPacket)
{
    //std::cout << "player offline. socket:" << pPacket->GetSocket() << std::endl;
    _playerMgr.RemovePlayer(pPacket->GetSocket());
}

void Account::HandleAccountCheck(Packet* pPacket)
{
    auto protoCheck = pPacket->ParseToProto<Proto::AccountCheck>();
    const auto socket = pPacket->GetSocket();

    //std::cout << "account check account:" << protoCheck.account() << " socket:" << socket << std::endl;

    // ��ͬ�˺����ڵ�¼
    auto pPlayer = _playerMgr.GetPlayer(protoCheck.account());
    if (pPlayer != nullptr)
    {
        Proto::AccountCheckRs protoResult;
        protoResult.set_return_code(Proto::AccountCheckRs::ARC_LOGGING);

        auto pResultPacket = new Packet(Proto::MsgId::C2L_AccountCheckRs, socket);
        pResultPacket->SerializeToBuffer(protoResult);

        SendPacket(pResultPacket);

        std::cout << "account check failed. same account:" << protoCheck.account() << " socket:" << socket << std::endl;

        // �ر�����
        const auto pPacketDis = new Packet(Proto::MsgId::MI_NetworkDisconnectToNet, socket);
        DispatchPacket(pPacketDis);

        return;
    }

    // ������Ϣ
    _playerMgr.AddPlayer(socket, protoCheck.account(), protoCheck.password());

    // ��֤�˺�(HTTP)
    HttpRequestAccount* pHttp = new HttpRequestAccount(protoCheck.account(), protoCheck.password());
    ThreadMgr::GetInstance()->AddObjToThread(pHttp);
}

void Account::HandleAccountCheckToHttpRs(Packet* pPacket)
{
    auto proto = pPacket->ParseToProto<Proto::AccountCheckToHttpRs>();

    auto pPlayer = _playerMgr.GetPlayer(proto.account());
    if (pPlayer == nullptr)
    {
        std::cout << "can't find player. account:" << proto.account().c_str() << std::endl;
        return;
    }

    //std::cout << "account check result account:" << proto.account() << " rs:" << proto.return_code() << " socket:" << pPlayer->GetSocket() << std::endl;

    Proto::AccountCheckRs protoResult;
    protoResult.set_return_code(proto.return_code());

    auto pResultPacket = new Packet(Proto::MsgId::C2L_AccountCheckRs, pPlayer->GetSocket());
    pResultPacket->SerializeToBuffer(protoResult);
    SendPacket(pResultPacket);
}
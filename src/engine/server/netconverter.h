
#ifndef ENGINE_SERVER_NETCONVERTER_H
#define ENGINE_SERVER_NETCONVERTER_H

#include <engine/netconverter.h>

class CNetConverter : public INetConverter
{
    IServer *m_pServer;
    class CGameContext *m_pGameServer;
    class CConfig *m_pConfig;

private:
    IServer *Server() { return m_pServer; }
    class CGameContext *GameServer() { return m_pGameServer; }
    class CConfig *Config() { return m_pConfig; }

    bool DeepSnapConvert6(void *pItem, void *pSnapClass, int Type, int ID, int Size, int ToClientID);
    int DeepMsgConvert6(CMsgPacker *pMsg, int Flags, int ToClientID);
    int DeepGameMsgConvert6(CMsgPacker *pMsg, int Flags, int ToClientID);
    int DeepSystemMsgConvert6(CMsgPacker *pMsg, int Flags, int ToClientID);

    int m_GameFlags;
public:
    CNetConverter(IServer *pServer, class CConfig *pConfig);
    void Init(class CGameContext *pGameServer) override 
    {
        m_pGameServer = pGameServer;
    }

	bool SnapNewItemConvert(void *pItem, void *pSnapClass, int Type, int ID, int Size, int ToClientID) override;
	int SendMsgConvert(CMsgPacker *pMsg, int Flags, int ToClientID, int Depth = 0) override;
	int SendSystemMsgConvert(CMsgPacker *pMsg, int Flags, int ToClientID, int Depth = 0) override;
};

#endif

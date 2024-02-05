
#include <engine/shared/config.h>
#include <engine/shared/protocol.h>

#include <game/localization.h>

#include <game/server/entities/character.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>

#include "bomb.h"

static CTeeInfo s_BombInfo;
static CTeeInfo s_PlayerInfo;

static int HSLA_to_int(int H, int S, int L, int Alpha = 255)
{
	int color = 0;
	color = (color & 0xFF00FFFF) | (H << 16);
	color = (color & 0xFFFF00FF) | (S << 8);
	color = (color & 0xFFFFFF00) | L;
	color = (color & 0x00FFFFFF) | (Alpha << 24);
	return color;
}

CGameControllerBomb::CGameControllerBomb(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "Bomb Glue";
	m_GameFlags = GAMEFLAG_SURVIVAL;

	str_copy(s_BombInfo.m_apSkinPartNames[SKINPART_BODY], "standard", sizeof(s_BombInfo.m_apSkinPartNames[SKINPART_BODY]));
	str_copy(s_BombInfo.m_apSkinPartNames[SKINPART_MARKING], "donny", sizeof(s_BombInfo.m_apSkinPartNames[SKINPART_MARKING]));
	str_copy(s_BombInfo.m_apSkinPartNames[SKINPART_DECORATION], "unimelo", sizeof(s_BombInfo.m_apSkinPartNames[SKINPART_DECORATION]));
	str_copy(s_BombInfo.m_apSkinPartNames[SKINPART_HANDS], "standard", sizeof(s_BombInfo.m_apSkinPartNames[SKINPART_HANDS]));
	str_copy(s_BombInfo.m_apSkinPartNames[SKINPART_FEET], "standard", sizeof(s_BombInfo.m_apSkinPartNames[SKINPART_FEET]));
	str_copy(s_BombInfo.m_apSkinPartNames[SKINPART_EYES], "standard", sizeof(s_BombInfo.m_apSkinPartNames[SKINPART_EYES]));

	s_BombInfo.m_aUseCustomColors[SKINPART_BODY] = true;
	s_BombInfo.m_aUseCustomColors[SKINPART_MARKING] = true;
	s_BombInfo.m_aUseCustomColors[SKINPART_DECORATION] = true;
	s_BombInfo.m_aUseCustomColors[SKINPART_HANDS] = true;
	s_BombInfo.m_aUseCustomColors[SKINPART_FEET] = true;
	s_BombInfo.m_aUseCustomColors[SKINPART_EYES] = false;

	s_BombInfo.m_aSkinPartColors[SKINPART_BODY] = HSLA_to_int(0, 0, 0);
	s_BombInfo.m_aSkinPartColors[SKINPART_MARKING] = HSLA_to_int(255, 0, 94);
	s_BombInfo.m_aSkinPartColors[SKINPART_DECORATION] = HSLA_to_int(0, 0, 0);
	s_BombInfo.m_aSkinPartColors[SKINPART_HANDS] = HSLA_to_int(0, 0, 0);
	s_BombInfo.m_aSkinPartColors[SKINPART_FEET] = HSLA_to_int(0, 0, 0);
	s_BombInfo.m_aSkinPartColors[SKINPART_EYES] = HSLA_to_int(0, 0, 0);

	str_copy(s_BombInfo.m_aSkinName, "bomb", sizeof(s_BombInfo.m_aSkinName));
	s_BombInfo.m_UseCustomColor = false;
	s_BombInfo.m_ColorBody = 0;
	s_BombInfo.m_ColorFeet = 0;

	str_copy(s_PlayerInfo.m_apSkinPartNames[SKINPART_BODY], "standard", sizeof(s_PlayerInfo.m_apSkinPartNames[SKINPART_BODY]));
	str_copy(s_PlayerInfo.m_apSkinPartNames[SKINPART_MARKING], "cammostripes", sizeof(s_PlayerInfo.m_apSkinPartNames[SKINPART_MARKING]));
	s_PlayerInfo.m_apSkinPartNames[SKINPART_DECORATION][0] = 0;
	str_copy(s_PlayerInfo.m_apSkinPartNames[SKINPART_HANDS], "standard", sizeof(s_PlayerInfo.m_apSkinPartNames[SKINPART_HANDS]));
	str_copy(s_PlayerInfo.m_apSkinPartNames[SKINPART_FEET], "standard", sizeof(s_PlayerInfo.m_apSkinPartNames[SKINPART_FEET]));
	str_copy(s_PlayerInfo.m_apSkinPartNames[SKINPART_EYES], "standard", sizeof(s_PlayerInfo.m_apSkinPartNames[SKINPART_EYES]));

	s_PlayerInfo.m_aUseCustomColors[SKINPART_BODY] = true;
	s_PlayerInfo.m_aUseCustomColors[SKINPART_MARKING] = true;
	s_PlayerInfo.m_aUseCustomColors[SKINPART_DECORATION] = false;
	s_PlayerInfo.m_aUseCustomColors[SKINPART_HANDS] = true;
	s_PlayerInfo.m_aUseCustomColors[SKINPART_FEET] = true;
	s_PlayerInfo.m_aUseCustomColors[SKINPART_EYES] = false;

	s_PlayerInfo.m_aSkinPartColors[SKINPART_BODY] = HSLA_to_int(0, 0, 255);
	s_PlayerInfo.m_aSkinPartColors[SKINPART_MARKING] = HSLA_to_int(0, 0, 155);
	s_PlayerInfo.m_aSkinPartColors[SKINPART_DECORATION] = HSLA_to_int(0, 0, 0);
	s_PlayerInfo.m_aSkinPartColors[SKINPART_HANDS] = HSLA_to_int(0, 0, 255);
	s_PlayerInfo.m_aSkinPartColors[SKINPART_FEET] = HSLA_to_int(0, 0, 255);
	s_PlayerInfo.m_aSkinPartColors[SKINPART_EYES] = HSLA_to_int(0, 0, 0);

	str_copy(s_PlayerInfo.m_aSkinName, "cammostripes", sizeof(s_PlayerInfo.m_aSkinName));
	s_PlayerInfo.m_UseCustomColor = true;
	s_PlayerInfo.m_ColorBody = 255;
	s_PlayerInfo.m_ColorFeet = 255;

	m_BombLeftTick = -1;
	m_BombPlayer = -1;
}

void CGameControllerBomb::ChooseBomb()
{
	if(m_BombPlayer != -1)
		return;

	int aAlivePlayers[MAX_CLIENTS];
	int AlivePlayerCount = 0;
	for(int i = 0; i < MAX_CLIENTS; i ++)
	{
		if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS &&
			(!GameServer()->m_apPlayers[i]->m_RespawnDisabled ||
			(GameServer()->m_apPlayers[i]->GetCharacter() && GameServer()->m_apPlayers[i]->GetCharacter()->IsAlive())))
		{
			aAlivePlayers[AlivePlayerCount ++] = i;

			if(GameServer()->m_apPlayers[i]->GetCharacter())
			{
				// default health
				GameServer()->m_apPlayers[i]->GetCharacter()->IncreaseHealth(minimum(10, Config()->m_SvBombExplodeTime));
				// default armor
				GameServer()->m_apPlayers[i]->GetCharacter()->IncreaseArmor(Config()->m_SvBombExplodeTime > 10 ? Config()->m_SvBombExplodeTime - 10 : 0);
			}
		}
	}

	if(AlivePlayerCount > 1)
	{
		m_BombPlayer = aAlivePlayers[random_int() % AlivePlayerCount];
		GameServer()->m_apPlayers[m_BombPlayer]->m_TeeInfos = s_BombInfo;
		m_BombLeftTick = Server()->TickSpeed() * Config()->m_SvBombExplodeTime;
		// update all clients
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(!GameServer()->m_apPlayers[i] || 
				(!Server()->ClientIngame(i) && !GameServer()->m_apPlayers[i]->IsDummy()) || 
					Server()->GetClientVersion(i) < CGameContext::MIN_SKINCHANGE_CLIENTVERSION)
				continue;

			GameServer()->SendSkinChange(m_BombPlayer, i);
		}
		for(int i = 0; i < MAX_CLIENTS; i ++)
		{
			if(!GameServer()->m_apPlayers[i])
				continue;
			char aChat[128];
			str_format(aChat, sizeof(aChat), Localize(Server()->GetClientLanguage(i), "'%s' became bomb!"), Server()->ClientName(m_BombPlayer));
			
			CNetMsg_Sv_Broadcast Msg;
			Msg.m_pMessage = aChat;
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, i);
		}
	}
}

void CGameControllerBomb::Tick()
{
	if(IsGameRunning())
	{
		if(m_BombPlayer != -1 && m_BombLeftTick <= 0)
		{
			GameServer()->CreateExplosion(GameServer()->GetPlayerChar(m_BombPlayer)->GetPos(), -1, WEAPON_GAME, 0);
			GameServer()->CreateSound(GameServer()->GetPlayerChar(m_BombPlayer)->GetPos(), SOUND_GRENADE_EXPLODE);
			GameServer()->GetPlayerChar(m_BombPlayer)->Die(m_BombPlayer, WEAPON_GAME);
			GameServer()->m_apPlayers[m_BombPlayer]->m_TeeInfos = s_PlayerInfo;
			// update all clients
			for(int i = 0; i < MAX_CLIENTS; ++i)
			{
				if(!GameServer()->m_apPlayers[i] || 
					(!Server()->ClientIngame(i) && !GameServer()->m_apPlayers[i]->IsDummy()) || 
						Server()->GetClientVersion(i) < CGameContext::MIN_SKINCHANGE_CLIENTVERSION)
					continue;

				GameServer()->SendSkinChange(m_BombPlayer, i);
			}
			m_BombPlayer = -1;
		}
		if(m_BombPlayer != -1)
		{
			m_BombLeftTick --;

			if(m_BombLeftTick % Server()->TickSpeed() == 0)
			{
				for(int i = 0; i < MAX_CLIENTS; i ++)
				{
					if(GameServer()->GetPlayerChar(i))
					{
						if(m_BombLeftTick / Server()->TickSpeed() > 10) 
							GameServer()->GetPlayerChar(i)->IncreaseArmor(-1);
						else
							GameServer()->GetPlayerChar(i)->IncreaseHealth(-1);
					}
				}
			}
		}
		ChooseBomb();

		if(m_BombLeftTick % Server()->TickSpeed() == 0 && GameServer()->GetPlayerChar(m_BombPlayer))
		{
			GameServer()->CreateSound(GameServer()->GetPlayerChar(m_BombPlayer)->GetPos(), SOUND_HOOK_NOATTACH);
		}
	}

	for(int i = 0; i < MAX_CLIENTS; i ++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			GameServer()->m_apPlayers[i]->m_LastChangeInfoTick = Server()->Tick();
		}
	}
	IGameController::Tick();
}

void CGameControllerBomb::OnCharacterSpawn(CCharacter *pChr)
{
	// default health
	pChr->IncreaseHealth(minimum(10, Config()->m_SvBombExplodeTime));
	// default armor
	pChr->IncreaseArmor(Config()->m_SvBombExplodeTime > 10 ? Config()->m_SvBombExplodeTime - 10 : 0);

	// give default weapons
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->SetWeapon(WEAPON_HAMMER);

	// prevent respawn
	pChr->GetPlayer()->m_RespawnDisabled = GetStartRespawnState();
}

void CGameControllerBomb::OnCharacterDamage(CCharacter *pChr, int From)
{
	if(From < 0 || From >= MAX_CLIENTS)
		return;

	if(!GameServer()->GetPlayerChar(From))
		return;
	if(From == m_BombPlayer)
	{
		GameServer()->m_apPlayers[From]->m_TeeInfos = s_PlayerInfo;
		// update all clients
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(!GameServer()->m_apPlayers[i] || 
				(!Server()->ClientIngame(i) && !GameServer()->m_apPlayers[i]->IsDummy()) || 
					Server()->GetClientVersion(i) < CGameContext::MIN_SKINCHANGE_CLIENTVERSION)
				continue;

			GameServer()->SendSkinChange(From, i);
		}

		m_BombPlayer = pChr->GetPlayer()->GetCID();
		pChr->GetPlayer()->m_TeeInfos = s_BombInfo;
		// update all clients
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(!GameServer()->m_apPlayers[i] || 
				(!Server()->ClientIngame(i) && !GameServer()->m_apPlayers[i]->IsDummy()) || 
					Server()->GetClientVersion(i) < CGameContext::MIN_SKINCHANGE_CLIENTVERSION)
				continue;

			GameServer()->SendSkinChange(m_BombPlayer, i);
		}
		for(int i = 0; i < MAX_CLIENTS; i ++)
		{
			if(!GameServer()->m_apPlayers[i])
				continue;
			char aChat[128];
			str_format(aChat, sizeof(aChat), Localize(Server()->GetClientLanguage(i), "'%s' became bomb!"), Server()->ClientName(m_BombPlayer));
			
			CNetMsg_Sv_Broadcast Msg;
			Msg.m_pMessage = aChat;
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, i);
		}
	}
}

bool CGameControllerBomb::IsFriendlyFire(int ClientID1, int ClientID2) const
{
	if(ClientID1 < 0 || ClientID1 >= MAX_CLIENTS)
		return false;
	if(ClientID2 < 0 || ClientID2 >= MAX_CLIENTS)
		return false;

    return true;
}

// game
void CGameControllerBomb::DoWincheckRound()
{
	// check for time based win
	if(m_GameInfo.m_TimeLimit > 0 && (Server()->Tick()-m_GameStartTick) >= m_GameInfo.m_TimeLimit*Server()->TickSpeed()*60)
	{
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS &&
				(!GameServer()->m_apPlayers[i]->m_RespawnDisabled ||
				(GameServer()->m_apPlayers[i]->GetCharacter() && GameServer()->m_apPlayers[i]->GetCharacter()->IsAlive())))
				GameServer()->m_apPlayers[i]->m_Score++;
		}

		EndRound();
	}
	else
	{
		// check for survival win
		CPlayer *pAlivePlayer = 0;
		int AlivePlayerCount = 0;
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS &&
				(!GameServer()->m_apPlayers[i]->m_RespawnDisabled ||
				(GameServer()->m_apPlayers[i]->GetCharacter() && GameServer()->m_apPlayers[i]->GetCharacter()->IsAlive())))
			{
				++AlivePlayerCount;
				pAlivePlayer = GameServer()->m_apPlayers[i];
			}
		}

		if(AlivePlayerCount == 0)		// no winner
			EndRound();
		else if(AlivePlayerCount == 1)	// 1 winner
		{
			pAlivePlayer->m_Score++;
			EndRound();
		}
	}
}

void CGameControllerBomb::OnPlayerConnect(CPlayer *pPlayer)
{	
	pPlayer->m_TeeInfos = s_PlayerInfo;
	// update all clients
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(!GameServer()->m_apPlayers[i] || 
			(!Server()->ClientIngame(i) && !GameServer()->m_apPlayers[i]->IsDummy()) || 
				Server()->GetClientVersion(i) < CGameContext::MIN_SKINCHANGE_CLIENTVERSION)
			continue;

		GameServer()->SendSkinChange(pPlayer->GetCID(), i);
	}

	IGameController::OnPlayerConnect(pPlayer);
}
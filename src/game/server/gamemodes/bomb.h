#ifndef GAME_SERVER_GAMEMODES_BOMB_H
#define GAME_SERVER_GAMEMODES_BOMB_H
#include <game/server/gamecontroller.h>

class CGameControllerBomb : public IGameController
{
	int m_BombPlayer;
	// Left times of bomb explode
	int m_BombLeftTick;
public:
	CGameControllerBomb(class CGameContext *pGameServer);

	void ChooseBomb();

	void Tick() override;
	void DoWincheckRound() override;
	void OnCharacterSpawn(class CCharacter *pChr) override;
	void OnCharacterDamage(class CCharacter *pChr, int From) override;
	void OnPlayerConnect(CPlayer *pPlayer) override;
	bool IsFriendlyFire(int ClientID1, int ClientID2) const;
};

#endif
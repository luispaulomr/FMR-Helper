#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include <memory>
#include "CHandleProcess.h"

class CModGame {

private:

	enum ADDR_OFFSET_INDEXES {
		I_ENEMY_HEALTH = 0,
		I_MY_HAND_CARDS,
		I_MY_TABLE_CARDS,
		I_MAX_ADDR_OFFSET
	};

	typedef struct GameConsts {
		uintptr_t offset;
		size_t len;
	} GameConsts_t;

	const GameConsts_t GAME_CONSTS[I_MAX_ADDR_OFFSET] = {
		{0xa82020 + 0xea024,	2},		/* ENEMY_HEALTH */
		{0xa82020 + 0x1a7ae4,	112},	/* MY_HAND_CARDS */
		{0xa82020 + 0x1a7b70,	123},	/* MY_TABLE_CARDS */
	};

	std::unique_ptr<CHandleProcess> m_pHandleProcess;

	const uint32_t NUM_CARDS = 722;

private:

	uint16_t _GetEnemyHealth() const;

	std::vector<uint8_t> _ReadData(GameConsts_t) const;

public:

	CModGame(const std::wstring& str_window_name, const std::wstring& str_exe_name);

	bool RetryConnection();

	std::vector<uint16_t> GetMyHandCards() const;

	std::vector<uint16_t> GetMyTableCards() const;

	bool IsGameRunning() const;

	bool IsDuel() const;

};

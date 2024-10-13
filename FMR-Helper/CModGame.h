#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include <memory>
#include "CHandleProcess.h"

class CModGame {

private:

	const uint32_t MAX_CARDS = 722;
	const uint32_t MAX_HAND_CARDS = 5;
	const uint32_t LEN_DATA_SMALL_IMAGE = 40 * 32 + 512;

	enum ADDR_OFFSET_INDEXES {
		I_ENEMY_HEALTH = 0,
		I_MY_HAND_CARDS,
		I_MY_TABLE_CARDS,
		I_PATH_BIN_FILE,
		MAX_ADDR_OFFSET_INDEX
	};

	typedef struct GameConsts {
		uintptr_t offset;
		size_t len;
	} GameConsts_t;

	const GameConsts_t GAME_CONSTS[MAX_ADDR_OFFSET_INDEX] = {
		{0xa82020 + 0xea024,	2},		/* ENEMY_HEALTH */
		{0xa82020 + 0x1a7ae4,	113},	/* MY_HAND_CARDS */
		{0xa82020 + 0x1a7b70,	124},	/* MY_TABLE_CARDS */
		{0x36f100,				255},	/* PATH_BIN_FILE */
	};

	std::unique_ptr<CHandleProcess> m_pHandleProcess;
	std::vector<std::vector<uint8_t>> m_small_cards;
	//uint8_t m_small_cards[MAX_CARDS][LEN_DATA_SMALL_IMAGE];

private:

	uint16_t _GetEnemyHealth() const;

	std::vector<uint8_t> _ReadData(GameConsts_t) const;

public:

	CModGame(const std::wstring& str_window_name, const std::wstring& str_exe_name);

	bool RetryAttach();

	std::vector<uint16_t> GetMyHandCards() const;

	std::vector<uint16_t> GetMyTableCards() const;

	std::string GetPathBinFile() const;

	bool IsAttached() const;

	bool IsDuel() const;

};

#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include <memory>
#include "CHandleProcess.h"

class CModGame {

private:

	const size_t MAX_CARDS = 722;
	const size_t MAX_HAND_CARDS = 5;
	const size_t LEN_DATA_SMALL_IMAGE = 40 * 32;
	const size_t LEN_CLUT_SMALL_IMAGE = 512;
	const size_t LEN_TOTAL_SMALL_IMAGE = LEN_DATA_SMALL_IMAGE + LEN_CLUT_SMALL_IMAGE;
	const size_t BIN_FILE_SMALL_IMAGES_OFFSET = 0x16a8c38;
	const size_t BIN_FILE_INC = 0x930;

	enum ADDR_OFFSET_INDEXES {
		I_ENEMY_HEALTH = 0,
		I_MY_HAND_CARDS,
		I_MY_TABLE_CARDS,
		I_PATH_BIN_FILE,
		MAX_ADDR_OFFSET_INDEX
	};

	typedef struct ImageData {
		std::vector<BYTE> data;
		std::vector<BYTE> clut;
	} ImageData_t;

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
	std::vector<ImageData_t> m_small_cards;

private:

	uint16_t _GetEnemyHealth() const;

	std::vector<BYTE> _ReadData(GameConsts_t) const;

	bool _LoadGameData();

	bool _ReadBinFile(std::string path_file, std::vector<ImageData_t>& data) const;

public:

	CModGame(const std::wstring& str_window_name, const std::wstring& str_exe_name);

	bool RetryAttach();

	std::vector<uint16_t> GetMyHandCards() const;

	std::vector<uint16_t> GetMyTableCards() const;

	std::string GetPathBinFile() const;

	bool IsAttached() const;

	bool IsDuel() const;

	bool LoadGameData();

};

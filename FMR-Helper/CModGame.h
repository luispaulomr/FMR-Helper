#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include <memory>
#include "CHandleProcess.h"
#include "Cards.h"

class CModGame {

private:

	const size_t MAX_CARDS = 722;
	const size_t MAX_HAND_CARDS = 5;
	const size_t LEN_DATA_SMALL_IMAGE = 40 * 32;
	const size_t LEN_CLUT_SMALL_IMAGE = 512;
	const size_t LEN_TOTAL_SMALL_IMAGE = LEN_DATA_SMALL_IMAGE + LEN_CLUT_SMALL_IMAGE;
	const size_t LEN_TOTAL_IMAGE = 14336;
	const size_t LEN_TOTAL_IMAGES = 25000000;
	const size_t BIN_FILE_IMAGES_OFFSET = 0x1847598;
	const size_t BIN_FILE_SMALL_IMAGES_OFFSET = 0x16a8c38;
	const size_t BIN_FILE_SMALL_IMAGES_INC = 10304 + 672;
	const size_t BIN_FILE_FUSIONS_OFFSET = 0x023e6608;
	const size_t LEN_TOTAL_FUSIONS = 100000;
	const size_t BIN_FILE_CARDS_OFFSET = 0x21598C;
	const size_t BIN_FILE_CARDS_OFFSET_2 = 0x216078;
	const size_t BIN_FILE_CARDS_INC = 4;

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
		{0xa82020 + 0x1a7ae4,	140},	/* MY_HAND_CARDS */
		{0xa82020 + 0x1a7b70,	124},	/* MY_TABLE_CARDS */
		{0x36f100,				255},	/* PATH_BIN_FILE */
	};

	typedef struct CardData {
		uint16_t atk;
		uint16_t def;
	} CardData_t;

	typedef struct ImageData {
		std::vector<BYTE> data;
		std::vector<BYTE> clut;
	} ImageData_t;

	std::unique_ptr<CHandleProcess> m_pHandleProcess;
	std::vector<ImageData_t> m_small_images;
	std::vector<std::vector<Card_t>> m_fusions;
	std::vector<CardData_t> m_cards;
	std::string m_path_bin_file;

private:

	uint16_t _GetEnemyHealth() const;

	std::vector<BYTE> _ReadData(GameConsts_t) const;

	bool _ReadBinFile(std::string path_file, size_t offset, size_t len, std::vector<BYTE>& buf) const;

	bool _LoadSmallImages(std::vector<ImageData_t>&) const;

	bool _LoadFusions(std::vector<std::vector<Card_t>>&) const;

	bool _LoadCards(std::vector<CardData_t>&) const;

	bool _LoadGameData();

	std::string _GetPathBinFile() const;

	//bool _cmp_cards(Card_t a, Card_t b) const;

public:

	CModGame(const std::wstring& str_window_name, const std::wstring& str_exe_name);

	bool RetryAttach();

	std::vector<uint16_t> GetMyHandCards() const;

	std::vector<uint16_t> GetMyTableCards() const;

	bool IsAttached() const;

	bool IsDuel() const;

	std::vector<Card_t> GetMyFusions();

	void PrintMyFusions(const std::vector<Card_t>& fusions) const;

};

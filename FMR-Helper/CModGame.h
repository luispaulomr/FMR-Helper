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
	const size_t BIN_FILE_SMALL_IMAGES_INC = 0x930;
	const size_t BIN_FILE_FUSIONS_OFFSET = 0x023e6608;
	const size_t LEN_TOTAL_FUSIONS = 100000;

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

	typedef struct ImageData {
		std::vector<BYTE> data;
		std::vector<BYTE> clut;
	} ImageData_t;

	typedef struct FusionData {
		uint16_t card_1;
		uint16_t card_2;
		uint16_t result;
	} FusionData_t;

	std::unique_ptr<CHandleProcess> m_pHandleProcess;
	std::vector<ImageData_t> m_small_images;
	std::vector<std::vector<FusionData_t>> m_fusions;
	std::string m_path_bin_file;

private:

	uint16_t _GetEnemyHealth() const;

	std::vector<BYTE> _ReadData(GameConsts_t) const;

	bool _ReadBinFile(std::string path_file, size_t offset, size_t len, std::vector<BYTE>& buf) const;

	bool _LoadSmallImages(std::vector<ImageData_t>&) const;

	bool _LoadFusions(std::vector<std::vector<FusionData_t>>&) const;

	bool _LoadGameData();

	std::string _GetPathBinFile() const;

public:

	CModGame(const std::wstring& str_window_name, const std::wstring& str_exe_name);

	bool RetryAttach();

	std::vector<uint16_t> GetMyHandCards() const;

	std::vector<uint16_t> GetMyTableCards() const;

	bool IsAttached() const;

	bool IsDuel() const;

};

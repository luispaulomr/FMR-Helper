#include "CModGame.h"

#include <iomanip>
#include <filesystem>
#include <fstream>

CModGame::CModGame(const std::wstring& str_window_name, const std::wstring& str_exe_name)
{
	auto handle_process = std::make_unique<CHandleProcess>(str_window_name, str_exe_name);

	m_pHandleProcess = std::move(handle_process);

	if (!m_pHandleProcess) {
		std::cout << "[CModGame::CModGame] ERROR: Failed to create CModGame::CModGame object." << "\n";
		return;
	}

	if (!m_pHandleProcess->AttachToProcess()) {
		std::cout << "[CModGame::CModGame] ERROR: Call to CHandleProcess::AttachToProcess failed." << "\n";
		return;
	}

	if (!_LoadGameData()) {
		std::cout << "[CModGame::CModGame] ERROR: Could not load game data." << "\n";
	}
}

bool CModGame::RetryAttach()
{
	if (!m_pHandleProcess) {
		std::cout << "[CModGame::RetryAttach] ERROR: Trying to access an invalid pointer.." << "\n";
		return false;
	}

	if (!m_pHandleProcess->AttachToProcess()) {
		std::cout << "[CModGame::RetryAttach] ERROR: Call to CHandleProcess::AttachToProcess failed." << "\n";
		return false;
	}

	if (!_LoadGameData()) {
		std::cout << "[CModGame::CModGame] ERROR: Could not load game data." << "\n";
	}

	return true;
}

bool CModGame::IsAttached() const
{
	if (!m_pHandleProcess) {
		std::cout << "[CModGame::IsGameRunning] ERROR: Trying to access an invalid pointer." << "\n";
		return false;
	}

	return m_pHandleProcess->IsAttached();
}

std::vector<uint8_t> CModGame::_ReadData(GameConsts_t gd) const
{
	return m_pHandleProcess->ReadMemoryFromBaseAddr(gd.offset, gd.len);
}

uint16_t CModGame::_GetEnemyHealth() const
{
	auto data = _ReadData(GAME_CONSTS[I_ENEMY_HEALTH]);

	uint16_t health = 0;
	std::memcpy(&health, data.data(), data.size());

	return health;
}

bool CModGame::_ReadBinFile
	(
	std::string path_file,
	size_t offset,
	size_t len,
	std::vector<BYTE>& buf
	) const
{
	if (!buf.size()) {
		std::cout << "[CModGame::_ReadBinFile] ERROR: Buf size is 0." << "\n";
		return false;
	}

	if (path_file.empty()) {
		std::cout << "[CModGame::_ReadBinFile] ERROR: Empty path for binary file." << "\n";
		return false;
	}

	std::ifstream in_file(path_file, std::ios_base::in | std::ios_base::binary);

	if (!in_file) {
		std::cout << "[CModGame::_ReadBinFile] ERROR: Could not open binary file for reading." << "\n";
		return false;
	}

	in_file.seekg(offset, std::ios::beg);
	in_file.read(reinterpret_cast<char*>(buf.data()), len);
	
	if (len != in_file.gcount()) {
		std::cout << "[CModGame::_ReadBinFile] ERROR: Could not read from binary file." << "\n";
		return false;
	}

	return true;
}

bool CModGame::_LoadSmallImages(std::vector<ImageData_t>& images) const
{
	images.resize(MAX_CARDS);
	size_t len_to_read = (LEN_TOTAL_SMALL_IMAGE + BIN_FILE_INC) * images.size();
	std::vector<BYTE> buf(len_to_read);

	if (!_ReadBinFile(GetPathBinFile(), BIN_FILE_SMALL_IMAGES_OFFSET, len_to_read, buf)) {
		std::cout << "[CModGame::_LoadSmallImages] ERROR: Could not read binary file." << "\n";
		images.resize(0);
		return false;
	}

	for (auto i = 0; i < buf.size(); ++i) {
		images[i].data.resize(LEN_DATA_SMALL_IMAGE);
		images[i].clut.resize(LEN_CLUT_SMALL_IMAGE);
		size_t inc = i * BIN_FILE_INC;

		std::copy(buf.begin() + inc,
			buf.begin() + inc + images[i].data.size(),
			images[i].data.data());

		std::copy(buf.begin() + inc + LEN_DATA_SMALL_IMAGE,
			buf.begin() + inc + LEN_DATA_SMALL_IMAGE + images[i].clut.size(),
			images[i].clut.data());
	}

	return true;
}

bool CModGame::_LoadFusions(std::vector<FusionData_t>& fusions) const
{
	fusions.resize(MAX_CARDS);
	//size_t len_to_read = (LEN_TOTAL_SMALL_IMAGE + BIN_FILE_INC) * images.size();
	//std::vector<BYTE> buf(len_to_read);

	//if (!_ReadBinFile(GetPathBinFile(), BIN_FILE_SMALL_IMAGES_OFFSET, len_to_read, buf)) {
	//	std::cout << "[CModGame::_LoadSmallImages] ERROR: Could not read binary file." << "\n";
	//	images.resize(0);
	//	return false;
	//}

	//for (auto i = 0; i < buf.size(); ++i) {
	//	images[i].data.resize(LEN_DATA_SMALL_IMAGE);
	//	images[i].clut.resize(LEN_CLUT_SMALL_IMAGE);
	//	size_t inc = i * BIN_FILE_INC;

	//	std::copy(buf.begin() + inc,
	//		buf.begin() + inc + images[i].data.size(),
	//		images[i].data.data());

	//	std::copy(buf.begin() + inc + LEN_DATA_SMALL_IMAGE,
	//		buf.begin() + inc + LEN_DATA_SMALL_IMAGE + images[i].clut.size(),
	//		images[i].clut.data());
	//}

	return true;
}

bool CModGame::_LoadGameData()
{
	if (m_small_images.size()) {
		return true;
	}

	if (!_LoadSmallImages(m_small_images)) {
		std::cout << "[CModGame::LoadGameData] ERROR: Could not load small images from binary file." << "\n";
		return false;
	}

	if (!_LoadFusions(m_fusions)) {
		std::cout << "[CModGame::LoadGameData] ERROR: Could not load fusions from binary file." << "\n";
		m_fusions.resize(0);
		return false;
	}
}

std::vector<uint16_t> CModGame::GetMyHandCards() const
{
	std::vector<uint16_t> cards(MAX_HAND_CARDS);

	auto mem = _ReadData(GAME_CONSTS[I_MY_HAND_CARDS]);

	for (auto i = 0; i < cards.size(); ++i) {
		uint16_t card = 0;
		std::memcpy(&card, mem.data() + 28 * i, 2);
		cards[i] = card - 1;
	}

	return cards;
}

std::vector<uint16_t> CModGame::GetMyTableCards() const
{
	std::vector<uint16_t> cards;

	auto mem = _ReadData(GAME_CONSTS[I_MY_TABLE_CARDS]);

	for (int i = 0; i < MAX_HAND_CARDS; ++i) {
		uint16_t card = 0;
		std::memcpy(&card, mem.data() + 28 * i, 2);

		uint8_t validity = 0;
		std::memcpy(&validity, mem.data() + 28 * i + 11, 1);

		if (validity) {
			cards.push_back(card - 1);
		}
	}

	return cards;
}

std::string CModGame::GetPathBinFile() const
{
	auto mem = _ReadData(GAME_CONSTS[I_PATH_BIN_FILE]);
	return std::string(mem.begin(), mem.end());
}

bool CModGame::IsDuel() const
{
	return (_GetEnemyHealth() != 0);
}
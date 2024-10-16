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
	size_t len_to_read = (LEN_TOTAL_SMALL_IMAGE + BIN_FILE_SMALL_IMAGES_INC) * images.size();
	std::vector<BYTE> buf(len_to_read);

	if (!_ReadBinFile(m_path_bin_file, BIN_FILE_SMALL_IMAGES_OFFSET, len_to_read, buf)) {
		std::cout << "[CModGame::_LoadSmallImages] ERROR: Could not read binary file." << "\n";
		images.resize(0);
		return false;
	}

	for (auto i = 0; i < images.size(); ++i) {
		images[i].data.resize(LEN_DATA_SMALL_IMAGE);
		images[i].clut.resize(LEN_CLUT_SMALL_IMAGE);
		auto addr_to_read = buf.begin() + i * BIN_FILE_SMALL_IMAGES_INC;

		size_t _addr_to_read = addr_to_read - buf.begin();

		std::copy(addr_to_read,
				  addr_to_read + images[i].data.size(),
				  images[i].data.data());

		addr_to_read += LEN_DATA_SMALL_IMAGE;

		std::copy(addr_to_read,
				  addr_to_read + images[i].clut.size(),
				  images[i].clut.data());
	}

	return true;
}

bool CModGame::_LoadFusions(std::vector<FusionData_t>& fusions) const
{
	fusions.resize(MAX_CARDS);
	size_t len_to_read = LEN_TOTAL_FUSIONS;
	std::vector<BYTE> buf(len_to_read);

	static size_t max_pos_to_read = 0;

	if (!_ReadBinFile(m_path_bin_file, BIN_FILE_FUSIONS_OFFSET, len_to_read, buf)) {
		std::cout << "[CModGame::_LoadFusions] ERROR: Could not read binary file." << "\n";
		fusions.resize(0);
		return false;
	}

	for (auto i = 0; i < fusions.size(); ++i) {
		//images[i].data.resize(LEN_DATA_SMALL_IMAGE);
		//images[i].clut.resize(LEN_CLUT_SMALL_IMAGE);
		//size_t inc = i * BIN_FILE_INC;

		uint16_t pos_to_read = 0;
		std::memcpy(&pos_to_read, buf.data() + 2 + i * 2, 2);

		if (!pos_to_read) {
			continue;
		}

		uint8_t num_of_fusions = 0;
		std::memcpy(&num_of_fusions, buf.data() + pos_to_read, 1);


	/* CHECK THIS CODE!!! if num_of_fusions is 0 and next number is not 0 */
	/* Maybe the number of fusion is given by n + 1 (low byte) and n (high byte), not sure */
	//if (num_of_fusions == 0) {
	//	uint8_t tmp_byte;

	//	input_stream.read(reinterpret_cast<char*>(&tmp_byte), 1);

	//	num_of_fusions = 511 - tmp_byte;
	//}

		//std::copy(buf.begin() + inc,
		//	buf.begin() + inc + images[i].data.size(),
		//	images[i].data.data());

		//std::copy(buf.begin() + inc + LEN_DATA_SMALL_IMAGE,
		//	buf.begin() + inc + LEN_DATA_SMALL_IMAGE + images[i].clut.size(),
		//	images[i].clut.data());
	}

	return true;




	//while (num_of_fusions > 0) {
	//	uint8_t bytes_to_read[5];
	//	uint32_t bytes_fusion[4] = { 0 };

	//	input_stream.read(reinterpret_cast<char*>(&bytes_to_read), sizeof(bytes_to_read));

	//	bytes_fusion[0] = (bytes_to_read[0] & 3) << 8 | bytes_to_read[1];
	//	bytes_fusion[1] = (bytes_to_read[0] >> 2 & 3) << 8 | bytes_to_read[2];
	//	bytes_fusion[2] = (bytes_to_read[0] >> 4 & 3) << 8 | bytes_to_read[3];
	//	bytes_fusion[3] = (bytes_to_read[0] >> 6 & 3) << 8 | bytes_to_read[4];

	//	fusions.push_back(Fusion{ static_cast<uint16_t>(index), static_cast<uint16_t>(bytes_fusion[0] - 1), static_cast<uint16_t>(bytes_fusion[1] - 1) });

	//	--num_of_fusions;

	//	if (num_of_fusions > 0) {

	//		fusions.push_back(Fusion{ static_cast<uint16_t>(index), static_cast<uint16_t>(bytes_fusion[2] - 1) , static_cast<uint16_t>(bytes_fusion[3] - 1) });
	//		--num_of_fusions;

	//	}
	//}

	//return fusions;
}

bool CModGame::_LoadGameData()
{
	if (m_path_bin_file.empty()) {
		m_path_bin_file = _GetPathBinFile();

		if (m_path_bin_file.empty()) {
			std::cout << "[CModGame::LoadGameData] ERROR: Could not get path to binary file." << "\n";
			return false;
		}
	}

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

std::string CModGame::_GetPathBinFile() const
{
	auto mem = _ReadData(GAME_CONSTS[I_PATH_BIN_FILE]);
	return std::string(mem.begin(), mem.end());
}

bool CModGame::IsDuel() const
{
	return (_GetEnemyHealth() != 0);
}
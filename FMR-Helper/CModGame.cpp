#include "CModGame.h"

#include <iomanip>
#include <filesystem>

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

	std::cout << health << "\n";

	return health;
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
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

bool CModGame::RetryConnection()
{
	if (!m_pHandleProcess) {
		std::cout << "[CModGame::RetryConnection] ERROR: Trying to access an invalid pointer.." << "\n";
		return false;
	}

	if (!m_pHandleProcess->AttachToProcess()) {
		std::cout << "[CModGame::RetryConnection] ERROR: Call to CHandleProcess::AttachToProcess failed." << "\n";
		return false;
	}

	return true;
}

std::vector<uint16_t> CModGame::GetMyHandCards() const
{
	std::vector<uint16_t> current_hand_cards;

	return current_hand_cards;
}

std::vector<uint16_t> CModGame::GetMyTableCards() const
{
	std::vector<uint16_t> current_hand_cards;

	return current_hand_cards;
}

bool CModGame::IsGameRunning() const
{
	if (!m_pHandleProcess) {
		std::cout << "[CModGame::IsGameRunning] ERROR: Trying to access an invalid pointer." << "\n";
		return false;
	}

	return m_pHandleProcess->IsProcessRunning();
}

std::vector<uint8_t> CModGame::_ReadData(GameConsts_t gd) const
{
	return m_pHandleProcess->ReadMemoryFromBaseAddr(gd.offset, gd.len);
}

uint16_t CModGame::_GetEnemyHealth() const
{
	auto mem = _ReadData(GAME_CONSTS[I_ENEMY_HEALTH]);

	uint16_t health = 0;
	std::memcpy(&health, mem.data(), mem.size());

	std::cout << health << "\n";

	return health;
}

bool CModGame::IsDuel() const
{
	return (_GetEnemyHealth() != 0);
}
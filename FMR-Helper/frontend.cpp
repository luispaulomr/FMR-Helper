#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#include "CModGame.h"

std::unique_ptr<std::thread> p_GameThread;
std::unique_ptr<CModGame> p_ModGame;

static bool is_equal(const std::vector<Card_t>& fusions_1, const std::vector<Card_t>& fusions_2)
{
	if (fusions_1.size() != fusions_2.size()) {
		return false;
	}

	for (auto i = 0; i < fusions_1.size(); ++i) {
		if (fusions_1[i].card != fusions_2[i].card) {
			return false;
		}

		if (fusions_1[i].cards != fusions_2[i].cards) {
			return false;
		}

		if (fusions_1[i].uid_cards != fusions_2[i].uid_cards) {
			return false;
		}
	}

	return true;
}

/* 
 * Next:
 * 1. imgui example: and try to read a global and set the images
 */

//void hookApp(void)
int main(int argc, char *argv[])
{
	p_ModGame = std::unique_ptr<CModGame>(new CModGame(L"ePSXe - Enhanced PSX Emulator", L"ePSXe.exe"));
	uint32_t DELAY_MS = 1000;
	std::vector<Card_t> prev_fusions;

	while (true) {
		Sleep(DELAY_MS);

		if (p_ModGame->IsAttached()) {

			//std::cout << "[MAIN] Game is running" << "\n";

			if (p_ModGame->IsDuel()) {
				//std::cout << "[MAIN] Duel has started" << "\n";

				auto curr_fusions = p_ModGame->GetMyFusions();

				if (!is_equal(prev_fusions, curr_fusions)) {
					std::cout << "[MAIN] Fusions:" << "\n";
					p_ModGame->PrintMyFusions(curr_fusions);
					prev_fusions = curr_fusions;
				}
			}
		}
		else {
			std::cout << "[MAIN] Game is not running" << "\n";
			p_ModGame->RetryAttach();
		}
	}
}
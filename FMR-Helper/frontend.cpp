#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#include "CModGame.h"

std::unique_ptr<std::thread> p_GameThread;
std::unique_ptr<CModGame> p_ModGame;

//void hookApp(void)
int main(int argc, char *argv[])
{
	p_ModGame = std::unique_ptr<CModGame>(new CModGame(L"ePSXe - Enhanced PSX Emulator", L"ePSXe.exe"));
	uint32_t DELAY_MS = 5000;

	while (true) {
		Sleep(DELAY_MS);

		if (p_ModGame->IsAttached()) {

			std::cout << "[MAIN] Game is running" << "\n";

			if (p_ModGame->IsDuel()) {
				std::cout << "[MAIN] Duel has started" << "\n";

				p_ModGame->GetMyFusions();
			}
		}
		else {
			std::cout << "[MAIN] Game is not running" << "\n";
			p_ModGame->RetryAttach();
		}
	}
}
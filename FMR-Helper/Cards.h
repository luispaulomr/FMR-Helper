#pragma once

#include <vector>
#include <cmath>

typedef struct Card {

    /* cards that generated this card */

    std::vector<uint16_t> cards;
    std::vector<uint16_t> uid_cards;

    /* actual card */

    uint16_t card = 0;
} Card_t;

typedef struct CardData {
    uint16_t atk = 0;
    uint16_t def = 0;
} CardData_t;

std::vector<Card_t> GetFusions(
    std::vector<uint16_t>& table_cards,
    std::vector<uint16_t>& hand_cards,
    const std::vector<std::vector<Card_t>>& fusions);
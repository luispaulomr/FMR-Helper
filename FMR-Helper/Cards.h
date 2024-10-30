#pragma once

#include <vector>
#include <cmath>

typedef struct Card {

    /* cards that generated this card */

    std::vector<uint16_t> cards;
    std::vector<uint16_t> uid_cards;

    /* actual card */

    uint16_t card;
} Card_t;

std::vector<Card_t> GetFusions(
    std::vector<uint16_t>& table_cards,
    std::vector<uint16_t>& hand_cards,
    std::vector<std::vector<Card_t>>& fusions);
#include "Cards.h"

static void _CreateCard(uint16_t card, Card_t& card_1, Card_t& card_2, Card_t& card_result)
{
    card_result.cards = card_1.cards;
    card_result.cards.insert(card_result.cards.end(), card_2.cards.begin(), card_2.cards.end());

    card_result.uid_cards = card_1.uid_cards;
    card_result.uid_cards.insert(card_result.uid_cards.end(), card_2.uid_cards.begin(), card_2.uid_cards.end());

    card_result.card = card;
}

static bool _GetFusion(Card_t& card_1, Card_t& card_2, Card_t& card_result, std::vector<std::vector<Card_t>>& fusions)
{
    for (auto uid_card_1 : card_1.uid_cards) {
        for (auto uid_card_2 : card_2.uid_cards) {
            if (uid_card_1 == uid_card_2) {
                return false;
            }
        }
    }

    /* need to check both ways. Some cards only list fusions on one way but not the other */

    for (auto& t_card : fusions[card_1.card]) {
        
        if (t_card.cards[1] == card_2.card) {

            /* this is a valid fusion!! */

            _CreateCard(t_card.card, card_1, card_2, card_result);
            return true;
        }

    }

    for (auto& t_card : fusions[card_2.card]) {

        if (t_card.cards[1] == card_1.card) {

            /* this is a valid fusion!! */

            _CreateCard(t_card.card, card_1, card_2, card_result);
            return true;
        }

    }

    return false;
}

static std::vector<Card_t> _GetFusionFirstIteration(
    std::vector<uint16_t>& table_cards,
    std::vector<uint16_t>& hand_cards,
    std::vector<std::vector<Card_t>>& fusions)
{
    std::vector<Card_t> ret;

    /* get fusions for hand cards */

    for (auto i = 0; i < 5; ++i) {
        for (auto j = i + 1; j < 4; ++j) {

            Card_t card_1;
            card_1.card = hand_cards[i];
            card_1.cards.push_back(card_1.card);
            card_1.uid_cards.push_back(i + 5);

            Card_t card_2;
            card_2.card = hand_cards[j];
            card_2.cards.push_back(card_2.card);
            card_2.uid_cards.push_back(j + 5);

            Card_t card_result;

            if (_GetFusion(card_1, card_2, card_result, fusions)) {
                ret.push_back(card_result);
            }

        }
    }

    /* get fusions for table cards */

    for (auto i = 0; i < table_cards.size(); ++i) {
        for (auto j = 0; j < 5; ++j) {

            Card_t card_1;
            card_1.card = table_cards[i];
            card_1.cards.push_back(card_1.card);
            card_1.uid_cards.push_back(i);

            Card_t card_2;
            card_2.card = hand_cards[j];
            card_2.cards.push_back(card_2.card);
            card_2.uid_cards.push_back(j + 5);

            Card_t card_result;

            if (_GetFusion(card_1, card_2, card_result, fusions)) {
                ret.push_back(card_result);
            }

        }
    }

    return ret;
}

static std::vector<Card_t> _GetFusionsNthIteration(
    std::vector<uint16_t>& table_cards,
    std::vector<uint16_t>& hand_cards,
    std::vector<std::vector<Card_t>>& fusions,
    std::vector<Card_t>& prev_results)
{
    std::vector<Card_t> ret;

    /* get fusions for hand cards */

    for (auto& prev_result : prev_results) {
        for (auto i = 0; i < 5; ++i) {

            Card_t card_2;
            card_2.cards.push_back(hand_cards[i]);
            card_2.uid_cards.push_back(i + 5);

            Card_t card_result;

            if (_GetFusion(prev_result, card_2, card_result, fusions)) {
                ret.push_back(card_result);
            }

        }
    }

    return ret;
}

std::vector<Card_t> GetFusions(
    std::vector<uint16_t>& table_cards,
    std::vector<uint16_t>& hand_cards,
    std::vector<std::vector<Card_t>>& fusions)
{
    std::vector<Card_t> ret;
    bool isFirstRun = true;

    while (1) {

        std::vector<Card_t> tmp;

        if (isFirstRun) {
            tmp = _GetFusionFirstIteration(table_cards, hand_cards, fusions);
            isFirstRun = false;
        } else {
            tmp = _GetFusionsNthIteration(table_cards, hand_cards, fusions, tmp);
        }

        /* no more fusions, leave */

        if (!tmp.size()) {
            break;
        }

        ret.insert(ret.end(), tmp.begin(), tmp.end());
    }

    return ret;
}
/**
*
* Solution to course project # 2
* Introduction to programming course
* Faculty of Mathematics and Informatics of Sofia University
* Winter semester 2025/2026
*
* @author Nikolay Keremedchiev
* @idnumber 9MI0600633 * @compiler VCC
*
* <file with helper functions>
* 
*/

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

const int DECK_SIZE = 24;
const int HAND_SIZE = 6;
const int WIN_POINTS = 66;
const int SCHNEIDER = 33;
const int TARGET_GAME_POINTS = 11;
const int MARRIAGE_POINTS = 20;
const int TRUMP_MARRIAGE_POINTS = 40;
const int LAST_TRICK_BONUS = 10;

struct Card {
    int suit;
    int rank;
    bool valid;
};

struct LastTrick {
    Card card1;
    Card card2;
    int winner;
    bool exists;
};

struct RoundResult {
    int winner;
    int winnerPoints;
    int p1Points;
    int p2Points;
};

Card deck[DECK_SIZE];
Card hand1[HAND_SIZE], hand2[HAND_SIZE];

int handSize1 = 0;
int handSize2 = 0;
int deckTop = 0;

Card trumpCard;
int trumpSuit = 0;

int score1 = 0, score2 = 0;
int gamePoints1 = 0, gamePoints2 = 0;

int tricks1 = 0, tricks2 = 0;
int currentPlayer = 1;
int roundNumber = 0;

bool gameStarted = false;
bool gameEnded = false;

bool deckClosed = false;
int closedBy = 0;

Card leadCard;
int leadPlayer = 0;

LastTrick lastTrick;

bool mustPlayMarriage = false;
int marriageSuit = 0;

int targetPoints = 11;
int marriagePointsNormal = 20;
int marriagePointsTrump = 40;
bool showPoints = true;
bool lastTrickBonus = true;

RoundResult history[50];
int historyCount = 0;

int getPoints(int rank) {
    int pts[] = { 0, 2, 3, 4, 10, 11 };
    return pts[rank];
}

void printRank(int rank) {
    switch (rank) {
    case 0: cout << '9'; break;
    case 1: cout << 'J'; break;
    case 2: cout << 'Q'; break;
    case 3: cout << 'K'; break;
    case 4: cout << "10"; break;
    case 5: cout << 'A'; break;
    }
}

void printSuit(int suit) {
    switch (suit) {
    case 0: cout << "\xE2\x99\xA0"; break;
    case 1: cout << "\033[91m\xE2\x99\xA5\033[0m"; break;
    case 2: cout << "\033[91m\xE2\x99\xA6\033[0m"; break;
    case 3: cout << "\xE2\x99\xA3"; break;
    }
}

const char* suitName(int suit) {
    switch (suit) {
    case 0: return "Spades";
    case 1: return "Hearts";
    case 2: return "Diamonds";
    case 3: return "Clubs";
    }
    return "?";
}

void printCard(Card c) {
    if (!c.valid) {
        cout << "--";
        return;
    }
    printRank(c.rank);
    printSuit(c.suit);
}

void printHand(Card* hand, int size) {
    cout << "[ ";
    for (int i = 0; i < size; i++) {
        if (i > 0) cout << ", ";
        printCard(hand[i]);
    }
    cout << " ]" << endl;
}

void printHandWithIndex(Card* hand, int size) {
    for (int i = 0; i < size; i++) {
        cout << i << ':';
        printCard(hand[i]);
        if (i < size - 1) cout << ' ';
    }
    cout << endl;
}

void initDeck() {
    int idx = 0;
    for (int s = 0; s < 4; s++) {
        for (int r = 0; r < 6; r++) {
            deck[idx].suit = s;
            deck[idx].rank = r;
            deck[idx].valid = true;
            idx++;
        }
    }
    deckTop = 0;
    deckClosed = false;
}

void shuffle() {
    for (int i = DECK_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}

Card drawCard() {
    if (deckTop >= DECK_SIZE - 1) {
        Card c;
        c.valid = false;
        return c;
    }
    return deck[deckTop++];
}

int cardsLeft() {
    int left = (DECK_SIZE - 1) - deckTop;
    if (left < 0) left = 0;
    if (trumpCard.valid) left++;
    return left;
}

void removeCard(Card* hand, int& size, int index) {
    for (int i = index; i < size - 1; i++) {
        hand[i] = hand[i + 1];
    }
    size--;
}

void sortHand(Card* hand, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            int v1 = hand[j].suit * 10 + (5 - hand[j].rank);
            int v2 = hand[j + 1].suit * 10 + (5 - hand[j + 1].rank);
            if (v1 > v2) {
                Card tmp = hand[j];
                hand[j] = hand[j + 1];
                hand[j + 1] = tmp;
            }
        }
    }
}

bool hasSuit(Card* hand, int size, int suit) {
    for (int i = 0; i < size; i++) {
        if (hand[i].valid && hand[i].suit == suit)
            return true;
    }
    return false;
}

bool hasMarriage(Card* hand, int size, int suit) {
    bool hasK = false, hasQ = false;
    for (int i = 0; i < size; i++) {
        if (hand[i].valid && hand[i].suit == suit) {
            if (hand[i].rank == 3) hasK = true;
            if (hand[i].rank == 2) hasQ = true;
        }
    }
    return hasK && hasQ;
}

int findCard(Card* hand, int size, int suit, int rank) {
    for (int i = 0; i < size; i++) {
        if (hand[i].valid && hand[i].suit == suit && hand[i].rank == rank)
            return i;
    }
    return -1;
}

bool strictRules() {
    return deckClosed || cardsLeft() == 0;
}

bool winsOver(Card c1, Card c2, int leadSuit) {
    if (c1.suit == trumpSuit && c2.suit != trumpSuit) return true;
    if (c2.suit == trumpSuit && c1.suit != trumpSuit) return false;
    if (c1.suit == c2.suit) return c1.rank > c2.rank;
    if (c1.suit == leadSuit) return true;
    return false;
}

bool validPlay(Card* hand, int size, int index, Card lead) {
    if (!strictRules() || !lead.valid) return true;

    Card card = hand[index];
    int leadSuit = lead.suit;

    if (hasSuit(hand, size, leadSuit)) {
        if (card.suit != leadSuit) return false;

        bool canBeat = false;
        for (int i = 0; i < size; i++) {
            if (hand[i].suit == leadSuit && hand[i].rank > lead.rank) {
                canBeat = true;
                break;
            }
        }
        if (canBeat && card.rank <= lead.rank) return false;
        return true;
    }

    if (hasSuit(hand, size, trumpSuit)) {
        return card.suit == trumpSuit;
    }

    return true;
}

void dealCards() {
    handSize1 = 0;
    handSize2 = 0;

    for (int i = 0; i < 3; i++) hand1[handSize1++] = drawCard();
    for (int i = 0; i < 3; i++) hand2[handSize2++] = drawCard();
    for (int i = 0; i < 3; i++) hand1[handSize1++] = drawCard();
    for (int i = 0; i < 3; i++) hand2[handSize2++] = drawCard();

    trumpCard = deck[DECK_SIZE - 1];
    trumpCard.valid = true;
    trumpSuit = trumpCard.suit;

    sortHand(hand1, handSize1);
    sortHand(hand2, handSize2);
}

void drawAfterTrick(int winner) {
    if (deckClosed || cardsLeft() == 0) return;

    Card* winHand = (winner == 1) ? hand1 : hand2;
    int& winSize = (winner == 1) ? handSize1 : handSize2;
    Card* loseHand = (winner == 1) ? hand2 : hand1;
    int& loseSize = (winner == 1) ? handSize2 : handSize1;

    if (deckTop < DECK_SIZE - 1) {
        winHand[winSize++] = drawCard();
    }

    if (deckTop < DECK_SIZE - 1) {
        loseHand[loseSize++] = drawCard();
    }
    else if (trumpCard.valid) {
        loseHand[loseSize++] = trumpCard;
        trumpCard.valid = false;
    }

    sortHand(hand1, handSize1);
    sortHand(hand2, handSize2);
}

void showTurnInfo() {
    Card* hand = (currentPlayer == 1) ? hand1 : hand2;
    int size = (currentPlayer == 1) ? handSize1 : handSize2;

    cout << endl;
    cout << "=== Player " << currentPlayer << "'s turn ===" << endl;

    if (leadCard.valid) {
        cout << "Opponent played: "; printCard(leadCard); cout << endl;
        cout << "Your hand: ";
        printHandWithIndex(hand, size);
        if (strictRules()) cout << "[Strict rules]" << endl;
        cout << "Commands: play <0-" << size - 1 << ">" << endl;
    }
    else {
        cout << "You are under hand." << endl;
        cout << "Your hand: ";
        printHandWithIndex(hand, size);
        if (showPoints) {
            cout << "Points: P1=" << score1 << " | P2=" << score2 << endl;
        }
        if (strictRules()) cout << "[Strict rules]" << endl;
        cout << "Commands: play <0-" << size - 1 << ">, marriage, switch-nine, close, stop" << endl;
        cout << "Info: trump, status, last-trick" << endl;
    }
}

void endRound(bool stopped, int stopper) {
    if (!stopped && lastTrick.exists && lastTrickBonus) {
        if (lastTrick.winner == 1) score1 += LAST_TRICK_BONUS;
        else score2 += LAST_TRICK_BONUS;
    }

    int winner = 0;
    int pts = 0;

    if (stopped) {
        int stopperScore = (stopper == 1) ? score1 : score2;
        int otherScore = (stopper == 1) ? score2 : score1;
        int otherTricks = (stopper == 1) ? tricks2 : tricks1;

        if (stopperScore >= WIN_POINTS) {
            winner = stopper;
            if (otherScore >= SCHNEIDER) pts = 1;
            else if (otherTricks > 0) pts = 2;
            else pts = 3;
        }
        else {
            winner = (stopper == 1) ? 2 : 1;
            pts = (stopperScore == 0) ? 3 : 2;
        }
    }
    else {
        int lastW = lastTrick.winner;
        int lastScore = (lastW == 1) ? score1 : score2;
        int otherScore = (lastW == 1) ? score2 : score1;
        int otherTricks = (lastW == 1) ? tricks2 : tricks1;

        if (lastScore >= WIN_POINTS) {
            winner = lastW;
            if (otherScore >= SCHNEIDER) pts = 1;
            else if (otherTricks > 0) pts = 2;
            else pts = 3;
        }
        else if (otherScore >= WIN_POINTS) {
            winner = (lastW == 1) ? 2 : 1;
            int lTricks = (lastW == 1) ? tricks1 : tricks2;
            if (lastScore >= SCHNEIDER) pts = 1;
            else if (lTricks > 0) pts = 2;
            else pts = 3;
        }
        else if (score1 == score2) {
            winner = 0;
            pts = 0;
        }
        else {
            winner = (score1 > score2) ? 1 : 2;
            pts = 1;
        }
    }

    if (winner == 1) gamePoints1 += pts;
    else if (winner == 2) gamePoints2 += pts;

    if (historyCount < 50) {
        history[historyCount].winner = winner;
        history[historyCount].winnerPoints = pts;
        history[historyCount].p1Points = score1;
        history[historyCount].p2Points = score2;
        historyCount++;
    }

    cout << endl;
    cout << "Round " << roundNumber << " ended." << endl;
    cout << "Calculating points..." << endl;
    if (winner > 0) {
        cout << "Player " << winner << " wins the round! (+" << pts << " game points)" << endl;
    }
    else {
        cout << "Tie! No points awarded." << endl;
    }
    cout << "Player 1: " << score1 << " | Player 2: " << score2 << endl;
}

void startRound() {
    roundNumber++;
    score1 = 0;
    score2 = 0;
    tricks1 = 0;
    tricks2 = 0;
    deckClosed = false;
    closedBy = 0;
    leadCard.valid = false;
    leadPlayer = 0;
    lastTrick.exists = false;
    mustPlayMarriage = false;

    initDeck();
    shuffle();
    dealCards();

    cout << "Starting Round " << roundNumber << "." << endl;
    cout << "Trump suit: "; printSuit(trumpSuit);
    cout << " (" << suitName(trumpSuit) << ")" << endl;
    cout << "Bottom card: "; printCard(trumpCard); cout << endl;
    cout << endl;
}

bool saveGame(const char* filename) {
    ofstream file(filename);
    if (!file.is_open()) return false;

    file << "SANTASE66" << endl;
    file << roundNumber << " " << currentPlayer << " " << gameStarted << " " << gameEnded << endl;
    file << score1 << " " << score2 << " " << gamePoints1 << " " << gamePoints2 << endl;
    file << tricks1 << " " << tricks2 << " " << deckClosed << " " << closedBy << endl;
    file << trumpSuit << " " << trumpCard.suit << " " << trumpCard.rank << " " << trumpCard.valid << endl;
    file << deckTop << endl;

    file << handSize1 << endl;
    for (int i = 0; i < handSize1; i++) {
        file << hand1[i].suit << " " << hand1[i].rank << " " << hand1[i].valid << endl;
    }

    file << handSize2 << endl;
    for (int i = 0; i < handSize2; i++) {
        file << hand2[i].suit << " " << hand2[i].rank << " " << hand2[i].valid << endl;
    }

    file << leadCard.suit << " " << leadCard.rank << " " << leadCard.valid << " " << leadPlayer << endl;
    file << lastTrick.exists << " " << lastTrick.winner << endl;

    file << targetPoints << " " << marriagePointsNormal << " " << marriagePointsTrump << endl;
    file << showPoints << " " << lastTrickBonus << endl;

    file << historyCount << endl;
    for (int i = 0; i < historyCount; i++) {
        file << history[i].winner << " " << history[i].winnerPoints << " ";
        file << history[i].p1Points << " " << history[i].p2Points << endl;
    }

    file.close();
    return true;
}

bool loadGame(const char* filename) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    char header[20];
    file.getline(header, 20);

    file >> roundNumber >> currentPlayer >> gameStarted >> gameEnded;
    file >> score1 >> score2 >> gamePoints1 >> gamePoints2;
    file >> tricks1 >> tricks2 >> deckClosed >> closedBy;
    file >> trumpSuit >> trumpCard.suit >> trumpCard.rank >> trumpCard.valid;
    file >> deckTop;

    file >> handSize1;
    for (int i = 0; i < handSize1; i++) {
        file >> hand1[i].suit >> hand1[i].rank >> hand1[i].valid;
    }

    file >> handSize2;
    for (int i = 0; i < handSize2; i++) {
        file >> hand2[i].suit >> hand2[i].rank >> hand2[i].valid;
    }

    file >> leadCard.suit >> leadCard.rank >> leadCard.valid >> leadPlayer;
    file >> lastTrick.exists >> lastTrick.winner;

    file >> targetPoints >> marriagePointsNormal >> marriagePointsTrump;
    file >> showPoints >> lastTrickBonus;

    file >> historyCount;
    for (int i = 0; i < historyCount; i++) {
        file >> history[i].winner >> history[i].winnerPoints;
        file >> history[i].p1Points >> history[i].p2Points;
    }

    file.close();
    return true;
}

bool strEq(const char* a, const char* b) {
    while (*a && *b) {
        char ca = (*a >= 'A' && *a <= 'Z') ? (*a + 32) : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? (*b + 32) : *b;
        if (ca != cb) return false;
        a++; b++;
    }
    return *a == *b;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    srand(time(nullptr));

    cout << "SANTASE (66)" << endl;
    cout << "\xE2\x99\xA0 Spades  \033[91m\xE2\x99\xA5\033[0m Hearts  \033[91m\xE2\x99\xA6\033[0m Diamonds  \xE2\x99\xA3 Clubs" << endl;
    cout << "A=11 10=10 K=4 Q=3 J=2 9=0" << endl;
    cout << endl;
    cout << "Commands: start, rules, settings, hand, play <index>," << endl;
    cout << "marriage, switch-nine, close, stop, trump, last-trick," << endl;
    cout << "status, history, surrender, save, load, quit" << endl;
    cout << endl;

    char input[256];

    while (true) {
        cout << "> ";

        cin.getline(input, 256);
        if (cin.eof()) break;

        char cmd[32] = "";
        char arg[64] = "";

        int i = 0;
        while (input[i] == ' ') i++;
        int j = 0;
        while (input[i] && input[i] != ' ' && j < 31) cmd[j++] = input[i++];
        cmd[j] = '\0';

        while (input[i] == ' ') i++;
        j = 0;
        while (input[i] && j < 63) arg[j++] = input[i++];
        arg[j] = '\0';

        if (strEq(cmd, "start")) {
            if (gameStarted && !gameEnded) {
                cout << "Game already in progress." << endl;
            }
            else {
                gameStarted = true;
                gameEnded = false;
                gamePoints1 = 0;
                gamePoints2 = 0;
                roundNumber = 0;
                historyCount = 0;
                currentPlayer = 1;
                cout << "The game started!" << endl;
                startRound();
                cout << "Player 1's turn" << endl;
                showTurnInfo();
            }
        }
        else if (strEq(cmd, "rules")) {
            cout << endl;
            cout << "SANTASE (66)" << endl;
            cout << "Each player gets 6 cards. The Trump suit is chosen at random." << endl;
            cout << "Card values: A=11, 10=10, K=4, Q=3, J=2, 9=0." << endl;
            cout << "A marriage (K+Q of the same suit) gives 20 points, or 40 if trump suit." << endl;
            cout << "The first player to reach 66 points wins the round." << endl;
            cout << "The first player to reach " << targetPoints << " game points wins the game." << endl;
            cout << endl;
        }
        else if (strEq(cmd, "settings")) {
            if (gameStarted && !gameEnded) {
                cout << "Cannot change settings during a game." << endl;
            }
            else {
                bool inSettings = true;
                while (inSettings) {
                    cout << endl;
                    cout << "SETTINGS" << endl;
                    cout << "1) Target points to win [" << targetPoints << "]" << endl;
                    cout << "2) Marriage points (non-trump/trump) [" << marriagePointsNormal << "/" << marriagePointsTrump << "]" << endl;
                    cout << "3) Show players' points [" << (showPoints ? "on" : "off") << "]" << endl;
                    cout << "4) Last trick +10 [" << (lastTrickBonus ? "on" : "off") << "]" << endl;
                    cout << "Enter number to change or 'back' to return: ";

                    char choice[10];
                    cin.getline(choice, 10);

                    if (choice[0] == '1') {
                        cout << "Enter new target points: ";
                        cin.getline(choice, 10);
                        int val = 0;
                        for (int k = 0; choice[k] >= '0' && choice[k] <= '9'; k++) {
                            val = val * 10 + (choice[k] - '0');
                        }
                        if (val > 0) {
                            targetPoints = val;
                            cout << "Target points set to " << targetPoints << endl;
                        }
                    }
                    else if (choice[0] == '2') {
                        cout << "Enter marriage points (non-trump): ";
                        cin.getline(choice, 10);
                        int val = 0;
                        for (int k = 0; choice[k] >= '0' && choice[k] <= '9'; k++) {
                            val = val * 10 + (choice[k] - '0');
                        }
                        if (val > 0) marriagePointsNormal = val;

                        cout << "Enter marriage points (trump): ";
                        cin.getline(choice, 10);
                        val = 0;
                        for (int k = 0; choice[k] >= '0' && choice[k] <= '9'; k++) {
                            val = val * 10 + (choice[k] - '0');
                        }
                        if (val > 0) marriagePointsTrump = val;
                        cout << "Marriage points set to " << marriagePointsNormal << "/" << marriagePointsTrump << endl;
                    }
                    else if (choice[0] == '3') {
                        showPoints = !showPoints;
                        cout << "Show points: " << (showPoints ? "on" : "off") << endl;
                    }
                    else if (choice[0] == '4') {
                        lastTrickBonus = !lastTrickBonus;
                        cout << "Last trick bonus: " << (lastTrickBonus ? "on" : "off") << endl;
                    }
                    else if (strEq(choice, "back")) {
                        inSettings = false;
                    }
                }
                cout << endl;
            }
        }
        else if (strEq(cmd, "help")) {
            cout << endl;
            cout << "COMMANDS:" << endl;
            cout << "  start              - Start new game" << endl;
            cout << "  rules              - Show rules" << endl;
            cout << "  settings           - Game settings" << endl;
            cout << "  hand               - Show your hand" << endl;
            cout << "  play <index>       - Play card" << endl;
            cout << "  marriage <S/H/D/C> - Declare marriage" << endl;
            cout << "  switch-nine        - Exchange trump 9" << endl;
            cout << "  close              - Close deck" << endl;
            cout << "  stop               - Declare 66+" << endl;
            cout << "  trump              - Show trump" << endl;
            cout << "  last-trick         - Show last trick" << endl;
            cout << "  status             - Show status" << endl;
            cout << "  history            - Show history" << endl;
            cout << "  surrender          - Surrender round" << endl;
            cout << "  surrender-forever  - Surrender game" << endl;
            cout << "  save <name>        - Save game" << endl;
            cout << "  load <name>        - Load game" << endl;
            cout << "  quit               - Exit" << endl;
            cout << endl;
        }
        else if (strEq(cmd, "hand")) {
            if (!gameStarted || gameEnded) {
                cout << "No game in progress." << endl;
            }
            else {
                Card* hand = (currentPlayer == 1) ? hand1 : hand2;
                int size = (currentPlayer == 1) ? handSize1 : handSize2;
                cout << "Your hand (P" << currentPlayer << "): ";
                printHand(hand, size);
            }
        }
        else if (strEq(cmd, "play")) {
            if (!gameStarted || gameEnded) {
                cout << "No game in progress." << endl;
            }
            else if (arg[0] < '0' || arg[0] > '9') {
                cout << "Usage: play <index>" << endl;
            }
            else {
                int idx = arg[0] - '0';
                Card* hand = (currentPlayer == 1) ? hand1 : hand2;
                int& size = (currentPlayer == 1) ? handSize1 : handSize2;

                if (idx >= size) {
                    cout << "Invalid index." << endl;
                }
                else if (mustPlayMarriage && (hand[idx].suit != marriageSuit ||
                    (hand[idx].rank != 2 && hand[idx].rank != 3))) {
                    cout << "You must play K"; printSuit(marriageSuit);
                    cout << " or Q"; printSuit(marriageSuit); cout << endl;
                }
                else if (!validPlay(hand, size, idx, leadCard)) {
                    cout << "Invalid play - strict rules in effect." << endl;
                }
                else {
                    Card played = hand[idx];
                    removeCard(hand, size, idx);

                    cout << "P" << currentPlayer << " played ";
                    printCard(played);
                    cout << endl;

                    mustPlayMarriage = false;

                    if (!leadCard.valid) {
                        leadCard = played;
                        leadPlayer = currentPlayer;
                        currentPlayer = (currentPlayer == 1) ? 2 : 1;
                        cout << endl;
                        showTurnInfo();
                    }
                    else {
                        Card first = (leadPlayer == 1) ? leadCard : played;
                        Card second = (leadPlayer == 1) ? played : leadCard;
                        bool firstWins = winsOver(first, second, first.suit);
                        int winner = firstWins ? leadPlayer : ((leadPlayer == 1) ? 2 : 1);

                        int trickPts = getPoints(leadCard.rank) + getPoints(played.rank);

                        lastTrick.card1 = (leadPlayer == 1) ? leadCard : played;
                        lastTrick.card2 = (leadPlayer == 1) ? played : leadCard;
                        lastTrick.winner = winner;
                        lastTrick.exists = true;

                        if (winner == 1) {
                            score1 += trickPts;
                            tricks1++;
                        }
                        else {
                            score2 += trickPts;
                            tricks2++;
                        }

                        cout << "P" << winner << " wins the trick! (+" << trickPts << " points)" << endl;

                        drawAfterTrick(winner);
                        currentPlayer = winner;
                        leadCard.valid = false;
                        leadPlayer = 0;

                        if (handSize1 == 0 && handSize2 == 0) {
                            endRound(false, 0);

                            if (gamePoints1 >= targetPoints) {
                                gameEnded = true;
                                cout << endl;
                                cout << "*** GAME OVER! Player 1 wins the game! ***" << endl;
                                cout << "Final score: Player 1 - " << gamePoints1 << " | Player 2 - " << gamePoints2 << endl;
                            }
                            else if (gamePoints2 >= targetPoints) {
                                gameEnded = true;
                                cout << endl;
                                cout << "*** GAME OVER! Player 2 wins the game! ***" << endl;
                                cout << "Final score: Player 1 - " << gamePoints1 << " | Player 2 - " << gamePoints2 << endl;
                            }
                            else {
                                cout << endl;
                                startRound();
                                showTurnInfo();
                            }
                        }
                        else {
                            cout << endl;
                            showTurnInfo();
                        }
                    }
                }
            }
        }
        else if (strEq(cmd, "marriage")) {
            if (!gameStarted || gameEnded) {
                cout << "No game in progress." << endl;
            }
            else if (leadCard.valid) {
                cout << "You must be under hand to declare marriage." << endl;
            }
            else if ((currentPlayer == 1 ? tricks1 : tricks2) == 0) {
                cout << "You must have at least one trick." << endl;
            }
            else if (arg[0] == '\0') {
                cout << "Usage: marriage <S/H/D/C>" << endl;
            }
            else {
                int suit = 0;
                char c = (arg[0] >= 'a') ? (arg[0] - 32) : arg[0];
                if (c == 'S') suit = 0;
                else if (c == 'H') suit = 1;
                else if (c == 'D') suit = 2;
                else if (c == 'C') suit = 3;

                Card* hand = (currentPlayer == 1) ? hand1 : hand2;
                int size = (currentPlayer == 1) ? handSize1 : handSize2;
                int& score = (currentPlayer == 1) ? score1 : score2;

                if (!hasMarriage(hand, size, suit)) {
                    cout << "You don't have K+Q of " << suitName(suit) << endl;
                }
                else {
                    int pts = (suit == trumpSuit) ? marriagePointsTrump : marriagePointsNormal;
                    score += pts;
                    mustPlayMarriage = true;
                    marriageSuit = suit;
                    cout << "Marriage declared: K"; printSuit(suit);
                    cout << " + Q"; printSuit(suit);
                    if (suit == trumpSuit) cout << " (trump suit)";
                    cout << endl;
                    cout << "You earned " << pts << " points." << endl;
                    cout << "You must play K"; printSuit(suit);
                    cout << " or Q"; printSuit(suit); cout << endl;
                }
            }
        }
        else if (strEq(cmd, "switch-nine")) {
            if (!gameStarted || gameEnded) {
                cout << "No game in progress." << endl;
            }
            else if (leadCard.valid) {
                cout << "You must be under hand." << endl;
            }
            else if ((currentPlayer == 1 ? tricks1 : tricks2) == 0) {
                cout << "You must have at least one trick." << endl;
            }
            else if (deckClosed || cardsLeft() <= 2) {
                cout << "You cannot switch now. Stock is closed or not enough cards." << endl;
            }
            else {
                Card* hand = (currentPlayer == 1) ? hand1 : hand2;
                int size = (currentPlayer == 1) ? handSize1 : handSize2;
                int idx = findCard(hand, size, trumpSuit, 0);

                if (idx < 0) {
                    cout << "You don't have 9"; printSuit(trumpSuit); cout << endl;
                }
                else {
                    Card old = trumpCard;
                    trumpCard = hand[idx];
                    hand[idx] = old;
                    sortHand(hand, size);
                    cout << "You exchanged 9"; printSuit(trumpSuit);
                    cout << " for "; printCard(old);
                    cout << " (trump suit)" << endl;
                }
            }
        }
        else if (strEq(cmd, "close")) {
            if (!gameStarted || gameEnded) {
                cout << "No game in progress." << endl;
            }
            else if (leadCard.valid) {
                cout << "You must be under hand." << endl;
            }
            else if ((currentPlayer == 1 ? tricks1 : tricks2) == 0) {
                cout << "You must have at least one trick." << endl;
            }
            else if (deckClosed || cardsLeft() <= 2) {
                cout << "Stock already closed or not enough cards." << endl;
            }
            else {
                deckClosed = true;
                closedBy = currentPlayer;
                cout << "Stock closed. No more cards will be drawn." << endl;
                cout << "Strict rules are now in effect." << endl;
            }
        }
        else if (strEq(cmd, "stop")) {
            if (!gameStarted || gameEnded) {
                cout << "No game in progress." << endl;
            }
            else if (leadCard.valid) {
                cout << "You must be under hand." << endl;
            }
            else {
                cout << "Player " << currentPlayer << " stops the round!" << endl;
                endRound(true, currentPlayer);

                if (gamePoints1 >= targetPoints) {
                    gameEnded = true;
                    cout << endl;
                    cout << "*** GAME OVER! Player 1 wins the game! ***" << endl;
                }
                else if (gamePoints2 >= targetPoints) {
                    gameEnded = true;
                    cout << endl;
                    cout << "*** GAME OVER! Player 2 wins the game! ***" << endl;
                }
                else {
                    cout << endl;
                    startRound();
                    showTurnInfo();
                }
            }
        }
        else if (strEq(cmd, "trump")) {
            if (!gameStarted) {
                cout << "No game in progress." << endl;
            }
            else {
                cout << "Trump suit: "; printSuit(trumpSuit); cout << endl;
            }
        }
        else if (strEq(cmd, "last-trick")) {
            if (!lastTrick.exists) {
                cout << "No tricks played yet." << endl;
            }
            else {
                cout << "Player 1: "; printCard(lastTrick.card1); cout << endl;
                cout << "Player 2: "; printCard(lastTrick.card2); cout << endl;
                cout << "Winner: Player " << lastTrick.winner << endl;
            }
        }
        else if (strEq(cmd, "status")) {
            if (!gameStarted) {
                cout << "No game in progress." << endl;
            }
            else {
                cout << endl;
                cout << "Round: " << roundNumber << endl;
                cout << "Game points: Player 1 - " << gamePoints1 << " | Player 2 - " << gamePoints2 << endl;
                cout << "Round points: Player 1 - " << score1 << " | Player 2 - " << score2 << endl;
                cout << "Tricks won: Player 1 - " << tricks1 << " | Player 2 - " << tricks2 << endl;
                cout << "Trump suit: "; printSuit(trumpSuit); cout << endl;
                cout << "Bottom card: "; printCard(trumpCard); cout << endl;
                cout << "Cards in deck: " << cardsLeft();
                if (deckClosed) cout << " (CLOSED)";
                cout << endl << endl;
            }
        }
        else if (strEq(cmd, "history")) {
            if (historyCount == 0) {
                cout << "No rounds completed yet." << endl;
            }
            else {
                for (int k = 0; k < historyCount; k++) {
                    cout << "Round " << (k + 1) << ": Winner - Player " << history[k].winner;
                    cout << " (+" << history[k].winnerPoints << ") | ";
                    cout << "Player 1: " << history[k].p1Points << " points | ";
                    cout << "Player 2: " << history[k].p2Points << " points" << endl;
                }
                if (gameStarted && !gameEnded) {
                    cout << "Round " << roundNumber << ": Ongoing" << endl;
                }
                cout << "Overall: Player 1 - " << gamePoints1 << " | Player 2 - " << gamePoints2 << endl;
            }
        }
        else if (strEq(cmd, "surrender")) {
            if (!gameStarted || gameEnded) {
                cout << "No game in progress." << endl;
            }
            else {
                int loser = currentPlayer;
                int winner = (loser == 1) ? 2 : 1;
                int loserScore = (loser == 1) ? score1 : score2;
                int loserTricks = (loser == 1) ? tricks1 : tricks2;

                int pts = (loserScore >= SCHNEIDER) ? 1 : (loserTricks > 0) ? 2 : 3;
                if (winner == 1) gamePoints1 += pts;
                else gamePoints2 += pts;

                cout << "Player " << loser << " surrenders!" << endl;
                cout << "Player " << winner << " wins the round! (+" << pts << " game points)" << endl;

                if (historyCount < 50) {
                    history[historyCount].winner = winner;
                    history[historyCount].winnerPoints = pts;
                    history[historyCount].p1Points = score1;
                    history[historyCount].p2Points = score2;
                    historyCount++;
                }

                if (gamePoints1 >= targetPoints || gamePoints2 >= targetPoints) {
                    gameEnded = true;
                    int w = (gamePoints1 >= targetPoints) ? 1 : 2;
                    cout << endl;
                    cout << "*** GAME OVER! Player " << w << " wins the game! ***" << endl;
                }
                else {
                    cout << endl;
                    startRound();
                    showTurnInfo();
                }
            }
        }
        else if (strEq(cmd, "surrender-forever")) {
            if (!gameStarted || gameEnded) {
                cout << "No game in progress." << endl;
            }
            else {
                int winner = (currentPlayer == 1) ? 2 : 1;
                gameEnded = true;
                cout << "Player " << currentPlayer << " surrenders the game!" << endl;
                cout << "*** GAME OVER! Player " << winner << " wins! ***" << endl;
            }
        }
        else if (strEq(cmd, "save")) {
            if (!gameStarted) {
                cout << "No game to save." << endl;
            }
            else if (arg[0] == '\0') {
                cout << "Usage: save <name>" << endl;
            }
            else {
                char filename[80];
                int k = 0;
                while (arg[k] && k < 70) { filename[k] = arg[k]; k++; }
                filename[k++] = '.'; filename[k++] = 't';
                filename[k++] = 'x'; filename[k++] = 't'; filename[k] = '\0';

                if (saveGame(filename)) {
                    cout << "Game saved successfully as '" << filename << "'." << endl;
                }
                else {
                    cout << "Error saving game." << endl;
                }
            }
        }
        else if (strEq(cmd, "load")) {
            if (arg[0] == '\0') {
                cout << "Usage: load <name>" << endl;
            }
            else {
                char filename[80];
                int k = 0;
                while (arg[k] && k < 70) { filename[k] = arg[k]; k++; }
                filename[k++] = '.'; filename[k++] = 't';
                filename[k++] = 'x'; filename[k++] = 't'; filename[k] = '\0';

                if (loadGame(filename)) {
                    cout << "Game loaded successfully from '" << filename << "'." << endl;
                    showTurnInfo();
                }
                else {
                    cout << "Error loading game." << endl;
                }
            }
        }
        else if (strEq(cmd, "quit") || strEq(cmd, "exit")) {
            cout << "Goodbye!" << endl;
            break;
        }
        else if (cmd[0] != '\0') {
            cout << "Unknown command. Type 'help' for available commands." << endl;
        }
    }

    return 0;
}
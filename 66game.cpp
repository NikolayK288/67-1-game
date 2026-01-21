#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

const int DECK_SIZE = 24;
const int HAND_SIZE = 6;

struct Card {
    int suit;    
    int rank;    
};

Card deck[DECK_SIZE];
Card hand1[HAND_SIZE], hand2[HAND_SIZE];
int handSize1 = 6, handSize2 = 6;
int deckTop = 0;
Card trumpCard;
int score1 = 0, score2 = 0;

int getPoints(int rank) {
    int points[] = { 0, 2, 3, 4, 10, 11 };
    return points[rank];
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
    case 0: cout << 'S'; break;
    case 1: cout << 'H'; break;
    case 2: cout << 'D'; break;
    case 3: cout << 'C'; break;
    }
}

void printCard(Card c) {
    printRank(c.rank);
    printSuit(c.suit);
}

void printHand(Card* hand, int size) {
    for (int i = 0; i < size; i++) {
        cout << i << ':';
        printCard(hand[i]);
        if (i < size - 1) cout << ' ';
    }
    cout << endl;
}

void initDeck() {
    int i = 0;
    for (int s = 0; s < 4; s++) {
        for (int r = 0; r < 6; r++) {
            deck[i].suit = s;
            deck[i].rank = r;
            i++;
        }
    }
}

void shuffle() {
    for (int i = DECK_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

Card drawCard() {
    return deck[deckTop++];
}

void dealCards() {
    for (int i = 0; i < 3; i++) hand1[i] = drawCard();
    for (int i = 0; i < 3; i++) hand2[i] = drawCard();
    for (int i = 3; i < 6; i++) hand1[i] = drawCard();
    for (int i = 3; i < 6; i++) hand2[i] = drawCard();
    trumpCard = drawCard();
    cout << "Trump: ";
    printCard(trumpCard);
    cout << endl;
}

void removeCard(Card* hand, int& size, int index) {
    for (int i = index; i < size - 1; i++) {
        hand[i] = hand[i + 1];
    }
    size--;
}

bool winsOver(Card card1, Card card2, int leadSuit) {
    if (card1.suit == trumpCard.suit && card2.suit != trumpCard.suit) return true;
    if (card2.suit == trumpCard.suit && card1.suit != trumpCard.suit) return false;
    if (card1.suit == card2.suit) return card1.rank > card2.rank;
    if (card1.suit == leadSuit) return true;
    return false;
}

int main() {
    srand(time(nullptr));

    cout << "SANTASE (66)" << endl;
    cout << "S=Spades H=Hearts D=Diamonds C=Clubs" << endl;
    cout << "A=11 10=10 K=4 Q=3 J=2 9=0" << endl << endl;

    initDeck();
    shuffle();
    dealCards();

    bool player1Turn = true;

    while (handSize1 > 0 && handSize2 > 0) {
        cout << "\n--- P1:" << score1 << " P2:" << score2 << " ---" << endl;

        int currP = player1Turn ? 1 : 2;
        Card* currHand = player1Turn ? hand1 : hand2;
        int& currSize = player1Turn ? handSize1 : handSize2;
        int& currScore = player1Turn ? score1 : score2;

        cout << "P" << currP << " Hand: ";
        printHand(currHand, currSize);
        cout << "Choose card (0-" << currSize - 1 << "): ";

        char input;
        cin >> input;
        int idx1 = input - '0';

        if (idx1 < 0 || idx1 >= currSize) {
            cout << "Invalid!" << endl;
            continue;
        }

        Card card1 = currHand[idx1];
        cout << "P" << currP << " plays: ";
        printCard(card1);
        cout << endl;
        removeCard(currHand, currSize, idx1);

        int otherP = player1Turn ? 2 : 1;
        Card* otherHand = player1Turn ? hand2 : hand1;
        int& otherSize = player1Turn ? handSize2 : handSize1;
        int& otherScore = player1Turn ? score2 : score1;

        cout << "P" << otherP << " Hand: ";
        printHand(otherHand, otherSize);
        cout << "Choose card (0-" << otherSize - 1 << "): ";

        cin >> input;
        int idx2 = input - '0';

        if (idx2 < 0 || idx2 >= otherSize) {
            cout << "Invalid!" << endl;
            currHand[currSize++] = card1;  
            continue;
        }

        Card card2 = otherHand[idx2];
        cout << "P" << otherP << " plays: ";
        printCard(card2);
        cout << endl;
        removeCard(otherHand, otherSize, idx2);

        int trickPts = getPoints(card1.rank) + getPoints(card2.rank);
        bool firstWins = winsOver(card1, card2, card1.suit);

        if (firstWins) {
            currScore += trickPts;
            cout << "P" << currP << " wins! +" << trickPts << endl;
        }
        else {
            otherScore += trickPts;
            cout << "P" << otherP << " wins! +" << trickPts << endl;
            player1Turn = !player1Turn; 
        }

        if (score1 >= 66) {
            cout << "\n*** P1 WINS with " << score1 << " points! ***" << endl;
            return 0;
        }
        if (score2 >= 66) {
            cout << "\n*** P2 WINS with " << score2 << " points! ***" << endl;
            return 0;
        }

        if (deckTop <= DECK_SIZE) {
            Card* winner = firstWins ? currHand : otherHand;
            int& winSize = firstWins ? currSize : otherSize;
            Card* loser = firstWins ? otherHand : currHand;
            int& loseSize = firstWins ? otherSize : currSize;

            if (deckTop < DECK_SIZE) {
                winner[winSize++] = drawCard();
            }
            if (deckTop < DECK_SIZE) {
                loser[loseSize++] = drawCard();
            }
            else {
                loser[loseSize++] = trumpCard;
            }
        }
    }
    cout << "\nGAME OVER" << endl;
    cout << "P1: " << score1 << " P2: " << score2 << endl;
    if (score1 > score2) cout << "Player wins" << endl;
    else if (score2 > score1) cout << "Player wins" << endl;
    else cout << "Draw" << endl;

    return 0;
}
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
int deckTop = 0;
Card trumpCard;

void printRank(int rank) {
	if (rank == 0) cout << '9';
	if (rank == 1) cout << 'J';
	if (rank == 2) cout << 'Q';
	if (rank == 3) cout << 'K';
	if (rank == 4) cout << "10";
	if (rank == 5) cout << 'A';
}

void printSuit(int suit) {
	if (suit == 0) cout << 'S';
	if (suit == 1) cout << 'H';
	if (suit == 2) cout << 'D';
	if (suit == 3) cout << 'C';
}

void printCard(Card c) {
	printRank(c.rank);
	printSuit(c.suit);
}

void printHand(Card* hand, int size) {
	for (int i = 0;i < size;i++) {
		cout << i << ':';
		printCard(hand[i]);
		if (i < size - 1) cout << ' ';
	}
	cout << endl;
}

void initDeck() {
	int i = 0;
	for (int s = 0;s < 4;s++) {
		for (int r = 0;r < 6;r++) {
			deck[i].suit = s;
			deck[i].rank = r;
			i++;
		}
	}
}

void shuffle() {
	for (int i = DECK_SIZE - 1;i > 0;i--) {
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
	for (int i = 0;i < 3;i++) hand1[i] = drawCard();
	for (int i = 0;i < 3;i++) hand2[i] = drawCard();
	for (int i = 3;i < 6;i++) hand1[i] = drawCard();
	for (int i = 3;i < 6;i++) hand2[i] = drawCard();
	trumpCard = drawCard();
}

int main() {
	srand(time(nullptr));

	cout << "=== SANTASE ===" << endl;

	initDeck();
	shuffle();
	dealCards();

	cout << "Trump: ";
	printCard(trumpCard);
	cout << endl << endl;

	cout << "Player 1: ";
	printHand(hand1, HAND_SIZE);

	cout << "Player 2: ";
	printHand(hand2, HAND_SIZE);

	cout << endl << "Deck: " << (DECK_SIZE - deckTop) << endl;

	return 0;
}
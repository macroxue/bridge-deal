#include <vector>
#include "big-int.c"

char rank_sign[] = "AKQJT98765432";

int read_rank(char c) {
  auto pos = strchr(rank_sign, c);
  return pos ? pos - rank_sign : -1;
}

int main(int argc, char *argv[]) {
  std::vector<int> deck;
  for (int c = 0; c < 52; ++c) deck.push_back(c);
  for (int i = 1; i < argc; ++i) {
    int suit = 0;
    int hand_size = 13;
    size_t p = 0;
    std::vector<int> dealt;
    big_t index;
    big_set(index, 0);
    for (char *hand = argv[i]; *hand; ++hand) {
      int rank = read_rank(*hand);
      if (rank == -1) {
        ++suit;
        continue;
      }
      int card = suit * 13 + rank;
      assert(0 <= card && card < 52);
      for (; p < deck.size(); ++p) {
        if (deck[p] == card) {
          dealt.push_back(p);
          --hand_size;
          ++p;
          break;
        }
        // C(m,n)=C(m-1,n-1)+C(m,n-1)
        big_t half_index;
        big_choose(half_index, deck.size() - 1 - p, hand_size - 1);
        big_add(index, index, half_index);
      }
    }
    assert(hand_size == 0);
    big_show(index);
    printf(" ");

    // Remove dealt cards.
    while (!dealt.empty()) {
      int p = dealt.back();
      dealt.pop_back();
      deck.erase(deck.begin() + p);
    }
  }
  puts("");
  return 0;
}

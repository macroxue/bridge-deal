#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////
// BIG integer
//////////////////////////////////////////////////
#define BIG_SIZE   6
#define BASE       10000
#define BASE_LOG   4

typedef int big_t[BIG_SIZE];

big_set(big_t r, int n)
{
    int i;
    assert(n < BASE);
    for (i = 0; i < BIG_SIZE; i++) r[i] = 0;
    r[0] = n;
}

big_add(big_t r, big_t m, big_t n)
{
    int i, c = 0;
    for (i = 0; i < BIG_SIZE; i++) {
        r[i] = m[i] + n[i] + c;
        c = (r[i] >= BASE);
        if (c) r[i] -= BASE;
    }
    assert(c == 0);
}

big_sub(big_t r, big_t m, big_t n)
{
    int i, c = 0;
    for (i = 0; i < BIG_SIZE; i++) {
        r[i] = m[i] - n[i] - c;
        c = r[i] < 0;
        if (c) r[i] += BASE;
    }
    assert(c == 0);
}

int
big_cmp(big_t m, big_t n)
{
    int i, c = 0, z = 1, r;
    for (i = 0; i < BIG_SIZE; i++) {
        r = m[i] - n[i] - c;
        c = r < 0;
        if (r) z = 0;
    }
    if (c == 1) return -1;
    else if (z) return 0;
    else return 1;
}

big_mul_i(big_t r, big_t m, int n)
{
    int i, c = 0;
    assert(n < BASE);
    for (i = 0; i < BIG_SIZE; i++) {
        r[i] = m[i] * n + c;
        c = r[i] / BASE;
        r[i] -= c*BASE;
    }
    assert(c == 0);
}

big_mul(big_t r, big_t m, big_t n)
{
    int i, j, c = 0;

    assert(r != m && r != n);
    big_set(r, 0);
    for (i = 0; i < BIG_SIZE; i++) {
        for (j = 0; j < BIG_SIZE; j++) {
            r[i+j] += m[i] * n[j];
        }
    }
    c = 0;
    for (i = 0; i < BIG_SIZE; i++) {
        r[i] += c;
        c = r[i] / BASE;
        r[i] -= c*BASE;
    }
    assert(c == 0);
}

big_div_i(big_t r, big_t m, int n)
{
    int i, c = 0, d;
    assert(n < BASE);
    for (i = BIG_SIZE - 1; i >= 0; i--) {
        d = c * BASE + m[i];
        r[i] = d / n;
        c = d - r[i] * n;
    }
    assert(c == 0);
}

big_choose(big_t r, int m, int n)
{
    int i;
    big_set(r, 1);
    for (i = m-n+1; i <= m; i++)
        big_mul_i(r, r, i);
    for (i = 1; i <= n; i++)
        big_div_i(r, r, i);
}

big_rand(big_t r, big_t t)
{
    int i;
    for (i = BIG_SIZE-1; i >= 0 && t[i] == 0; i--);

    big_set(r, 0);
    r[i] = rand() % (t[i]+1);
    for (i--; i >= 0; i--) {
        r[i] = rand() % BASE;
    }
    while (big_cmp(r, t) >= 0) {
        big_sub(r, r, t);
    }
}

big_show(big_t r)
{
    int i;
    for (i = BIG_SIZE - 1; i >= 0 && r[i] == 0; i--);
    if (i >= 0) {
        printf("%d", r[i]);
        for (i--; i >= 0; i--)
            printf("%04d", r[i]);
    } else {
        printf("0");
    }
}

big_read(big_t r, char *s)
{
    int i, l;

    big_set(r, 0);

    l = strlen(s);
    for (i = l-1; i >= 0; i--) {
        r[i/BASE_LOG] = r[i/BASE_LOG]*10 + s[l-1-i] - '0';
    }
}

//////////////////////////////////////////////////
// Dealing cards
//////////////////////////////////////////////////
#define DECK_SIZE  52
#define HAND_SIZE  13
#define SUIT_SIZE  13
typedef char card_t;

char suit_sign[] = "SHDC";
char rank_sign[] = "AKQJT98765432";
char *seat_sign[] = { "North", "South", "West", "East" };
//float point_table[SUIT_SIZE] = {4.5,3,1.5,1,0.5,0,0,0,0,0,0,0,0};
#if 0
float point_table[SUIT_SIZE] = {1.5,1,.5,0,0,0,0,0,0,0,0,0,0};
float dp_table[SUIT_SIZE]  = {0,0,0,0,1,2,3,4,5,6,7,8,9};
#else
float point_table[SUIT_SIZE] = {4,3,2,1,0,0,0,0,0,0,0,0,0};
float dp_table[SUIT_SIZE] = {3,2,1,0,0,0,0,0,0,0,0,0,0};
#endif
float dp_table2[SUIT_SIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0};

struct {
    int  offset;
    char *string;
} center[] = //{ {24, ""}, {24, "N"}, {22, "W   E"}, {24, "S"} };
{ {23, "+----+"}, {23, "|    |"}, {23, "|    |"}, {23, "+----+"} };

deck_reset(card_t *deck, int deck_size)
{
    int i;
    for (i = 0; i < deck_size; i++)
        deck[i] = i;
}

card_deal(big_t index, card_t *deck, int deck_size, card_t *hand, int hand_size)
{
    if (hand_size == 0) {
        return;
    } else if (hand_size == deck_size) {
        int i;
        for (i = 0; i < deck_size; i++)
            hand[i] = deck[i];
        return;
    } else {
        big_t half_index;
        big_choose(half_index, deck_size-1, hand_size-1);
        if (big_cmp(index, half_index) == -1) {
            hand[0] = deck[0];
            card_deal(index, deck+1, deck_size-1, hand+1, hand_size-1);
        } else {
            big_sub(half_index, index, half_index);
            card_deal(half_index, deck+1, deck_size-1, hand, hand_size);
        }
    }
}

int
suit_show(card_t *card, int count, int suit)
{
    int i, out_len = 3;
    printf("%c: ", suit_sign[suit]);
    for (i = 0; i < count; i ++) {
        if (card[i] / SUIT_SIZE == suit) {
            printf("%c ", rank_sign[card[i] % SUIT_SIZE]);
            out_len += 2;
        }
    }
    return out_len;
}

card_show(card_t *card, int count)
{
    int suit;
    for (suit = 0; suit < 4; suit++) {
        suit_show(card, count, suit);
        printf("\n");
    }
    printf("\n");
}

card_pack(card_t *deck, int deck_size, card_t *hand, int hand_size)
{
    int deck_index, hand_index = 0, new_deck_size = 0;
    for (deck_index = 0; deck_index < deck_size; deck_index++) {
        if (deck[deck_index] == hand[hand_index]) {
            hand_index++;
            if (hand_index == hand_size)
                break;
        } else {
            deck[new_deck_size++] = deck[deck_index];
        }
    }
    for (deck_index++; deck_index < deck_size; deck_index++)
        deck[new_deck_size++] = deck[deck_index];
    assert(new_deck_size == deck_size - hand_size);
}

space(int count)
{
    while (count-- > 0)
        printf(" ");
}

float
calculate_hcp(card_t *hand, int count)
{
    float hcp = 0;
    int i;
    for (i = 0; i < count; i++) {
        hcp += point_table[hand[i] % SUIT_SIZE];
    }
    return hcp;
}

int
calculate_dp(card_t *hand, int count, float dp_table[])
{
    int i, dp = 0, suit = 0, len = 0;
    for (i = 0; i < count; i ++) {
        if (hand[i] / SUIT_SIZE == suit) {
            len++;
        } else {
            dp += dp_table[len];
            len = 0;
            suit++;
            i--;
        }
    }
    while (suit < 4) {
        dp += dp_table[len];
        len = 0;
        suit++;
    }
    return dp;
}

/////// save deal in Deep Finesse format ///////
void
save_suit(FILE *fp, card_t *card, int count, int suit)
{
    int i, len = 0;
    for (i = 0; i < count; i ++) {
        if (card[i] / SUIT_SIZE == suit) {
            fprintf(fp, "%c", rank_sign[card[i] % SUIT_SIZE]);
            len++;
        }
    }
    if (len == 0)
        fprintf(fp, "-");
}

void
save_space(FILE *fp, int count)
{
    while (count-- > 0)
        fprintf(fp, " ");
}

void
save_for_deep_finesse(card_t hand[][HAND_SIZE])
{
    int  suit;
    char *name = "deep_finesse.txt";
    FILE *fp = fopen(name, "w");
    if (fp == NULL) {
        printf("Cannot open %s to save.\n", name);
        return;
    }
    fprintf(fp, "Deal: 1");
    save_space(fp, 28);
    for (suit = 0; suit < 4; suit ++) {
        save_suit(fp, hand[1], HAND_SIZE, suit);
        save_space(fp, 1);
    }
    fprintf(fp, "\n");
    fprintf(fp, "Contract: 3N-South");
    save_space(fp, 5);
    for (suit = 0; suit < 4; suit ++) {
        save_suit(fp, hand[3], HAND_SIZE, suit);
        save_space(fp, 1);
    }
    save_space(fp, 5);
    for (suit = 0; suit < 4; suit ++) {
        save_suit(fp, hand[2], HAND_SIZE, suit);
        save_space(fp, 1);
    }
    fprintf(fp, "\n");
    fprintf(fp, "OnLead: West");
    save_space(fp, 23);
    for (suit = 0; suit < 4; suit ++) {
        save_suit(fp, hand[0], HAND_SIZE, suit);
        save_space(fp, 1);
    }
    fprintf(fp, "\n");
    fprintf(fp, "Lead: Any\n");

    fclose(fp);
}
//////////////////////////////////////////////

main(int argc, char *argv[])
{
    card_t deck[DECK_SIZE], hand[4][HAND_SIZE];
    big_t index[4], total[4];
    int hands_to_show, arg_pos;
    int seat, suit, suit_len;
    float hcp[4], dp[4], dp2[4];

    {
        struct timeval time;
        gettimeofday(&time,NULL);
        srand((time.tv_sec * 1000) + (time.tv_usec / 1000));
    }

    for (seat = 0; seat < 4; seat++) {
        big_choose(total[seat], 52 - HAND_SIZE*seat, HAND_SIZE);
        big_set(index[seat], 0);
    }
    deck_reset(deck, DECK_SIZE);

    if (argc > 1 && argv[1][0]=='-') {
        hands_to_show = atoi(&argv[1][1]);
        arg_pos = 2;
    } else {
        hands_to_show = 4;
        arg_pos = 1;
    }

    /* Seats:  0 = South, 1 = North, 2 = East, 3 = West */
    for (seat = 0; seat < 4; seat++) {
        if (argc > arg_pos + seat) {
            big_read(index[seat], argv[arg_pos + seat]);
            if ( big_cmp(index[seat], total[seat] ) >= 0) {
                printf("Maximum hand number for %s is ", seat_sign[seat]);
                big_show(total[seat]);
                printf("\n");
                exit(1);
            }
        } else {
            big_rand(index[seat], total[seat]);
        }
        card_deal(index[seat], deck, DECK_SIZE - seat * HAND_SIZE,
                  hand[seat], HAND_SIZE);
        card_pack(deck, DECK_SIZE - seat * HAND_SIZE,
                  hand[seat], HAND_SIZE);
    }

    printf("%s ", argv[0]);
    for (seat = 0; seat < 4; seat++) {
        big_show(index[seat]);
        printf(" ");
    }
    printf("\n\n");

    for (seat = 0; seat < 4; seat++) {
        hcp[seat] = calculate_hcp(hand[seat], HAND_SIZE);
        dp[seat]  = calculate_dp(hand[seat], HAND_SIZE, dp_table);
        dp2[seat] = calculate_dp(hand[seat], HAND_SIZE, dp_table2);
    }

    if (hands_to_show >= 2) {
        space(20);
        if (dp2[1])
            printf("North: %g + %g,%g = %g,%g\n",
                    hcp[1], dp[1], dp2[1], hcp[1] + dp[1], hcp[1] + dp2[1]);
        else
            printf("North: %g + %g = %g\n", hcp[1], dp[1], hcp[1] + dp[1]);
        for (suit = 0; suit < 4; suit ++) {
            space(20);
            suit_show(hand[1], HAND_SIZE, suit);
            printf("\n");
        }
    }

    if (hands_to_show >= 4) {
        char out[80];
        if (dp2[3])
            sprintf(out, "West: %g + %g,%g = %g,%g",
                    hcp[3], dp[3], dp2[3], hcp[3] + dp[3], hcp[3] +dp2[3]);
        else
            sprintf(out, "West: %g + %g = %g", hcp[3], dp[3], hcp[3] + dp[3]);
        printf("%s", out);
        space(40 - strlen(out));
        if (dp2[2])
            printf("East: %g + %g,%g = %g,%g\n",
                    hcp[2], dp[2], dp2[2], hcp[2] + dp[2], hcp[2] +dp2[2]);
        else
            printf("East: %g + %g = %g\n", hcp[2], dp[2], hcp[2] + dp[2]);
        for (suit=0; suit<4; suit++) {
            suit_len = suit_show(hand[3], HAND_SIZE, suit);
            space(center[suit].offset - suit_len);
            printf("%s", center[suit].string);
            suit_len = center[suit].offset + strlen(center[suit].string);
            space(40 - suit_len);
            suit_show(hand[2], HAND_SIZE, suit);
            printf("\n");
        }
    }

    if (hands_to_show >= 2)
        printf("\n");
    space(20);
    if (dp2[0])
        printf("South: %g + %g,%g = %g,%g\n",
                hcp[0], dp[0], dp2[0], hcp[0] + dp[0], hcp[0] +dp2[0]);
    else
        printf("South: %g + %g = %g\n", hcp[0], dp[0], hcp[0] + dp[0]);
    for (suit=0; suit<4; suit++) {
        space(20);
        suit_show(hand[0], HAND_SIZE, suit);
        printf("\n");
    }

    save_for_deep_finesse(hand);
}


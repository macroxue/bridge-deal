#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "big-int.c"

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
float point_table[SUIT_SIZE] = {4,3,2,1,0,0,0,0,0,0,0,0,0};
float dp_table[SUIT_SIZE] = {3,2,1,0,0,0,0,0,0,0,0,0,0};

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
    float hcp[4], dp[4];

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
    }

    if (hands_to_show >= 2) {
        space(20);
        printf("North: %g + %g = %g\n", hcp[1], dp[1], hcp[1] + dp[1]);
        for (suit = 0; suit < 4; suit ++) {
            space(20);
            suit_show(hand[1], HAND_SIZE, suit);
            printf("\n");
        }
    }

    if (hands_to_show >= 4) {
        char out[80];
        sprintf(out, "West: %g + %g = %g", hcp[3], dp[3], hcp[3] + dp[3]);
        printf("%s", out);
        space(40 - strlen(out));
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
    printf("South: %g + %g = %g\n", hcp[0], dp[0], hcp[0] + dp[0]);
    for (suit=0; suit<4; suit++) {
        space(20);
        suit_show(hand[0], HAND_SIZE, suit);
        printf("\n");
    }

    save_for_deep_finesse(hand);
}


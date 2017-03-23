/*
  Program to simulate Enigma cipher

  Consistent with the excellent flash Enigma simulator
  at http://www.enigmaco.de/

  As with the flash simulator, here we do not
  allow the rings to be adjusted.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getInversePerm(int invPerm[], int perm[], int n);


// "natural" positions of the notches,
// that is, permutations are given relative to
// these notch positions, where the the notch causes
// rotor to step after indicated position
// Q
#define NOTCH_0 16
// E
#define NOTCH_1 4
// V
#define NOTCH_2 21
// J
#define NOTCH_3 9
// Z
#define NOTCH_4 25

#define ROTOR_0 "EKMFLGDQVZNTOWYHXUSPAIBRCJ"
#define ROTOR_1 "AJDKSIRUXBLHWTMCQGZNPYFVOE"
#define ROTOR_2 "BDFHJLCPRTXVZNYEIWGAKMUSQO"
#define ROTOR_3 "ESOVPZJAYQUIRHXLNFTGKDCMWB"
#define ROTOR_4 "VZBRGITYUPSDNHLXAWMJQOFECK"

#define REFLECTOR_B "YRUHQSLDPXNGOKMIEBFZCWVJAT"
#define REFLECTOR_C "FVPJIAOYEDRZXWGCTKUQSBNMHL"

// no stecker cables (i.e., stecker == identity perm)
#define STECKER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

// stecker with 10 cables connecting: (0,10) (1,25) (2,6) (3,11) (5,20)
// (7,9) (12,23) (13,17) (14,24) (15,22): 4,8,16,18,19,21 are un-steckered
//#define STECKER "KZGLEUCJIHADXRYWQNSTFVPMOB"


// Print flags
// print rotors, reflector, and stecker permutations
//#define PR_PERMS
// print output for each step of encryption/decryption
//#define PR_STEPS
// print the key
#define PR_KEY


// function prototypes
void getInversePerm(int invPerm[], int perm[], int n);
int stepRotor(int a, int b);
void getEngimaresult(char* encryptedText, char* plainText, int numL, int numM, int numR, int reflect, int init_L, int init_M, int init_R);

void getEngimaresult(char* encryptedText, char* plainText, int numL, int numM, int numR, int reflect, int init_L, int init_M, int init_R){

    int i,
        j,
        n,
        temp,
        cur_L,
        cur_M,
        cur_R,
        notch_M,
        notch_R;

    int Lrotor[26],
        Mrotor[26],
        Rrotor[26],
        reflector[26],
        stecker[26];

    int L[26][26],
        M[26][26],
        R[26][26],
        L_inv[26][26],
        M_inv[26][26],
        R_inv[26][26];

    char letter[26] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
        rot[5][26] = {ROTOR_0, ROTOR_1, ROTOR_2, ROTOR_3, ROTOR_4},
        ref[2][26] = {REFLECTOR_B, REFLECTOR_C},
        steck[26] = STECKER,
        notch[5] = {NOTCH_0, NOTCH_1, NOTCH_2, NOTCH_3, NOTCH_4},
        infname[100];

    unsigned char inChar,
        outChar;

    notch_M = notch[numM];
    notch_R = notch[numR];

    reflect = reflect - 66;

    init_L = init_L - 65;
    init_M = init_M - 65;
    init_R = init_R - 65;

    // initialize rotor and reflector arrays
    for(i = 0; i < 26; ++i)
    {
        Lrotor[i] = (int)rot[numL][i] - 65;
        Mrotor[i] = (int)rot[numM][i] - 65;
        Rrotor[i] = (int)rot[numR][i] - 65;
        reflector[i] = (int)ref[reflect][i] - 65;
        stecker[i] = (int)steck[i] - 65;

    }// next i

    for(i = 0; i < 26; ++i)
    {
        L[0][i] = Lrotor[i];
        M[0][i] = Mrotor[i];
        R[0][i] = Rrotor[i];

    }// next i

    for(j = 0; j < 26; ++j)
    {
        L[0][j] = Lrotor[j];
        M[0][j] = Mrotor[j];
        R[0][j] = Rrotor[j];

    }// next j

    for(i = 1; i < 26; ++i)
    {
        for(j = 0; j < 26; ++j)
        {
            L[i][j] = (Lrotor[(i + j) % 26] + 26 - i) % 26;
            M[i][j] = (Mrotor[(i + j) % 26] + 26 - i) % 26;
            R[i][j] = (Rrotor[(i + j) % 26] + 26 - i) % 26;

        }// next j

    }// next i

    for(i = 0; i < 26; ++i)
    {
        getInversePerm(L_inv[i], L[i], 26);
        getInversePerm(M_inv[i], M[i], 26);
        getInversePerm(R_inv[i], R[i], 26);

    }// next i

    /*

    #ifdef PR_KEY
        printf("\nKey:\n");
        printf("rotors (L,M,R) = (%d,%d,%d)\n", numL, numM, numR);
        printf("reflector: %c\n", (char)(reflect + 66));
        printf("initial positions (L,M,R) = (%c,%c,%c)\n",
               (char)(init_L + 65), (char)(init_M + 65), (char)(init_R + 65));
    #endif
     */

    cur_L = init_L;
    cur_M = init_M;
    cur_R = init_R;

    int rIndex = 0;
    int bufIndex = 0;
    while(1) {
        temp = encryptedText[bufIndex++];
        if (temp == EOF || temp == '\0') {
            break;
        }
        temp -= 65;
        if (temp < 0 || temp > 25) {
            fprintf(stderr, "\nError --- all input characters must be upper case A thru Z\n");
            exit(0);
        }

        inChar = (unsigned char) temp;

        // Note that Enigma stepping is not exactly odometer-like
        // R is fast rotor
        // M is medium rotor
        // L is slow rotor
        // rotors step _before_ encryption/decryption
        if (cur_M == notch_M)// all 3 step (so step left and middle here)
        {
            cur_L = stepRotor(cur_L, 26);
            cur_M = stepRotor(cur_M, 26);
        } else {
            if (cur_R == notch_R)// M and R both step (so step middle here)
            {
                cur_M = stepRotor(cur_M, 26);

            }// end if

        }// end if

        // step right (fast) rotor --- always steps
        cur_R = stepRotor(cur_R, 26);


        outChar = stecker[R_inv[cur_R][M_inv[cur_M][L_inv[cur_L][
                reflector[L[cur_L][M[cur_M][R[cur_R][stecker[inChar]]]]]]]]];

        plainText[rIndex++] = letter[outChar];
    }

    plainText[rIndex] = '\0';
  //  printf("\n plainText : %s", plainText);

    return;
}

void getInversePerm(int invPerm[], int perm[], int n)
{
    int i;

    for(i = 0; i < n; ++i)
    {
        invPerm[perm[i]] = i;

    }// next i

}// end getInversePerm


int stepRotor(int a, int b)
{
    int t;
    t = a + 1;
    if(t >= b)
    {
        t = 0;
    }
    return(t);

}// end stepRotor

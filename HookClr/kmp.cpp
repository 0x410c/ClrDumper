#include "kmp.h"
#include<Windows.h>

void computeLPSArray(unsigned char* pat, int M, int* lps)
{
    int len = 0;
    lps[0] = 0;
    int i = 1;
    while (i < M) {
        if (pat[i] == pat[len]) {
            len++;
            lps[i] = len;
            i++;
        }
        else
        {
            if (len != 0) {
                len = lps[len - 1];
            }
            else
            {
                lps[i] = 0;
                i++;
            }
        }
    }
}

int KMPSearch(unsigned char* pat, int patlen, unsigned char* txt, int txtLen)
{
    //matchFound = 0;
    int M = patlen;
    int N = txtLen;
    int *lps = (int*)malloc(M);
    computeLPSArray(pat, M, lps);
    int i = 0;
    int j = 0;
    while (i < N) {
        if (pat[j] == txt[i]) {
            j++;
            i++;
        }
        if (j == M) {
            //matchFound++;
            //free(lps);
            return i - j;
            j = lps[j - 1];
        }
        else if (i < N && pat[j] != txt[i]) {
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }
    //free(lps);
    return -1;
}

//wildcard kmp implememntation
char WILDCARD = '?';
void computeLPSArrayWild(unsigned char* pat, int M, int* lps)
{
    int len = 0;
    lps[0] = 0;
    int i = 1;
    while (i < M) {
        if (pat[i] == pat[len] || pat[i] == WILDCARD || pat[len] == WILDCARD) {
            len++;
            lps[i] = len;
            i++;
        }
        else
        {
            if (len != 0) {
                len = lps[len - 1];
            }
            else
            {
                lps[i] = 0;
                i++;
            }
        }
    }
}

int KMPSearchWild(unsigned char* pat, int patlen, unsigned char* txt, int txtLen)
{
    //matchFound = 0;
    int M = patlen;
    int N = txtLen;
    int* lps = (int*)malloc(M);
    computeLPSArrayWild(pat, M, lps);
    int i = 0;
    int j = 0;
    while (i < N) {
        if (pat[j] == txt[i] || pat[j] == WILDCARD) {
            j++;
            i++;
        }
        if (j == M) {
            //matchFound++;
            free(lps);
            return i - j;
            j = lps[j - 1];
        }
        else if (i < N && pat[j] != txt[i]) {
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }
    free(lps);
    return -1;
}
#ifndef _TOKENS_H
#define _TOKENS_H

/**
  Tells you if the input character is a special ()<>; character.
*/
int isSpecialCharacter(char input);

/**
  Reads input until delimiter found OR end of string
  input will be placed in output variable.
  Disregards special characters while in ".
*/
int readQuotedNextInput(char *input, char *output);
/**
  Reads input until delimiter found OR end of string
  input will be placed in output variable.
*/
int readNextInput(char *input, char *output);

// adds token to tokens string array.
void addToken(char *buffer, char **tokens, int tokenId);

// frees tokens array.
void freeTokens(char **tokens);

// should return pointer to pointer of a char
char **tokenize(char *input);


#define MAX_INPUT_SIZE 256
#define MAX_NUM_TOKENS 160

#endif /* ifndef _TOKENS_H */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

// TODO: Implement the tokenize demo.

/**
  Tells you if the input character is a special ()<>; character.
*/
int isSpecialCharacter(char input) {
  if (input == '(' || input == ')' || input == '>' || input == '<' || input == ';' || input == '|') {
    return 1;
  }
  else {
    return 0;
  }
}

/**
  Reads input until delimiter found OR end of string
  input will be placed in output variable.
  Disregards special characters while in ".
*/
int readQuotedNextInput(char *input, char *output) {
  int outputLength = 0;

  while (input[outputLength] != '"' && input[outputLength] != '\0') {
    output[outputLength] = input[outputLength];
    outputLength++;
  }
  output[outputLength] = '\0'; // add terminating character at end after delimiter found / '\0'
  return outputLength;
}

/**
  Reads input until delimiter found OR end of string
  input will be placed in output variable.
*/
int readNextInput(char *input, char *output) {
  int outputLength = 0;

  while (isSpecialCharacter(input[outputLength]) == 0 && input[outputLength] != ' ' && input[outputLength] != '\0') {
    output[outputLength] = input[outputLength];
    outputLength++;
  }
  output[outputLength] = '\0'; // add terminating character at end after delimiter found / '\0'
  return outputLength;
}

// adds token to tokens string array.
void addToken(char *buffer, char **tokens, int tokenId) {
  // Allocate memory for the token, including space for the null terminator
  tokens[tokenId] = malloc(strlen(buffer) + 1);
  
  // Copy the contents of buffer into the newly allocated memory
  strcpy(tokens[tokenId], buffer);
}


// frees tokens array.
void freeTokens(char **tokens) {
  int i = 0;
  while (tokens[i] != NULL) {
    int j = 0;
    while (*tokens[j] != '\0') {
      free(tokens[j]);
    }
    free(*tokens);
    i++;
  }
}

// should return pointer to pointer of a char
char **tokenize(char *input) {
  char **tokens = malloc(MAX_NUM_TOKENS * sizeof(char *) + 1);
  char buf[MAX_INPUT_SIZE + 1]; 
  // index of the input string
  int i = 0;
  int tokenId = 0;
  while (input[i] != '\0') {
    // Check for quotes
    if (input[i] == '"') {
      ++i; // do not want to put beginning '"' into buf
      i += readQuotedNextInput(&input[i], buf);
      addToken(buf, tokens, tokenId);
      ++tokenId;
      ++i; // do not want to put ending '"' into buf
      continue; // skip the rest of this iteration
    } // Check for special character
    else if (isSpecialCharacter(input[i])) {
      // Add single-character token including this input to the tokens list.
      buf[0] = input[i];
      buf[1] = '\0';
      addToken(buf, tokens, tokenId);
      ++tokenId;
      ++i;
    } // Check for spaces
    else if (input[i] ==' ') {
      ++i; // skip past spaces
    }
    else {
      i += readNextInput(&input[i], buf);
      addToken(buf, tokens, tokenId);
      ++tokenId;
      continue;
    }
  }
  tokens[tokenId] = NULL; // terminate end of tokens array.
  return tokens;
}


int main(int argc, char **argv) {
  char input[MAX_INPUT_SIZE];
  if (argc > 1) {
    // printf("num arguments: %d", argc);
    strcpy(input, argv[1]);
    for (int i = 2; i < argc; i++) {
        strcat(input, " ");
        strcat(input, argv[i]);
    }
  } else {
    fgets(input, sizeof(input), stdin);
  }
  input[MAX_INPUT_SIZE - 1] = '\0';
  char **tokens = malloc(MAX_NUM_TOKENS * sizeof(char *));
  tokens = tokenize(input);
  int i = 0;
  while (tokens[i] != NULL) {
    printf("%s\n", tokens[i]);
    i++;
  }
  return 0;
}
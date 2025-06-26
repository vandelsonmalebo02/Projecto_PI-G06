#define MAX 100

enum tokenType{
  NUMERO,
  PARENTESES,
  OPERADOR,
  NONE
};

typedef struct {
  char value[20];
  enum tokenType type;
  int peso;
}Token;

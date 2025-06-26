#include "dstlib_include.h"


// ------------------ Funcoes auxiliares ------------------
// Verifica se um caractere e um operador aritmetico valido
int validar_operador(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
    // Retorna 1 (true) se caractere pertence a este operadores, senao retorna 0
}

// Retorna a precedencia (peso) de um operador
int receber_pesos_operador(char op) {
    switch(op) {
        case '^': return 4; // Maior precedencia: exponenciacao
        case '*':
        case '/': return 3; // Multiplicacao e divisao tem precedência intermediaria
        case '+':
        case '-': return 2; // Soma e subtracao tem menor precedencia
        default: return 0; // Qualquer outro caractere tem peso 0 (invalido ou nao operador)
    }
}

// Valida expressao aritmetica com base na sequencia de tokens
int validar_expressao(Token *tokens, int token_count) {
    int parenteses = 0; // Contador de parenteses abertos
    int esperar_operando = 1; // Indica se estamos esperando um operando (numero ou '(')

    for (int i = 0; i < token_count; i++) {
        Token t = tokens[i]; // Token atual da expressao

        // Se o token e um parentese
        if (t.type == PARENTESES) {
            if (t.value[0] == '(') {
                parenteses++; // Incrementa (Abrindo um novo parentese)
                esperar_operando = 1; // Depois do parentese (', espera-se um numero ou outro parentese '('
            } else {
                parenteses--; // Decrementa (Fechando um parentese)
                if (parenteses < 0) return -2; // erro de parentese fechado sem um aberto
                esperar_operando = 0; // Depois de fecha ')', espera-se um operador
            }
        }
        // Se o token e um numero
        else if (t.type == NUMERO) {
            if (!esperar_operando) return -3; //erro esperado um operador mas encontrou numero
            // Verifica se ha divisao por zero
            if (i > 0 && tokens[i-1].type == OPERADOR &&
                tokens[i-1].value[0] == '/' &&
                strcmp(t.value, "0") == 0) return -4; //erro de divisao por zero
            esperar_operando = 0; // Apos um numero, espera-se um operador ou ')'
        }
        // Se o token e um operador
        else if (t.type == OPERADOR) {
            if (esperar_operando) return -5; // erro numero esperado, mas operador encontrado
            esperar_operando = 1; // Depois de operador, espera-se um numero ou '('
        }
    }

    // Verifica se todos os parenteses foram fechados corretamente
    if (parenteses != 0) return -2;
    // Verifica se a expressao termina com um operando valido
    if (esperar_operando) return -5;

    return 1; // expressao valida
}


// Funçao que divide a expressao em tokens validos (numeros, operadores e parenteses)
void tokenizar(const char *express, Token *tokens, int *token_count) {
    int i = 0; // Indice para percorrer a string da expressao
    *token_count = 0; // Inicializa a contagem de tokens com 0

    // Loop ate o final da string
    while (express[i] != '\0') {
        if (isspace(express[i])) {
            i++; // Ignora os espacos em branco
            continue;
        }

        Token token_atual = { .type = NONE, .peso = 0 }; // Cria um token vazio para preencher

        // Se o caractere atual e um digito, iniciamos a leitura de um numero
        if (isdigit(express[i])) {
            int j = 0; // Indice interno para montar o numero
            while (isdigit(express[i])) {
                token_atual.value[j++] = express[i++]; // recebe os digitos para o token
            }
            token_atual.value[j] = '\0'; // Finaliza a string do numero
            token_atual.type = NUMERO; // Marca o token como numero
        }
        // Se o caractere e um parentese (abre ou fecha)
        else if (express[i] == '(' || express[i] == ')') {
            token_atual.value[0] = express[i]; // recebe o parentese do token
            token_atual.value[1] = '\0'; // Finaliza a string
            token_atual.type = PARENTESES; // Marca o token como parentese
            i++; // Incrementa para avança para o proximo caractere
        }
        // Se o caractere e um operador aritmetico valido (+ - * / ^)
        else if (validar_operador(express[i])) {
            token_atual.value[0] = express[i];  // recebe o operador
            token_atual.value[1] = '\0'; // Finaliza a string
            token_atual.type = OPERADOR; // Marca como operador
            token_atual.peso = receber_pesos_operador(express[i]); // Define a precedencia
            i++; // Incrementa para avança para o proximo caractere
        }
        // Se o caractere nao e reconhecido como numero, parentese ou operador
        else {
            printf("Erro: Caracteres invalidos.\n");
            *token_count = 0; // Zera a contagem de tokens para indicar falha
            return; // Encerra a funcao imediatamente retornando vazio
        }

        // Adiciona o token criado ao array de tokens e incrementa a contagem
        tokens[(*token_count)++] = token_atual;
    }
}

// ------------------ Conversao Infixa -> Pos-fixa ------------------

// Converte uma expressao em notacao infixa (normal) para notacao pós-fixa
// Recebe: vetor de tokens da expressao infixa, tamanho n, vetor de saida posfixa
// Retorna: quantidade de tokens na expressao pos-fixa gerada
int infixa_para_posfixa(Token *infixa, int n, Token *posfixa) {
    Pilha operadores; // criacao da Pilha para armazenar os operadores temporariamente
    initPilha(&operadores); // Inicializa a pilha
    int j = 0; // Indice de posicao da expressao pos-fixa

    // Percorre todos os tokens da expressao infixa
    for (int i = 0; i < n; i++) {
        Token t = infixa[i];  // Token atual

        // Se o token for um numero, ele vai direto para a saida (pos-fixa)
        if (t.type == NUMERO) {
            posfixa[j++] = t;
        }
        // Se o token for um parentese
        else if (t.type == PARENTESES) {
            if (t.value[0] == '(') {
                push(&operadores, t);  // Abre parentese → empilha
            } else {
                // Fecha parentese → desempilha operadores ate encontrar o '('
                while (!vaziaPilha(&operadores) && top(&operadores).value[0] != '(') {
                    posfixa[j++] = top(&operadores);
                    pop(&operadores);
                }
                pop(&operadores); // Remove o parentese de abertura '('
            }
        }
        // Se for um operador
        else if (t.type == OPERADOR) {
            // Enquanto houver operadores na pilha com precedência maior ou igual como em '^'
            while (!vaziaPilha(&operadores) &&
                   top(&operadores).type == OPERADOR &&
                   ((t.peso <= top(&operadores).peso && t.value[0] != '^') ||  // Associativo a esquerda
                    (t.peso < top(&operadores).peso))) { // Associativo a direita
                posfixa[j++] = top(&operadores); 
                pop(&operadores);
            }
             // Empilha o operador atual
            push(&operadores, t);
        }
    }

    // Ao final, desempilha todos os operadores restantes
    while (!vaziaPilha(&operadores)) {
        posfixa[j++] = top(&operadores);
        pop(&operadores);
    }

    // Retorna o numero de tokens pos-fixa
    return j;
}

// ------------------ Avaliacao Pos-fixa ------------------
// Aplica um operador binário (como +, -, *, /, ^) a dois operandos a e b
int aplicar_operador(int a, int b, char op) {
    switch (op) {
        case '+': return a + b; // Soma
        case '-': return a - b; // Subtracao
        case '*': return a * b; // Multiplicacao
        case '/': return b == 0 ? 0 : a / b; // Divisao com verificacao de zero
        case '^': {  // Exponenciação: a elevado a b
            int r = 1;
            for (int i = 0; i < b; i++) r *= a;
            return r;
        }
        default: return 0; // Operador desconhecido retorna 0
    }
}

// Avalia uma expressao em notacao pos-fixa e armazena o resultado em 'resultado_final'
// Retorna: 1 se for sucesso, 0 se der erro de pilha, -1 se for uma divisao por zero
int avaliar_posfixa(Token *posfixa, int n, int *resultado_final) {
    Pilha operandos;
    initPilha(&operandos); // Inicializa pilha de operandos

    // Percorre todos os tokens da expressao pos-fixa
    for (int i = 0; i < n; i++) {
        Token t = posfixa[i];
        // Se o token for um numero, empilha diretamente
        if (t.type == NUMERO) {
            push(&operandos, t);
        } 
        // Se o token for um operador, desempilha dois operandos e aplica o operador
        else if (t.type == OPERADOR) {
            if (vaziaPilha(&operandos)) return 0; // erro operandos insuficientes
            int b = atoi(top(&operandos).value); pop(&operandos); // Ultimo valor empilhado, operando 2

            if (vaziaPilha(&operandos)) return 0; // erro operandos insuficientes
            int a = atoi(top(&operandos).value); pop(&operandos); // Valor anterior, operando 1

            if (t.value[0] == '/' && b == 0) {
                return -1; // erro divisao por zero
            }

            int resultado = aplicar_operador(a, b, t.value[0]); // Aplica o operador nos operandos

            // Cria novo token com o resultado
            Token resultado_token;
            sprintf(resultado_token.value, "%d", resultado);  // Converte resultado para string
            resultado_token.type = NUMERO;
            resultado_token.peso = 0;

            push(&operandos, resultado_token); // Empilha o resultado
        }
    }

     // Ao final, o topo da pilha deve conter o resultado da expressão
    if (!vaziaPilha(&operandos)) {
        *resultado_final = atoi(top(&operandos).value);  // Converte string para inteiro
        return 1;
    }

    return 0; // erro sem resultado final
}


// ------------------ Funcao principal ------------------

int main(int argc, char *argv[]) {
    char txt_str[MAX]; // variavel para armazenar cada linha lida do arquivo

    // Verifica se os argumentos da linha de comando sao validos
    if (argc >= 3 && strcmp(argv[1], "in.txt") == 0 && strcmp(argv[2], "out.txt") == 0) {
        FILE *arq_in = fopen(argv[1], "r"); // Abre o arquivo de entrada para leirura
        FILE *arq_out = fopen(argv[2], "w"); // Abre (ou cria) o arquivo de saída para escrita

        // verifica se o arquivo de entrada foi aberto com sucesso
        if (arq_in != NULL) {
            printf("\n LEITURA DOS DADOS NO ARQUIVO DE ENTRADA: in.txt \n");
            printf("-------------------------------------------------\n");

            // Primeira leitura apenas para mostrar as expressoaaaes do arquivo na tela
            while (fgets(txt_str, MAX, arq_in) != NULL) {
                printf("%s", txt_str);
            }

            rewind(arq_in); // Retorna o ponteiro para o inicio do arquivo para processar de fato

            printf("\n          VALIDAR EXPRESSOES E CALCULAR \n");
            printf("-------------------------------------------------\n");
            //char previous_line[MAX] = {0}; 

            // Processa cada linha do arquivo um a um
            while (fgets(txt_str, MAX, arq_in) != NULL) {
                printf("Expressao: %s", txt_str);

                // Lê próxima linha so para saber se ainda ha mais (mas neste trecho nao e usada)
                char next_line[MAX];
                char *next = fgets(next_line, MAX, arq_in);
                
                // Se nao conseguiu ler proxima linha, esta e a ultima
                if (next == NULL) {
                    // Caso seja a ultima linha, imprime quebra de linha
                    printf("\n");
                }
                
                // Remove caractere de nova linha '\n' no final da expressao
                txt_str[strcspn(txt_str, "\n")] = '\0';

                Token tokens[100]; // Vetor para armazenar tokens
                int token_count = 0;

                // chamada da funcao Tokeniza a expressao
                tokenizar(txt_str, tokens, &token_count);

                // Verifica se nao foi possivel tokenizar (caractere inválido)
                if (token_count == 0) {
                    fprintf(arq_out, "%s = Erro: Caractere inválido\n", txt_str);
                    printf("-------------------------------------------------\n");
                    continue;
                }

                //chamada da funçao de Valida a estrutura da expressao
                int validacao = validar_expressao(tokens, token_count);

                //verifica se expressão for valida
                if (validacao == 1) {
                    printf("Expressao valida!\n");
                    Token posfixa[100];
                    int posfixa_count = infixa_para_posfixa(tokens, token_count, posfixa);
                    int resultado;
                    int status = avaliar_posfixa(posfixa, posfixa_count, &resultado);
                    if (status == -1) {
                        // erro de divisao por zero detectada durante avaliacao
                        fprintf(arq_out, "%s = Erro: Divisão por zero\n", txt_str);
                        printf("Erro: Divisao por zero\n");
                    } else if (status == 1) {
                        // Avaliacao bem-sucedida
                        fprintf(arq_out, "%s = %d\n", txt_str, resultado);
                        printf("Resultado: %d\n", resultado);
                    } else {
                        // erro inesperado na avaliacao
                        printf("Erro: Avaliação falhou\n");
                    }
                } else {
                    // Se a expressao foi malformada ou invalida — trata com base no codigo de erro
                    switch (validacao) {
                        case -2:
                            fprintf(arq_out, "%s = Erro: Parênteses desbalanceados\n", txt_str);
                            printf("Erro: Parenteses desbalanceados\n");
                            break;
                        case -4:
                            fprintf(arq_out, "%s = Erro: Divisão por zero\n", txt_str);
                            printf("Erro: Divisao por zero\n");
                            break;
                        case -3:
                        case -5:
                            fprintf(arq_out, "%s = Erro: Expressão malformada\n", txt_str);
                            printf("Erro: Expressao malformada\n");
                            break;
                        default:
                            printf("Erro: Sintaxe invalida\n");
                            break;
                    }
                }


                printf("-------------------------------------------------\n");
            }

            // Fecha arquivos abertos
            fclose(arq_in);
            fclose(arq_out);
        } else {
            // Caso nao consiga abrir o arquivo de entrada
            printf("Arquivo %s nao existe\n", argv[1]);
            return 1;
        }
    } else {
        // erro de Argumentos invalidos passados na linha de comando
        printf("Foi passado argumentos que nao fazem parte da regra.\n");
        printf("Regra: ./avaliador in.txt out.txt\n");
        return 1;
    }

    return 0;
}

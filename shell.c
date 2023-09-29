    // Comandos make //
// Criar executavel -> make
// Executar o arquivo -> ./shell
// Limpar arquivo executavel -> make clean

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>

// Declarações das funções
// Funções Gerais
char *InputDinamico();
void ExecuteCommand(char *cmd);
void TokenizacaoCommands(char* input, char* delimiters, char*** result);

// Função Sequencial
void ExecuteSequencial(char* input);

// Funções Paralelo
void ExecuteParalelo(char* input);
pthread_t* AlocacaoDeThreads(int count);
void CreateThread(char** commands, pthread_t* threads);
void *ExecutaThread(void *arg);

// Funções de Comandos
int CheckForExitCommand(char *input);
int StyleCommand(char *input);
void HistoryCommand(char **input);
void InteractiveMode(char *input);
void ExecuteCommandWithPipe(char *cmd1, char *cmd2);

// Funções de Arquivo
int DetectRedirection(char **args, char **inputFile, char **outputFile, int *append);
void ProcessFileInput(char *filename);
char* FileInputDinamico(FILE* file);

// Declaração do Struct
typedef struct CommandNode 
{
    char* command;
    struct CommandNode* next;
} CommandNode;

// Variáveis Globais
int modoExecucao = 0; // 0 para Sequencial (Default), 1 para Paralelo
char *ultimoComando = NULL;

// Tudo começa aqui (:
int main(int argc, char **argv)
{
    if (argc == 2) // Se um arquivo foi fornecido como argumento
    {
        ProcessFileInput(argv[1]);
        exit(0);
    }
    else if(argc > 2)
    {
        fprintf(stderr, "Número incorreto de argumentos.\n");
        exit(EXIT_FAILURE);
    }

    // Caso contrário, inicia o modo interativo
    while (1)
    {
        printf(modoExecucao == 0 ? "phavb seq> " : "phavb par> ");
        char *input = InputDinamico();

        HistoryCommand(&input);

        if (input == NULL) continue;

        if (CheckForExitCommand(input))
        {
            exit(0);
        }

        if(StyleCommand(input))
        {
            continue;
        }

        InteractiveMode(input);

        free(input);
    }

    return 0;
}

// Retorna um ponteiro para a input que foi digitado
char *InputDinamico() 
{
    int bufferSize = 1024; // tamanho inicial
    int position = 0; // Marca onde está a string
    char *buffer = malloc(sizeof(char) * bufferSize); // Aloca memória do tamanho inicial de bufferSize 
    int ch; // Vai armazenar cada caracter que vai ser lido do input

    if (!buffer)
    {
        fprintf(stderr, "Falha na alocação de memória :(\n");
        exit(EXIT_FAILURE);
    }

    while (1) 
    {
        ch = fgetc(stdin);      // Pega cada caracter

        if (ch == EOF)          // Se o usuário pressionar Ctrl+D
        {
            if (position == 0)  // Se for o primeiro caracter
            {
                free(buffer);   // Libera a memória antes de sair
                exit(0);        // Finaliza o programa
            }
            else                // Se Ctrl+D for pressionado depois alguns caracter, vai considerar como final da entrada
            {
                buffer[position] = '\0';
                return buffer;
            }
        } 
        else if (ch == '\n') // Se for um caractere de nova linha
        {
            buffer[position] = '\0'; 
            return buffer;
        } 
        else 
        {
            buffer[position] = ch; // Coloca o caractere no buffer
        }

        position++;

        if (position >= bufferSize) // Se atingiu o limite do buffer
        {
            bufferSize += 1024; // Aumenta o tamanho
            buffer = realloc(buffer, bufferSize); // Realoca mais espaço para o buffer
            if (!buffer) // Checa se houve erro na realocação
            {
                fprintf(stderr, "Falha na realocação de memória\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

// Faz a 2 Tokenização e executa os comandos
void ExecuteCommand(char *cmd) 
{
    // Declaração de Variaveis
    char *commands[100];
    char *token;
    char *saveptr1;
    int i = 0;
    char *pipeChar = "|";
    int append = 0;
    char *inputFile = NULL;
    char *outputFile = NULL;

    if (strstr(cmd, pipeChar) != NULL) 
    {
        char *cmd1 = strtok(cmd, pipeChar); // Verifica se teve |
        char *cmd2 = strtok(NULL, pipeChar); // Verifica se teve | 

        ExecuteCommandWithPipe(cmd1, cmd2); // Executa comando com Pipe
        return;
    }

    token = strtok_r(cmd, " ", &saveptr1); // Faz a 2 Tokenização
    while (token != NULL) 
    {
        commands[i] = token;
        i++;
        token = strtok_r(NULL, " ", &saveptr1); // Continua a Tokenização de onde parou anteriormente
    }
    commands[i] = NULL; // Limpa o array de comandos

    DetectRedirection(commands, &inputFile, &outputFile, &append); // Detecta se teve redirecionamento

    if (fork() == 0) 
    {
        if(inputFile) // Se teve redirecionamento de entrada
        {
            int fileIdentificator_in = open(inputFile, O_RDONLY);
            if (fileIdentificator_in == -1) 
            {
                perror("Failed to open input file");
                exit(EXIT_FAILURE);
            }
            dup2(fileIdentificator_in, STDIN_FILENO);
            close(fileIdentificator_in);
        }

        if(outputFile) // Se teve redirecionamento de saída
        {
            int fileIdentificator_out;
            if(append) 
            {
                fileIdentificator_out = open(outputFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
            } 
            else 
            {
                fileIdentificator_out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            }
            
            if (fileIdentificator_out == -1) 
            {
                perror("Failed to open output file");
                exit(EXIT_FAILURE);
            }
            dup2(fileIdentificator_out, STDOUT_FILENO);
            close(fileIdentificator_out);
        }
        
        execvp(commands[0], commands);        
        perror("Erro ao executar o comando");
        exit(EXIT_FAILURE); 
    } 
    else 
    {
        wait(NULL); 
    }
}

// Divide o input em varios Tokens (Tokenização) para serem lidos e executados depois | Aloca memoria dinamicamente
void TokenizacaoCommands(char* input, char* delimiters, char*** result)
{
    // Declaração de Variaveis
    char *token;
    char *saveptr2;
    int i = 0;

    *result = (char **)malloc(sizeof(char *)); // Alocação de memoria 

    token = strtok_r(input, delimiters, &saveptr2); // Faz a Tokenização e salva onde parou
    while (token != NULL) 
    {
        *result = (char **)realloc(*result, (i + 1) * sizeof(char *)); // Realoca memoria pra caber o token
        (*result)[i] = token; // Coloca o token que foi tirado, em result

        i++;
        token = strtok_r(NULL, delimiters, &saveptr2); // Continua a Tokenização, mas agora de onde parou da ultima vez
    }
    *result = (char **)realloc(*result, (i + 1) * sizeof(char *)); // Realoca de novo pra colocar NULL 
    (*result)[i] = NULL; // Colocamos NULL pra facilitar na proxima Tokenização
}

// Modo Interativo Sequencial | Faz a primeira Tokenização 
void ExecuteSequencial(char* input) 
{
    // Declaração de Variaveis
    char** commands = NULL;

    TokenizacaoCommands(input, ";", &commands); // Faz a primeira Tokenização com o ;

    for(int i = 0; commands[i] != NULL; i++)
    {
        if (strcmp(commands[i], "exit") == 0) // Verifica se algum dos comandos é exit
        {
            free(commands); // Limpa memoria antes de encerrar
            exit(0); // Encerra
        }
        
        ExecuteCommand(commands[i]);
    }

    free(commands); // Limpa memoria após executar os comandos
}

// Modo Interativo Paralelo | Faz a primeira Tokenização 
void ExecuteParalelo(char* input)
{
    // Declaração de Variaiveis
    char** commands;
    TokenizacaoCommands(input, ";", &commands); // Faz a 1 Tokenização com o ;

    int commandCount = 0;
    while(commands[commandCount] != NULL) commandCount++; // Conta a quantidade de comandos

    pthread_t* threads = AlocacaoDeThreads(commandCount); // Aloca espaço para as threads
    
    CreateThread(commands, threads); // Executa as threads

    free(commands); // Limpa memória alocada dos comandos
    free(threads);  // Limpa memória alocada das threads
}

// Aloca memoria pra o numero de threads de acordo com a quantidade de comandos
pthread_t* AlocacaoDeThreads(int count)
{
    pthread_t *threads = (pthread_t *)realloc(NULL, count * sizeof(pthread_t));
    if (!threads)
    {
        perror("Erro ao alocar memória pra as threads");
        exit(EXIT_FAILURE);
    }
    return threads;
}

// Cria e chama a função que as threads irão executar pra o modo Interativo Paralelo
void CreateThread(char** commands, pthread_t* threads)
{
    int i = 0;
    while(commands[i] != NULL) 
    {
        if (strcmp(commands[i], "exit") == 0) // Verifica se teve exit
        {
            free(commands); // Limpa memoria dos comandos
            free(threads); // Limpa memoria das Threads
            exit(0); // Encerra
        }

        int threadError = pthread_create(&threads[i], NULL, ExecutaThread, commands[i]); // Cria a thread e chama a função que ela vai executar
        if (threadError) // Qualquer coisa diferente de 0
        {
            fprintf(stderr, "Erro ao criar thread: %d\n", threadError);
            exit(EXIT_FAILURE);
        }
        
        i++;
    }

    for(int j = 0; j < i; j++) // Espera que todas as threads terminem
    {
        pthread_join(threads[j], NULL);
    }
}

// Executa a thread com o comando passado no argumento
void *ExecutaThread(void *arg)
{
    char *cmd = (char *)arg;
    system(cmd); // Executa o comando 
    pthread_exit(NULL); // Termina a thread
}

// Verifica se teve o input pra fechar o shell
int CheckForExitCommand(char *input)
{
    if (strcmp(input, "exit") == 0) // Se teve exit
    {
        if (ultimoComando) // Se tem alguma coisa como ultimo comando
        {
            free(ultimoComando); // Libera a memoria que tinha aqui
        } 
        return 1; // retorna true pra fechar o shell
    }
    return 0; // Retorna false pra continuar a execução
}

// Verifica se teve o !! e executa o ultimo comando, caso tenha algum
void HistoryCommand(char **input)
{
    if (strcmp(*input, "!!") == 0) // Checa pra ver se teve o comando !!
    {
        if (ultimoComando) // Se tiver algum comando ja aqui
        {
            free(*input); // Libera a memoria que tinha
            *input = strdup(ultimoComando); // Aponta para a string que tem o ultimo comando
        }
        else // Se nao tiver nenhum comando
        {
            printf("No commands\n"); // Printa que nao tem comando
            free(*input); // Libera a memoria
            *input = NULL; // Define para NULL
        }
    }
    else // Se nao teve !!
    {
        if (ultimoComando) // Se tiver alguma coisa ja aqui
        {
            free(ultimoComando); // Libera a memoria que tinha
        }
        ultimoComando = strdup(*input); // Adiciona o novo comando, como ultimo comando
    }
}

// Verifica se teve input para mudar o modo interativo (Sequencial ou Paralelo)
int StyleCommand(char *input)
{
    if (strcmp(input, "style sequential") == 0) // Verifica se teve o comando pra mudar pra o Sequencial
    {
        modoExecucao = 0; // Muda o modo de execução
        return 1;
    }
    else if (strcmp(input, "style parallel") == 0) // Verifica se teve o comando pra mudar pra o Paralelo
    {
        modoExecucao = 1; // Muda o modo de execução
        return 1;
    }
    return 0;
}

// Lida como executar o input de acordo com o modo interativo atual
void InteractiveMode(char *input)
{
    if (modoExecucao == 0) 
    {
        ExecuteSequencial(input); // Excuta em Modo Sequencial
    } 
    else 
    {
        ExecuteParalelo(input); // Executa em modo Paralelo
    }
}

// Executa os comandos com Pipe
void ExecuteCommandWithPipe(char *cmd1, char *cmd2) 
{
    int fileIdentificator[2];
    if (pipe(fileIdentificator) == -1) 
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if (fork() == 0) 
    {
        close(fileIdentificator[0]);  // Fechando o lado de leitura do pipe
        dup2(fileIdentificator[1], STDOUT_FILENO);  // Redirecionando a saída padrão para o pipe
        close(fileIdentificator[1]);

        ExecuteCommand(cmd1);
        exit(EXIT_FAILURE);
    }

    if (fork() == 0) 
    {
        close(fileIdentificator[1]);  // Fechando o lado de escrita do pipe
        dup2(fileIdentificator[0], STDIN_FILENO);  // Redirecionando a entrada padrão do pipe
        close(fileIdentificator[0]);

        ExecuteCommand(cmd2);
        exit(EXIT_FAILURE);
    }

    close(fileIdentificator[0]);
    close(fileIdentificator[1]);
    wait(NULL);  // Esperando o primeiro processo filho
    wait(NULL);  // Esperando o segundo processo filho
}

// Redirecionamento
int DetectRedirection(char **args, char **inputFile, char **outputFile, int *append)
{
    int i;
    for (i = 0; args[i] != NULL; i++) 
    {
        if (strcmp(args[i], "<") == 0) 
        {
            *inputFile = args[i + 1];
            args[i] = NULL; // Aqui quebra a lista de argumentos
        } 
        else if (strcmp(args[i], ">") == 0)
        {
            *outputFile = args[i + 1];
            *append = 0; // Sobrescreve
            args[i] = NULL; // Aqui quebra a lista de argumentos
        }
        else if (strcmp(args[i], ">>") == 0) 
        {
            *outputFile = args[i + 1];
            *append = 1; // Adiciona
            args[i] = NULL; // Aqui quebra a lista de argumentos
        }
    }
    return 0;
}

// Processo pra abrir o arquivo, ler e imprimir os comandos no terminal e só então executa
void ProcessFileInput(char* filename) 
{
    FILE* file = fopen(filename, "r"); // Abre o arquivo
    if (!file) // Se nao conseguiu abrir o arquivo, apresenta um erro
    {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    CommandNode* head = NULL;    // Inicia a head da lista como NULL
    CommandNode* current = NULL; // Inicia o comando atual como NULL

    while (!feof(file)) // Leitura do arquivo
    {
        char* line = FileInputDinamico(file); // chama a função pra ler as linhas do arquivo
        CommandNode* newNode = malloc(sizeof(CommandNode)); // Aloca memoria pra o novo node
        newNode->command = line; // Coloca a string de comando
        newNode->next = NULL;

        if (!head) // Se a lista tiver vazia
        {
            head = newNode; // Novo node vira a head
            current = head;
        } 
        else // se nao tiver vazia
        {
            current->next = newNode; // Novo node adionado o fim da lista
            current = newNode;
        }
    }

    fclose(file); // fecha o arquivo

    current = head;
    while (current) // Imprime os comandos do arquivo no terminal
    {
        printf("%s\n", current->command); // Imprime o comando
        current = current->next; // Avança na lista
    }

    current = head;
    while (current) // Executa os comandos que ja foram lidos e impressos
    {
        StyleCommand(current->command); // Verifica se teve comando para mudar o modo iterativo
        InteractiveMode(current->command); // Executa o comando
        
        current = current->next; // Avança na lista
    }

    while (head) // Libera a memoria da lista
    {
        current = head;
        head = head->next;
        free(current->command);
        free(current);
    }
}

// Retorna uma string referente a linha do arquivo que foi lido
char* FileInputDinamico(FILE* file) 
{
    int bufferSize = 1024;
    int position = 0;
    char* buffer = malloc(sizeof(char) * bufferSize);

    if (!buffer) // Verifica se a aloção de memoria deu certo
    {
        fprintf(stderr, "Falha na alocação de memória :(\n"); // Se nao imprime a mensagem de erro
        exit(EXIT_FAILURE);
    }

    while (1) // Loop para ler todos os caracteres do arquivo
    {
        int ch = fgetc(file); // Lê o character e salva no ch

        if (ch == EOF || ch == '\n') // Se chegou no fim do arquivo ou no fim da linha
        {
            buffer[position] = '\0'; // Fecha a string com o \0
            return buffer; // Retorna a string que foi lida
        } 
        else // Se nao foi
        {
            buffer[position] = ch; // Coloca o caracter no buffer
        }

        position++; // Aumenta a posição

        if (position >= bufferSize) // Se a posição chegou no limite do buffer
        {
            bufferSize += 1024; // Adiciona para aumentar o tamanho do buffer
            buffer = realloc(buffer, bufferSize); // Aumenta o tamanho do buffer
            if (!buffer) // Verifica de novo se a alocação deu certo
            {
                fprintf(stderr, "Falha na realocação de memória\n"); // Se nao imprime o erro
                exit(EXIT_FAILURE);
            }
        }
    }
}
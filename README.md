# Shell Simplificado em C

## Intro
Este projeto consiste em um interpretador de comandos (shell) simplificado, escrito inteiramente em C, que pode ser executado em modo interativo ou batch, com suporte para execução sequencial ou paralela de comandos, redirecionamento de entrada/saída, pipes e repetição do último comando (Histórico).

## Funcionalidades
* Execução em Modo Interativo e Batch
* Execução Sequencial e Paralela de Comandos
* Redirecionamento de Entrada/Saída
* Pipe entre comandos
* Repetição do último comando com !!
* Tratamento coerente para linhas de comando vazias, múltiplos espaços em branco e outros.

## Compilação
O projeto utiliza um Makefile para compilação. Para compilar o projeto, basta executar o comando abaixo no terminal, no diretório do projeto:

Utilizando o comando `make`, isso gerará um arquivo executável nomeado _shell_.


## Execução
Você pode executar o Shell de duas formas:

### Interativo
Para executar o shell em modo interativo, use o comando abaixo:

Utilizando o comando `./shell`, você entrará no modo _Interativo Sequencial_ por default.
O shell também dá suporte para o modo _Interativo Paralelo_, para a execução de processos de forma paralela.
Para alternar entre os modos, verifique mais abaixo na aréa de "Uso".

### Batch
Para executar em modo batch, você deve fornecer um arquivo contendo uma lista de comandos:

Utilizando o comando `./shell [batchFile]`, onde [batchFile] é o nome do arquivo que contem os comandos.


## Uso
### Modo e Estilo de Execução
Para alternar entre os modos de execução Interativo Sequencial e Paralelo, utilize os seguintes comandos abaixo:

Enquanto no modo Sequencial:
`phavb seq> style parallel`, para mudar para o modo paralelo.

Enquanto no modo Paralelo:
`phavb par> style sequential`, para mudar para o modo sequencial.

Para executar comandos de forma sequencial:
`phavb seq> [comando1]; [comando2]; ...`

Para executar comandos de forma paralela:
`phavb par> [comando1]; [comando2]; ...`

## Redirecionamento e Pipes

### Redirecionar saída:
`phavb seq> [comando] > [arquivo]`

### Redirecionar entrada:
`phavb seq> [comando] < [arquivo]`

## Uso de pipes:
`phavb seq> [comando1] | [comando2]`

## Comando Especial:
Executar último comando (Histórico):
`phavb seq> !!`

## Erros e Exceções
O shell apresenta mensagens de erro coerentes e continua processando nos casos em que o comando não existe ou não pode ser executado. Além disso, o programa é capaz de lidar com situações como linhas de comando vazias, múltiplos espaços em branco, espaços antes ou depois do ponto-e-vírgula (;), e mais.

## Observações:
> Apesar dos exemplos de comandos estarem todos com o prefixo `seq> `, todos os comandos funcionam nos dois modos Interativos e, também caso possua dentro do arquivo, no modo Batch.

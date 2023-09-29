# Shell Simplificado em C

## Intro
Este projeto consiste em um interpretador de comandos (shell) simplificado, integralmente desenvolvido em C, podendo ser operado tanto em modo interativo quanto batch. Oferece suporte para execução sequencial e paralela de comandos, redirecionamento de entrada/saída, uso de pipes, e a capacidade de repetir o último comando através de um histórico.

## Funcionalidades
* Modo Interativo e Batch:
  * Permite a execução de comandos de maneira interativa ou a partir de um arquivo.
* Execução Sequencial e Paralela:
  * Os comandos podem ser executados um após o outro ou simultaneamente.
* Redirecionamento de Entrada/Saída e Pipes:
  * Suporte para modificar as entradas e saídas padrões dos comandos e conectar a saída de um comando à entrada de outro.
* Histórico de Comandos:
  * Acesso rápido ao último comando utilizado através do comando !!.
* Tratamento de Erros e Exceções:
  * Fornece feedback útil ao usuário em casos de comandos inválidos ou outros erros.

## Compilação
O projeto utiliza um Makefile para compilação. Para compilar o projeto, basta executar o comando `make` no terminal, isso gerará um arquivo executável nomeado _shell_ no seu diretório.


## Execução
Você pode executar o Shell de duas formas:

### Interativo
Utilizando o comando `./shell`, você entrará no modo _Interativo Sequencial_ por default.
O shell também dá suporte para o modo _Interativo Paralelo_, para a execução de processos de forma paralela.
Para alternar entre os modos, verifique mais abaixo na aréa de "Uso".

### Batch
Para executar em modo batch, você deve fornecer um arquivo contendo uma lista de comandos:
Utilizando o comando `./shell [batchFile]`, onde _[batchFile]_ é o nome do arquivo que contem os comandos.


## Uso
### Modo e Estilo de Execução
Para alternar entre os modos de execução Interativo Sequencial e Paralelo, utilize os seguintes comandos abaixo:

* Enquanto no modo Sequencial:
  * `phavb seq> style parallel`, para mudar para o modo paralelo.

* Enquanto no modo Paralelo:
  * `phavb par> style sequential`, para mudar para o modo sequencial.

* Para executar comandos de forma sequencial:
  * `phavb seq> [comando1]; [comando2]; ...`

* Para executar comandos de forma paralela:
  * `phavb par> [comando1]; [comando2]; ...`

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
> * Apesar dos exemplos de comandos estarem todos com o prefixo `seq> `, todos os comandos funcionam nos dois modos Interativos e, também caso possua dentro do arquivo, no modo Batch.
> * O shell permite a combinação de diferentes tipos de comandos em um único prompt, como mesclar um comando com pipe, seguido por um comando comum e depois um redirecionamento, entre outros. Experimente e **Divirta-se**!

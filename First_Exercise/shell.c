#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#define CHARSIZE 1024			 //Tamanho do vetor
#define RED_COLOR "\033[1;31m"	 //Cor de erro
#define GREEN_COLOR "\033[1;32m" //Cor de sucesso
#define RESET_COLOR "\033[0m"	 //Final da string de cor
#define STRING_EMPTY ""			 //String vazia.

#ifdef __linux__
#define PATH "/bin/" //Caso o SO seja linux, o Path será /bin/
#elif __APPLE__
#define PATH "/sbin/" //Caso o SO seja apple, o Path será /sbin/
#else
#define PATH "UNDEFINED" //Caso seja outro SO, o Path será UNDEFINED
#endif

volatile sig_atomic_t stop = 0;			  //Define se o código deve resetar ou não
volatile sig_atomic_t ignore_handler = 0; //Faz com que o handler não seja ativado após executar o fork

//Função responsável por capturar o Signal USR1
void signal_handler(int sigNumber)
{
	if (sigNumber == SIGUSR1)
	{
		//Se o ignore handler for 0, ele irá alterar o valor de stop, caso contrário não fará nada
		if (!ignore_handler)
		{
			printf("O Shell será reiniciado. Insira qualquer argumento para prosseguir.\n");
			stop = 1;
		}
	}
}

//Função responsável por converter a string para lower case
char *to_lower(char string[])
{
	//Enquanto não tiver um char \0 (fim de string)
	for (int i = 0; string[i] != '\0'; i++)
	{
		//Adiciono 32 ao char, para converte-lo à lower case caso seja uma letra
		if (string[i] >= 'A' && string[i] <= 'Z')
		{
			string[i] = string[i] + 32;
		}
	}
	return string;
}

//Função responsável por imprimir uma mensagem de erro definida pelo usuário
void return_error(char errorMessage[])
{
	printf(RED_COLOR);
	printf("%s\n\n", errorMessage);
	printf(RESET_COLOR);
}

int main(void)
{
	char errorMessage[CHARSIZE] = STRING_EMPTY; //String contendo uma mensagem de erro.
	char programPath[CHARSIZE] = PATH;			//Array do path para a função (tamanho 1024)
	char *function;								//Array para armazenar o input do número de parâmetros.
	char *numberOfParameters;					//Argumentos inputados pelo usuário.
	char **listOfArguments;						//Número de inputs setado pelo usuário.
	int parameters;								//Número de inputs setado pelo usuário.
	int step = 0;								//Passos dentro do switch-case.

	//Caso o sistema operacional não seja Linux ou Apple, retorna uma mensagem de erro.
	if (strcmp(programPath, "UNDEFINED") == 0)
	{
		return_error(strcpy(errorMessage, "Sistema Operacional Desconhecido. Este código só funcionará em Linux ou Apple"));
	}

	signal(SIGUSR1, signal_handler); //Instancia o signal Handler

	while (1)
	{
		//Se a variável stop é modificada, eu reseto os parâmetros e pulo para a próxima iteração do loop.
		if (stop)
		{
			stop = 0;				   //Redefine o valor de stop
			step = 0;				   //Redefine o valor de step
			strcpy(programPath, PATH); //Redefine o programPath
		}

		step++; //A cada vez que o loop rodar, iremos ao próximo passo, onde este será selecionado pelo switch-case

		switch (step)
		{
		case 1: //1 = primeiro passo, pego o comando e verifico se a função existe dentro do /bin

			printf("Qual comando quer executar ?\n");

			function = malloc(CHARSIZE); //Aloca um espaço de memória para armazenar a função
			scanf("%s", function);		 //Executa um scanf da função que o usuário deseja executar;

			strcat(programPath, to_lower(function)); //Concatena a função escolhida ao programPath a fim de buscar esse arquivo no source;

			//Access procura o source, caso ele não exista, retornará uma mensagem de erro. E retorna ao mesmo step.
			if (access(programPath, F_OK) == -1 && stop == 0)
			{

				return_error(strcpy(errorMessage, "Nenhuma operação encontrada. Insira uma operação válida."));

				strcpy(programPath, PATH); //Redefine programPath como PATH

				step = 0; //Redefine o valor de step para fazer com que volte a este mesmo passo
			}
			break;

		case 2: //2 = segundo passo, pego o número de argumentos e verifico se o número de argumentos é um inteiro

			printf("\nQuantos argumentos você quer digitar ?\n");

			numberOfParameters = malloc(CHARSIZE); //Aloca um espaço de memória para armazenar o número de parâmetros
			scanf("%s", numberOfParameters);	   //Faz um scanf do número de argumentos a serem passados na função

			//Checa se o input é realmente um inteiro. Caso não, retorna uma mensagem e retorna ao mesmo step.
			//Caso o numberOfParameters seja 0, o atoi também retornará 0, neste caso verifico a string numberOfParameters
			if ((parameters = atoi(numberOfParameters)) == 0 && strcmp(numberOfParameters, "0") != 0 && stop == 0)
			{

				return_error(strcpy(errorMessage, "Insira um número de argumento válido. O argumento digitado não é um inteiro."));

				step = 1; //Redefine o valor de step para fazer com que volte a este mesmo passo
			}
			break;

		case 3:													//3 = terceiro passo, pego os argumentos e aloco eles em um vetor.
			listOfArguments = malloc(parameters + 2);			//Instancia um array com todos os argumentos a serem passados pelo usuário, de tamanho máximo parameters.
			listOfArguments[0] = malloc(CHARSIZE);				//Aloca um espaço de memória para armazenar a string e pula para o próximo i
			listOfArguments[parameters + 1] = malloc(CHARSIZE); //Aloca um espaço de memória para armazenar a string e finaliza a execução do loop
			listOfArguments[0] = programPath;					//O primeiro elemento deve ser o path para a função
			listOfArguments[parameters + 1] = NULL;				//O ultimo elemento deve ser NULL

			for (int i = 1; i <= parameters; i++)
			{

				printf("\nDigite o argumento %i:\n", i);

				listOfArguments[i] = malloc(CHARSIZE); //A loca um espaço para a string arguments
				scanf("%s", listOfArguments[i]);	   //Executa um scanf para obter o argumento inputado pelo usuário

				if (stop)
				{
					break;
				}
			}
			break;
		}

		//Caso tenha chegado no último passo e o handler não tenha sido chamado no terceiro passo, encerra o loop.
		if (step == 3 && !stop)
		{
			break;
		}
	}

	pid_t child_pid = fork();

	//Impossibilita de fazer com que o signal imprima a mensagem de resetando o código.
	ignore_handler = 1;

	if (child_pid == 0)
	{
		printf("\n");
		//O uso do execv permite passar um vetor com os argumentos para executar a função que está no programPath
		execv(programPath, listOfArguments);
		return 0;
	}
	else
	{
		wait(NULL);
		//Limpa todas as memórias alocadas
		free(function);
		free(numberOfParameters);
		free(listOfArguments);
		printf(GREEN_COLOR);
		printf("\nTask is done\n\n");
		return 0;
	}
}

/*=====================================================================

     ------------------- CÓDIGO DINO PARA RCX --------------------

     Autor(es): Canton                          Data: Janeiro de 2022
     Projeto: Mini-Sumô                         TRINCABOTZ CEFET-MG

  ====================================================================*/



/* ------------------- Bibliotecas -------------------- */
//#include <IRremote.h>
/* ---------------------------------------------------- */


/* -------------- Mapeamento de Hardware -------------- */


/*Fix me:


- Arrumar delay da estrategia 2, ja esta funcionando OK e esta sem o break pra testar dentro do laboratrio
- DELAY É 1,344 PARA CADA 1 SEGUNDO DE DELAY ANTERIOR
*/



#define Start 13         // Entrada digital MicroStart
#define LedIn A5         // Led interno a placa RSM

#define Dp1 10         //entrada de estrategias
#define Dp2 11         //entrada de estrategias
#define Dp3 12         //entrada de estrategias

#define PWMB  9          // PMW do motor B
#define INB2  8          // Entrada 2 do motor B
#define INB1  7          // Entrada 1 do motor B

#define PWMA   6          // PWM do motor A
#define INA2   5          // Entrada 2 do motor A
#define INA1   4          // Entrada 1 do motor A


#define SensorE A2      // Sensor    Esquerdo
#define SensorC A1        // Sensor Centro 
#define SensorD A0       // Sensor Direito

/* ---------------------------------------------------- */

//valor maximo perto 584, valor minimo distante 250
/* -------------- Limiares dos sensores -------------- */
#define LimiarDistancia 480  // Valor para o Dino ir com tudo
#define InimigoVisto 240   // valor para considerar vizualização
#define LimiarCruzamento 500   // Valor pra compensar o sensor central pra cima
#define VistoCentral 225  //225 -> outro lado do dojo/ 265 -> meio do dojo
/* ---------------------------------------------------- */


/* -------------- Definição para os motores -------------- */
#define Parado             0  // Padronização de valores para atuação em motores
#define SentidoHorario     1  // Padronização de valores para atuação em motores
#define SentidoAntiHorario 2  // Padronização de valores para atuação em motores
#define FreioMotor         3  // Padronização de valores para atuação em motores
#define Frente             4  // Padronização de valores para atuação em motores

#define motorE 1    // Definição do motor esquerdo para facilitar a vida   
#define motorD 2    // Definição do motor direito  para facilitar a vida
/* ----------------------------------------------------- */


/* ------------ Objetos e Variáveis Globais ------------ */
int inimigoLocal;
// 2 -> Direita
// 3 -> Esquerda
int  estrategia = 0;

int valorD = analogRead(SensorD);
int valorE = analogRead(SensorE);
int valorC = analogRead(SensorC);



/* ----------------------------------------------------- */


/* ------------------- Funções ---------------------- */
void setup(); // Função de configurações e inicializações padrão da Arduino.
void loop();  // Função de loop infinito padrão da Arduino.

void configGPIOS();                                            // Função resposável por definir o modo de todos os pino. deve ser chama na função setup!!!
void codigoBusca();                                            // Codigo que faz a busca do robô adversário
int  escolheEstrategia();                                      // Faz os procedimentos do controle e retorna a estrategia selecionada
void estrategia_escolhida();                                   // Chama a estrategia escolhida

void estrategia0();                                            // Começa indo pra frente em 150 durante 0,3 segundos
void estrategia1();                                            // Busca estática, começando pela direita
void estrategia2();                                            // Posicionado atrás e virado para a direita -> Arco pra direita e uma investida central
void estrategia3();                                            // Posicionado atrás e virado para a esquerda -> Arco pra esquerda e uma investida central
void estrategia4();                                            // Busca estática, começando pela esquerda
void estrategia5();                                            // Zerada -> código de busca direto
void estrategia6();                                            // A ser pensada
void estrategia7();                                            // A ser pensada

void leituraSensores();                                        // Faz a leitura de todos os sensores e identifica o lado do inimigo
void controleMotor(byte motor, byte sentido, int intensidade); // Função de acionamento de motor através do driver ponteh tb66
//int  Controle();                                             // Função que retorna a tecla apertada do controle
/* ----------------------------------------------------- */



void setup() {
  
  //IrReceiver.begin(8); //pino que recebe o sinal do controle
  
  configGPIOS();
  
  controleMotor(motorE, Parado, 0); // Setando uma condição inicial de acionamento do motor esquerdo
  controleMotor(motorD, Parado, 0); // Setando uma condição inicial de acionamento do motor direito
  
  Serial.begin(115200); // Setando configuração da comunicção serial (velocidade de 115200 bits/S), a mesma será usada para debugs/verificações

  delay(50);

}

void loop() {
  estrategia = escolheEstrategia(); //guarda o numero da estratégia

  if (digitalRead(Start) == HIGH) {
     
    digitalWrite(LedIn, HIGH);   

    //roda estrategia, depois roda o código de busca
    estrategia_escolhida(estrategia); // Recebe qual estrategia será utilizada
    while (digitalRead(Start) == HIGH) {
      codigoBusca();
    }
    
  }
  else {
    digitalWrite(LedIn, LOW);
    controleMotor(motorE, Parado, 0);
    controleMotor(motorD, Parado, 0);
  }

}

void configGPIOS() {
  pinMode(Start, INPUT);// Entrada do microStart
  pinMode(LedIn, HIGH); // Led interno da placa RSM

  pinMode(Dp1, INPUT);// Escolher estrategia
  pinMode(Dp2, INPUT);// Escolher estrategia
  pinMode(Dp3, INPUT);// Escolher estrategia

  pinMode(PWMB, OUTPUT);// Saídas para o motor B
  pinMode(INB2, OUTPUT);// Saídas para o motor B
  pinMode(INB1, OUTPUT);// Saídas para o motor B

  pinMode(INA1, OUTPUT);// Saídas para o motor A
  pinMode(INA2, OUTPUT);// Saídas para o motor A
  pinMode(PWMA, OUTPUT);// Saídas para o motor A
  

  pinMode(SensorD, INPUT);// Entrada para o Sensor da Direita
  pinMode(SensorE, INPUT);// Entrada para o Sensor da Esquerda
  pinMode(SensorC, INPUT);// Entrada para o Sensor da Centro

}// FIM configGPIOS()

void codigoBusca() { //Rotina padrão do robo
    leituraSensores();
  
  if(valorC >= LimiarDistancia){
    controleMotor(motorE, Parado, 0);
    controleMotor(motorD, Parado, 0);
    //Para antes de mudar tudo
    for (int i = 0; i <= 250; i+=10) {//vai ate 250 apenas
      controleMotor(motorE, Frente, i);
      controleMotor(motorD, Frente, i);
      delay(0.1);
    }
    controleMotor(motorE, Frente, 255);
    controleMotor(motorD, Frente, 255);
    while(valorC >= LimiarDistancia && digitalRead(Start) == HIGH){
      valorC = analogRead(SensorC);
    }
    
  delay(20);
  }
  
  else if ((valorE >= InimigoVisto) && (valorC >= InimigoVisto) && (valorD >= InimigoVisto)){ //Caso todos vejam
    controleMotor(motorE, Parado, 0);
    controleMotor(motorD, Parado, 0);
    //Para antes de mudar tudo
    for (int i = 0; i <= 250; i+=10) {//vai ate 250 apenas
      controleMotor(motorE, Frente, i);
      controleMotor(motorD, Frente, i);
      delay(0.1);
    } 
  delay(20);
  }
  else if((valorC >= VistoCentral) && (valorC <= LimiarDistancia)){
    controleMotor(motorE, Parado, 0);
    controleMotor(motorD, Parado, 0);
    //Para antes de mudar tudo
    for (int i = 0; i <= 180; i+=10) {//vai ate 250 apenas
      controleMotor(motorE, Frente, i);
      controleMotor(motorD, Frente, i);
      delay(0.1);
    }
  delay(20);
  }

  else if ((valorE >= LimiarCruzamento) && (valorD <= InimigoVisto)) {
    //Inimigo na esquerda pertinho
    controleMotor(motorE, Parado, 0);
    controleMotor(motorD, Parado, 0);
    //Para antes de mudar tudo
    for (int i = 0; i <= 120; i+=10) {
      controleMotor(motorE, Frente, i);
      controleMotor(motorD, Frente, i);
      delay(0.1);
    }
    for (int i = 120; i <= 180; i+=10) {
      controleMotor(motorD, Frente, i);
      delay(0.1);
    }
  delay(5);
  }
  
  else if ((valorE <= InimigoVisto) && (valorD >= LimiarCruzamento)) {
    //Inimigo na diretia pertinho
    controleMotor(motorE, Parado, 0);
    controleMotor(motorD, Parado, 0);
    //Para antes de mudar tudo
    for (int i = 0; i <= 120; i+=10) {
      controleMotor(motorE, Frente, i);
      controleMotor(motorD, Frente, i);
      delay(0.1);
    }
    for (int i = 120; i <= 180; i+=10) {
      controleMotor(motorE, Frente, i);
      delay(0.1);
    }
    delay(5);
  }
  
  else if ((valorE >= InimigoVisto) && (valorD <= InimigoVisto)) {
    //Inimigo na direita la na frente
    controleMotor(motorE, Parado, 0);
    controleMotor(motorD, Parado, 0);
    //Para antes de mudar tudo
    for (int i = 0; i <= 100; i+=10) {
      controleMotor(motorE, SentidoHorario, i);
      controleMotor(motorD, SentidoHorario, i);
      delay(0.1);
    }
  delay(5);
  }
  
  else if ((valorE <= InimigoVisto) && (valorD >= InimigoVisto)) {
    //Inimigo na esquerda la na frente
    controleMotor(motorE, Parado, 0);
    controleMotor(motorD, Parado, 0);
    //Para antes de mudar tudo
    for (int i = 0; i <= 100; i+=10) {
      controleMotor(motorE, SentidoAntiHorario, i);
      controleMotor(motorD, SentidoAntiHorario, i);
      delay(0.1);
    }
  delay(5);
  }
  else{
    //Procura o ultimo lugar em que foi visto
    if (inimigoLocal == 2) {
      controleMotor(motorE, Parado, 0);
      controleMotor(motorD, Parado, 0);
      //Para antes de mudar tudo
      for (int i = 0; i <= 100; i+=10) {
        controleMotor(motorE, SentidoHorario, i);
        controleMotor(motorD, SentidoHorario, i);
        delay(0.1);
      }
      delay(5);
    }
    else if(inimigoLocal == 3) { // Gira para a esquerda
      controleMotor(motorE, Parado, 0);
      controleMotor(motorD, Parado, 0);
      //Para antes de mudar tudo
      for (int i = 0; i <= 100; i+=10) {
        controleMotor(motorE, SentidoAntiHorario, i);
        controleMotor(motorD, SentidoAntiHorario, i);
        delay(0.1);
      }
    delay(5);
    }
    else{
      inimigoLocal = 2;
    }
  }


}

int escolheEstrategia() {
  
  if((digitalRead(Dp1) == LOW) && (digitalRead(Dp2) == LOW) && (digitalRead(Dp3) == LOW)){
    // 0 0 0
    return 0;
  }
  else if((digitalRead(Dp1) == HIGH) && (digitalRead(Dp2) == LOW) && (digitalRead(Dp3) == LOW)){
  // 0 0 1
  return 1;
  }
  else if((digitalRead(Dp1) == LOW) && (digitalRead(Dp2) == HIGH) && (digitalRead(Dp3) == LOW)){
  // 0 1 0
  return 2;
  }
  else if((digitalRead(Dp1) == HIGH) && (digitalRead(Dp2) == HIGH) && (digitalRead(Dp3) == LOW)){
  // 0 1 1
  return 3;
  }
  else if((digitalRead(Dp1) == LOW) && (digitalRead(Dp2) == LOW) && (digitalRead(Dp3) == HIGH)){
  // 1 0 0
  return 4;
  }
  else if((digitalRead(Dp1) == HIGH) && (digitalRead(Dp2) == LOW) && (digitalRead(Dp3) == HIGH)){
  // 1 0 1
  return 5;
  }
  else if((digitalRead(Dp1) == LOW) && (digitalRead(Dp2) == HIGH) && (digitalRead(Dp3) == HIGH)){
  // 1 1 0
  return 6;
  }
  else if((digitalRead(Dp1) == HIGH) && (digitalRead(Dp2) == HIGH) && (digitalRead(Dp3) == HIGH)){
  // 1 1 1
  return 7;
  }
  else{
  // 0 0 0
  return 0;
  }
}

void estrategia_escolhida(int i) {
  switch (i) {
    case 0:
      estrategia0();
      break;
    case 1:
      estrategia1();
      break;
    case 2:
      estrategia2();
      break;
    case 3:
      estrategia3();
      break;
    case 4:
      estrategia4();
      break;
    case 5:
      estrategia5();
      break;
    case 6:
      estrategia6();
      break;
    case 7:
      estrategia7();
      break;
    default:
      estrategia0();
  }// Fim switch grandão
}

void leituraSensores(){
  valorD = analogRead(SensorD);
  valorE = analogRead(SensorE);
  valorC = analogRead(SensorC);
  
  if(valorD >= InimigoVisto){
    inimigoLocal = 2;
  }
  
  if(valorE >= InimigoVisto){
    inimigoLocal = 3;
  } 
   
}

void controleMotor(byte motor, byte sentido, int intensidade) {

  if (motor == motorE) {
    switch (sentido) {
      case Parado:
        digitalWrite(INB1, HIGH);
        digitalWrite(INB2, HIGH);
        digitalWrite(PWMB, HIGH);
        break;
      case SentidoHorario:
        digitalWrite(INB1, HIGH);
        digitalWrite(INB2, LOW);
        analogWrite(PWMB, intensidade);
        break;
      case SentidoAntiHorario:
        digitalWrite(INB1, LOW);
        digitalWrite(INB2, HIGH);
        analogWrite(PWMB, intensidade);
        break;
      case FreioMotor:
        digitalWrite(INB1, LOW);
        digitalWrite(INB2, LOW);
        digitalWrite(PWMB, HIGH);
        break;
        //O mesmo que o sentido horário para o motor esquerdo
      case Frente: 
        digitalWrite(INB1, HIGH);
        digitalWrite(INB2, LOW);
        analogWrite(PWMB, intensidade);
    }// Fim switch
  }
  else if (motor == motorD) {
    switch (sentido) {
      case Parado:
        digitalWrite(INA1, HIGH);
        digitalWrite(INA2, HIGH);
        digitalWrite(PWMA, HIGH);
        break;
      case SentidoHorario:
        digitalWrite(INA1, HIGH);
        digitalWrite(INA2, LOW);
        analogWrite(PWMA, intensidade);
        break;
      case SentidoAntiHorario:
        digitalWrite(INA1, LOW);
        digitalWrite(INA2, HIGH);
        analogWrite(PWMA, intensidade);
        break;
      case FreioMotor:
        digitalWrite(INA1, LOW);
        digitalWrite(INA2, LOW);
        digitalWrite(PWMA, HIGH);
        break;
      case Frente:
      // O mesmo que o sentido Anti-Horário para o direito
        digitalWrite(INA1, LOW);
        digitalWrite(INA2, HIGH);
        analogWrite(PWMA, intensidade);
    }// Fim switch
  }
  else {
    Serial.println("BUG1: Motor de entrada inválido na função controleMotor\n");//Debug
  }// Fim else

}// FIM controleMotor


  /* Ir pra frente sem dó
   * for (int i = 0; i <= 120; i+=3) {//0,04 delay 
    controleMotor(motorE, Frente, i);
    controleMotor(motorD, Frente, i);
    delay(1);
  }
  for (int i = 130; i <= 250; i+=2) {//0,04 delay 
    controleMotor(motorE, Frente, i);
    controleMotor(motorD, Frente, i);
    delay(1);
  }
  controleMotor(motorE, Frente, 255);
  controleMotor(motorD, Frente, 255);
  
  int  i = 0;
  while(i<179){//240
    leituraSensores();
    //VistoCentral
    if ((valorE >= InimigoVisto) || (valorC >= InimigoVisto) || (valorD >= InimigoVisto)){
      break;
    }
    
    delay(1);
    i++;
  }
  controleMotor(motorE, Parado, 0);//Apenas 
  controleMotor(motorD, Parado, 0);//para testar se ele vai empinar
  int valorC = analogRead(SensorC);
  if (valorC >= LimiarDistancia) {
    for (int i = 200; i <= 255; i+=5) {//Não precisa de rampa pois vai manter a mesma velocidade anterior
    controleMotor(motorE, Frente, i);
    controleMotor(motorD, Frente, i);
    delay(0.1);
    }
    while(valorC >= LimiarDistancia){
      valorC = analogRead(SensorC);
    }
    controleMotor(motorE, Parado, 0);
    controleMotor(motorD, Parado, 0);
    //Para depois disso tudo pra poder chamar a função de busca
  }
  else {
    controleMotor(motorE, Parado, 0);
    controleMotor(motorD, Parado, 0);
    //Para antes de chamar a função de busca
  }
  
}
   */
void estrategia0() { //Avança um pouco pra tentar identificar o robô inimigo
     //Omitida  
}

void estrategia1() {  //Começar atras e buscar o oponente pela direita, se atingir limite de tempo vai pra frente
     //Omitida  
}

void estrategia2(){ //arco para a direita, começando virado para a direita
     //Omitida  
}

void estrategia3(){ //Arco para a esquerda, começando virado para a esquerda
     //Omitida  
}

void estrategia4() {  //Começar atras e buscar o oponente pela esquerda, se atingir limite de tempo vai pra frente
     //Omitida  
}

void estrategia5() { //Estrategia para casos de desempate
     //Omitida  
}

void estrategia6() { //Ré
     //Omitida  
}

void estrategia7() { //Debug
     //Omitida  
}



/*int Controle() {//Função que retorna a tecla apertada do controle
  int tecla = 0;

  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.command == 0x01 || IrReceiver.decodedIRData.command == 0x15) {
      Serial.println("Seleciona");//Tecla para iniciar o combate
      tecla = seleciona;
    } if (IrReceiver.decodedIRData.command == 0x02 || IrReceiver.decodedIRData.command == 0x45) {
      Serial.println("desliga");//Tecla de desligar o robo ou reiniciar a programação
      tecla = desliga;
    } if (IrReceiver.decodedIRData.command == 0x00 || IrReceiver.decodedIRData.command == 0x47) {
      Serial.println("paridade");
      tecla = paridade;
    } if (IrReceiver.decodedIRData.command == 0xC) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x18) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x5E) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x8) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x1C) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x5A) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x42) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x52) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x4A) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x9) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x7) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x40) {
      tecla =
    } if (IrReceiver.decodedIRData.command == 0x19) {
      tecla =
    }
    IrReceiver.resume();
    return tecla;
  }
  }*/
  //Fim da função do controle


/* Thats All Folks Guyssss */

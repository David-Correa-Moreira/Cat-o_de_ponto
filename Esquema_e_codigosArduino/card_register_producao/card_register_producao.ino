
//#include <avr/pgmspace.h>             //aloca memonria sram na memória de programa
#include <MFRC522.h>                  //liby para RFID
#include <SPI.h>                      //liby para comunicação SPI
#include <EEPROM.h>                   //liby para alocar memoria na EEPROM
#include <Ethernet.h>                 //conexão com a internet
//#include <MySQL_Connection.h>         //conexão com o servidor MYSQL
#include <MySQL_Cursor.h>

// INCLUSÃO DE BIBLIOTECAS
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//======DEFINIÇÕES E VARIAVEIS=====//
#define ledRed    7
#define ledYellow 6
#define ledGreen  5
#define buzzer     2 //orignal porta 4
#define pinBtn    3
//---SPI PROTOCOL-------
#define SS_PIN    8
#define RST_PIN   9
#define modoDebug 0
#define lcdOff    60000
/*-----------pino 13 arduino, clock-------------------------*/
// DEFINIÇÕES

#define endereco  0x27 // Endereços comuns: 0x27, 0x3F
#define colunas   16
#define linhas    2

byte storedCard[4];             // guarda o ID lido da EEPROM 
byte buffer2[18];               //armazena o nome da pessoa gravado no cartão
byte readCard[4];               //armazena temporariamente o valor lido do cartão
byte masterCard[4];             //guarda o master card da EMPROM
bool successRead = false;       //variavel de controle da leitura do cartão
bool programMode = false;       // incicia o programa fora do modo de programação
bool match = false;
unsigned long lcdBefore = 0;
bool lcdStatus = true;          //controla iluminação do display
//============FIM=================//

const char string_0[] PROGMEM = "Bem Vindo!"; // "String 0" etc são as strings a serem armazenadas - adapte ao seu programa.
const char string_1[] PROGMEM = "Conectando...";
const char string_2[] PROGMEM = "Pronto";
const char string_3[] PROGMEM = "               ";
const char string_4[] PROGMEM = "Falha MySQL";
const char string_5[] PROGMEM = "Falha Internet"; 
const char string_6[] PROGMEM = "Registrado!";
const char string_7[] PROGMEM = "Sem Cadastro!";
const char string_8[] PROGMEM = "Aguarde...";
const char string_9[] PROGMEM = "Modo Config.";
const char string_10[] PROGMEM = "...Atencao...";
const char string_11[] PROGMEM = "Saindo...";
const char string_12[] PROGMEM = "Deletando...";
const char string_13[] PROGMEM = "Gravando...";
const char string_14[] PROGMEM = "Master Card";
const char string_15[] PROGMEM = "...Reinicie...";

const char *const string_table[] PROGMEM = {string_0, string_1, string_2, string_3, string_4, string_5, string_6, string_7, string_8, string_9, string_10, string_11, string_12, string_13, string_14, string_15 };
char buffer[15];                // Tenha certeza que esse buffer é grande o suficiente para armazenar a maior string

//=======INSTANCIAS==========//
EthernetClient client;                                       //cria instancia para conectar na internet
MySQL_Connection conn((Client *)&client);                    //Cria e o objeto con, que será ultilizado para conectar MYSQL server
MFRC522 mfrc522(SS_PIN, RST_PIN);                            //cria instancia para comunicação com modulo RFID
LiquidCrystal_I2C lcd(endereco, colunas, linhas);           //Cria instância para objeto LCD
//======FIM================//

//===FUNÇÃO PARA CONETACTAR A INTERNET======//
bool connectToInternet()
{
  byte i = 5, mac[] ={ 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };       //cria um enderoço fisico para o hardware de internet

  if (Ethernet.begin(mac) == 0)                              //se não conseguiu conexão via HDCP
  {
    while (i) {
      #if modoDebug == 1
        Serial.println(F("FALHA AO TENTAR CONECTAR USANDO DHCP"));
        Serial.print(F(" Ethernet.begin(mac):"));
        Serial.println(Ethernet.begin(mac));
      #endif
      if(Ethernet.begin(mac))
        return true;
      i--;
    }
    return false;
    
  }else {

    //se conseguiu via HDCP
    delay(100);
    
    #if modoDebug == 1
        Serial.println(F("CONEXÃO COM SUCESSO"));
        Serial.print(F("IP ADDRESS: "));
        Serial.println(Ethernet.localIP());
        Serial.println(Ethernet.begin(mac));
        Serial.println(F("------------------------"));
    #endif
    return true;
  }  
}
//==========================================//


//========FUNÇÃO PARA CONECTAR MYSQL SERVER====//
byte connectToMySQL ()
{
  IPAddress server_addr(192,168,15,195);                     // IP of the MySQL *server* here
  const char user[] = "rubia";                                   // MySQL user login username
  const char password[] = "rubia";                           // MySQL user login password

  if (connectToInternet()) 
  {
      byte i = 3; 
      while ( (!conn.connect(server_addr, 3306, user, password)) )    //loop para tentar se conectar ao servidor MYSQL
      {
        #if (modoDebug == 1)
          Serial.println(F("CONEXÃO COM MYSQL FALHOU"));
        #endif
        
        #if (modoDebug == 1)
          Serial.print(F("TENTANDO CONEXÃO NOVAMENTE"));    
        #endif
        
        conn.close();                                             //fecha conexão
 
        delay(500);
        
        //i--;
        //if (!i) break;
      }

      delay(1000);
      
      if (i) return 1;                                        //retorna sucesso na conexão
      else   return 2;                                        //retorna falha mysql
      
  }else {
  
     #if (modoDebug == 1)
          Serial.println(F("SEM INTERNET"));
          Serial.println(F("--------------------------"));
     #endif
     return 0;                                              //retorna falha internet
  }
}
//=============================================//


//======FUNÇÃO EXECUTAR INSERT/UPDATE QUERY MYSQL============//
char *MySQL()                                                
{
  char INSERT_DATA[72], query[90], idCard[10], *ptrLocal;  //ponteiro local para manipular o armazenamento na funação snprinf
  const byte *ptrGlobal;                                    //ponteiro gobal para evitar alteração na variavel global.
  void messageErase ();
  byte connectToMySQL ();
  ptrGlobal = readCard;
  ptrLocal  = idCard;
  
  memset(INSERT_DATA, 0, sizeof(INSERT_DATA));
  memset(query, 0, sizeof(query));
  memset(idCard, 0, sizeof idCard);
  
  const static char long_str[] PROGMEM = "INSERT INTO t.r(r) VALUES('%s')";
  //const static char long_str2[] PROGMEM = "INSERT INTO t.u(u,s) VALUES('%s','%i') ON DUPLICATE KEY UPDATE s='%i'"; //adicionado
    
  //edita o array no formato hexadecimal
  for (int i = 0; i < 4; ++i){  
    snprintf(ptrLocal ,3, "%02X", *ptrGlobal);                   //essa função grava na string, o numero de caracteres delimitado, incluindo o '/0'
    ptrLocal += 2;                                              //posiciona o ponteiro na a cada duas posições do vetor
    ptrGlobal++;                                                //ponteiro desloca para o prox. endereço do array
  } 
  
  #if (modoDebug == 1)
    Serial.print("\n-------------------------\n");
    Serial.print(" idCard :");
    Serial.print(idCard);
    Serial.print(" connectToMySQL() == 1: ");
    Serial.println(connectToMySQL());  
  #endif
  
  //digitalWrite(ledYellow, HIGH);
   
  if ( connectToMySQL() == 1 ) {

      String text;
      // Initiate the query class instance
      MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

     // if (num == 0 || num == 1){
        
        // Lê a string da flash e entrega para o array
     //   for (byte k = 0; k < strlen_P(long_str2); k++) 
     //     INSERT_DATA[k] = pgm_read_byte_near(long_str2 + k);  
  
        /* Formatar uma string e guardar o resultado em um array */
     //   if (*buffer2) sprintf(query, INSERT_DATA, buffer2, num);
     //   else sprintf(query, INSERT_DATA, idCard, num);      
         
     // } else {
        
        // Lê a string da flash e entrega para o array
        for (byte k = 0; k < strlen_P(long_str); k++) 
          INSERT_DATA[k] = pgm_read_byte_near(long_str + k);  
  
        /* Formatar uma string e guardar o resultado em um array */
        if (*buffer2) sprintf(query, INSERT_DATA, buffer2);
        else sprintf(query, INSERT_DATA, idCard);                           
      //}
      #if (modoDebug == 1)
        Serial.print("\nquery_MSQL: ");
        Serial.println(query);
      #endif
      
      // Execute the query
      cur_mem->execute(query);
      
      delay (100);
      cur_mem->close();              // Deleting the cursor also frees up memory used
      delete cur_mem;                //limapa memória do cursor       
      delay(100);     
      
      conn.close();                 //fecha conexão

       posCursor (1, 1);
       messageErase ();
       posCursor (2, 0);
       messageErase ();
   
      if (*buffer2){
        ptrLocal = buffer2;
        posCursor (1, 0);
        return ptrLocal;
      }else{
        ptrLocal = idCard;
        posCursor (4, 0);
        return ptrLocal;
      }
    
  }else if (connectToMySQL() == 2){                            //retorna um erro para o usuário
      
      #if modoDebug == 1
          Serial.println F(("Falha MySQL 2"));
      #endif
      
      posCursor (2, 1);
      messageErase ();                                   //apaga a segunda linha do display
      posCursor (3, 1);
      messageFail (false);                                   //mensagem de falha mysql
      
      while(1){
        turnLedRed(250);   //função controla acionamento do led vermelho
        turnLedGreen(250); //função controla acionamento do led verde
        turnLedYellow(250); //função controla acionamento do led amarelo
      }
  
  }else{
      
      #if modoDebug == 1
          Serial.println F(("Falha Internet 1"));
      #endif
      
      posCursor (2, 1);
      messageErase ();                                   //apaga a segunda linha do display
      posCursor (1, 1);
      messageFail (true);                                   //mensagem de falha internet
      
      while(1){                                         //loop infinito
        turnLedRed(250);   //função controla acionamento do led vermelho
        turnLedGreen(250); //função controla acionamento do led verde
        turnLedYellow(250); //função controla acionamento do led amarelo
      }
  }
}
/****************** fim ************************************/

void setCard (char *ptr)
{
  lcd.print(ptr);
}

void delayDisplay(unsigned int tempo)
{
  delay(tempo);
}

void posCursor (byte C, byte L)
{
  lcd.setCursor(C, L);
}

void messageSucess ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[6]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void messageReady ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[2]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);  
}

void messageFail (bool fail)
{
  if (fail){
    strcpy_P(buffer, (char *)pgm_read_word(&(string_table[5]))); // falha internet
    lcd.print (buffer);
  }else{
    strcpy_P(buffer, (char *)pgm_read_word(&(string_table[4]))); // falha MySQL
    lcd.print (buffer);
  }
}

void messageErase ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[3]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void messageConection ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[1]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void messageInitial ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[0]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void messageWaiting ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[8]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void noRegister ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[7]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void modeSetup ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[9]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void messageWarning ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[10]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void messageLeaving ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[11]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void messageInsert ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[13]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}
void messageDelete ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[12]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void messageMasterCard ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[14]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}

void messageRestart ()
{
  strcpy_P(buffer, (char *)pgm_read_word(&(string_table[15]))); // Casts e desreferência necessários, apenas copie.
  lcd.print (buffer);
}
/*************************  setup   *****************/

void setup() {
   
  lcd.init();                             // INICIA A COMUNICAÇÃO COM O DISPLAY
  lcd.backlight();                        // LIGA A ILUMINAÇÃO DO DISPLAY
  lcd.clear();                            // LIMPA O DISPLAY
  delay(500);
  
  //definição de GPIO do controlador
  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(pinBtn, INPUT_PULLUP);
  
  //----define estado inicial low----------//
  turnOffLeds();                                      //apaga todos os leds
 
  //-------configuração dos protocolos--------//
  Serial.begin(115200);                               // habilita comunicação serial
  while (!Serial);                                    // wait for serial port to connect
  SPI.begin();                                       // habilita comunicação SPI
  mfrc522.PCD_Init();                                 // Initialize MFRC522 Hardware 
  
  posCursor (3, 0); 
  messageInitial ();                                  //chama tela de bem vindo
  delayDisplay (2000);
  posCursor (2, 1);
  messageConection ();                                //exibe no display a mensagem conectando
  delayDisplay(2000);
  
  if(!connectToInternet()){                            // conecta na internet
    
    posCursor (1, 1);
    messageErase ();                                   //apaga a segunda linha do display
    posCursor (1, 1);
    messageFail (true);
    //lcd.print ("Falha Internet");                      //mensagem de falha
    
    while(1){                                         //loop infinito
      turnLedRed(250);   //função controla acionamento do led vermelho
      turnLedGreen(250); //função controla acionamento do led verde
      turnLedYellow(250); //função controla acionamento do led amarelo
    }
  }  
  //----------fim-------------------------//
  
  //====FUNÇÃO DELETAR MASTER CARD DA EEPROM=============//
  if (!digitalRead(pinBtn)) {
    
    turnOffLeds();                                    // apaga todos os leds

    /************** Diplay deletar EEPROM*****************/
    
    posCursor (2, 0); 
    messageErase ();
    posCursor (2, 1);
    messageErase ();

    posCursor (2, 0);
    messageDelete ();
    posCursor (1, 1);
    messageWarning ();
    
    /******************************************************/
    
    #if modoDebug == 1
      Serial.print(F("Botão Eraser Acionado :"));
      Serial.println(F("Você tem 10s para cancelar"));
      Serial.println(F("Isso irá apagar todos os dados.."));
      Serial.println(F("................................"));
    #endif

    //----confirma se o botão ficou apertado por 10s---//
    bool buttonState = monitorPinBtn(10000);          // conta 10s para prosseguir
    
    if ( buttonState && (!digitalRead(pinBtn)) ) {
       
       #if modoDebug == 1
        Serial.println(F("Começando a pagar a EEPROM..."));
       #endif
       
       for (int x = 0 ; x <= EEPROM.length(); x++) {
        if ( EEPROM.read(x)== 0 );  //se já estiver vazio, não faz nada na EEPROM
        else EEPROM.write(x, 0);    //caso contrário, limpa a EEPROM
       }
        
       #if modoDebug == 1
        Serial.println(F("EEPROM apagada com sucesso!"));
        Serial.println(F("-----------------------------------"));
        Serial.println(F("Necessário reiniciar o equipamento...!"));
       #endif

        /********** Display message **********/
       
        posCursor (2, 0); 
        messageErase ();
        posCursor (1, 1);
        messageErase ();
        
        posCursor (2, 0);
        noRegister ();
        posCursor (1, 1);
        messageRestart ();
        
        /************************************/
              
       while (1){
        turnLedRed(250);   //função controla acionamento do led vermelho
        turnLedGreen(250); //função controla acionamento do led verde
        turnLedYellow(250); //função controla acionamento do led amarelo
       }
       
        turnOffLeds();   
        
        posCursor (2, 0); 
        messageErase ();
        posCursor (2, 1);
        messageErase ();
  
        posCursor (3, 0);
        messageLeaving ();
        delayDisplay (2500);
  
        posCursor (3, 0); 
        messageInitial ();
        posCursor (5, 1);
        messageReady ();
        
    }
    else {
      
      #if modoDebug == 1
        Serial.println(F("Reset EEPROM cancelada")); // Show some feedback that the wipe button did not pressed for 15 seconds
      #endif

      /************** Desistiu de apagar a EEPROM display ************/
      /***************************************************************/
      
      turnOffLeds(); //apaga todos os leds
    }
  }
//==== FIM FUNÇÃO DELETAR MASTER CARD DA EEPROM=============//


//===========CHECA SE O MASTER CARD ESTÁ GRAVADO===================//
  if (EEPROM.read(1) != 143) {
    #if modoDebug == 1
      Serial.println(F("NENHUM CARTÃO DEFINIDO:"));
      Serial.println(F("-----------------------------------------"));
      Serial.println(F("SCANEAR CARTÃO PARA DEFINIR MASTER CARD:"));
    #endif

    /************** DISPLAY CADASTRAR CARTÃO **************/
    
    posCursor (2, 0); 
    messageErase ();
    posCursor (2, 1);
    messageErase ();

    posCursor (2, 0);
    messageMasterCard ();
    posCursor (1, 1);
    noRegister ();
    
    /*****************************************************/
    
    //=====looping até definição de uma MASTER CARD=========//
    do {
      successRead = getID();      
      turnLedRed(250); //função controla acionamento do led vermelho
      turnLedGreen(250); //função controla acionamento do led verde
      turnLedYellow(1000); //função controla acionamento do led amarelo
    }
    while(!successRead);
    //==========FIM DO LOOPING =====================//
    
    //======GRAVA NA EEPROM O MASTER CARD===========//
    for (byte j = 0; j < 4; j++) {
      EEPROM.write(2 + j, readCard[j]); //Grava na EEPROM , a partir do endereço EEPROM[2]
    }
    EEPROM.write(1, 143);               //diz para o controlador que tem um marter cadastrado no intevalo EEPROM[2]..EEPROM[5]
    #if modoDebug == 1
      Serial.println(F("Master Card definido"));
      Serial.println(F("-------------------"));
      Serial.println(F("Master Card's UID"));
    #endif
    //========FIM GRAVAÇÃO EEPROM=====//
  }
//===========FIM DA CHECAGEM DO MASTER CARD===============//

  
  //========PASSA O VALOR DO MASTER CARD DA EEPROM PARA ARRAY MASTER CARD==========//
  #if modoDebug == 1
      Serial.print(F("MASTER CARD: "));
  #endif
  
  for (byte i = 0; i<4; i++) {
    masterCard[i] = EEPROM.read(2 + i) ;
    //card +=  (String(masterCard[i], HEX));
    #if modoDebug == 1
      Serial.print(masterCard[i], HEX);
    #endif
  }
  /**************************/
  
    posCursor (1, 1);
    messageErase ();
    posCursor (1, 0);
    messageErase ();
    
    posCursor (3, 0); 
    messageInitial ();
    posCursor (5, 1);
    messageReady ();
    
  /*************************/   
    #if modoDebug == 1
      Serial.println("");
    #endif
  //=============FIM=======================//
  
  #if modoDebug == 1
    Serial.println(F(""));
    Serial.println(F("-------------------"));
    Serial.println(F("Tudo Pronto:"));
    Serial.println(F("Esperando Cartões para Scanear..."));
    Serial.println(F("-------------------"));
  #endif
}
/*************FIM DO SETUP******************/



//=====FUNÇÃO getID()-> OBTER O ID =========//
bool getID() {

  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return false;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return false;
  }
  
  lcdStatus = true;                            // Zera o time do backligth
  lcd.backlight();
  lcdBefore = millis();
  
  #if modoDebug == 1
    Serial.println(F("UID escaneado:"));
  #endif
  //---escreve o PICC's UID---//
  for (byte i = 0; i<4; i++) {
    readCard[i] = mfrc522.uid.uidByte[i];
    #if modoDebug == 1
      Serial.print(readCard[i], HEX);
    #endif
  }
  
  #if modoDebug == 1
    Serial.println(F(""));
  #endif


  /**********************************/
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  #if (modoDebug == 1)
    Serial.print(F("Name: "));
  #endif
  
  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    #if (modoDebug == 1)
      Serial.print(F("Authentication failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif
    //return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    #if (modoDebug == 1)
      Serial.print(F("Reading failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif
    //return;
  }

  //PRINT FIRST NAME
  #if (modoDebug == 1)
    for (uint8_t i = 0; i < 16; i++)
    {
      if (buffer1[i] != 32)
      {
        Serial.write(buffer1[i]);
      }
    }
  
    Serial.print(" ");
  #endif

  //---------------------------------------- GET LAST NAME

  //byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    #if (modoDebug == 1)
      Serial.print(F("Authentication failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif
    //return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    #if (modoDebug == 1)
      Serial.print(F("Reading failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif
    //return;
  }

  #if (modoDebug == 1)
    //PRINT LAST NAME
    for (uint8_t i = 0; i < 16; i++) {
      Serial.write(buffer2[i] );
    }

    //----------------------------------------
    Serial.println(F("\n**End Reading**\n"));
  #endif

  delay(1000); //change value if you want to read cards faster
/**************************************/




  /**********em analise*******************/ 
   snprintf(buffer2, 17, "%s", buffer2);
   byte j = strlen(buffer2);
   for (; (buffer2[j] == NULL) || (buffer2[j] == ' '); --j); 
   buffer2[j + 1] = '\0';
   //for (byte i = 0; buffer2[i] != '\0'; ++i) Serial.write(buffer2[i]);
   //Serial.println(strlen(buffer2));
   /**************************************/


  tone(buzzer, 261);              // buzzer
  delay(100);
  noTone(buzzer);

  
  mfrc522.PICC_HaltA(); // parar leitura
  mfrc522.PCD_StopCrypto1();
  return true; 
}
//========== FIM==================//



//========turnLedRed()==========//
void turnLedRed(int timeLedRed) {
  
  static unsigned long previousMillisLedRed = 0;
  
  if (millis() - previousMillisLedRed < timeLedRed) { //SE O TEMPO ATUAL MENOS O TEMPO ANTERIOR FOR MENOR QUE O INTERVALO, FAZ
    digitalWrite(ledRed, HIGH);                                            
  }
  if (millis() - previousMillisLedRed >= timeLedRed) {
    digitalWrite(ledRed, LOW);
  }
  if (millis() - previousMillisLedRed >= 2*timeLedRed) {
      previousMillisLedRed = millis();
  } 
}
//===============FIM==========================
//==============turnLedGreen()==============//
void turnLedGreen(int timeLedGreen) {
  
  static unsigned long previousMillisLedGreen = 0;
  
  if (millis() - previousMillisLedGreen < timeLedGreen) { //SE O TEMPO ATUAL MENOS O TEMPO ANTERIOR FOR MENOR QUE O INTERVALO, FAZ
    digitalWrite(ledGreen, HIGH);                                            
  }
  if (millis() - previousMillisLedGreen >= timeLedGreen) {
    digitalWrite(ledGreen, LOW);
  }
  if (millis() - previousMillisLedGreen >= 2*timeLedGreen) {
      previousMillisLedGreen = millis();
  } 
}
//===============FIM==========================
//==============turnLedYellow()==============//
void turnLedYellow(int timeLedYellow) {
  
  static unsigned long previousMillisLedYellow = 0;
  
  if (millis() - previousMillisLedYellow < timeLedYellow) { //SE O TEMPO ATUAL MENOS O TEMPO ANTERIOR FOR MENOR QUE O INTERVALO, FAZ
    digitalWrite(ledYellow, HIGH);                                            
  }
  if (millis() - previousMillisLedYellow >= timeLedYellow) {
    digitalWrite(ledYellow, LOW);
  }
  if (millis() - previousMillisLedYellow >= 2*timeLedYellow) {
      previousMillisLedYellow = millis();
  } 
}
//===============FIM==========================//

//==========turnOffLeds======================//
void turnOffLeds() {
  digitalWrite(ledRed, LOW);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledGreen, LOW);
}
//============FIM==========================//

//===========turnOneds()==================//
void turnOneds() {
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledYellow, HIGH);
  digitalWrite(ledGreen, HIGH);
}
//========FIM============================//

//========FUNÇÃO QUE AGUARDA 10S======//
bool monitorPinBtn(int timeWaiting) {
  
  unsigned long timeNow = (unsigned long)millis();

  while((unsigned long)millis() - timeNow < timeWaiting) { //enquanto não for menor que 10s
    turnLedGreen(300); //função controla acionamento do led verde
    turnLedRed(300);   //função controla acionamento do led amarelo
    if (((unsigned long)millis() % 500) == 0) { //verifca a cada .5s se o botão foi acionado
      if (digitalRead(pinBtn) != LOW)
        return false;
    }
  }
  return true;
}
//===========FIM=========================//

//============ Check readCard IF is masterCard=========================//
boolean isMaster( byte test[] ) {
  if ( checkTwo( test, masterCard ) )
    return true;
  else
    return false;
}
//---------- Check Bytes-------------------//
boolean checkTwo ( byte ReadCard[], byte CardMaster[] ) { //COMPARA O CARTÃO LIDO a[] COM O STOREDCARD b[]
  if ( ReadCard[0] != 0 )                                 // assegura que tem alguma coisa no cartão lido
    match = true;       
//-----varre as 4 posiçãos do array----//    
  for ( byte k = 0; k < 4; k++ ) {   
    if ( ReadCard[k] != CardMaster[k] )                   // IF a != b then set match = false, one fails, all fail
      match = false;
  }
  if ( match ) {                                          // checa estatus da variavel global match
    return true;                                          // Return true
  }
  else  {
    return false;                                         // Return false
  }
}
//================FIM========================//

//=========== Encontre o ID na EEPROM====================//
boolean findID( byte find[] ) {               // (find[] = cartão lido) => recebe o cartão lido
  int count = EEPROM.read(0);                 // leia o primeiro byte da EEPROM q contem o numero de UID gravados
  for ( int i = 1; i <= count; i++ ) {        // Procure em cada endereço de EEPROM
    readID(i);                                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {     // compara para verificar se cartão lido está na EEPROM
      return true; 
      break;                                  // Para e retorna verdadeiro
    }
    else {                                    // Caso contrário, faz nada
    }
  }
  return false;                               //se não encontrar nehum cartão lido na EEPROM retorna falso
}
//--------- Leia o ID da EEPROM-----------------
void readID( int &number ) {
  int start = (number * 4 ) + 2;              // Procure a partir da prox. posição depois do master card, ou seja, posição 6,
  for ( byte i = 0; i < 4; i++ ) {            // varre os 4 bytes
    storedCard[i] = EEPROM.read(start + i);   // passa os valores da EEPROM para o array
  }
}
//===========FIM================================//

//==============ACESSO PERMITIDO================//
void granted () {
  
  digitalWrite(ledGreen, HIGH);   // liga o led verde
  digitalWrite(ledYellow, LOW);   // desliga led amarelo
  
  //-------------inserir no MySQL (acesso) ------------
  posCursor (2, 1);
  messageErase ();
  posCursor (3, 1);
  messageWaiting ();
 
  char *ptr = MySQL ();

  setCard (ptr);
  posCursor (3, 1);
  messageSucess ();

  tone(buzzer,261);    
  // Espera um tempo para Desativar
  delay(150);
  //Desativa o buzzer
  noTone(buzzer); 
  // Aciona o buzzer na frequencia relativa ao Ré em Hz   
  tone(buzzer,293);             
  delay(150);    
  noTone(buzzer); 
  // Aciona o buzzer na frequencia relativa ao Mi em Hz
  tone(buzzer,329);      
  delay(150);
  noTone(buzzer);     
  // Aciona o buzzer na frequencia relativa ao Fá em Hz
  tone(buzzer,349);    
  delay(150);    
  noTone(buzzer); 
  // Aciona o buzzer na frequencia relativa ao Sol em Hz
  tone(buzzer,392);            
  delay(150);
  noTone(buzzer);
  
  delayDisplay(2500);    

  posCursor (1, 0);
  messageErase ();
  posCursor (2, 1);
  messageErase ();
 
  posCursor (3, 0); 
  messageInitial (); 
  posCursor (5, 1);
  messageReady ();   
  
  //---------------------------------------------------
  
  //digitalWrite(relay, LOW);    // fecha relé           
  digitalWrite(ledYellow, HIGH);   // liga led amarelo
  digitalWrite(ledGreen, LOW);   // desliga o led verde
}
//============FIM============================//

//================== Acesso Negado ============//
void denied(unsigned int setDelay) {

  digitalWrite(ledYellow, LOW);   // desliga led amarelo
  digitalWrite(ledRed, HIGH);  // Turn off red LED
  
  posCursor (2, 1);
  messageErase ();
  posCursor (1, 1);
  noRegister ();
  delay(500);
  unsigned long timeNow = millis();
    while (millis() - timeNow < setDelay) {
    //digitalWrite(relay, HIGH);    // fecha relé
    tone(buzzer, 293);              // buzzer
    delay(150);
    noTone(buzzer); 
    delay(350);
  }   
  
  posCursor (1, 1); 
  messageErase ();
  posCursor (5, 1); 
  messageReady ();  
  

  //digitalWrite(relay, LOW);    // fecha relé
 //tone(buzzer, 293);              // buzzer
  
  
  digitalWrite(ledRed, LOW);  // Turn off red LED
  digitalWrite(ledYellow, HIGH);   // desliga led amarelo
  
}
//=======FIM=====================//

//=============== Remove ID from EEPROM==================//
void deleteID( byte a[] ) { //a[] == readCard[] -> cartão lido

  
  if ( !findID( a ) ) {     // confirma se esse ID realmente existe na EEPROM!
    failedWrite(3000);      // If not
    
    #if modoDebug == 1
      Serial.println(F("Falha! Algo de errado com ID ou EEPROM"));
    #endif
  }
  else {
    int num = EEPROM.read(0);                                 // Get the numer of used spaces, position 0 stores the number of ID cards
    int j;
    int slot = findIDSLOT( a );                               // retorna o numero do slot onde o cartão começa
    int start = (slot * 4) + 2;                               // marca a posição da EEPROM onde está o primeiro byte do cartão
    int looping = ((num - slot) * 4);                         // define o novo tamanho de bytes usados na EEPROM após o delete
    num--;                                                    // decrementa o contador de Entries de ID na EEPROM
    EEPROM.write( 0, num );                                   // Escreve a nova contagem de cartão no byte 0 da EEPROM
    for ( j = 0; j < looping; j++ ) {                         // Varre a EEPROM com o novo tamanho
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // desloca cada cartão 4 posições de traz para frente na EEPROM, ou seja, reescreve.
    }
    for ( byte k = 0; k < 4; k++ ) {                          // depois de deslocar 4 posições p/frente escreve 0 na ultima linha
      EEPROM.write( start + j + k, 0);
      //card +=  (String(a[k], HEX));                           // converte byte para string; 
    }

    turnOffLeds();

    /******* Display deletando cartão******/
      posCursor (1, 1);
      messageErase ();
      posCursor (3, 1);
      messageDelete ();
      
      successDelete(2500);

      tone(buzzer,294,300); //RE
      delay(150);   
      tone(buzzer,262,100); //DO
      delay(150);
      tone(buzzer, 294,300); //RE
      delay(150);
      noTone(buzzer);
      
      posCursor (2, 1);
      messageErase ();
      posCursor (2, 0);
      messageErase ();
      
      posCursor (3, 0); 
      messageInitial ();
      posCursor (5, 1);
      messageReady ();
      
      /*************************************/   
        
    #if modoDebug == 1
      Serial.println(F(" ID removido da EEPROM EEPROM"));
    #endif
  }
}
//=================FIM DA FUNÇÃO DELETE ID=========================//

//=====================failedWrite=====================//
// Pisca o led vermelhor por 3 segundos
void failedWrite( int timeFail) {

  unsigned long timeNow = millis();
  
  while (millis() - timeNow < timeFail) {
    turnLedRed(200); //função controla acionamento do led vermelho
  }
}
//==========-------====FIM==================================//

//=====================successDelete=====================//
// Pisca o led vermelhor por 3 segundos
void successDelete( int timeSuccess) {

  unsigned long timeNow = millis();
  
  while (millis() - timeNow < timeSuccess) {
    turnLedRed(250); //função controla acionamento do led amarelo
  }
}
//==============FIM==================//

//=================Find Slot==============================//
int findIDSLOT( byte find[] ) {
  int count = EEPROM.read(0);               // Read primeiro byte que contem numero ID salvos
  for ( int i = 1; i <= count; i++ ) {      // varre o numero de cartão salvos
    readID(i);                              // Lê o ID da EEPROM, e guardo no storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Verifica se o storedCard é igual ao cartão lido
      return i;                             // The slot number of the card
      break;                                // interrompe o loop
    }
  }
}
//==========================FIM========================//

//===============Adicionar cartão na EEPROM====================//
void writeID( byte a[] ) {//a[] == readCard[]
  
  if ( !findID( a ) ) {                 // Verifica se esse cartão já foi visto na EEPROM!
    int num = EEPROM.read(0);           // Pega o numero de cartões registrados
    int start = ( num * 4 ) + 6;        // Descubra onde o proximo slot começa
    num++;                              // incementa o contador
    EEPROM.write( 0, num );             // atualizao numero de cartões salvos
    for ( byte j = 0; j < 4; j++ ) {    // loop de 4X
      EEPROM.write( start + j, a[j] );  //Escreve o valor lido to EEPROM na posição exata
      //card +=  (String(a[j], HEX));     //converte byte para string e armazena na variavel
    }
    turnOffLeds();

    //MySQL (1);
    /********** Display Inserindo************/
    
      posCursor (1, 1);
      messageErase ();
      posCursor (3, 1);
      messageInsert ();
      
      successWrite(2500);               //delay

      tone(buzzer,349,300); //RE
      delay(150);   
      tone(buzzer,330,100); //DO
      delay(150);
      tone(buzzer, 330,300); //RE
      delay(150);
      noTone(buzzer);
            
      posCursor (2, 1);
      messageErase ();
      posCursor (2, 0);
      messageErase ();
      
      posCursor (3, 0); 
      messageInitial ();
      posCursor (5, 1);
      messageReady ();
      
      /****************************************/
    
    #if modoDebug == 1
      Serial.println(F("Novo cartão salvo com sucesso na EEPROM"));
    #endif

  }
  else {
    failedWrite(3000);
    #if modoDebug == 1
      Serial.println(F("Falha! Algo errado com o cartão ou a EEPROM"));
    #endif
  }
}
//================FIM===================================//

//=====================successDelete=====================//
// Pisca o led vermelhor por 3 segundos
void successWrite( int timeAddSuccess) {

  unsigned long timeNow = millis();
  
  while (millis() - timeNow < timeAddSuccess) {
    turnLedGreen(300); //função controla acionamento do led verde
  }
}
//=======================FIM======================//




void loop() {
  
  do {
    successRead = getID();  //se algum cartão for lido armazena  array readCar[] e retorna true
   
    if(( millis() - lcdBefore > lcdOff) && lcdStatus ){
      lcd.noBacklight();
      lcdStatus = false;
      lcdBefore = millis();
    }
    
    
    /**********RESET MARSTER CARD*********************/
    if (digitalRead(pinBtn) == LOW) { // Check if button is pressed
      
      #if (modoDebug == 1)
        Serial.println(F("Wipe Button Pressed"));
        Serial.println(F("Master Card will be Erased! in 10 seconds"));
      #endif

      lcdStatus = true;                            // Zera o time do backligth
      lcd.backlight();
      lcdBefore = millis();
      
      turnOffLeds();
      
      posCursor (2, 0); 
      messageErase ();
      posCursor (2, 1);
      messageErase ();
      
      posCursor (2, 1); 
      messageDelete ();
      posCursor (2, 0); 
      messageMasterCard (); 
      
      // Give some feedback
      bool buttonState = monitorPinBtn(10000);          // conta 10s para prosseguir 
      
      if (buttonState  && !digitalRead(pinBtn)) {    // If button still be pressed, wipe EEPROM
        EEPROM.write(1, 0);                  // Reset Magic Number.
        
        #if (modoDebug == 1)
          Serial.println(F("Master apagado do sistema"));
        #endif

        turnOffLeds();
        /********** Display message **********/
        
        posCursor (2, 0); 
        messageErase ();
        posCursor (2, 1);
        messageErase ();
        
        posCursor (1, 0);
        noRegister ();
        posCursor (4, 0);
        messageErase ();
        posCursor (5, 0);
        messageMasterCard ();
        
        posCursor (1, 1);
        messageRestart ();
        
        /************************************/
        for(int i = 262; i <= 500; i += 15){ 
        tone(buzzer,i,200); //DO
        delay(150);
        }
        
        while (1){
          turnLedRed(250);   //função controla acionamento do led vermelho
          turnLedGreen(250); //função controla acionamento do led verde
          turnLedYellow(250); //função controla acionamento do led amarelo
        }//fim do while
      }//fim do if

      turnOffLeds();
      /*********diplay***************/
      
      posCursor (2, 0); 
      messageErase ();
      posCursor (2, 1);
      messageErase ();

      posCursor (3, 0);
      messageLeaving ();
      delayDisplay (2000);

      posCursor (3, 0); 
      messageInitial ();
      posCursor (5, 1);
      messageReady ();      
      
    } //fim  do reset master
    /**************************************************/
    
    if ( programMode) {     //se for verdadeiro entra pisca o led amarelo
      turnLedGreen(300); //função controla acionamento do led verde
      turnLedYellow(300);   //função controla acionamento do led amarelo
    }
    else {                  //caso contrário led amarelo fica acesso
      turnOffLeds();
      if (lcdStatus) digitalWrite(ledYellow, HIGH);
    }
  }
  while(!successRead);      //permanece no loop até que algum cartão seja lido
  
  // =======CHECAGEM DO CARTAO LIDO==============//

//***************** MODO DE PROGRAMA *****************************  
  if (programMode) {            // se o modo de programaMode = true
    if ( isMaster(readCard) ) { //verifica se o cartão lido é o master card
      
      #if modoDebug == 1
        Serial.println(F("Master Card Scaneado"));
        Serial.println(F("Saindo do Modo de programa"));
        Serial.println(F("-----------------------------"));
      #endif

      /********* SAINDO MODO PROGRAMA ******************/
      
      posCursor (1, 1);
      messageErase ();
      posCursor (4, 1);
      messageLeaving ();
      
      delayDisplay (2500);
      
      posCursor (1, 1);
      messageErase ();
      posCursor (2, 0);
      messageErase ();
      
      posCursor (3, 0); 
      messageInitial ();
      posCursor (5, 1);
      messageReady (); 
      /******************************************/
      programMode = false;     //desabilita o modo de programa
      return;                  //sai do condicional e retorna para o loop
    }
    else {                     // se o cartão lido não for o master
      if ( findID(readCard) ) { //checa se cartão lido está na EEPROM
        #if modoDebug == 1
          Serial.println(F("ID encontrado na EEPROM, Removendo..."));
        #endif
        deleteID(readCard); //identifica a posição de memoria e sobrescre de traz pra frente a EEPROM
        #if modoDebug == 1
          Serial.println(F("-----------------------------"));
          Serial.println(F("Escaneie outro cartão para adicionar o deletar da EEPROM"));
        #endif
      }
      else {                    // Se ID não estiver na EEPROM adicione
        #if modoDebug == 1
          Serial.println(F("Cartão desconhecido, adicionando..."));
        #endif
        writeID(readCard);       //escreve em uma posição de EEPROM livre o novo cartão
        #if modoDebug == 1
          Serial.println(F("-----------------------------"));
          Serial.println(F("Escaneie outro cartão para adicionar o inserir da EEPROM"));
        #endif
      }
    }
  }
  //********************* ENTRA NO MODO DE PROGRAMA *************************
  else {
    if ( isMaster(readCard) ) {//verfica se o cartão lido foi o master card
      programMode = true;//habilita o modo programa
      #if modoDebug == 1
        Serial.println(F("Olá Master Card - Entrando em Program Mode"));
      #endif
      int count = EEPROM.read(0);   // Read the first Byte of EEPROM that
      #if modoDebug == 1
        Serial.print(F("Eu tenho "));     // stores the number of ID's in EEPROM
        Serial.print(count);
        Serial.print(F(" gravações na EEPROM"));
        Serial.println(F(""));
        Serial.println(F("Escaneio um novo cartão para remover ou adicionar da EEPROM"));
        Serial.println(F("Escaneie novamente o master card para sair do Program Mode"));
        Serial.println(F("-----------------------------"));
      #endif

      /********* MODO PROGRAMA ******************/  
      posCursor (2, 0);
      modeSetup ();
      posCursor (1, 1);
      messageWarning ();
      /******************************************/
      
    //********** PROCESSA SE O ACESSO ESTÁ LIBERADO OU NÃO*******************
    }
    else {                      // se o cartão lido não é o master card
      if ( findID(readCard) ) { // checar se o cartão está na EEPROM
        #if modoDebug == 1
          Serial.println(F("Bem-Vindo, Você pode passar"));
        #endif
        granted();         // permite acesso
        
      }
      else {      // caso contrário,não esteja na EEPROM, Mostra que ID não é valido
        #if modoDebug == 1
          Serial.println(F("Acesso Negado"));
        #endif
        denied(1500);
      }
    }
  }
 
}

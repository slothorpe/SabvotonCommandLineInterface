/* Command-Line Interface für den Sabvoton-Controller
 * Basis ModbusMaster Library
 * Holger Lesch, August 2019
 * holger.lesch@gmx.net
 */

#include <ModbusMaster.h>
#include <avr/pgmspace.h>

// instantiate ModbusMaster object
ModbusMaster node;

#define BUF_LENGTH 128  /* Buffer for the incoming command. */
static bool do_echo = true;
static unsigned long lastMillis = 0;
 
/* Ein Kommando ausführen */
static void exec(char *cmdline)
{
    char *command = strsep(&cmdline, " ");
    char buf[80];
    
    if (strcmp_P(command, PSTR("help")) == 0) {
        Serial.println(F(
            "write <register> <value>: writeSingleRegister\r\n"
            "read <register>: readHoldingRegister\r\n"
            "dump <startregister> <endregister>: try to read all registers in a range\r\n"
            "test <register> <startval> <endval> <increment> <delay in ms>: write incremental values to a single register\r\n"
            "echo <value>: set echo off (0) or on (1)"));
    } else if (strcmp_P(command, PSTR("write")) == 0) {
        int reg = atoi(strsep(&cmdline, " "));
        if (reg == 0 || strlen(cmdline)==0) {
          Serial.print(F("error: invalid command: "));
          Serial.println(command);
          Serial.println("usage: write <register> <value>");
          return;
        }
        int val = atoi(cmdline);
        int result = node.writeSingleRegister(reg, val);
        if (result != node.ku8MBSuccess) result = node.writeSingleRegister(reg, val);
        if (result == node.ku8MBSuccess)
        {
          sprintf(buf,"register %d set to %d (0x%04x)",reg, val, val);
          Serial.println(buf);
        } else {
          sprintf(buf,"write error: %02x", result);
          Serial.println(buf);
        }
    } else if (strcmp_P(command, PSTR("read")) == 0) {
        int reg = atoi(cmdline);
        if (reg == 0) {
          Serial.print(F("error: invalid command: "));
          Serial.println(command);
          Serial.println("usage: read <register>");
          return;
        }
        int result = node.readHoldingRegisters(reg, 1);
        if (result != node.ku8MBSuccess) result = node.readHoldingRegisters(reg, 1);
        if (result == node.ku8MBSuccess)
        {
          int r = node.getResponseBuffer(0);
          sprintf(buf,"register %d: %d (0x%04x)",reg, r,r);
          Serial.println(buf);
        } else {
          sprintf(buf,"read error: %02x", result);
          Serial.println(buf);
        }
    } else if (strcmp_P(command, PSTR("dump")) == 0) {
        int reg1 = atoi(strsep(&cmdline, " "));
        int reg2 = atoi(cmdline);
        if (reg1 == 0 || reg2==0 || reg1>reg2) {
          Serial.print(F("error: invalid command: "));
          Serial.println(command);
          Serial.println("usage: dump <startregister> <endregister>");
          return;
        }
        int j;
        for (j=reg1; j<=reg2;j++) {
          int result = node.readHoldingRegisters(j, 1);
          if (result != node.ku8MBSuccess) result = node.readHoldingRegisters(j, 1);
          if (result == node.ku8MBSuccess)
          {
            int r = node.getResponseBuffer(0);
            sprintf(buf,"%04d: %d (0x%04x)",j,r,r);
            Serial.println(buf);
          } else {
            //sprintf(buf,"%04d: error",j);
            //Serial.println(buf);
          }
        }
    } else if (strcmp_P(command, PSTR("test")) == 0) {
        int reg = atoi(strsep(&cmdline, " "));
        if (reg == 0 || strlen(cmdline)==0) {
          Serial.print(F("error: invalid command: "));
          Serial.println(command);
          Serial.println("usage: write test <register> <startval> <endval> <increment> <time in ms>");
          return;
        }
        int startval = atoi(strsep(&cmdline, " "));
        int endval = atoi(strsep(&cmdline, " "));
        int incre =  atoi(strsep(&cmdline, " "));
        int dtime = atoi(cmdline);
        if (startval >= endval || incre==0 || dtime ==0) {
          Serial.print(F("error: invalid command: "));
          Serial.println(command);
          Serial.println("usage: test <register> <startval> <endval> <increment> <time in ms>");
          return;
        }
        int i;
        for (i=startval; i<=endval;i+=incre) {
          int result = node.writeSingleRegister(reg, i);
          if (result != node.ku8MBSuccess) result = node.writeSingleRegister(reg, i);
          if (result == node.ku8MBSuccess) {
            sprintf(buf,"register %d set to %d (0x%04x)",reg, i, i);
            Serial.println(buf);
          } else {
            sprintf(buf,"write error: %02x", result);
            Serial.println(buf);
            return;
            }
          delay(dtime);
          }
        } else {
        Serial.print(F("error: unknown command: "));
        Serial.println(command);
    }
}


// Das MQCON Programm schreibt immer am Anfang drei mal den Wert 13345 in das Register 4039 !?
void startseq()
{
  uint32_t value;
  uint8_t result;
 
  value = 13345;
 
  // slave: write TX buffer to (1) 16-bit registers starting at register 4039
  digitalWrite(13,1);
  result = node.writeSingleRegister(4039, value);
  Serial.print(result);
  delay(50);
  result = node.writeSingleRegister(4039, value);
  Serial.print(result);
  delay(50);
  result = node.writeSingleRegister(4039, value);
  Serial.println(result);
  digitalWrite(13,0);
  if (result != node.ku8MBSuccess) {
    Serial.println("couldn't send startseq, please connect to Sabvoto controller and restart/reset.");
    while (1);
  }
}

void setup()
{
  pinMode(13,OUTPUT);
  digitalWrite(13,0);
  // use Serial3 (port 3); initialize Modbus communication baud rate
  Serial3.begin(19200,SERIAL_8O1);
  Serial.begin(115200);

  // communicate with Modbus slave ID 1 over Serial (port 3)
  node.begin(1, Serial3);

  startseq();
  Serial.println("Welcome to the SABVOTON command line interface");
  Serial.println("type help for help");
  Serial.println("");
  Serial.println("Note: in the arduino serial monitor you have to end any commandline with a '.'");
  Serial.println("");
  Serial.println("PLEASE BE CAREFULL WITH WRITING INTO UNKNOWN REGISTER");
  Serial.println("");
  Serial.print("> ");
}


#define NUM_STAT_REG  26

void loop()
{
  uint8_t result;
  uint16_t data[80];
  int j;
  static unsigned long lastMicros;
  
  // Alle 500ms einmal die Statusregister ab 2748 lesen
  if (millis()>lastMillis+500) {
    digitalWrite(13,1);
    result = node.readHoldingRegisters(2748, NUM_STAT_REG);
    digitalWrite(13,0);
    //Serial.println(result);
    if (result == node.ku8MBSuccess)
    {
      for (j = 0; j < NUM_STAT_REG-1; j++)
      {
        data[j] = node.getResponseBuffer(j);
        //Serial.print(data[j]);
        //Serial.print(" ");
      }
      //Serial.println();
    } else {
      //Serial.println("read error");
    }
    lastMillis=millis();
  }

  // Kommandos entgegen nehmen
  while (Serial.available()) {
        static char buffer[BUF_LENGTH];
        static int length = 0;

        int data = Serial.read();
        if (data == '\b' || data == '\177') {  // BS and DEL
            if (length) {
                length--;
                if (do_echo) Serial.write("\b \b");
            }
        }
        else if (data == '\r' || data =='.') {
            if (do_echo) Serial.write("\r\n");    // output CRLF
            buffer[length] = '\0';
            if (length) exec(buffer);
            Serial.print("> ");
            length = 0;
        }
        else if (length < BUF_LENGTH - 1) {
            buffer[length++] = data;
            if (do_echo) Serial.write(data);
        }
    }
}

 /**
  By Sanne Metten - May 2021
  Based on ryand1011 (https://github.com/ryand1011)
  
  Interactive Installation with multiple RFID reader on one Arduino nano
  Play audio fragment based on a placed character (tag) and stops when tag is removed
*/

#include <SPI.h>
#include <MFRC522.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include <DFMiniMp3.h>

//Class needed for mp3 library
class Mp3Notify
{
  public:
    static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action)
    {
      if (source & DfMp3_PlaySources_Sd)
      {
        Serial.print("SD Card, ");
      }
      if (source & DfMp3_PlaySources_Usb)
      {
        Serial.print("USB Disk, ");
      }
      if (source & DfMp3_PlaySources_Flash)
      {
        Serial.print("Flash, ");
      }
      Serial.println(action);
    }
    static void OnError(uint16_t errorCode)
    {
      // see DfMp3_Error for code meaning
      Serial.println();
      Serial.print("Com Error ");
      Serial.println(errorCode);
    }
    static void OnPlayFinished(DfMp3_PlaySources source, uint16_t track)
    {
      Serial.print("Play finished for #");
      Serial.println(track);
    }
    static void OnPlaySourceOnline(DfMp3_PlaySources source)
    {
      PrintlnSourceAction(source, "online");
    }
    static void OnPlaySourceInserted(DfMp3_PlaySources source)
    {
      PrintlnSourceAction(source, "inserted");
    }
    static void OnPlaySourceRemoved(DfMp3_PlaySources source)
    {
      PrintlnSourceAction(source, "removed");
    }
};

#define RST_PIN         9          
#define SS_1_PIN        10         // SS Pin is  for each RFID reader, other wires joint together
#define SS_2_PIN        8         // RFID reader #2

//When upgraded to 4 RFID readers
#define SS_3_PIN        7        // RFID reader #3
//#define SS_4_PIN      5        // RFID reader #4

#define NR_OF_READERS   3         // The amount of RFID readers
//#define NR_OF_READERS   4

byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN};
//byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN, SS_4_PIN}; // when more cards are added

MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.

String tagCharacter1 = "04 15 91 0A ED EF 14"; // This stores the UID of RFID tag
String tagCharacter2 = "04 15 91 0A 3A C7 14";

//String tagCharacter# = "01 12 93 0B 3B C7 18"; add more characters if needed, change UID

boolean isPlaying = false; 
SoftwareSerial mySoftwareSerial(2, 3); // RX, TX for DFpayer
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(mySoftwareSerial);

//*****************************************************************************************//
void setup() {
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);        // Initialize serial communications with the PC, COMMENT OUT IF IT FAILS TO PLAY WHEN DISCONNECTED FROM PC
  SPI.begin();                 // Init SPI bus

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each RFID reader
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }

  mp3.begin();                // Init Mp3 player
  uint16_t volume = mp3.getVolume();
  mp3.setVolume(28);         // Volume level, from 0-30

  delay(1000); //Ensure everything is setup before start
}

uint8_t control = 0x00; // Used for checking presence of RFID tag

//*****************************************************************************************//
void loop() {

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // For each RFID reader, look for new cards
    String tag = "";
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      // Show some details of the PICC (the type of RFID tag)
      Serial.print(F(": Card UID:"));

      for (byte i = 0; i < mfrc522[reader].uid.size; i++) {
        //Serial.print(mfrc522[reader].uid.uidByte[i] < 0x10 ? " 0" : " ");
        //Serial.print(mfrc522[reader].uid.uidByte[i], HEX);
        tag.concat(String(mfrc522[reader].uid.uidByte[i] < 0x10 ? " 0" : " "));
        tag.concat(String(mfrc522[reader].uid.uidByte[i], HEX));
      }
      tag.toUpperCase();

      Serial.println(tag);
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));

      // This is where the magic happens :D
      //*********************************************************************************//
      //Reader 0 is the remote control
      if ((reader == 0) && (tag.substring(1) == tagCharacter2))
      {
        Serial.println("Audiofragment plays: TV REMOTE, Character2");
        mp3.playMp3FolderTrack(1);
        isPlaying = true;
      }
      else if ((reader == 0) && (tag.substring(1) == tagCharacter1))
      {
        Serial.println("Audiofragment plays: TV REMOTE, Character1");
        mp3.playMp3FolderTrack(2);
        isPlaying = true;
      }

      //*********************************************************************************//
      //Reader 1 is the coffee
      if ((reader == 1) && (tag.substring(1) == tagCharacter2))
      {
        Serial.println("Audiofragment plays: COFFEE, Character2");
        mp3.playMp3FolderTrack(3);
        isPlaying = true;
      }
      else if ((reader == 1) && (tag.substring(1) == tagCharacter1))
      {
        Serial.println("Audiofragment plays: COFFEE, Character1");
        mp3.playMp3FolderTrack(4);
        isPlaying = true;

      }
      //*********************************************************************************//
      //Reader 2 is the telephone
      if ((reader == 2) && (tag.substring(1) == tagCharacter2))
      {
        Serial.println("Audiofragment plays: TELEPHONE, Character2");
        mp3.playMp3FolderTrack(5);
        isPlaying = true;
      }
      else if ((reader == 2) && (tag.substring(1) == tagCharacter1))
      {
        Serial.println("Audiofragment plays: TELEPHONE, Character1");
        mp3.playMp3FolderTrack(6);
        isPlaying = true;
      }
      //*********************************************************************************//
      //Reader 3 is the cassette player
      if ((reader == 3) && (tag.substring(1) == tagCharacter2))
      {
        Serial.println("Audiofragment plays: CASSETTE, Character2");
        mp3.playMp3FolderTrack(7);
        isPlaying = true;
      }
      else if ((reader == 3) && (tag.substring(1) == tagCharacter1))
      {
        Serial.println("Audiofragment plays: CASSETTE, Character1");
        mp3.playMp3FolderTrack(8);
        isPlaying = true;
      }

      //*****************************************************************************************//
      while (true) {
        control = 0;

        //  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
        for (int i = 0; i < 3; i++) {
          if (!mfrc522[reader].PICC_IsNewCardPresent()) {
            if (mfrc522[reader].PICC_ReadCardSerial()) {
              //          Serial.print('a');
              control |= 0x16;
            }
            if (mfrc522[reader].PICC_ReadCardSerial()) {
              //          Serial.print('b');
              control |= 0x16;
            }
            //        Serial.print('c');
            control += 0x1;
          }
          //      Serial.print('d');
          control += 0x4;
        }
        Serial.println(control);
        if (control == 15) {
          mp3.stop();
          break;
        }
        //}
      }

      //*****************************************************************************************//

      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();

    }
  }
}


//Helper routine to dump a byte array as hex values to Serial.
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

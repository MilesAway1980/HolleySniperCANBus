#include <Arduino.h>
#include <mcp_can.h>

class ArduinoPins
{
  /*          
    SCI Pins

              UNO       Mega 2650
    SCK       D13       D52
    (MO)SI    D11       D51
    (MI)SO    D12       D50
    CS/SS     D10       D53
  */

  public:
    unsigned int MISO = 50;
    unsigned int MOSI = 51;
    unsigned int SCK  = 52;
    unsigned int CS   = 53;

    unsigned int CANBusPin = 2;
};

ArduinoPins arduinoPins;

struct ValueTypeFloat
{
  float value;
  unsigned long int millis;
};

class Values
{
  public:

    ValueTypeFloat RPM;
    ValueTypeFloat injectorPulseWidth;
    ValueTypeFloat dutyCycle;
    ValueTypeFloat closedLoopCompensation;
    ValueTypeFloat targetAFR;
    ValueTypeFloat airFuelRatio;
    ValueTypeFloat fuelFlowlbsPerHour;
    ValueTypeFloat ignitionTiming;
    ValueTypeFloat IACPosition;
    ValueTypeFloat MAP;
    ValueTypeFloat TPS;
    ValueTypeFloat MAT;
    ValueTypeFloat CTS;
    ValueTypeFloat battery;
};

Values values;

class CANBus
{
  /*******  CAN Bus MCP2515  *******


      _________________
      |    |__|__|    |
      |     HI LO     |
      |               |
      |               |
      |               |
      |     pins      |
      | 1 2 3 4 5 6 7 |
      -----------------


    1   Int               2
    2   SCK               52
    3   (MO)SI            51
    4   (MI)SO            50
    5   CS                53
    6   GND
    7   VCC

  */

  bool initialized = false;
  bool firstInit = true;
  unsigned long messageCheckTimer;
  
  MCP_CAN* MCPCANBus;
  
  // Holley data is sent as a float.
  union CANData
  { 
    unsigned char payloadArray[4];
    float value;
  };

  public: CANBus()
  {

  }

  ~CANBus()
  {
    delete MCPCANBus;
  }

  public:Update()
  {
    if (initialized == true)
    {
      CheckForCANMessage();
      CheckForCANTimeout();
    }
    else
    {
      Initialize();
    }
  }

  public:Initialize()
  {
    MCPCANBus = new MCP_CAN(arduinoPins.CS);
    
    int status = MCPCANBus->begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ);

    if (status == CAN_OK)
    {
      initialized = true;
      Serial.println(F("CAN BUS initialized."));
      pinMode(arduinoPins.CANBusPin, INPUT);
      MCPCANBus->setMode(MCP_NORMAL);
    }
    else
    {
      if (firstInit)  //Keeps it from flooding the Serial Monitor with failed connections.
      {
        Serial.println("Canbus Initialization failure.  Status code of : " + String(status));
        firstInit = false;
      }

      //Delete the object so it doens't create a memory leak on repeat connection tries.
      delete MCPCANBus;
    }
  }

  private:void CheckForCANMessage()
  {
    if (!initialized)
    {
      return;
    }

    if (!digitalRead(arduinoPins.CANBusPin))
    {    
      unsigned long int messageID;
      unsigned char messageLength;
      unsigned char buffer[8];

      MCPCANBus->readMsgBuf(&messageID, &messageLength, buffer);

      //Filter out last 11 bits (Sniper ECU's serial number)
      messageID = messageID & 0xFFFFF800;

      union CANData messageValue;
      //union CANData messageStatus;

      messageValue.payloadArray[3] = buffer[0];
      messageValue.payloadArray[2] = buffer[1];
      messageValue.payloadArray[1] = buffer[2];
      messageValue.payloadArray[0] = buffer[3];

      float value = (float)(messageValue.value);

      /*
        //This isn't needed for acquiring values as it's just a status message.

        messageStatus.payloadArray[0] = CANBuffer[3];
        messageStatus.payloadArray[1] = CANBuffer[2];
        messageStatus.payloadArray[2] = CANBuffer[1];
        messageStatus.payloadArray[3] = CANBuffer[0];

        float CANStatus = (unsigned long)messageStatus.value;
      */

      ValueTypeFloat* newValue;
      bool update = true;

      switch (messageID)
      {
        case 0x9E005000: newValue = &values.RPM; break;
        case 0x9E009000: newValue = &values.injectorPulseWidth; break;
        case 0x9E00D000: newValue = &values.dutyCycle; break;
        case 0x9E011000: newValue = &values.closedLoopCompensation; break;
        case 0x9E015000: newValue = &values.targetAFR; break;
        case 0x9E019000: newValue = &values.airFuelRatio; break;
        case 0x9E039000: newValue = &values.fuelFlowlbsPerHour; break;
        case 0x9E049000: newValue = &values.ignitionTiming; break;
        case 0x9E059000: newValue = &values.IACPosition; break;
        case 0x9E05D000: newValue = &values.MAP; break;
        case 0x9E061000: newValue = &values.TPS; break;
        case 0x9E065000: newValue = &values.MAT; break;
        case 0x9E069000: newValue = &values.CTS; break;
        case 0x9E06D000: newValue = &values.battery; break;
        default: 
          update = false;
          break;
      }

      if (update == true)
      {
        newValue->value = value;
        newValue->millis = millis();
      }

      messageCheckTimer = millis();
    }
  }

  private:void CheckForCANTimeout()
  {
    if (millis() - messageCheckTimer > 500)
    {
      Serial.println(F("Canbus timeout."));
      messageCheckTimer = millis();
    }
  }
};

CANBus canbus;

class Display
{
  public: void Update()
  {
    Serial.print(F("RPM: "));
    Serial.print(values.RPM.value);
    Serial.print(F("  MAP: "));
    Serial.print(values.MAP.value);
    Serial.print(F("  fuelFlow: "));
    Serial.print(values.fuelFlowlbsPerHour.value);
    Serial.print(F("  TPS: "));
    Serial.print(values.TPS.value);
    Serial.print(F("  CTS: "));
    Serial.print(values.CTS.value);
    Serial.print(F("  battery: "));
    Serial.print(values.battery.value);
    Serial.println("");
  }
};

Display display;

void setup()
{
  Serial.begin(115200);  
}

void loop()
{
  canbus.Update();
  display.Update();
}


//#define DEBUG

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <cstring>

bool verboseMode;
std::string outputFileName;


std::vector <unsigned char>generatedBinary;

enum symbols
{
    NOP = 0x00,
    MOV = 0x01,
    LODSB = 0x02,
    LOD = 0x03,
    STB = 0x04,
    BNE = 0x05,
    BEQ = 0x06,
    BGR = 0x07,
    PUSH = 0x0F,
    PUSHA = 0x10,
    POPA = 0x11,
    POP = 0x1F,
    STOSB = 0x32,
    SPDR = 0x33,
    LDM = 0x47,
    INT = 0x08,
    IRET = 0x78,
    SIRET = 0x0E,
    RIRET = 0x1E,
    CLI = 0x3E,
    STI = 0x7E,
    SPIR = 0x70,
    RST = 0x71,
    SBP = 0x72,
    XCHG = 0x73,

    HALT = 0x18,
    CALL = 0x28,
    RTS = 0x38,
    BRA = 0x48,
    NSB = 0x40,
    BRZ = 0x60,

    DBH = 0xFE,
    DBC = 0xFF,

    //ALU
    OR = 0x80,
    AND = 0x81,
    SHL = 0x82,
    SHR = 0x83,
    CMP = 0x84,
    NOT = 0x85,
    XOR = 0x86,
    ADD = 0x87,
    SUB = 0x88,
    INC = 0x89,
    DEC = 0x8A,
    ROL = 0x8B,
    ROR = 0x8C
};

std::string symbolTable[] =
{
    //general commands
    "NOP",
    "MOV",
    "LODSB",
    "LOD",
    "STB",
    "BNE",
    "BEQ",
    "BGR",

    "PUSH",
    "PUSHA",
    "POPA",
    "POP",
    "LDM",
    "INT",
    "IRET",
    "SIRET",
    "RIRET",
    "CLI",
    "STI",
    "SPIR",
    "RST",

    "HALT",
    "CALL",
    "RTS",
    "BRA",
    "NSB",
    "BRZ",
    "SBP",
    "XCHG",
    "STOSB",
    "SPDR",

    //ALU
    "OR",
    "AND",
    "SHL",
    "SHR",
    "CMP",
    "NOT",
    "XOR",
    "ADD",
    "SUB",
    "INC",
    "DEC",
    "ROL",
    "ROR",

    ":", //function terminator
    "ORG",
    "DBH",
    "DBC"
};

int convertBinStrToInt(std::string bin)
{
    int total = 0;
    int multiplier = 1;

    for(int i = bin.size() - 1; i >= 0; --i)
    {
        if(bin.at(i) == '1')
            total += multiplier;

        multiplier *= 2;
    }

    return total;
}

bool checkValidHex(std::string hex)
{
    for(int i = 0; i < hex.size(); ++i)
    {
        if(hex.at(i) < '0' || hex.at(i) > '9' && hex.at(i) != 'A' && hex.at(i) != 'B' &&
        hex.at(i) != 'C' && hex.at(i) != 'D' && hex.at(i) != 'E' && hex.at(i) != 'F')
            return false;
    }
    return true;
}

bool checkValidBin(std::string bin)
{
    for(int i = 0; i < bin.size(); ++i)
    {
        if(bin.at(i) != '0' && bin.at(i) != '1')
            return false;
    }
    return true;
}

int convertHexStrToInt(std::string hex)
{
    if(hex.size() == 1)
        hex = "0" + hex;

   	int total = 0;
    int multiplier = 16;
    
    for(int i = 0; i < hex.size() - 2; ++i)
        multiplier *= 16;

	for(int i = 0; i < hex.size(); ++i)
	{
		switch(hex.at(i))
		{
			case '0':
                total += multiplier * 0;
                break;

            case '1':
                total += multiplier * 1;
                break;
                
            case '2':
                total += multiplier * 2;
                break;
                
            case '3':
                total += multiplier * 3;
                break;
                
            case '4':
                total += multiplier * 4;
                break;
                
            case '5':
                total += multiplier * 5;
                break;
                
            case '6':
                total += multiplier * 6;
                break;
                
            case '7':
                total += multiplier * 7;
                break;
                
            case '8':
                total += multiplier * 8;
                break;
                
            case '9':
                total += multiplier * 9;
                break;
                
            case 'A':
                total += multiplier * 10;
                break;
                
            case 'B':
                total += multiplier * 11;
                break;
                
            case 'C':
                total += multiplier * 12;
                break;
                
            case 'D':
                total += multiplier * 13;
                break;
                
            case 'E':
                total += multiplier * 14;
                break;
                
            case 'F':
                total += multiplier * 15;
                break;
		}
        
        multiplier /= 16;
	}
    return total; 
}

std::vector <std::pair<int, std::string> > commands;
std::vector <std::string> commands1;
void generateSymbols(std::string source)
{
    int lineNum = 1;
    std::string tmp;
    for(int i = 0; i < source.size(); ++i)
    {
        tmp += source.at(i);
        if(source.at(i) == '\n') //start of new line, check for command
        {        
            //push command to list
            std::string buff;
            bool once = true; //only look once per line for a command (because there is only one command in a line)
            for(int j = 0; j < tmp.size(); ++j)
            {
                bool supportingInstruction = false;
                if(once)
                {
                    if(tmp.at(j) == ' ' || tmp.at(j) == '\n')
                    {
                        if(buff.size() >= 1) //skip empty lines
                        {
                            std::pair <int, std::string>tmp;
                            tmp.first = lineNum; //store line number for command
                            tmp.second = buff; //store command string
                            commands.push_back(tmp); //and save the pair in order
                            
                            once = false;
                            supportingInstruction = true;
                        }
                    }

                    else
                    {
                        if(tmp.at(j) != '\t')
                            buff += tmp.at(j);
                    }

                    if(supportingInstruction) //get additional instruction command (registers, addresses, etc)
                    {                       
                        commands1.push_back(tmp.substr(buff.size(), tmp.size()));
                        buff = "";
                    }
                }
            }

            tmp = "";
            ++lineNum;
        }
    }

    int lineNumber = 0;
    int relativeByteCount = 0; //stores byte position for labels
    int startAddress = 0;

    std::vector <std::pair <int, std::string> > equates;
    std::vector <std::pair <int, std::string> > labels;
    std::vector <std::pair <int, std::string> > instructions;
    std::vector <int> instructionLineNum;
    for(int command = 0; command < commands.size(); ++command)
    {
        ++lineNumber;
        std::pair <int, std::string> tmp;
        unsigned char opType = 0;
        bool opFound = false;

        for(int sym = 0; sym < (sizeof(symbolTable) / sizeof(symbolTable[0])); ++sym)
        {
            //finds a matched string command, sets opType which is the base byte for the command
            //before any selection bits have been set (register selection etc), those will be or'd in later            
            if(strcmp(symbolTable[sym].c_str(), commands.at(command).second.c_str()) == 0)
            {
                switch(sym) //*NOTE* sym does not actually match to instruction numbers, is just index in array of ops
                {
                    case 0: //NOP
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = NOP;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    case 1: //MOV
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = MOV;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    case 2: //LODSB
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = LODSB;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    /*TODO need to check for syntax errors, if missing register to load into e.g.*/
                    case 3: //LOD
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = LOD;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;    

                    case 4: //STB
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = STB;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;    

                    case 5: //BNE
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = BNE;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;    

                    case 6: //BEQ
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = BEQ;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 7: //BGR
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = BGR;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;    

                   case 8: //PUSH
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = PUSH;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                   case 9: //PUSHA
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = PUSHA;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                   case 10: //POPA
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = POPA;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 11: //POP
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = POP;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;    

                    case 12: //LDM
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = LDM;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;     

                    case 13: //INT
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = INT;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;    

                    case 14: //IRET
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = IRET;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 15: //SIRET
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = SIRET;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 16: //RIRET
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = RIRET;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;    

                    case 17: //CLI
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = CLI;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;      

                    case 18: //STI
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = STI;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;  

                    case 19: //SPIR
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = SPIR;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    case 20: //RST
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = RST;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    case 21: //HALT
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = HALT;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 22: //CALL
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = CALL;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 23: //RTS
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = RTS;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 24: //BRA
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = BRA;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 25: //NSB
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = NSB;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 26: //BRZ
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = BRZ;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 27: //SBP
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = SBP;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    case 28: //XCHG
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = XCHG;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    case 29: //STOSB
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = STOSB;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    case 30: //SPDR
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 4;
                        opType = SPDR;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    ///////
                    //ALU//
                    ///////
                    case 31: //OR
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = OR;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;   

                    case 32: //AND
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = AND;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break; 

                    case 33: //SHL
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = SHL;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break; 

                    case 34: //SHR
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = SHR;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break; 

                    case 35: //CMP
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = CMP;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break; 

                    case 36: //NOT
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = NOT;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    case 37: //XOR
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = XOR;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break; 

                    case 38: //ADD
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = ADD;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break; 

                    case 39: //SUB
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 2;
                        opType = SUB;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break; 

                    case 40: //INC
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = INC;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break; 

                    case 41: //DEC
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = DEC;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    case 42: //ROL
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = ROL;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;

                    case 43: //ROR
                        instructionLineNum.push_back(commands.at(command).first);
                        relativeByteCount += 1;
                        opType = ROR;
                        opFound = true;

                        tmp.first = opType;
                        tmp.second = commands1.at(command);
                        instructions.push_back(tmp);
                        break;
                }

                #ifdef DEBUG
                    std::cout << commands.at(command).second << commands1.at(command) << '\t' << 
                    std::hex << "0x" << relativeByteCount << '\t' << std::dec << relativeByteCount << std::endl;
                #endif
            }

            else //failed to find command
            {
                //check if it is actually a label (or function)
                if(commands.at(command).second.at(commands.at(command).second.size() - 1) == ':')
                {
                    std::pair <int, std::string> label;
                    //label.first = relativeByteCount + 1;
                    label.first = relativeByteCount;
                    label.second = commands.at(command).second.substr(0, commands.at(command).second.size() - 1);

                    //check if already exists
                    for(int l = 0; l < labels.size(); ++l)
                    {
                        if(strcmp(labels.at(l).second.c_str(), label.second.c_str()) == 0)
                        {
                            std::cout << commands.at(command).first << ':'; 
                            std::cout << "Error: Label " << label.second << " already defined" << std::endl;
                            return;  
                        }
                    }

                    if(label.second.at(0) == '.')
                    {
                        if(labels.size() > 0)
                        {
                            std::string nestedLabel;
                            nestedLabel = labels.at(labels.size() - 1).second + label.second;
                            labels.push_back(std::pair<int, std::string>(label.first, nestedLabel));
                        }

                        else
                        {
                            labels.push_back(label);
                        }
                    }

                    else
                        labels.push_back(label);

                    opFound = true;
                    break;
                }

                //is it an equate?
                else if(commands.at(command).second.at(0) == 'E')
                {
                    if(commands.at(command).second.at(1) == 'Q')
                    {
                        if(commands.at(command).second.at(2) == 'U')
                        {
                            std::string equLbl;
                            int pos = 0;
                            for(int i = 1; i < commands1.at(command).size(); ++i)
                            {
                                if(commands1.at(command).at(i) == ' ')
                                {
                                    pos = i;
                                    break;
                                }

                                else
                                    equLbl += commands1.at(command).at(i);
                            }

                            std::string equVal;
                            for(int i = pos; i < commands1.at(command).size(); ++i)
                            {
                                if(commands1.at(command).at(i) != '\n')
                                    equVal += commands1.at(command).at(i);
                            }

                            std::pair <int, std::string> equate;
                            std::string equValBuff;
                            for(int k = 0; k < equVal.size(); ++k)
                            {
                                if(equVal.at(k) != ' ')
                                    equValBuff += equVal.at(k);
                            }

                            if(equValBuff.at(0) == '#')
                            {
                                if(equValBuff.at(1) == '$')
                                {
                                    if(equValBuff.size() > 8) //error equate address too long
                                    {
                                        std::cout << lineNumber << ':'; 
                                        std::cout << "Error: EQU address out of range" << std::endl;
                                        return;
                                    }

                                    //check for invalid value
                                    if(checkValidHex(equValBuff.substr(2, equValBuff.size() - 1)) == false)
                                    {
                                        std::cout << lineNumber << ':'; 
                                        std::cout << "Error: EQU syntax error" << std::endl;
                                        return;
                                    }

                                    equate.first = convertHexStrToInt(equValBuff.substr(1, equValBuff.size() - 1));
                                    equate.second = equLbl;
                                    equates.push_back(equate);

                                    opFound = true;
                                }

                                else if(equValBuff.at(1) == '%')
                                {
                                    if(equValBuff.size() > 18) //error equate address too long
                                    {
                                        std::cout << lineNumber << ':'; 
                                        std::cout << "Error: EQU address out of range" << std::endl;
                                        return;
                                    }

                                    //check for invalid value
                                    if(checkValidBin(equValBuff.substr(2, equValBuff.size() - 1)) == false)
                                    {
                                        std::cout << lineNumber << ':'; 
                                        std::cout << "Error: EQU syntax error" << std::endl;
                                        return;
                                    }

                                    equate.first = convertBinStrToInt(equValBuff.substr(1, equValBuff.size() - 1));
                                    equate.second = equLbl;
                                    equates.push_back(equate);

                                    opFound = true;
                                }

                                else
                                {
                                    equate.first = atoi(equValBuff.c_str());
                                    equate.second = equLbl;
                                    equates.push_back(equate);

                                    opFound = true;
                                }
                            }

                            else
                            {
                                std::cout << lineNumber << ':'; 
                                std::cout << "Error: EQU syntax error" << std::endl;
                            }

                            break;
                        }
                    }
                }
            }
        }

        if(opFound == false)
        {
            std::string buff;
            for(int k = 0; k < commands.at(command).second.size(); ++k)
            {
                if(commands.at(command).second.at(k) == ' ')
                    break;

                else
                    buff += commands.at(command).second.at(k);
            }

            if(strcmp(buff.c_str(), "ORG") == 0) //set new orign
            {
                if(commands1.at(command).at(1) == '$') //hex
                {
                    buff = "";
                    for(int k = 0; k < commands1.at(command).size(); ++k)
                    {
                        if(commands1.at(command).at(k) >= '0' && commands1.at(command).at(k) <= '9' || 
                        commands1.at(command).at(k) >= 'A' && commands1.at(command).at(k) <= 'F')
                            buff += commands1.at(command).at(k);
                    }
                    relativeByteCount = convertHexStrToInt(buff);
                    startAddress = relativeByteCount;
                }

                else if(commands1.at(command).at(1) == '%') //bin
                {
                    buff = "";
                    for(int k = 0; k < commands1.at(command).size(); ++k)
                    {
                        if(commands1.at(command).at(k) >= '0' && commands1.at(command).at(k) <= '9' || 
                        commands1.at(command).at(k) >= 'A' && commands1.at(command).at(k) <= 'F')
                            buff += commands1.at(command).at(k);
                    }
                    relativeByteCount = convertBinStrToInt(buff);
                    startAddress = relativeByteCount;
                }

                else
                {
                    buff = "";
                    for(int k = 0; k < commands1.at(command).size(); ++k)
                    {
                        if(commands1.at(command).at(k) >= '0' && commands1.at(command).at(k) <= '9')
                            buff += commands1.at(command).at(k);
                    }
                    relativeByteCount = atoi(buff.c_str());
                }
            }

            else if(strcmp(buff.c_str(), "DBH") == 0) //define byte(s) hex
            {
                int bytes = 0;
                for(int i = 0; i < commands1.at(command).size(); ++i)
                {
                    if(commands1.at(command).at(i) == ',')
                        ++bytes;
                }
                ++bytes;

                relativeByteCount += bytes;
                opType = DBH;
                opFound = true;

                tmp.first = opType;
                tmp.second = commands1.at(command);
                instructions.push_back(tmp);
            }

            else if(strcmp(buff.c_str(), "DBC") == 0) //define byte(s) char
            {
                int bytes = 0;                
                bytes = commands1.at(command).size();

                int strStart = 0;
                int strEnd = 0;
                for(int i = 0; i < commands1.at(command).size(); ++i)
                    if(commands1.at(command).at(i) == '"')
                    {
                        strStart = i;
                        break;
                    }

                for(int i = strStart + 1; commands1.at(command).size(); ++i)
                {
                    //find surrounding quote
                    if(commands1.at(command).at(i) == '"')
                    {
                        strEnd = i;
                        break;
                    }

                    //error, no quote found
                    else if(commands1.at(command).at(i) == '\n' || commands1.at(command).at(i) == '\0')
                    {
                        std::cout << "Syntax error on line: " << commands.at(command).first << std::endl;
                        return;
                    }
                }

                tmp.second = commands1.at(command).substr(strStart + 1, strEnd - 2);
                relativeByteCount += commands1.at(command).substr(strStart + 1, strEnd - 2).size();

                //relativeByteCount += bytes;
                opType = DBC;
                opFound = true;

                tmp.first = opType;
                tmp.second = commands1.at(command);
                instructions.push_back(tmp);
            }

            else
            {
                std::cout << "Syntax error on line: " << commands.at(command).first << std::endl;
                return;
            }
        }
    }

    int byteCount = 0; //needed to get location in memory for an instruction (JSP, CALL etc)
    std::vector <unsigned char> assembledBytes;
    for(int i = 0; i < instructions.size(); ++i)
    {
        std::string registerStr;
        std::string dataStr;
        unsigned int dataValues;

        std::string buff;
        std::string buff1;
        std::string buff2;
        std::vector <std::string>buffVec;

        int intNum = 0;
        unsigned int address;
        unsigned char byte;
        int pos = 0;
        bool switchArg = false;
        bool foundLabel = false;
        bool inQuote = false;
        bool memOffset = false;
        std::cout << std::dec; //make sure output is in decimal format again
        switch(instructions.at(i).first)
        {
            case NOP:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x0);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: NOP syntax error" << std::endl;
                    return;
                }
            break;

            case MOV:
                try
                {
                    std::cout << std::endl;
                    byte = 0x0;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(0x01);

                    //get register  
                    while(buff.at(pos++) == ' ');  
                    for(int j = pos - 1; j < instructions.at(i).second.size(); ++j)
                    {
                        if(buff.at(j) == ',')
                            switchArg = true;
                        
                        if(switchArg)
                            registerStr += buff.at(j);

                        else
                            buff1 += buff.at(j);
                    }
                    registerStr = registerStr.substr(1, 2);

                    //second register (from)
                    if(strcmp(registerStr.c_str(), "R1") == 0)
                        byte |= 0x0;

                    else if(strcmp(registerStr.c_str(), "R2") == 0)
                        byte |= 0x01;

                    else if(strcmp(registerStr.c_str(), "R3") == 0)
                        byte |= 0x02;

                    else if(strcmp(registerStr.c_str(), "R4") == 0)
                        byte |= 0x03;

                    else //error
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: MOV 1st register argument can only accept registers R1-R4" << std::endl;
                        return;
                    }

                    //first register (to)
                    if(strcmp(buff1.c_str(), "R1") == 0)
                        assembledBytes.push_back(byte | 0x0);

                    else if(strcmp(buff1.c_str(), "R2") == 0)
                        assembledBytes.push_back(byte | 0x04);

                    else if(strcmp(buff1.c_str(), "R3") == 0)
                        assembledBytes.push_back(byte | 0x08);

                    else if(strcmp(buff1.c_str(), "R4") == 0)
                        assembledBytes.push_back(byte | 0x0C);

                    else //error
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: MOV 2nd register argument can only accept registers R1-R4" << std::endl;
                        return;
                    }
                    byteCount += 2;
                }

            catch(std::exception& e)
            {
                std::cout << std::endl;
                std::cout << instructionLineNum.at(i) << ':';
                std::cout << "Error: MOV syntax error" << std::endl;
                return;
            }
            break;

            case LODSB:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x02);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: LODSB syntax error" << std::endl;
                    return;
                }
            break;            

            case LOD:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(0x03);
                    
                    while(buff.at(pos++) == ' ');  
                    for(int j = pos - 1; j < buff.size(); ++j)
                        buff1 += buff.at(j);
            
                    dataStr = buff1;
                    buff1 = "";
                    if(dataStr.at(0) == '#')
                    {
                        if(dataStr.at(1) == '$') //hex value
                        {
                            for(int k = 1; k < dataStr.size(); ++k)
                            {
                                if(dataStr.at(k) >= '0' && dataStr.at(k) <= '9' || dataStr.at(k) >= 'A' && dataStr.at(k) <= 'F')
                                    buff1 += dataStr.at(k);
                            }
                            dataValues = convertHexStrToInt(buff1);
                            assembledBytes.push_back(dataValues & 0xFF);
                        }

                        else if(dataStr.at(1) == '%') //bin value
                        {
                            for(int k = 1; k < dataStr.size(); ++k)
                            {
                                if(dataStr.at(k) == '0' || dataStr.at(k) == '1')
                                    buff1 += dataStr.at(k);
                            }
                            dataValues = convertBinStrToInt(buff1);
                            assembledBytes.push_back(dataValues & 0xFF);
                        }

                        else //non hex
                        {
                            buff1 = "";
                            for(int i = 1; i < dataStr.size(); ++i)
                                if(dataStr.at(i) >= '0' && dataStr.at(i) <= '9')
                                    buff1 += dataStr.at(i);
                            if(buff1.size() < 1)
                            {
                                std::cout << std::endl;
                                std::cout << instructionLineNum.at(i) << ':';
                                std::cout << "Error: LOD missing value" << std::endl;
                                return;
                            }
                            assembledBytes.push_back(atoi(buff1.c_str()));
                        }
                    }

                    else if(dataStr.at(0) == '\'') //char value
                    {
                        if(dataStr.at(2) == '\'') //matching '
                        {
                            if(dataStr.at(1) >= 0x20 && dataStr.at(1) <= 0x7E)
                                assembledBytes.push_back(dataStr.at(1));
                            
                            else
                            {
                                std::cout << std::endl;
                                std::cout << instructionLineNum.at(i) << ':';
                                std::cout << "Error: LOD improper ascii value" << std::endl;
                                return;          
                            }
                        }

                        else //error
                        {
                            std::cout << std::endl;
                            std::cout << instructionLineNum.at(i) << ':';
                            std::cout << "Error: LOD improper ascii value" << std::endl;
                            return;
                        }
                    }

                    else
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: LOD syntax error" << std::endl;
                        return;
                    }

                    byteCount += 2;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: LOD syntax error" << std::endl;
                    return;
                }
            break;

            case STB:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;

                    assembledBytes.push_back(0x04);

                    //check if immediate
                    for(int k = 0; k < buff.size(); ++k)
                    {
                        if(buff.at(k) == '#')
                        {
                            pos = k;
                            break;
                        }
                    }

                    //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                    for(int i = 0; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != '\n')
                            buff1 += buff.at(i);
                    }
                    buff = buff1;

                    if(buff.at(pos) == '#') //immediate
                    {
                        foundLabel = true;
                        if(buff.at(pos + 1) == '$') //hex
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertHexStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }
                        
                        else if(buff.at(pos + 1) == '%') //bin
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertBinStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else
                        {
                            buff1 = buff.substr(pos + 1, buff.size());
                            address = atoi(buff1.c_str());

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }
                    }

                    else
                    {
                        //removes any extra spaces in label, e.g. before a comment semicolon
                        for(int c = 0; c < buff.size(); ++c)
                        {
                            if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9')
                            {
                                pos = c;
                                break;
                            }
                        }

                        for(int i = pos; i < buff.size(); ++i)
                        {
                            if((buff.at(i) >= 'A' && buff.at(i) <= 'Z') || (buff.at(i) >= '0' && buff.at(i) <= '9'))
                                buff2 += buff.at(i);
                        }
                        
                        foundLabel = false;
                        for(int eq = 0; eq < equates.size(); ++eq)
                        {
                            if(strcmp(buff2.c_str(), equates.at(eq).second.c_str()) == 0)
                            {
                                address = equates.at(eq).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }
                    }

                    if(foundLabel == false)
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: STB '" << buff << "' Memory label not found." << std::endl;
                        return; 
                    }

                    byteCount += 4;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: STB syntax error" << std::endl;
                    return;
                }
            break;

            case BNE:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(0x05);

                    //check if immediate
                    for(int k = 0; k < buff.size(); ++k)
                    {
                        if(buff.at(k) == '#')
                        {
                            pos = k;
                            break;
                        }
                    }

                    //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                    for(int i = 0; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != '\n')
                            buff1 += buff.at(i);
                    }
                    buff = buff1;

                    if(buff.at(pos) == '#') //immediate
                    {
                        foundLabel = true;
                        if(buff.at(pos + 1) == '$') //hex
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertHexStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else if(buff.at(pos + 1) == '%') //bin
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertBinStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else
                        {
                            buff1 = buff.substr(pos + 1, buff.size());
                            address = atoi(buff1.c_str());

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }
                    }

                    else
                    {
                        //removes any extra spaces in label, e.g. before a comment semicolon
                        for(int c = 0; c < buff.size(); ++c)
                        {
                            if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9' || buff.at(c) == '.')
                            {
                                pos = c;
                                break;
                            }
                        }

                        for(int i = pos; i < buff.size(); ++i)
                        {
                            if((buff.at(i) >= 'A' && buff.at(i) <= 'Z') || (buff.at(i) >= '0' && buff.at(i) <= '9') || buff.at(i) == '.')
                                buff2 += buff.at(i);
                        }
                        
                        foundLabel = false;
                        for(int lbl = 0; lbl < labels.size(); ++lbl)
                        {
                            if(strcmp(buff2.c_str(), labels.at(lbl).second.c_str()) == 0)
                            {
                                address = labels.at(lbl).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }
                    }

                    if(foundLabel == false)
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: BNE '" << buff << "' Memory label not found." << std::endl;
                        return; 
                    }

                    byteCount += 4;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: BNE syntax error" << std::endl;
                    return;
                }
            break;

            case BEQ:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(0x06);

                    //check if immediate
                    for(int k = 0; k < buff.size(); ++k)
                    {
                        if(buff.at(k) == '#')
                        {
                            pos = k;
                            break;
                        }
                    }

                    //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                    for(int i = 0; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != '\n')
                            buff1 += buff.at(i);
                    }
                    buff = buff1;

                    if(buff.at(pos) == '#') //immediate
                    {
                        foundLabel = true;
                        if(buff.at(pos + 1) == '$') //hex
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertHexStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else if(buff.at(pos + 1) == '%') //bin
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertBinStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else
                        {
                            buff1 = buff.substr(pos + 1, buff.size());
                            address = atoi(buff1.c_str());

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }
                    }

                    else
                    {
                        //removes any extra spaces in label, e.g. before a comment semicolon
                        for(int c = 0; c < buff.size(); ++c)
                        {
                            if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9' || buff.at(c) == '.')
                            {
                                pos = c;
                                break;
                            }
                        }

                        for(int i = pos; i < buff.size(); ++i)
                        {
                            if((buff.at(i) >= 'A' && buff.at(i) <= 'Z') || (buff.at(i) >= '0' && buff.at(i) <= '9') || buff.at(i) == '.')
                                buff2 += buff.at(i);
                        }
                        
                        foundLabel = false;
                        for(int lbl = 0; lbl < labels.size(); ++lbl)
                        {
                            if(strcmp(buff2.c_str(), labels.at(lbl).second.c_str()) == 0)
                            {
                                address = labels.at(lbl).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }
                    }

                    if(foundLabel == false)
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: BEQ '" << buff << "' Memory label not found." << std::endl;
                        return; 
                    }
                    
                    byteCount += 4;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: BEQ syntax error" << std::endl;
                    return;
                }
            break;

            case BGR:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(0x07);

                    //check if immediate
                    for(int k = 0; k < buff.size(); ++k)
                    {
                        if(buff.at(k) == '#')
                        {
                            pos = k;
                            break;
                        }
                    }

                    //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                    for(int i = 0; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != '\n')
                            buff1 += buff.at(i);
                    }
                    buff = buff1;

                    if(buff.at(pos) == '#') //immediate
                    {
                        foundLabel = true;
                        if(buff.at(pos + 1) == '$') //hex
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertHexStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else if(buff.at(pos + 1) == '%') //bin
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertBinStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else
                        {
                            buff1 = buff.substr(pos + 1, buff.size());
                            address = atoi(buff1.c_str());

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }
                    }

                    else
                    {
                        //removes any extra spaces in label, e.g. before a comment semicolon
                        for(int c = 0; c < buff.size(); ++c)
                        {
                            if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9' || buff.at(c) == '.')
                            {
                                pos = c;
                                break;
                            }
                        }

                        for(int i = pos; i < buff.size(); ++i)
                        {
                            if((buff.at(i) >= 'A' && buff.at(i) <= 'Z') || (buff.at(i) >= '0' && buff.at(i) <= '9') || buff.at(i) == '.')
                                buff2 += buff.at(i);
                        }
                        
                        foundLabel = false;
                        for(int lbl = 0; lbl < labels.size(); ++lbl)
                        {
                            if(strcmp(buff2.c_str(), labels.at(lbl).second.c_str()) == 0)
                            {
                                address = labels.at(lbl).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }
                    }

                    if(foundLabel == false)
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: BGR '" << buff << "' Memory label not found." << std::endl;
                        return; 
                    }
                    
                    byteCount += 4;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: BGR syntax error" << std::endl;
                    return;
                }
            break;

            case INT:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(0x08);
                    
                    if(buff.at(1) == '#')
                    {
                        if(buff.at(2) == '$') //hex
                        {
                            buff1 = "";
                            for(int i = 2; i < buff.size(); ++i)
                                if(buff.at(i) != '\n' && buff.at(i) != '$')
                                    buff1 += buff.at(i);

                            if(buff1.size() > 2) //error
                            {
                                std::cout << std::endl;
                                std::cout << instructionLineNum.at(i) << ':';
                                std::cout << "Error: Syntax " << buff << "\nOnly accepts 2 digit hex numbers e.g. '$FF'" << std::endl;
                                return; 
                            }

                            else
                                assembledBytes.push_back(convertHexStrToInt(buff1));
                        }

                        else if(buff.at(2) == '%') //bin
                        {
                            buff1 = "";
                            for(int i = 2; i < buff.size(); ++i)
                                if(buff.at(i) != '\n' && buff.at(i) != '%')
                                    buff1 += buff.at(i);

                            if(buff1.size() > 8) //error
                            {
                                std::cout << std::endl;
                                std::cout << instructionLineNum.at(i) << ':';
                                std::cout << "Error: Syntax " << buff << "\nOnly accepts 8 digit bin numbers e.g. '$11111111'" << std::endl;
                                return; 
                            }

                            else
                                assembledBytes.push_back(convertBinStrToInt(buff1));
                        }

                        else
                        {
                            buff1 = "";
                            for(int i = 2; i < buff.size(); ++i)
                                if(buff.at(i) != '\n')
                                    buff1 += buff.at(i);

                            intNum = atoi(buff1.c_str());

                            if(intNum > 255)
                            {
                                std::cout << std::endl;
                                std::cout << instructionLineNum.at(i) << ':';
                                std::cout << "Error: Syntax " << buff << "\nOnly accepts dec numbers <= 255" << std::endl;
                                return;
                            }

                            else
                                assembledBytes.push_back(intNum);
                        }
                    }

                    else
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: INT syntax error" << std::endl;
                        return;
                    }

                    byteCount += 2;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: INT syntax error" << std::endl;
                    return;
                }
            break;

            case PUSH:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x0F);

                //get register  
                while(buff.at(pos++) == ' ');  
                for(int j = pos - 1; j < instructions.at(i).second.size(); ++j)
                    registerStr += buff.at(j);

                registerStr = registerStr.substr(0, 2);
                
                //first register
                if(strcmp(registerStr.c_str(), "R1") == 0)
                    assembledBytes.push_back(0x00);

                else if(strcmp(registerStr.c_str(), "R2") == 0)
                    assembledBytes.push_back(0x01);

                else if(strcmp(registerStr.c_str(), "R3") == 0)
                    assembledBytes.push_back(0x02);

                else if(strcmp(registerStr.c_str(), "R4") == 0)
                    assembledBytes.push_back(0x03);

                else //error
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: PUSH register argument can only accept registers R1-R4" << std::endl;
                    return;
                }
                byteCount += 2;
            break;

            case PUSHA:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x10);

                for(int i = 0; i < buff.size(); ++i) //error
                {
                    if(buff.at(i) != ' ' && buff.at(i) != '\0' && buff.at(i) != 0x0A)
                    {
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: PUSHA does not accept arguments" << std::endl;
                        return;
                    }
                }
            break;

            case POPA:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x11);

                for(int i = 0; i < buff.size(); ++i) //error
                {
                    if(buff.at(i) != ' ' && buff.at(i) != '\0' && buff.at(i) != 0x0A)
                    {
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: POPA does not accept arguments" << std::endl;
                        return;
                    }
                }
            break;

            case POP:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x1F);

                //get register  
                while(buff.at(pos++) == ' ');  
                for(int j = pos - 1; j < instructions.at(i).second.size(); ++j)
                    registerStr += buff.at(j);

                registerStr = registerStr.substr(0, 2);
                
                //first register
                if(strcmp(registerStr.c_str(), "R1") == 0)
                    assembledBytes.push_back(0x00);

                else if(strcmp(registerStr.c_str(), "R2") == 0)
                    assembledBytes.push_back(0x01);

                else if(strcmp(registerStr.c_str(), "R3") == 0)
                    assembledBytes.push_back(0x02);

                else if(strcmp(registerStr.c_str(), "R4") == 0)
                    assembledBytes.push_back(0x03);

                else //error
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: POP register argument can only accept registers R1-R4" << std::endl;
                    return;
                }
                byteCount += 2;
            break;

            case LDM:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(byte);
                    memOffset = false;

                    //check if immediate
                    for(int k = 0; k < buff.size(); ++k)
                    {
                        if(buff.at(k) == '#')
                        {
                            pos = k;
                            break;
                        }
                    }

                    //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                    for(int i = 0; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != '\n')
                            buff1 += buff.at(i);
                    }
                    buff = buff1;

                    if(buff.at(pos) == '#') //immediate
                    {
                        foundLabel = true;
                        if(buff.at(pos + 1) == '$') //hex
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertHexStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else if(buff.at(pos + 1) == '%') //bin
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertBinStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else
                        {
                            buff1 = buff.substr(pos + 1, buff.size());
                            address = atoi(buff1.c_str());

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }
                    }

                    else
                    {
                        //removes any extra spaces in label, e.g. before a comment semicolon
                        for(int c = 0; c < buff.size(); ++c)
                        {
                            if(buff.at(c) == ',')
                            {
                                pos = c;
                                memOffset = true;
                                break;
                            }

                            else if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9')
                                buff2 += buff.at(c);
                        }

                        //get offset amount, if any
                        buff1 = "";
                        for(int i = pos; i < buff.size(); ++i)
                        {
                            if(buff.at(i) != ',')
                                buff1 += buff.at(i);
                        }
                        pos = atoi(buff1.c_str()); //offset in memory

                        foundLabel = false;
                        for(int eq = 0; eq < equates.size(); ++eq)
                        {
                            if(strcmp(buff2.c_str(), equates.at(eq).second.c_str()) == 0)
                            {
                                if(memOffset)
                                    address = equates.at(eq).first + pos;

                                else
                                    address = equates.at(eq).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }

                        if(foundLabel == false) //check if regular label and not an equate
                        {
                            for(int lbl = 0; lbl < labels.size(); ++lbl)
                            {
                                if(strcmp(buff2.c_str(), labels.at(lbl).second.c_str()) == 0)
                                {
                                    if(memOffset)
                                        address = labels.at(lbl).first + pos;

                                    else
                                        address = labels.at(lbl).first;

                                    unsigned char lowByte, midByte, highByte;
                                    lowByte = address & 0xFF;
                                    midByte = ((address >> 8) & 0xFF);
                                    highByte = ((address >> 16) & 0xFF);

                                    assembledBytes.push_back(lowByte);
                                    assembledBytes.push_back(midByte);
                                    assembledBytes.push_back(highByte);
                                    foundLabel = true;
                                    break;
                                }
                            }
                        }
                    }

                    if(foundLabel == false)
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: LDM no matching label found." << std::endl;
                        return;
                    }

                    byteCount += 4;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: LDM syntax error" << std::endl;
                    return;
                }
            break;

            case IRET:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x78);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: IRET syntax error" << std::endl;
                    return;
                }
            break;

            case SIRET:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x0E);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: IRET syntax error" << std::endl;
                    return;
                }

            break;

            case RIRET:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x1E);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: IRET syntax error" << std::endl;
                    return;
                }
            break;

            case CLI:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x3E);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: IRET syntax error" << std::endl;
                    return;
                }
            break;

            case STI:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x7E);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: IRET syntax error" << std::endl;
                    return;
                }
            break;

            case SPIR:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x70);

                //check if immediate
                for(int k = 0; k < buff.size(); ++k)
                {
                    if(buff.at(k) == '#')
                    {
                        pos = k;
                        break;
                    }
                }

                //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                for(int i = 0; i < buff.size(); ++i)
                {
                    if(buff.at(i) != '\n')
                        buff1 += buff.at(i);
                }
                buff = buff1;

                if(buff.at(pos) == '#') //immediate
                {
                    foundLabel = true;
                    if(buff.at(pos + 1) == '$') //hex
                    {
                        buff1 = buff.substr(pos + 2, buff.size());                    
                        address = convertHexStrToInt(buff1);

                        unsigned char lowByte, midByte, highByte;
                        lowByte = address & 0xFF;
                        midByte = ((address >> 8) & 0xFF);
                        highByte = ((address >> 16) & 0xFF);

                        assembledBytes.push_back(lowByte);
                        assembledBytes.push_back(midByte);
                        assembledBytes.push_back(highByte);
                    }

                    else if(buff.at(pos + 1) == '%') //bin
                    {
                        buff1 = buff.substr(pos + 2, buff.size());                    
                        address = convertBinStrToInt(buff1);

                        unsigned char lowByte, midByte, highByte;
                        lowByte = address & 0xFF;
                        midByte = ((address >> 8) & 0xFF);
                        highByte = ((address >> 16) & 0xFF);

                        assembledBytes.push_back(lowByte);
                        assembledBytes.push_back(midByte);
                        assembledBytes.push_back(highByte);
                    }

                    else
                    {
                        buff1 = buff.substr(pos + 1, buff.size());
                        address = atoi(buff1.c_str());

                        unsigned char lowByte, midByte, highByte;
                        lowByte = address & 0xFF;
                        midByte = ((address >> 8) & 0xFF);
                        highByte = ((address >> 16) & 0xFF);

                        assembledBytes.push_back(lowByte);
                        assembledBytes.push_back(midByte);
                        assembledBytes.push_back(highByte);
                    }
                }

                else
                {
                    //removes any extra spaces in label, e.g. before a comment semicolon
                    for(int c = 0; c < buff.size(); ++c)
                    {
                        if(buff.at(c) == ',')
                        {
                            pos = c;
                            memOffset = true;
                            break;
                        }

                        else if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9')
                            buff2 += buff.at(c);
                    }

                    //get offset amount, if any
                    buff1 = "";
                    for(int i = pos; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != ',')
                            buff1 += buff.at(i);
                    }
                    pos = atoi(buff1.c_str()); //offset in memory

                    foundLabel = false;
                    for(int eq = 0; eq < equates.size(); ++eq)
                    {
                        if(strcmp(buff2.c_str(), equates.at(eq).second.c_str()) == 0)
                        {
                            if(memOffset)
                                address = equates.at(eq).first + pos;

                            else
                                address = equates.at(eq).first;

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                            foundLabel = true;
                            break;
                        }
                    }

                    if(foundLabel == false) //check if regular label and not an equate
                    {
                        for(int lbl = 0; lbl < labels.size(); ++lbl)
                        {
                            if(strcmp(buff2.c_str(), labels.at(lbl).second.c_str()) == 0)
                            {
                                if(memOffset)
                                    address = labels.at(lbl).first + pos;

                                else
                                    address = labels.at(lbl).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }
                    }
                }

                if(foundLabel == false)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: SPIR '" << buff << "' Memory label not found." << std::endl;
                    return;
                }

                byteCount += 4;
            break;

            case RST:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x71);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: RST syntax error" << std::endl;
                    return;
                }
            break;

            case SBP:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(0x72);

                    //check if immediate
                    for(int k = 0; k < buff.size(); ++k)
                    {
                        if(buff.at(k) == '#')
                        {
                            pos = k;
                            break;
                        }
                    }

                    //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                    for(int i = 0; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != '\n')
                            buff1 += buff.at(i);
                    }
                    buff = buff1;

                    if(buff.at(pos) == '#') //immediate
                    {
                        foundLabel = true;
                        if(buff.at(pos + 1) == '$') //hex
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertHexStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else if(buff.at(pos + 1) == '%') //bin
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertBinStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else
                        {
                            buff1 = buff.substr(pos + 1, buff.size());
                            address = atoi(buff1.c_str());

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }
                    }

                    else
                    {
                        //removes any extra spaces in label, e.g. before a comment semicolon
                        for(int c = 0; c < buff.size(); ++c)
                        {
                            if(buff.at(c) == ',')
                            {
                                pos = c;
                                memOffset = true;
                                break;
                            }

                            else if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9')
                                buff2 += buff.at(c);
                        }

                        //get offset amount, if any
                        buff1 = "";
                        for(int i = pos; i < buff.size(); ++i)
                        {
                            if(buff.at(i) != ',')
                                buff1 += buff.at(i);
                        }
                        pos = atoi(buff1.c_str()); //offset in memory

                        foundLabel = false;
                        for(int eq = 0; eq < equates.size(); ++eq)
                        {
                            if(strcmp(buff2.c_str(), equates.at(eq).second.c_str()) == 0)
                            {
                                if(memOffset)
                                    address = equates.at(eq).first + pos;

                                else
                                    address = equates.at(eq).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }

                        if(foundLabel == false) //check if regular label and not an equate
                        {
                            for(int lbl = 0; lbl < labels.size(); ++lbl)
                            {
                                if(strcmp(buff2.c_str(), labels.at(lbl).second.c_str()) == 0)
                                {
                                    if(memOffset)
                                        address = labels.at(lbl).first + pos;

                                    else
                                        address = labels.at(lbl).first;

                                    unsigned char lowByte, midByte, highByte;
                                    lowByte = address & 0xFF;
                                    midByte = ((address >> 8) & 0xFF);
                                    highByte = ((address >> 16) & 0xFF);

                                    assembledBytes.push_back(lowByte);
                                    assembledBytes.push_back(midByte);
                                    assembledBytes.push_back(highByte);
                                    foundLabel = true;
                                    break;
                                }
                            }
                        }
                    }

                    if(foundLabel == false)
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: SBP no matching label found." << std::endl;
                        return;
                    }   

                    byteCount += 4;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: SBP syntax error" << std::endl;
                    return;
                }
            break;

            case XCHG:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x73);

                //get register  
                while(buff.at(pos++) == ' ');  
                for(int j = pos - 1; j < instructions.at(i).second.size(); ++j)
                    registerStr += buff.at(j);

                registerStr = registerStr.substr(0, 2);
                
                //first register
                if(strcmp(registerStr.c_str(), "R1") == 0)
                    assembledBytes.push_back(0x00);

                else if(strcmp(registerStr.c_str(), "R2") == 0)
                    assembledBytes.push_back(0x01);

                else if(strcmp(registerStr.c_str(), "R3") == 0)
                    assembledBytes.push_back(0x02);

                else if(strcmp(registerStr.c_str(), "R4") == 0)
                    assembledBytes.push_back(0x03);

                else //error
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: XCH register argument can only accept registers R1-R4" << std::endl;
                    return;
                }

                byteCount += 2;
            break;

            case STOSB:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x32);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: STOSB syntax error" << std::endl;
                    return;
                }
            break;    

            case SPDR:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x33);

                //check if immediate
                for(int k = 0; k < buff.size(); ++k)
                {
                    if(buff.at(k) == '#')
                    {
                        pos = k;
                        break;
                    }
                }

                //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                for(int i = 0; i < buff.size(); ++i)
                {
                    if(buff.at(i) != '\n')
                        buff1 += buff.at(i);
                }
                buff = buff1;

                if(buff.at(pos) == '#') //immediate
                {
                    foundLabel = true;
                    if(buff.at(pos + 1) == '$') //hex
                    {
                        buff1 = buff.substr(pos + 2, buff.size());                    
                        address = convertHexStrToInt(buff1);

                        unsigned char lowByte, midByte, highByte;
                        lowByte = address & 0xFF;
                        midByte = ((address >> 8) & 0xFF);
                        highByte = ((address >> 16) & 0xFF);

                        assembledBytes.push_back(lowByte);
                        assembledBytes.push_back(midByte);
                        assembledBytes.push_back(highByte);
                    }

                    else if(buff.at(pos + 1) == '%') //bin
                    {
                        buff1 = buff.substr(pos + 2, buff.size());                    
                        address = convertBinStrToInt(buff1);

                        unsigned char lowByte, midByte, highByte;
                        lowByte = address & 0xFF;
                        midByte = ((address >> 8) & 0xFF);
                        highByte = ((address >> 16) & 0xFF);

                        assembledBytes.push_back(lowByte);
                        assembledBytes.push_back(midByte);
                        assembledBytes.push_back(highByte);
                    }

                    else
                    {
                        buff1 = buff.substr(pos + 1, buff.size());
                        address = atoi(buff1.c_str());

                        unsigned char lowByte, midByte, highByte;
                        lowByte = address & 0xFF;
                        midByte = ((address >> 8) & 0xFF);
                        highByte = ((address >> 16) & 0xFF);

                        assembledBytes.push_back(lowByte);
                        assembledBytes.push_back(midByte);
                        assembledBytes.push_back(highByte);
                    }
                }

                else
                {
                    //removes any extra spaces in label, e.g. before a comment semicolon
                    for(int c = 0; c < buff.size(); ++c)
                    {
                        if(buff.at(c) == ',')
                        {
                            pos = c;
                            memOffset = true;
                            break;
                        }

                        else if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9')
                            buff2 += buff.at(c);
                    }

                    //get offset amount, if any
                    buff1 = "";
                    for(int i = pos; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != ',')
                            buff1 += buff.at(i);
                    }
                    pos = atoi(buff1.c_str()); //offset in memory

                    foundLabel = false;
                    for(int eq = 0; eq < equates.size(); ++eq)
                    {
                        if(strcmp(buff2.c_str(), equates.at(eq).second.c_str()) == 0)
                        {
                            if(memOffset)
                                address = equates.at(eq).first + pos;

                            else
                                address = equates.at(eq).first;

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                            foundLabel = true;
                            break;
                        }
                    }

                    if(foundLabel == false) //check if regular label and not an equate
                    {
                        for(int lbl = 0; lbl < labels.size(); ++lbl)
                        {
                            if(strcmp(buff2.c_str(), labels.at(lbl).second.c_str()) == 0)
                            {
                                if(memOffset)
                                    address = labels.at(lbl).first + pos;

                                else
                                    address = labels.at(lbl).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }
                    }
                }

                if(foundLabel == false)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: SPDR '" << buff << "' Memory label not found." << std::endl;
                    return;
                }

                byteCount += 4;
            break;

            case HALT:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x18);

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: HALT syntax error" << std::endl;
                    return;
                }

                byteCount += 1;
            break;

            //***********************TODO***********************
            case CALL:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(0x28);

                //check if immediate
                    for(int k = 0; k < buff.size(); ++k)
                    {
                        if(buff.at(k) == '#')
                        {
                            pos = k;
                            break;
                        }
                    }

                    //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                    for(int i = 0; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != '\n')
                            buff1 += buff.at(i);
                    }
                    buff = buff1;

                    if(buff.at(pos) == '#') //immediate
                    {
                        foundLabel = true;
                        if(buff.at(pos + 1) == '$') //hex
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertHexStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else if(buff.at(pos + 1) == '%') //bin
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertBinStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else
                        {
                            buff1 = buff.substr(pos + 1, buff.size());
                            address = atoi(buff1.c_str());

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }
                    }

                    else
                    {
                        //removes any extra spaces in label, e.g. before a comment semicolon
                        for(int c = 0; c < buff.size(); ++c)
                        {
                            if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9')
                            {
                                pos = c;
                                break;
                            }
                        }

                        for(int i = pos; i < buff.size(); ++i)
                        {
                            if((buff.at(i) >= 'A' && buff.at(i) <= 'Z') || (buff.at(i) >= '0' && buff.at(i) <= '9'))
                                buff2 += buff.at(i);
                        }
                        
                        foundLabel = false;
                        for(int lbl = 0; lbl < labels.size(); ++lbl)
                        {
                            if(strcmp(buff2.c_str(), labels.at(lbl).second.c_str()) == 0)
                            {
                                address = labels.at(lbl).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }
                    }

                    if(foundLabel == false)
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: CALL '" << buff << "' Memory label not found." << std::endl;
                        return; 
                    }   

                    byteCount += 4;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: CALL syntax error" << std::endl;
                    return;
                }
            break;

            case RTS:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x38);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: RTS syntax error" << std::endl;
                    return;
                }
            break;

            case BRA:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(0x48);

                //check if immediate
                    for(int k = 0; k < buff.size(); ++k)
                    {
                        if(buff.at(k) == '#')
                        {
                            pos = k;
                            break;
                        }
                    }

                    //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                    for(int i = 0; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != '\n')
                            buff1 += buff.at(i);
                    }
                    buff = buff1;

                    if(buff.at(pos) == '#') //immediate
                    {
                        foundLabel = true;
                        if(buff.at(pos + 1) == '$') //hex
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertHexStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else if(buff.at(pos + 1) == '%') //bin
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertBinStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else
                        {
                            buff1 = buff.substr(pos + 1, buff.size());
                            address = atoi(buff1.c_str());

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }
                    }

                    else
                    {
                        //removes any extra spaces in label, e.g. before a comment semicolon
                        for(int c = 0; c < buff.size(); ++c)
                        {
                            if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9' || buff.at(c) == '.')
                            {
                                pos = c;
                                break;
                            }
                        }

                        for(int i = pos; i < buff.size(); ++i)
                        {
                            if((buff.at(i) >= 'A' && buff.at(i) <= 'Z') || (buff.at(i) >= '0' && buff.at(i) <= '9') || buff.at(i) == '.')
                                buff2 += buff.at(i);
                        }

                        foundLabel = false;
                        for(int lbl = 0; lbl < labels.size(); ++lbl)
                        {
                            if(strcmp(buff2.c_str(), labels.at(lbl).second.c_str()) == 0)
                            {
                                address = labels.at(lbl).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }
                    }

                    if(foundLabel == false)
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: BRA '" << buff << "' Memory label not found." << std::endl;
                        return; 
                    }

                    byteCount += 4;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: BRA syntax error" << std::endl;
                    return;
                }
            break;

            case NSB:
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x40);
                byteCount += 1;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: NSB syntax error" << std::endl;
                    return;
                }
            break;

            case BRZ:
                try
                {
                    byte = instructions.at(i).first;
                    buff = instructions.at(i).second;
                    assembledBytes.push_back(0x60);

                    //check if immediate
                    for(int k = 0; k < buff.size(); ++k)
                    {
                        if(buff.at(k) == '#')
                        {
                            pos = k;
                            break;
                        }
                    }

                    //remove newline in buff, fixes issue with address size being off when hex buff converted to int
                    for(int i = 0; i < buff.size(); ++i)
                    {
                        if(buff.at(i) != '\n')
                            buff1 += buff.at(i);
                    }
                    buff = buff1;

                    if(buff.at(pos) == '#') //immediate
                    {
                        foundLabel = true;
                        if(buff.at(pos + 1) == '$') //hex
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertHexStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else if(buff.at(pos + 1) == '%') //bin
                        {
                            buff1 = buff.substr(pos + 2, buff.size());                    
                            address = convertBinStrToInt(buff1);

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }

                        else
                        {
                            buff1 = buff.substr(pos + 1, buff.size());
                            address = atoi(buff1.c_str());

                            unsigned char lowByte, midByte, highByte;
                            lowByte = address & 0xFF;
                            midByte = ((address >> 8) & 0xFF);
                            highByte = ((address >> 16) & 0xFF);

                            assembledBytes.push_back(lowByte);
                            assembledBytes.push_back(midByte);
                            assembledBytes.push_back(highByte);
                        }
                    }

                    else
                    {
                        //removes any extra spaces in label, e.g. before a comment semicolon
                        for(int c = 0; c < buff.size(); ++c)
                        {
                            if(buff.at(c) >= 'A' && buff.at(c) <= 'Z' || buff.at(c) >= '0' && buff.at(c) <= '9' || buff.at(c) == '.')
                            {
                                pos = c;
                                break;
                            }
                        }

                        for(int i = pos; i < buff.size(); ++i)
                        {
                            if((buff.at(i) >= 'A' && buff.at(i) <= 'Z') || (buff.at(i) >= '0' && buff.at(i) <= '9') || buff.at(i) == '.')
                                buff2 += buff.at(i);
                        }

                        foundLabel = false;
                        for(int lbl = 0; lbl < labels.size(); ++lbl)
                        {
                            if(strcmp(buff2.c_str(), labels.at(lbl).second.c_str()) == 0)
                            {
                                address = labels.at(lbl).first;

                                unsigned char lowByte, midByte, highByte;
                                lowByte = address & 0xFF;
                                midByte = ((address >> 8) & 0xFF);
                                highByte = ((address >> 16) & 0xFF);

                                assembledBytes.push_back(lowByte);
                                assembledBytes.push_back(midByte);
                                assembledBytes.push_back(highByte);
                                foundLabel = true;
                                break;
                            }
                        }
                    }

                    if(foundLabel == false)
                    {
                        std::cout << std::endl;
                        std::cout << instructionLineNum.at(i) << ':';
                        std::cout << "Error: BRZ '" << buff << "' Memory label not found." << std::endl;
                        return; 
                    }

                    byteCount += 4;
                }

                catch(std::exception& e)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: BRZ syntax error" << std::endl;
                    return;
                }
            break;

            //ALU
            case OR:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x80);

                //get register, remove extra space before 'R'
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) == 'R')
                    {
                        pos = i;
                        break;
                    }
                buff = buff.substr(pos, 2); //just the register 'R1' or 'R2'

                if(strcmp(buff.c_str(), "R1") == 0)
                    assembledBytes.push_back(0x00);

                else if(strcmp(buff.c_str(), "R2") == 0)
                    assembledBytes.push_back(0x01);

                else if(strcmp(buff.c_str(), "R3") == 0)
                    assembledBytes.push_back(0x02);

                else if(strcmp(buff.c_str(), "R4") == 0)
                    assembledBytes.push_back(0x03);

                else //only accept R1 or R2 everything else is an error
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: OR " << buff << "\nOR can only accept registers R1-R4 e.g. 'OR R2'" << std::endl;
                    return;
                }

                //assembledBytes.push_back(0x80);
                byteCount += 2;
            break;

            case AND:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x81);

                //get register, remove extra space before 'R'
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) == 'R')
                    {
                        pos = i;
                        break;
                    }
                buff = buff.substr(pos, 2); //just the register 'R1' or 'R2'

                if(strcmp(buff.c_str(), "R1") == 0)
                    assembledBytes.push_back(0x00);

                else if(strcmp(buff.c_str(), "R2") == 0)
                    assembledBytes.push_back(0x01);

                else if(strcmp(buff.c_str(), "R3") == 0)
                    assembledBytes.push_back(0x02);

                else if(strcmp(buff.c_str(), "R4") == 0)
                    assembledBytes.push_back(0x03);

                else //only accept R1 or R2 everything else is an error
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: AND " << buff << "\nAND can only accept registers R1-R4 e.g. 'AND R2'" << std::endl;
                    return;
                }

                byteCount += 2;
            break;

            case SHL:
                assembledBytes.push_back(0x82);
                byteCount += 1;
                buff = instructions.at(i).second;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: SHL syntax error" << std::endl;
                    return;
                }
            break;

            case SHR:
                assembledBytes.push_back(0x83);
                byteCount += 1;
                buff = instructions.at(i).second;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: SHR syntax error" << std::endl;
                    return;
                }
            break;

            case CMP:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x84);

                //get register, remove extra space before 'R'
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) == 'R')
                    {
                        pos = i;
                        break;
                    }
                buff = buff.substr(pos, 2); //just the register 'R1' or 'R2'

                if(strcmp(buff.c_str(), "R1") == 0)
                    assembledBytes.push_back(0x00);

                else if(strcmp(buff.c_str(), "R2") == 0)
                    assembledBytes.push_back(0x01);

                else if(strcmp(buff.c_str(), "R3") == 0)
                    assembledBytes.push_back(0x02);

                else if(strcmp(buff.c_str(), "R4") == 0)
                    assembledBytes.push_back(0x03);

                else //only accept R1 or R2 everything else is an error
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: CMP " << buff << "\nCMP can only accept registers R1-R4 e.g. 'CMP R2'" << std::endl;
                    return;
                }

                byteCount += 2;
            break;

            case NOT:
                assembledBytes.push_back(0x85);
                byteCount += 1;
                buff = instructions.at(i).second;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: NOT syntax error" << std::endl;
                    return;
                }
            break;

            case XOR:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x86);

                //get register, remove extra space before 'R'
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) == 'R')
                    {
                        pos = i;
                        break;
                    }
                buff = buff.substr(pos, 2); //just the register 'R1' or 'R2'

                if(strcmp(buff.c_str(), "R1") == 0)
                    assembledBytes.push_back(0x00);

                else if(strcmp(buff.c_str(), "R2") == 0)
                    assembledBytes.push_back(0x01);

                else if(strcmp(buff.c_str(), "R3") == 0)
                    assembledBytes.push_back(0x02);

                else if(strcmp(buff.c_str(), "R4") == 0)
                    assembledBytes.push_back(0x03);

                else //only accept R1 or R2 everything else is an error
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: XOR " << buff << "\nXOR can only accept registers R1-R4 e.g. 'XOR R2'" << std::endl;
                    return;
                }

                byteCount += 2;
            break;

            case ADD:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x87);

                //get register, remove extra space before 'R'
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) == 'R')
                    {
                        pos = i;
                        break;
                    }
                buff = buff.substr(pos, 2); //just the register 'R1' or 'R2'

                if(strcmp(buff.c_str(), "R1") == 0)
                    assembledBytes.push_back(0x00);

                else if(strcmp(buff.c_str(), "R2") == 0)
                    assembledBytes.push_back(0x01);

                else if(strcmp(buff.c_str(), "R3") == 0)
                    assembledBytes.push_back(0x02);

                else if(strcmp(buff.c_str(), "R4") == 0)
                    assembledBytes.push_back(0x03);

                else //only accept R1 or R2 everything else is an error
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: ADD " << buff << "\nADD can only accept registers R1-R4 e.g. 'ADD R2'" << std::endl;
                    return;
                }

                byteCount += 2;
            break;

            case SUB:
                byte = instructions.at(i).first;
                buff = instructions.at(i).second;
                assembledBytes.push_back(0x88); 

                //get register, remove extra space before 'R'
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) == 'R')
                    {
                        pos = i;
                        break;
                    }
                buff = buff.substr(pos, 2); //just the register 'R1' or 'R2'

                if(strcmp(buff.c_str(), "R1") == 0)
                    assembledBytes.push_back(0x00);

                else if(strcmp(buff.c_str(), "R2") == 0)
                    assembledBytes.push_back(0x01);

                else if(strcmp(buff.c_str(), "R3") == 0)
                    assembledBytes.push_back(0x02);

                else if(strcmp(buff.c_str(), "R4") == 0)
                    assembledBytes.push_back(0x03);

                else //only accept R1 or R2 everything else is an error
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: SUB " << buff << "\nSUB can only accept registers R1-R4 e.g. 'SUB R2'" << std::endl;
                    return;
                }

                byteCount += 2;
            break;

            case INC:
                assembledBytes.push_back(0x89);
                byteCount += 1;
                buff = instructions.at(i).second;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: INC syntax error" << std::endl;
                    return;
                }
            break;

            case DEC:
                assembledBytes.push_back(0x8A);
                byteCount += 1;
                buff = instructions.at(i).second;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: DEC syntax error" << std::endl;
                    return;
                }
            break;

            case ROL:
                assembledBytes.push_back(0x8B);
                byteCount += 1;
                buff = instructions.at(i).second;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: ROL syntax error" << std::endl;
                    return;
                }
            break;

            case ROR:
                assembledBytes.push_back(0x8C);
                byteCount += 1;
                buff = instructions.at(i).second;

                //check for any other characters after instruction, error if found (except whitespace)
                for(int i = 0; i < buff.size(); ++i)
                    if(buff.at(i) != ' ')
                        ++pos;

                if(pos > 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: ROR syntax error" << std::endl;
                    return;
                }
            break;

            //MISC ASSEMBLER INSTRUCTIONS
            case DBH:
                buff = instructions.at(i).second;

                buffVec.clear();
                buff1 = "";

                if(buff.size() == 1)
                {
                    std::cout << std::endl;
                    std::cout << instructionLineNum.at(i) << ':';
                    std::cout << "Error: DBH missing subsequent data" << std::endl;
                    return;
                }

                for(int i = 0; i < buff.size() - 1; ++i) //size - 1, don't include newline
                {
                    if(buff.at(i) == ',')
                    {
                        buffVec.push_back(buff1);
                        buff1 = "";
                    }

                    else
                        if(buff.at(i) != ' ')
                            buff1 += buff.at(i);
                }
                buffVec.push_back(buff1);

                for(int i = 0; i < buffVec.size(); ++i)
                    assembledBytes.push_back(convertHexStrToInt(buffVec.at(i)));

                byteCount += buffVec.size();
            break;

            case DBC:
                buff = instructions.at(i).second;

                inQuote = false;
                for(int i = 0; i < buff.size(); ++i)
                {
                    if(buff.at(i) == '"')
                        inQuote = !inQuote;

                    else if(inQuote)
                        assembledBytes.push_back(buff.at(i));
                }

                byteCount += buff.size();
            break;
        }
    }

    //write binary to file
    std::ofstream file;
    file.open(outputFileName);
    for(int byte = 0; byte < assembledBytes.size(); ++byte)
    {
        file << assembledBytes.at(byte);
    }
    file.close();


    if(verboseMode)
    {
        std::cout << "\n\n\nFunctions:\n";
        for(int i = 0; i < labels.size(); ++i)
            std::cout << labels.at(i).second << std::endl;

        std::cout << "\n\nOutput bytes:\n";

        int byteCount = 0;
        std::cout << std::endl << byteCount << "\t";
        for(int i = 0; i < assembledBytes.size(); ++i)
        {
            if(i != 0)
                if(i % 16 == 0)
                    std::cout << std::endl << std::dec << byteCount << "\t";
                
            std::cout << std::hex << (int)assembledBytes.at(i) << "  ";
            ++byteCount;
        }
    }
}

void convertToAllUpper(std::string& source)
{
    bool inQuote = false;
    for(int i = 0; i < source.size(); ++i)
    {
        if(source.at(i) == '"')
            inQuote = !inQuote;

        //ignore anything in quote marks
        if(inQuote == false)
        {
            if(source.at(i) >= 0x61 && source.at(i) <= 0x7A) //a-z
                source.at(i) -= 0x20;
        }
    }
}

void removeComments(std::string& source)
{
    bool lineComment = false;
    std::string sourceBuffer;
    for(int i = 0; i < source.size(); ++i)
    {
        if(source.at(i) == ';')
            lineComment = true;

        if(source.at(i) == '\n')
            lineComment = false;

        if(lineComment == false)
            sourceBuffer += source.at(i);
    }

    source = "";
    for(int i = 0; i < sourceBuffer.size(); ++i)
        source += sourceBuffer.at(i);
}

void assembler(std::string fileName)
{
    std::ifstream file;
    std::string source;

    file.open(fileName);
    if(file.good())
    {
        while(file.eof() == false)
        {
            char buff[80];
            file.getline(buff, 80);

            //remove tabs before commands
            int pos = 0;
            char parsedBuff[80];
            bool skipSpace = true;
            for(int i = 0; i < 80; ++i)
            {
                if(buff[i] != ' ' && buff[i] != '\t')
                    skipSpace = false;

                if(skipSpace)
                {continue;}

                else
                    parsedBuff[pos++] = buff[i];
            }

            source += parsedBuff;
            source += "\n";
        }
    }

    else
        std::cout << "Error: Input file " << fileName << " not found\n";
    file.close();

    removeComments(source);
    convertToAllUpper(source);

    generateSymbols(source);
}

int main(int args, char* argv[])
{
    std::string helpStr = "\nZ8 assembler commmands:\n-h = help\n-v = verbose\n-o = output file name\n"
    "For example: z8 file.asm -v -o file.bin\n\n";

    verboseMode = false;
    outputFileName = "a.out";

    if(args > 1)
    {
        if(strcmp(argv[1], "-h") == 0)
        {
            std::cout << helpStr;
            return 0;
        }
    }

    if(args <= 1)
    {
        std::cout << "Error: No input file\n";
        std::cout << helpStr;
    }

    else
    {
        if(args > 2)
        {
            //count arg flags
            int numArgs = 0;
            for(int i = 0; i < args - 2; ++i)
            {
                try
                {
                    if(argv[i + 2][0] == '-')
                        ++numArgs;
                }

                catch(std::exception& e)
                {}
            }
            
            //std::cout << "num args " << numArgs << std::endl;
            //std::cout << argv[2][1] << std::endl;
            
            for(int i = 0; i < numArgs; ++i)
            {
                if(argv[i + 2][0] == '-')
                {
                    switch(argv[i + 2][1])
                    {
                        case 'v':
                            verboseMode = true;
                        break;

                        case 'o':
                            if(args > i + 3)
                            {
                                std::cout << argv[i + 3] << std::endl;
                                outputFileName = argv[i + 3];
                            }

                            else
                            {
                                std::cout << "Assembler flag -o switch does not have a valid name following it\n";
                            }
                        break;

                        default:
                            std::cout << argv[i + 2] << "is not a valid argument flag\n";
                    }
                }
            }
        }

        std::string fileName = argv[1];
        assembler(fileName);
    }
}

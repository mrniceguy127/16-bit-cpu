#include <fstream>
#include <iostream>
#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "data-structures.hpp"

using namespace std;
using namespace DataStructures;

namespace tokenBinaryTable {
  namespace instructionSet {
    static const string inp = "00000000";
    static const string add = "00000001";
    static const string sub = "00000010";
    static const string jmp = "00000011";
    static const string tac = "00000100";
    static const string sto = "00000101";
    static const string cla = "00000110";
    static const string out = "00000111";
    static const string hrs = "00001000";
    static const string sft = "00001001";
  }

  namespace specials {
    static const string comment = "#"; // If token starts with this chracter, the rest of the line is ignored and excluded from compiled output.
    static const string data = "DATA";
  }
}

static const unsigned instructionsSetSize = 10;
static const string validInstructions[instructionsSetSize] = {
  "INP",
  "ADD",
  "SUB",
  "JMP",
  "TAC",
  "STO",
  "CLA",
  "OUT",
  "HRS",
  "SFT"
};

static const unsigned specialsSize = 2;
static const string validSpecials[specialsSize] = {
  "#",
  "DATA"
};

static const unsigned wordLength = 16;
//static const unsigned instructionLength = 4;
static const unsigned addressLength = 8;

unsigned calculateNumLength(int num) {
  int tempNum = num;
  unsigned length = 1;

  while (tempNum > 1) {
    tempNum /= 2;
    length++;
  }

  return length;
}

// Get all lines of file
LinkedList<string> getLines(ifstream & file) {
  LinkedList<string> lines = LinkedList<string>();
  string line;

  while (getline(file, line)) {
    if (line.length() > 0) {
      unsigned i = 0;
      string charToCheck = line.substr(i, 1);
      while (isspace(charToCheck.at(0)) > 0 && i < line.length()) {
        charToCheck = line.substr(i, 1);
        i++;
      }

      if (charToCheck != tokenBinaryTable::specials::comment) {
        lines.push(line);
      }
    }
  }

  file.close();

  int maxLines = static_cast<int>(pow(2, addressLength));

  if (lines.size() > maxLines) {
    cout << "Compiler error: This CPU architecture does not support more than "
         << maxLines
         << " words of memory. That means your program may not exceed more than "
         << maxLines
         << " lines of memory!"
         << endl;
    exit(EXIT_FAILURE);
  } else {
    return lines;
  }
}

// Get a pointer to an array of strings from a line by
// splitting the line into multiple strings after every
// space (removing the space).
LinkedList<string> getLineTokens(string line) {
  LinkedList<string> tokens = LinkedList<string>();
  string nextToken = "";
  string commentPrefix = tokenBinaryTable::specials::comment;

  unsigned i;
  for (i = 0; i < line.length(); i++) {
    string charToCheck = string(1, line.at(i));

    if (charToCheck == commentPrefix) {
      break;
    } else if (isspace(charToCheck.at(0)) > 0) {
      if (nextToken != "") {
        nextToken += charToCheck == " " ? "" : charToCheck;
        tokens.push(nextToken);
        nextToken = "";
      }
    } else if (i == line.length() - 1) {
      nextToken += charToCheck == " " ? "" : charToCheck;
      tokens.push(nextToken);
    } else {
      nextToken += charToCheck;
    }
  }

  return tokens;
}

// Get a linked list of linked lists of line tokens.
LinkedList<LinkedList<string>> getTokens(LinkedList<string> & lines) {
  LinkedList<LinkedList<string>> tokens = LinkedList<LinkedList<string>>();
  unsigned lineCount = static_cast<unsigned>(lines.size());
  unsigned i;
  for (i = 0; i < lineCount; i++) {
    LinkedList<string> tokenLine = getLineTokens(lines.get(static_cast<int>(i)));
    unsigned tokenLineSize = static_cast<unsigned>(tokenLine.size());
    if (tokenLineSize == 2 || tokenLineSize == 3) {
      tokens.push(tokenLine);
    } else {
      cout << "Compiler error: Invalid line of code at line " << i + 1 << "(" << tokenLine.get(0) << tokenLine.get(1) << tokenLine.get(2) << ")." << endl;
      exit(EXIT_FAILURE);
    }
  }

  return tokens;
}

// Verify a character is a hex digit.
bool validateHexDigit(char hexChar) {
  char validChars[16] = {
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F'
  };

  unsigned i;
  for (i = 0; i < 16; i++) {
    if (hexChar == validChars[i]) {
      return true;
    }
  }

  return false;
}

bool validateHex(string & hexValue) {
  bool valid = true;

  unsigned i;
  for (i = 0; i < hexValue.length(); i++) {
    valid = validateHexDigit(hexValue.at(i));
    if (!valid) {
      break;
    }
  }

  return valid;
}

// Get the unsigned number represented by the address string
unsigned parseHexString(string & hexValue, unsigned lineIndex) {
  bool valid = true;

  unsigned i;
  for (i = 0; i < hexValue.length(); i++) {
    if (!validateHexDigit(hexValue.at(i))) {
      valid = false;
      break;
    }
  }

  if (valid) {
    unsigned parsedHex;
    stringstream ss;
    ss << hex << hexValue;
    ss >> parsedHex;
    return static_cast<unsigned>(parsedHex);
  } else {
    cout << "Compiler error: Invalid address \"" << hexValue << "\"" << " at line " << lineIndex + 1 << "." << endl;
    exit(EXIT_FAILURE);
  }
}

// Validate the assembly instructions
bool validateLine(LinkedList<string> line, LinkedList<string> & validLabels, unsigned lineIndex) {
  bool isValidLine = true;

  unsigned tokenIndex;
  for (tokenIndex = 0; tokenIndex < static_cast<unsigned>(line.size()); tokenIndex++) {
    if (tokenIndex == 0) { // Check if checking first token
      if (line.size() == 3) { // Check if line includes label
        tokenIndex += 1;
      }

      if (line.get(static_cast<int>(tokenIndex)) != validSpecials[1]) {
        string token = line.get(static_cast<int>(tokenIndex));

        bool isValidInstruction = false;

        unsigned i;
        for (i = 0; i < instructionsSetSize; i++) {
          if (token == validInstructions[i]) {
            isValidInstruction = true;
            break; // Instruction exists, so stop checking
          }
        }

        if (!isValidInstruction) {
          isValidLine = false;
          cout << "Compiler error: Invalid instruction \"" << token << "\"" << " at line " << lineIndex + 1 << "." << endl;
          exit(EXIT_FAILURE);
        }
      }
    } else if (tokenIndex >= 1) {
      if (line.get(static_cast<int>(tokenIndex) - 1) != validSpecials[1]) {
        string address = line.get(static_cast<int>(tokenIndex));
        bool isValidAddress = false;

        unsigned i;
        for (i = 0; i < static_cast<unsigned>(validLabels.size()); i++) {
          if (address == validLabels.get(static_cast<int>(i))) {
            isValidAddress = true;
            break;
          } else if (validateHex(address)) {
            if (calculateNumLength(static_cast<int>(parseHexString(address, lineIndex))) > addressLength) {
              cout << "Compiler error: Overflow error at line " << lineIndex + 1 << "." << endl;
              exit(EXIT_FAILURE);
            } else {
              isValidAddress = true;
            }
          }
        }
      } else {
        string hexValue = line.get(static_cast<int>(tokenIndex));

        if (calculateNumLength(static_cast<int>(parseHexString(hexValue, lineIndex))) > wordLength) {
          cout << "Compiler error: Overflow error at line " << lineIndex + 1 << "." << endl;
          exit(EXIT_FAILURE);
        }
        isValidLine = validateHex(hexValue);
      }
    }
  }

  return isValidLine;
}

// Get the label of every line of the assembly file.
LinkedList<string> getLabels(LinkedList<LinkedList<string>> & tokens) {
  LinkedList<string> labels = LinkedList<string>();

  unsigned i;
  for (i = 0; i < static_cast<unsigned>(tokens.size()); i++) {
    if (tokens.get(static_cast<int>(i)).size() == 3) {
      string labelName = tokens.get(static_cast<int>(i)).get(0);
      labels.push(labelName);
    } else {
      labels.push("");
    }
  }
  return labels;
}

string convertToBinary(int value, unsigned length) {
  string bin = "";
  unsigned i = 0;

  while (pow(2, i) <= value) {
    string bit;
    stringstream ss;
    ss << (value / static_cast<int>(pow(2, i))) % 2;
    ss >> bit;
    bin = bit + bin;
    i++;
  }

  unsigned binLength = static_cast<unsigned>(bin.length());
  int bitsLeft = static_cast<int>(length - binLength);
  while (bitsLeft > 0) {
    bin = "0" + bin;
    binLength = static_cast<unsigned>(bin.length());
    bitsLeft = static_cast<int>(length - binLength);
  }

  return bin;
}

string convertLabelReferenceToMachineCode(unsigned labelIndex) {
  string binaryString = convertToBinary(static_cast<int>(labelIndex), addressLength);

  return binaryString;
}

string convertInstructionToMachineCode(string instruction) {
  if (instruction == validInstructions[0]) {
    return tokenBinaryTable::instructionSet::inp;
  } else if (instruction == validInstructions[1]) {
    return tokenBinaryTable::instructionSet::add;
  } else if (instruction == validInstructions[2]) {
    return tokenBinaryTable::instructionSet::sub;
  } else if (instruction == validInstructions[3]) {
    return tokenBinaryTable::instructionSet::jmp;
  } else if (instruction == validInstructions[4]) {
    return tokenBinaryTable::instructionSet::tac;
  } else if (instruction == validInstructions[5]) {
    return tokenBinaryTable::instructionSet::sto;
  } else if (instruction == validInstructions[6]) {
    return tokenBinaryTable::instructionSet::cla;
  } else if (instruction == validInstructions[7]) {
    return tokenBinaryTable::instructionSet::out;
  } else if (instruction == validInstructions[8]) {
    return tokenBinaryTable::instructionSet::hrs;
  } else if (instruction == validInstructions[9]) {
    return tokenBinaryTable::instructionSet::sft;
  } else {
    return "";
  }
}

string convertLineToMachineCode(LinkedList<string> line, LinkedList<string> labels, unsigned lineIndex) {
  int tokenOffset = 0;
  bool hasLabel = line.size() > 2;

  if (hasLabel) {
    tokenOffset = 1;
  }

  if (line.get(0 + tokenOffset) != validSpecials[1]) {
    string machineCodeInstruction = convertInstructionToMachineCode(line.get(0 + tokenOffset));
    string machineCodeLabelReference = "";

    string referencedLabel = line.get(1 + tokenOffset);

    unsigned i;
    for (i = 0; i < static_cast<unsigned>(labels.size()); i++) {
      if (labels.get(static_cast<int>(i)) == referencedLabel) {
        machineCodeLabelReference = convertLabelReferenceToMachineCode(i);
      }
    }

    if (machineCodeLabelReference.length() == 0) {
      machineCodeLabelReference = convertLabelReferenceToMachineCode(parseHexString(referencedLabel, lineIndex));
    }

    string machineCodeLine = machineCodeInstruction + machineCodeLabelReference;

    return machineCodeLine;
  } else {
    string hexValue = line.get(1 + tokenOffset);
    if (validateHex(hexValue)) {
      return convertToBinary(static_cast<int>(parseHexString(hexValue, lineIndex)), wordLength);
    } else {
      cout << "Compiler error: Invalid hex value at line " << lineIndex + 1 << "." << endl;
      exit(EXIT_FAILURE);
    }
  }
}

// Convert tokens to machine code
LinkedList<string> getMachineCode(LinkedList<LinkedList<string>> & tokens) {
  LinkedList<string> machineCode = LinkedList<string>();
  LinkedList<string> labels = getLabels(tokens);

  unsigned lineIndex;
  for (lineIndex = 0; lineIndex < static_cast<unsigned>(tokens.size()); lineIndex++) {
    LinkedList<string> assemblyCodeLine = tokens.get(static_cast<int>(lineIndex));
    if (validateLine(assemblyCodeLine, labels, lineIndex)) {
      string machineCodeLine = convertLineToMachineCode(assemblyCodeLine, labels, lineIndex);

      machineCode.push(machineCodeLine);
    } else {
      cout << "Compiler error: Invalid line of code at line " << lineIndex + 1 << "." << endl;
      exit(EXIT_FAILURE);
    }
  }

  return machineCode;
}

// Compile assembly code
string compile(LinkedList<string> lines) {
  LinkedList<LinkedList<string>> tokens = getTokens(lines);
  LinkedList<string> machineCodeLines = getMachineCode(tokens);

  string output = "";

  unsigned i;
  for (i = 0; i < static_cast<unsigned>(machineCodeLines.size()); i++) {
    output += machineCodeLines.get(static_cast<int>(i)) + "\n";
  }

  return output;
}

string compileBootable(LinkedList<string> lines) {
  LinkedList<string> linesToConcat = lines.slice(0, lines.size());

  LinkedList<string> newLines = LinkedList<string>();

  newLines.push("INP 2");
  newLines.push("JMP 00");
  newLines.push("INP 00");

  newLines.concat(&linesToConcat);

  LinkedList<LinkedList<string>> tokens = getTokens(newLines);
  LinkedList<string> machineCodeLines = getMachineCode(tokens);

  string output = "";

  unsigned i;
  for (i = 0; i < static_cast<unsigned>(machineCodeLines.size()); i++) {
    output += machineCodeLines.get(static_cast<int>(i)) + "\n";
    if (i >= 2) {
      output += convertToBinary(static_cast<int>(i) + 1, 16) + "\n";
    }
  }
  output += "0000000000000000\n0000001100000011";

  return output;
}







string convertBinaryToHex(string binary) {
  string hexOutput;
  unsigned sections = static_cast<unsigned>(binary.length()) / 4;

  unsigned i = 0;
  while (i < sections) {
    string tempHex = "";

    int decimal = 0;
    int remainder;
    int num = stoi(binary.substr(i*sections, 4));
    int base = 1;

    while (num > 0) {
      remainder = num % 10;
      decimal += remainder * base;
      base *= 2;
      num /= 10;
    }

    stringstream ss;
    ss << hex << decimal;
    ss >> tempHex;

    hexOutput += tempHex;

    i++;
  }

  return hexOutput;
}

// Convert default binary output to hexadecimal.
string convertBinaryOutputToHex(string input) {
  string hexOutput;

  // FIXME Useless variables and useless operations here!!!
  stringstream ss;
  ss << input;
  string line;
  while (getline(ss, input)) {
    hexOutput += convertBinaryToHex(input) + "\n";
  }

  return hexOutput;
}

// Save machine code to file.
void saveOutput(string & fileName, string data) {
  string binFileName = fileName + ".bin.txt";
  string hexFileName = fileName + ".hex.txt";

  ofstream binFile (binFileName);
  ofstream hexFile (hexFileName);

  if (binFile.is_open()) {
    binFile << data;
    binFile.close();
  } else {
    cout << "Unable to open file \"" << binFileName << "\"!";
  }

  if (hexFile.is_open()) {
    hexFile << convertBinaryOutputToHex(data);
    hexFile.close();
  } else {
    cout << "Unable to open file \"" << hexFileName << "\"!";
  }
}

void saveBootOutput(string & fileName, string data) {
  string hexBootFileName = fileName + ".hex.boot.txt";

  ofstream hexBootFile (hexBootFileName);

  if (hexBootFile.is_open()) {
    hexBootFile << convertBinaryOutputToHex(data);
    hexBootFile.close();
  } else {
    cout << "Unable to open file \"" << hexBootFileName << "\"!";
  }
}

// Get file with code in it.
ifstream getCodeFile(string & relativePath) {
  ifstream file (relativePath);
  return file;
}

void run() {
  string inputFileName = "in.txt";
  string outputFileName = "out";

  ifstream inputFile = getCodeFile(inputFileName);

  if (inputFile.is_open()) {
    LinkedList<string> lines = getLines(inputFile);
    string output = compile(lines);
    string bootOutput = compileBootable(lines);
    saveOutput(outputFileName, output);
    saveBootOutput(outputFileName, bootOutput);
    inputFile.close();
  } else {
    cout << "Unable to open file \"" << inputFileName << "\"!" << endl;
  }
}

int main()
{
  run();
  return 0;
}

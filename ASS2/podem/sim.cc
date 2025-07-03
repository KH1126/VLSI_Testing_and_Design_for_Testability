/* Logic Simulator
 * Last update: 2006/09/20 */
#include <iostream>
#include "gate.h"
#include "circuit.h"
#include "ReadPattern.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

#define LOGIC_0 0b00 // 0
#define LOGIC_1 0b11 // 1
#define LOGIC_X 0b01 // X (unknown)

void setLogicValue(int &result, VALUE value) {
    switch (value) { 
        case S0:
            result = LOGIC_0;
            break;
        case S1:
            result = LOGIC_1;
            break;
        case X:
            result = LOGIC_X;
            break;
        default:
            result = LOGIC_X; // Default to X for undefined
            break;    
    }
}

void getLogicValue(int input, VALUE &value) {
    switch (input) { 
        case LOGIC_0:
            value = S0;
            break;
        case LOGIC_1:
            value = S1;
            break;
        case LOGIC_X:
        case 0b10: // Treat 0b10 as unknown
            value = X;
            break;
        default:
            value = X; // Default to X for invalid input
            break;    
    }
}

// AND 
void logic_and(VALUE &result, VALUE a, VALUE b) {
    int a_val, b_val;
    setLogicValue(a_val, a);
    setLogicValue(b_val, b);
    int fin = (a_val & b_val);
    getLogicValue(fin, result);
}

// OR 
void logic_or(VALUE &result, VALUE a, VALUE b) {
    int a_val, b_val;
    setLogicValue(a_val, a);
    setLogicValue(b_val, b);
    int fin = (a_val | b_val);
    getLogicValue(fin, result);
}

// NOT 
void logic_not(VALUE &result, VALUE a) {
    int a_val;
    setLogicValue(a_val, a);
    int fin = ~a_val & 0b11; 
    getLogicValue(fin, result);
}


//do logic simulation for test patterns
void CIRCUIT::LogicSimVectors()
{
    cout << "Run logic simulation" << endl;
    //read test patterns
    while (!Pattern.eof()) {
        Pattern.ReadNextPattern();
        SchedulePI();
        LogicSim();
        PrintIO();
    }
    return;
}
void CIRCUIT::LogicSimVectorsmod()
{
    cout << "Run logic simulation" << endl;
    //read test patterns
    while (!Pattern.eof()) {
        Pattern.ReadNextPattern();
        SchedulePI();
        LogicSim_MOD();
        PrintIO();
    }
    return;
}
//do event-driven logic simulation
void CIRCUIT::LogicSim()
{
    GATE* gptr;
    VALUE new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = Evaluate(gptr);
            if (new_value != gptr->GetValue()) {
                gptr->SetValue(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}
void CIRCUIT::LogicSim_MOD()
{
    GATE* gptr;
    VALUE new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = Evaluate_MOD(gptr);
            if (new_value != gptr->GetValue()) {
                gptr->SetValue(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}
//Used only in the first pattern
void CIRCUIT::SchedulePI()
{
    for (unsigned i = 0;i < No_PI();i++) {
        if (PIGate(i)->GetFlag(SCHEDULED)) {
            PIGate(i)->ResetFlag(SCHEDULED);
            ScheduleFanout(PIGate(i));
        }
    }
    return;
}

//schedule all fanouts of PPIs to Queue
void CIRCUIT::SchedulePPI()
{
    for (unsigned i = 0;i < No_PPI();i++) {
        if (PPIGate(i)->GetFlag(SCHEDULED)) {
            PPIGate(i)->ResetFlag(SCHEDULED);
            ScheduleFanout(PPIGate(i));
        }
    }
    return;
}

//set all PPI as 0
void CIRCUIT::SetPPIZero()
{
    GATE* gptr;
    for (unsigned i = 0;i < No_PPI();i++) {
        gptr = PPIGate(i);
        if (gptr->GetValue() != S0) {
            gptr->SetFlag(SCHEDULED);
            gptr->SetValue(S0);
        }
    }
    return;
}

//schedule all fanouts of gate to Queue
void CIRCUIT::ScheduleFanout(GATE* gptr)
{
    for (unsigned j = 0;j < gptr->No_Fanout();j++) {
        Schedule(gptr->Fanout(j));
    }
    return;
}

//initial Queue for logic simulation
void CIRCUIT::InitializeQueue()
{
    SetMaxLevel();
    Queue = new ListofGate[MaxLevel + 1];
    return;
}

//evaluate the output value of gate
VALUE CIRCUIT::Evaluate(GATEPTR gptr)
{
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    VALUE value(gptr->Fanin(0)->GetValue());
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = AndTable[value][gptr->Fanin(i)->GetValue()];
            }
            break;
        case G_OR:
        case G_NOR:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = OrTable[value][gptr->Fanin(i)->GetValue()];
            }
            break;
        default: break;
    }
    //NAND, NOR and NOT
    if (gptr->Is_Inversion()) { value = NotTable[value]; }
    return value;
}

VALUE CIRCUIT::Evaluate_MOD(GATEPTR gptr) {
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]);
    VALUE value(gptr->Fanin(0)->GetValue()); 
    VALUE temp_value; 

    // 根據邏輯函數進行計算
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1; i < gptr->No_Fanin() && value != cv; ++i) {
                logic_and(temp_value, value, gptr->Fanin(i)->GetValue());
                value = temp_value; 
            }
            break;

        case G_OR:
        case G_NOR:
            for (unsigned i = 1; i < gptr->No_Fanin() && value != cv; ++i) {
                logic_or(temp_value, value, gptr->Fanin(i)->GetValue());
                value = temp_value; 
            }
            break;

        default:
            break;
    }

    // NAND NOR NOT
    if (gptr->Is_Inversion()) {
        logic_not(value, value);
    }

    return value; 
}


extern GATE* NameToGate(string);

void PATTERN::Initialize(char* InFileName, int no_pi, string TAG)
{
    patterninput.open(InFileName, ios::in);
    if (!patterninput) {
        cout << "Unable to open input pattern file: " << InFileName << endl;
        exit( -1);
    }
    string piname;
    while (no_pi_infile < no_pi) {
        patterninput >> piname;
        if (piname == TAG) {
            patterninput >> piname;
            inlist.push_back(NameToGate(piname));
            no_pi_infile++;
        }
        else {
            cout << "Error in input pattern file at line "
                << no_pi_infile << endl;
            cout << "Maybe insufficient number of input\n";
            exit( -1);
        }
    }
    return;
}

//Assign next input pattern to PI
void PATTERN::ReadNextPattern()
{
    char V;
    for (int i = 0;i < no_pi_infile;i++) {
        patterninput >> V;
        if (V == '0') {
            if (inlist[i]->GetValue() != S0) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(S0);
            }
        }
        else if (V == '1') {
            if (inlist[i]->GetValue() != S1) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(S1);
            }
        }
        else if (V == 'X') {
            if (inlist[i]->GetValue() != X) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(X);
            }
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();
    return;
}

void CIRCUIT::PrintIO()
{
    register unsigned i;
    cout<< "PI: ";
    for (i = 0;i<No_PI();++i) { cout << PIGate(i)->GetValue(); }
    cout << " PO: ";
    for (i = 0;i<No_PO();++i) { cout << POGate(i)->GetValue(); }
    cout << endl;
    return;
}


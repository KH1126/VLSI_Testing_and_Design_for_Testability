#include <iostream>
#include <ctime>
#include "circuit.h"
#include "GetLongOpt.h"
#include "ReadPattern.h"
using namespace std;

// All defined in readcircuit.l
extern char* yytext;
extern FILE *yyin;
extern CIRCUIT Circuit;
extern int yyparse (void);
extern bool ParseError;

extern void Interactive();

GetLongOpt option;
void findtrack(string PI, string PO, GATE*& PI_PTR, GATE*& PO_PTR);
void backward(GATE* PO_PTR);
void recursive(GATE* PI_PTR, GATE* PO_PTR, vector<string>& namelist,unsigned &count);
int SetupOption(int argc, char ** argv)
{
    option.usage("[options] input_circuit_file");
    option.enroll("help", GetLongOpt::NoValue,
            "print this help summary", 0);
    option.enroll("logicsim", GetLongOpt::NoValue,
            "run logic simulation", 0);
    option.enroll("plogicsim", GetLongOpt::NoValue,
            "run parallel logic simulation", 0);
    option.enroll("fsim", GetLongOpt::NoValue,
            "run stuck-at fault simulation", 0);
    option.enroll("stfsim", GetLongOpt::NoValue,
            "run single pattern single transition-fault simulation", 0);
    option.enroll("transition", GetLongOpt::NoValue,
            "run transition-fault ATPG", 0);
    option.enroll("input", GetLongOpt::MandatoryValue,
            "set the input pattern file", 0);
    option.enroll("output", GetLongOpt::MandatoryValue,
            "set the output pattern file", 0);
    option.enroll("bt", GetLongOpt::OptionalValue,
            "set the backtrack limit", 0);
    /*******************************************************************************/
    option.enroll("path", GetLongOpt::NoValue,
            "list and count all possible paths connecting the given PI and PO", 0);
    option.enroll("start", GetLongOpt::MandatoryValue,
            "set the input path", 0);
    option.enroll("end", GetLongOpt::MandatoryValue,
            "set the output path", 0);
    /*******************************************************************************/
    int optind = option.parse(argc, argv);
    if ( optind < 1 ) { exit(0); }
    if ( option.retrieve("help") ) {
        option.usage();
        exit(0);
    }
    return optind;
}

int main(int argc, char ** argv)
{
    GATE* PI_PTR;
    GATE* PO_PTR;
    unsigned count=0;
    int optind = SetupOption(argc, argv);
    clock_t time_init, time_end;
    time_init = clock();
    //Setup File
    if (optind < argc) {
        if ((yyin = fopen(argv[optind], "r")) == NULL) {
            cout << "Can't open circuit file: " << argv[optind] << endl;
            exit( -1);
        }
        else {
            string circuit_name = argv[optind];
            string::size_type idx = circuit_name.rfind('/');
            if (idx != string::npos) { circuit_name = circuit_name.substr(idx+1); }
            idx = circuit_name.find(".bench");
            if (idx != string::npos) { circuit_name = circuit_name.substr(0,idx); }
            Circuit.SetName(circuit_name);
        }
    }
    else {
        cout << "Input circuit file missing" << endl;
        option.usage();
        return -1;
    }
    cout << "Start parsing input file\n";
    yyparse();
    if (ParseError) {
        cerr << "Please correct error and try Again.\n";
        return -1;
    }
    fclose(yyin);
    Circuit.FanoutList();
    Circuit.SetupIO_ID();
    Circuit.Levelize();
    Circuit.Check_Levelization();
    Circuit.InitializeQueue();

    if (option.retrieve("logicsim")) {
        //logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.LogicSimVectors();
    }
    else if (option.retrieve("plogicsim")) {
        //parallel logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.ParallelLogicSimVectors();
    }
    else if (option.retrieve("stfsim")) {
        //single pattern single transition-fault simulation
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.TFaultSimVectors();
    }
    else if (option.retrieve("transition")) {
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.SortFaninByLevel();
        if (option.retrieve("bt")) {
            Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
        }
        Circuit.TFAtpg();
    }
    else if(option.retrieve("path")){
        string PI_name,PO_name;
        vector<string> namelist;
        PI_name=option.retrieve("start");
        PO_name=option.retrieve("end");
        findtrack(PI_name,PO_name,PI_PTR,PO_PTR);
        recursive(PI_PTR,PO_PTR,namelist,count);
        cout<<"The paths from "<<PI_name<<" to "<<PO_name<<": "<<count<<endl;
    }
    else {
        Circuit.GenerateAllFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        if (option.retrieve("fsim")) {
            //stuck-at fault simulator
            Circuit.InitPattern(option.retrieve("input"));
            Circuit.FaultSimVectors();
        }

        else {
            if (option.retrieve("bt")) {
                Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
            }
            //stuck-at fualt ATPG
            Circuit.Atpg();
        }
    }
    time_end = clock();
    cout << "total CPU time = " << double(time_end - time_init)/CLOCKS_PER_SEC << endl;
    cout << endl;
    return 0;
}
void findtrack(string PI,string PO,GATE*& PI_PTR, GATE*& PO_PTR)
{
    unsigned i = 0;
    int signal=0;
    GATE* gptr;
    string out= PO;
    for (;i < Circuit.No_Gate();i++) {
        gptr = Circuit.Gate(i);
        if(gptr->GetName()==PI)
        {
            signal++;
            PI_PTR=gptr;
        }
        else if (gptr->GetName()==out)
        {
            signal++;
            PO_PTR=gptr;
        }
        if (signal==2) break;
    }
    backward(PO_PTR); 
}
void backward(GATE* PO_PTR)
{
    GATE* now=PO_PTR;
    now->Setflag1();
    if(now->GetFunction()==G_PI) return;
    else {
        for (unsigned j = 0;j < now->No_Fanin();j++) {
            if(now->Fanin(j)->Getflag1())continue;
            now->Fanin(j)->Setflag1();
            //cout<<now->Fanin(j)->GetName()<<endl;
            backward(now->Fanin(j));
        }
    }
}
void recursive(GATE* PI_PTR, GATE* PO_PTR, vector<string>& namelist,unsigned &count)
{
    if(PI_PTR==PO_PTR)
    {
        count++;
        for (unsigned i=0;i<namelist.size();i++)
        {
            cout<<namelist[i]<<" ";
        }
        cout<<PO_PTR->GetName();
        cout<<endl;
        return;
    }
    else{
        for (unsigned j = 0;j < PI_PTR->No_Fanout();j++) {
            if(PI_PTR->Getflag1()){
                namelist.push_back(PI_PTR->GetName());
                recursive(PI_PTR->Fanout(j),PO_PTR,namelist,count);
                namelist.pop_back();
            }
        }
    }
}
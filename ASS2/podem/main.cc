#include <iostream>
#include <ctime>
#include "circuit.h"
#include "GetLongOpt.h"
#include "ReadPattern.h"
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>   
#include <vector>  
using namespace std;

// All defined in readcircuit.l
extern char* yytext;
extern FILE *yyin;
extern CIRCUIT Circuit;
extern int yyparse (void);
extern bool ParseError;

extern void Interactive();

GetLongOpt option;
void pattern_generate(string file_name,int number,bool check);
char random_value(bool check);
int SetupOption(int argc, char ** argv)
{
    option.usage("[options] input_circuit_file");
    option.enroll("help", GetLongOpt::NoValue,
            "print this help summary", 0);
    option.enroll("logicsim", GetLongOpt::NoValue,
            "run logic simulation", 0);
    option.enroll("mod_logicsim", GetLongOpt::NoValue,
            "run mod logic simulation", 0);
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
    option.enroll("pattern", GetLongOpt::NoValue,
    "generate pattern", 0);
    option.enroll("unknown", GetLongOpt::NoValue,
    "generate pattern", 0);
    option.enroll("num", GetLongOpt::MandatoryValue,
    "set number of pattern", 0);
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
    // int pid=(int) getpid();
    // char buf[1024];
    // sprintf(buf, "cat /proc/%d/statm",pid);
    // system(buf);
    // system("ps aux | grep atpg");
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
    else if (option.retrieve("mod_logicsim"))
    {
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.LogicSimVectorsmod();
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
    else if (option.retrieve("pattern")) {
        string pattern_num=option.retrieve("num");
        int No_pattern;
        stringstream ss(pattern_num);
        ss >> No_pattern;
        string file_name=option.retrieve("output");
        //cout<<file_name<<" "<<pattern_num<<endl;
        if(option.retrieve("unknown")){
            pattern_generate(file_name,No_pattern,1);
        }
        else{
            pattern_generate(file_name,No_pattern,0);
        }
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
void pattern_generate(string file_name,int number,bool check)
{
    int signal=0;
    unsigned i = 0;
    GATE* gptr;
    ofstream outfile;
    outfile.open(file_name.c_str());
    if (!outfile.is_open()) {
        cerr << "Can't open file!" << endl;
    }
    for (;i < Circuit.No_Gate();i++) {
        gptr = Circuit.Gate(i);
        if(gptr->GetFunction()==G_PI)
        {
            outfile<<"PI "<<gptr->GetName()<<" ";
            signal++;
        }
    }
    outfile<<endl;
    srand(static_cast<unsigned>(time(0)));
    for (int i=0;i<number;i++)
    {
        for (int j=0;j<signal;j++)
        {
            outfile<< random_value(check);
        }
        outfile<<endl;
    }
}
char random_value(bool check) {
    int random_num = 0;
    if(check) random_num = rand() % 3;
    else random_num = rand() % 2;
    switch (random_num) {
        case 0:
            return '0';
        case 1:
            return '1';
        case 2:
            return 'X';
        default:
            return '0';
    }
}
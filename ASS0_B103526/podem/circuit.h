#ifndef CIRCUIT_H
#define CIRCUIT_H
#include "fault.h"
#include "tfault.h"
#include "ReadPattern.h"
#include <stdlib.h>

typedef GATE* GATEPTR;

class CIRCUIT
{
    private:
        unsigned PI,PO,PPI,PPO,NOT,AND,NAND,OR,NOR,DFF,MergeGate,Signal_one,stemNO,branchNO;
        unsigned not_fanout,and_fanout,nand_fanout,or_fanout,nor_fanout;
        string Name;
        PATTERN Pattern;
        vector<GATE*> Netlist;
        vector<GATE*> PIlist; //store the gate indexes of PI
        vector<GATE*> POlist;
        vector<GATE*> PPIlist;
        vector<GATE*> PPOlist;
        list<FAULT*> Flist; //collapsing fault list
        list<FAULT*> UFlist; //undetected fault list
        list<TFAULT*> TFlist; //collapsing fault list
        list<TFAULT*> UTFlist; //undetected fault list
        unsigned MaxLevel;
        unsigned BackTrackLimit; //backtrack limit for Podem
        typedef list<GATE*> ListofGate;
        typedef list<GATE*>::iterator ListofGateIte;
        ListofGate* Queue;
        ListofGate GateStack;
        ListofGate PropagateTree;
        ListofGateIte QueueIte;

    public:
        //NEW FOR ASS0
        unsigned getNOT() const { return NOT; }
        unsigned getOR() const { return OR; }
        unsigned getNOR() const { return NOR; }
        unsigned getAND() const { return AND; }
        unsigned getNAND() const { return NAND; }
        unsigned getDFF() const { return DFF; }
        unsigned getMerge() const{return MergeGate;}
        unsigned signal() const{return Signal_one;}
        unsigned stemNumber() const{return stemNO;}
        unsigned branchNumber() const{return branchNO;}
        unsigned getnotf() const{return not_fanout;}
        unsigned getandf() const{return and_fanout;}
        unsigned getnandf() const{return nand_fanout;}
        unsigned getorf() const{return or_fanout;}
        unsigned getnorf() const{return nor_fanout;}
        //Initialize netlist
        CIRCUIT(): PI(0), PO(0), PPI(0), PPO(0), NOT(0), AND(0), NAND(0), OR(0), NOR(0), DFF(0), MergeGate(0), Signal_one(0), stemNO(0), branchNO(0),
            not_fanout(0),and_fanout(0),nand_fanout(0),or_fanout(0),nor_fanout(0),MaxLevel(0), BackTrackLimit(10000) {
            Netlist.reserve(32768);
            PIlist.reserve(128);
            POlist.reserve(512);
            PPIlist.reserve(2048);
            PPOlist.reserve(2048);
        }
        CIRCUIT(unsigned NO_GATE, unsigned NO_PI = 128, unsigned NO_PO = 512,
                unsigned NO_PPI = 2048, unsigned NO_PPO = 2048) {
            Netlist.reserve(NO_GATE);
            PIlist.reserve(NO_PI);
            POlist.reserve(NO_PO);
            PPIlist.reserve(NO_PPI);
            PPOlist.reserve(NO_PPO);
        }
        ~CIRCUIT() {
            for (unsigned i = 0;i<Netlist.size();++i) { delete Netlist[i]; }
            list<FAULT*>::iterator fite;
            for (fite = Flist.begin();fite!=Flist.end();++fite) { delete *fite; }
        }

        void AddGate(GATE* gptr) { Netlist.push_back(gptr); }
        void SetName(string n){ Name = n;}
        string GetName(){ return Name;}
        int GetMaxLevel(){ return MaxLevel;}
        void SetBackTrackLimit(unsigned bt) { BackTrackLimit = bt; }

        //Access the gates by indexes
        GATE* Gate(unsigned index) { return Netlist[index]; }
        GATE* PIGate(unsigned index) { return PIlist[index]; }
        GATE* POGate(unsigned index) { return POlist[index]; }
        GATE* PPIGate(unsigned index) { return PPIlist[index]; }
        GATE* PPOGate(unsigned index) { return PPOlist[index]; }
        unsigned No_Gate() { return Netlist.size(); }
        unsigned No_PI() { return PIlist.size(); }
        unsigned No_PO() { return POlist.size(); }
        unsigned No_PPI() { return PPIlist.size(); }
        unsigned No_PPO() { return PPOlist.size(); }

        void InitPattern(const char *pattern) {
            Pattern.Initialize(const_cast<char *>(pattern), PIlist.size(), "PI");
        }

        void Schedule(GATE* gptr)
        {
            if (!gptr->GetFlag(SCHEDULED)) {
                gptr->SetFlag(SCHEDULED);
                Queue[gptr->GetLevel()].push_back(gptr);
            }
        }

        //defined in circuit.cc
        void Levelize();
        void FanoutList();
        void Check_Levelization();
        void SetMaxLevel();
        void SetupIO_ID();

        //defined in sim.cc
        void SetPPIZero(); //Initialize PPI state
        void InitializeQueue();
        void ScheduleFanout(GATE*);
        void SchedulePI();
        void SchedulePPI();
        void LogicSimVectors();
        void LogicSim();
        void PrintIO();
        VALUE Evaluate(GATEPTR gptr);

        //defined in atpg.cc
        void GenerateAllFaultList();
        void GenerateCheckPointFaultList();
        void GenerateFaultList();
        void Atpg();
        void SortFaninByLevel();
        bool CheckTest();
        bool TraceUnknownPath(GATEPTR gptr);
        bool FaultEvaluate(FAULT* fptr);
        ATPG_STATUS Podem(FAULT* fptr, unsigned &total_backtrack_num);
        ATPG_STATUS SetUniqueImpliedValue(FAULT* fptr);
        ATPG_STATUS BackwardImply(GATEPTR gptr, VALUE value);
        GATEPTR FindPropagateGate();
        GATEPTR FindHardestControl(GATEPTR gptr);
        GATEPTR FindEasiestControl(GATEPTR gptr);
        GATEPTR FindPIAssignment(GATEPTR gptr, VALUE value);
        GATEPTR TestPossible(FAULT* fptr);
        void TraceDetectedStemFault(GATEPTR gptr, VALUE val);

        //defined in fsim.cc
        void MarkOutputGate();
        void MarkPropagateTree(GATEPTR gptr);
        void FaultSimVectors();
        void FaultSim();
        void FaultSimEvaluate(GATE* gptr);
        bool CheckFaultyGate(FAULT* fptr);
        void InjectFaultValue(GATEPTR gptr, unsigned idx,VALUE value);

	//defined in psim.cc for parallel logic simulation
	void ParallelLogicSimVectors();
	void ParallelLogicSim();
	void ParallelEvaluate(GATEPTR gptr);
	void PrintParallelIOs(unsigned idx);
	void ScheduleAllPIs();

	//defined in stfsim.cc for single pattern single transition-fault simulation
	void GenerateAllTFaultList();
	void TFaultSimVectors();
	void TFaultSim_t();
	void TFaultSim();
	bool CheckTFaultyGate(TFAULT* fptr);
	bool CheckTFaultyGate_t(TFAULT* fptr);
	VALUE Evaluate_t(GATEPTR gptr);
	void LogicSim_t();
        void PrintTransition();
        void PrintTransition_t();
        void PrintIO_t();

	//defined in tfatpg.cc for transition fault ATPG
	void TFAtpg();
	ATPG_STATUS Initialization(GATEPTR gptr, VALUE target, unsigned &total_backtrack_num);
	ATPG_STATUS BackwardImply_t(GATEPTR gptr, VALUE value);
	GATEPTR FindPIAssignment_t(GATEPTR gptr, VALUE value);
	GATEPTR FindEasiestControl_t(GATEPTR gptr);
	GATEPTR FindHardestControl_t(GATEPTR gptr);
};
#endif

#ifndef FAULT_H
#define FAULT_H
#include "gate.h"

class FAULT
{
    private:
        VALUE Value;
        GATE* Input;
        GATE* Output; //record output gate for branch fault
        //if stem, Input = Output
        bool Branch; //fault is on branch
        unsigned EqvFaultNum; //equivalent fault number (includes itself)
        FAULT_STATUS Status;
    public:
        FAULT(GATE* gptr, GATE* ogptr, VALUE value): Value(value), Input(gptr),
        Output(ogptr), Branch(false), EqvFaultNum(1), Status(UNKNOWN) {}
        ~FAULT() {}
        VALUE GetValue() { return Value; }
        GATE* GetInputGate() { return Input; }
        GATE* GetOutputGate() { return Output; }
        void SetBranch(bool b) { Branch = b; }
        bool Is_Branch() { return Branch; }
        void SetEqvFaultNum(unsigned n) { EqvFaultNum = n; }
        void IncEqvFaultNum() { ++EqvFaultNum; }
        unsigned GetEqvFaultNum() { return EqvFaultNum; }
        void SetStatus(FAULT_STATUS status) { Status = status; }
        FAULT_STATUS GetStatus() { return Status; }
};


class BridgingFAULT
{
    private:
        Bridging bridge_Value;
        GATE* n0;
        GATE* n1; //record output gate for branch fault
        //if stem, Input = Output
        bool Branch; //fault is on branch
        unsigned EqvFaultNum; //equivalent fault number (includes itself)
        FAULT_STATUS Status;
    public:
        BridgingFAULT(GATE* gptr, GATE* ogptr, Bridging bridge): bridge_Value(bridge), n0(gptr),
        n1(ogptr), Branch(false), EqvFaultNum(1), Status(UNKNOWN) {}
        ~BridgingFAULT() {}
        Bridging GetType() { return bridge_Value; }
        GATE* Getn0() { return n0; }
        GATE* Getn1() { return n1; }
        void SetBranch(bool b) { Branch = b; }
        bool Is_Branch() { return Branch; }
        void SetEqvFaultNum(unsigned n) { EqvFaultNum = n; }
        void IncEqvFaultNum() { ++EqvFaultNum; }
        unsigned GetEqvFaultNum() { return EqvFaultNum; }
        void SetStatus(FAULT_STATUS status) { Status = status; }
        FAULT_STATUS GetStatus() { return Status; }
};
#endif

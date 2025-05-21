#include <iostream> 
//#include <alg.h>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

void CIRCUIT::FanoutList()
{
    unsigned i = 0, j;
    GATE* gptr;
    for (;i < No_Gate();i++) {
        gptr = Gate(i);
        //cout<<gptr->GetName()<<endl;
        for (j = 0;j < gptr->No_Fanin();j++) {
            gptr->Fanin(j)->AddOutput_list(gptr);
            //cout<<gptr->Fanin(j)->GetName()<<" ";
        }
        //cout<<endl;
    }
}

void CIRCUIT::Levelize()
{
    list<GATE*> Queue;
    GATE* gptr;
    GATE* out;
    unsigned j = 0;
    for (unsigned i = 0;i < No_PI();i++) {
        gptr = PIGate(i);
        gptr->SetLevel(0);
        for (j = 0;j < gptr->No_Fanout();j++) {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI) {
                out->IncCount();
                if (out->GetCount() == out->No_Fanin()) {
                    out->SetLevel(1);
                    Queue.push_back(out);
                }
            }
        }
    }
    for (unsigned i = 0;i < No_PPI();i++) {
        gptr = PPIGate(i);
        gptr->SetLevel(0);
        for (j = 0;j < gptr->No_Fanout();j++) {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI) {
                out->IncCount();
                if (out->GetCount() ==
                        out->No_Fanin()) {
                    out->SetLevel(1);
                    Queue.push_back(out);
                }
            }
        }
    }
    int l1, l2;
    while (!Queue.empty()) {
        gptr = Queue.front();
        Queue.pop_front();
        l2 = gptr->GetLevel();
        for (j = 0;j < gptr->No_Fanout();j++) {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI) {
                l1 = out->GetLevel();
                if (l1 <= l2)
                    out->SetLevel(l2 + 1);
                out->IncCount();
                if (out->GetCount() ==
                        out->No_Fanin()) {
                    Queue.push_back(out);
                }
            }
        }
    }
    for (unsigned i = 0;i < No_Gate();i++) {
        Gate(i)->ResetCount();
    }
}

void CIRCUIT::Check_Levelization()
{

    GATE* gptr;
    GATE* in;
    unsigned i, j;
    for (i = 0;i < No_Gate();i++) {
        gptr = Gate(i);
        if (gptr->GetFunction() == G_PI) {
            if (gptr->GetLevel() != 0) {
                cout << "Wrong Level for PI : " <<
                gptr->GetName() << endl;
                exit( -1);
            }
        }
        else if (gptr->GetFunction() == G_PPI) {
            if (gptr->GetLevel() != 0) {
                cout << "Wrong Level for PPI : " <<
                gptr->GetName() << endl;
                exit( -1);
            }
        }
        else {
            for (j = 0;j < gptr->No_Fanin();j++) {
                in = gptr->Fanin(j);
                if (in->GetLevel() >= gptr->GetLevel()) {
                    cout << "Wrong Level for: " <<
                    gptr->GetName() << '\t' <<
                    gptr->GetID() << '\t' <<
                    gptr->GetLevel() <<
                    " with fanin " <<
                    in->GetName() << '\t' <<
                    in->GetID() << '\t' <<
                    in->GetLevel() <<
                    endl;
                }
            }
        }
    }
}

void CIRCUIT::SetMaxLevel()
{
    for (unsigned i = 0;i < No_Gate();i++) {
        if (Gate(i)->GetLevel() > MaxLevel) {
            MaxLevel = Gate(i)->GetLevel();
        }
    }
}

//Setup the Gate ID and Inversion
//Setup the list of PI PPI PO PPO
void CIRCUIT::SetupIO_ID()
{
    unsigned i = 0;
    GATE* gptr;
    vector<GATE*>::iterator Circuit_ite = Netlist.begin();
    for (; Circuit_ite != Netlist.end();Circuit_ite++, i++) {
        gptr = (*Circuit_ite);
        gptr->SetID(i);
        switch (gptr->GetFunction()) {
            case G_PI: PIlist.push_back(gptr);
                PI++;
                if(gptr->No_Fanout()>=static_cast<unsigned int>(2)) 
                {
                    branchNO+=gptr->No_Fanout();
                    stemNO++;
                }
                else if(gptr->No_Fanout()==static_cast<unsigned int>(1)) Signal_one++;
                break;
            case G_PO: POlist.push_back(gptr);
                PO++;
                if(gptr->No_Fanout()>=static_cast<unsigned int>(2)) 
                {
                    branchNO+=gptr->No_Fanout();
                    stemNO++;
                }
                else if(gptr->No_Fanout()==static_cast<unsigned int>(1)) Signal_one++;
                break;
            case G_PPI: PPIlist.push_back(gptr);
                //cout<<gptr->GetName()<<endl;
                PPI++;
                DFF++;
                if(gptr->No_Fanout()>=static_cast<unsigned int>(2)) 
                {
                    branchNO+=gptr->No_Fanout();
                    stemNO++;
                }
                else if(gptr->No_Fanout()==static_cast<unsigned int>(1)) Signal_one++;
                break;
            case G_PPO: PPOlist.push_back(gptr);
                if(gptr->No_Fanout()>=static_cast<unsigned int>(2)) 
                {
                    branchNO+=gptr->No_Fanout();
                    stemNO++;
                }
                else if(gptr->No_Fanout()==static_cast<unsigned int>(1)) Signal_one++;
                break;
            case G_NOT: gptr->SetInversion();
                NOT++;
                not_fanout+=gptr->No_Fanout();
                if(gptr->No_Fanout()>=static_cast<unsigned int>(2)) 
                {
                    branchNO+=gptr->No_Fanout();
                    stemNO++;
                }
                else if(gptr->No_Fanout()==static_cast<unsigned int>(1)) Signal_one++;
                break;
            case G_AND:
                AND++;
                and_fanout+=gptr->No_Fanout();
                if(gptr->No_Fanout()>=static_cast<unsigned int>(2)) 
                {
                    branchNO+=gptr->No_Fanout();
                    stemNO++;
                }
                else if(gptr->No_Fanout()==static_cast<unsigned int>(1)) Signal_one++;
                break;
            case G_NAND: gptr->SetInversion();
                NAND++;
                nand_fanout+=gptr->No_Fanout();
                if(gptr->No_Fanout()>=static_cast<unsigned int>(2)) 
                {
                    branchNO+=gptr->No_Fanout();
                    stemNO++;
                }
                else if(gptr->No_Fanout()==static_cast<unsigned int>(1)) Signal_one++;
                break;
            case G_OR:
                OR++;
                or_fanout+=gptr->No_Fanout();
                if(gptr->No_Fanout()>=static_cast<unsigned int>(2)) 
                {
                    branchNO+=gptr->No_Fanout();
                    stemNO++;
                }
                else if(gptr->No_Fanout()==static_cast<unsigned int>(1)) Signal_one++;
                break;
            case G_NOR: gptr->SetInversion();
                NOR++;
                nor_fanout+=gptr->No_Fanout();
                if(gptr->No_Fanout()>=static_cast<unsigned int>(2)) 
                {
                    branchNO+=gptr->No_Fanout();
                    stemNO++;
                }
                else if(gptr->No_Fanout()==static_cast<unsigned int>(1)) Signal_one++;
                break;
            case G_BUF:
                if(gptr->No_Fanout()>=static_cast<unsigned int>(2)) 
                {
                    branchNO+=gptr->No_Fanout();
                    stemNO++;
                }
                else if(gptr->No_Fanout()==static_cast<unsigned int>(1)) Signal_one++;
                break;
            default:
                break;
        }
    }
    MergeGate=NOT+OR+NAND+AND+NOR;
}
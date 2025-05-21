#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;
unsigned gate_eval=0;
// Event-driven Parallel Pattern Logic simulation
void CIRCUIT::ParallelLogicSimVectors()
{
    cout << "Run Parallel Logic simulation" << endl;
    unsigned pattern_num(0);
    unsigned pattern_idx(0);
    while(!Pattern.eof()){ 
	for(pattern_idx=0; pattern_idx<PatternNum; pattern_idx++){
	    if(!Pattern.eof()){ 
	        ++pattern_num;
	        Pattern.ReadNextPattern(pattern_idx);
	    }
	    else break;
	}
	ScheduleAllPIs();
	ParallelLogicSim();
	//PrintParallelIOs(pattern_idx);
    }
    cout<<"gate evaluation number: "<<gate_eval<<endl;
    cout<<"gate evaluation number/pattern: "<<(double)gate_eval/10000<<endl;
    cout<<"number of gate: "<<No_Gate()<<endl;
    cout<<"gate evaluation number/number of gates: "<<(double)gate_eval/(No_Gate()*10000)<<endl;
}

//Assign next input pattern to PI's idx'th bits
void PATTERN::ReadNextPattern(unsigned idx)
{
    char V;
    for (int i = 0; i < no_pi_infile; i++) {
        patterninput >> V;
        if (V == '0') {
            inlist[i]->ResetWireValue(0, idx);
            inlist[i]->ResetWireValue(1, idx);
        }
        else if (V == '1') {
            inlist[i]->SetWireValue(0, idx);
            inlist[i]->SetWireValue(1, idx);
        }
        else if (V == 'X') {
            inlist[i]->SetWireValue(0, idx);
            inlist[i]->ResetWireValue(1, idx);
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();
    return;
}

//Simulate PatternNum vectors
void CIRCUIT::ParallelLogicSim()
{
    GATE* gptr;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            ParallelEvaluate(gptr);
        }
    }
    return;
}
// NEW Simulate PatternNum vectors
void CIRCUIT::MyParallelLogicSim(fstream &outfile)
{
    GATE* gptr;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            MyParallelEvaluate(gptr,outfile);
        }
    }
    return;
}
// NEW Simulate Patternevaluate
void CIRCUIT::MyParallelEvaluate(GATEPTR gptr,fstream &outfile)
{
    register unsigned i;
    bitset<PatternNum> new_value1(gptr->Fanin(0)->GetValue1());
    bitset<PatternNum> new_value2(gptr->Fanin(0)->GetValue2());
    outfile<<"G_"<<gptr->GetName()<<"[0] = G_"<<gptr->Fanin(0)->GetName()<<"[0] ;"<<endl;
    outfile<<"G_"<<gptr->GetName()<<"[1] = G_"<<gptr->Fanin(0)->GetName()<<"[1] ;"<<endl;
    switch(gptr->GetFunction()) {
        case G_AND:
        case G_NAND:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 &= gptr->Fanin(i)->GetValue1();
                outfile<< "G_" << gptr->GetName() << "[0] &= " << "G_" << gptr->Fanin(i)->GetName() << "[0] ;" << endl;
                new_value2 &= gptr->Fanin(i)->GetValue2();
                outfile<< "G_" << gptr->GetName() << "[1] &= " << "G_" << gptr->Fanin(i)->GetName() << "[1] ;" << endl;
            }
            break;
        case G_OR:
        case G_NOR:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 |= gptr->Fanin(i)->GetValue1();
                outfile<< "G_" << gptr->GetName() << "[0] |= " << "G_" << gptr->Fanin(i)->GetName() << "[0] ;" << endl;
                new_value2 |= gptr->Fanin(i)->GetValue2();
                outfile<< "G_" << gptr->GetName() << "[1] |= " << "G_" << gptr->Fanin(i)->GetName() << "[1] ;" << endl;
            }
            break;
        default: break;
    } 
    //swap new_value1 and new_value2 to avoid unknown value masked
    if (gptr->Is_Inversion()) {
        new_value1.flip(); new_value2.flip();
        bitset<PatternNum> value(new_value1);
        outfile<< "temp = " << "G_" << gptr->GetName() << "[0] ;"<< endl;
        new_value1 = new_value2; new_value2 = value;
        outfile << "G_" << gptr->GetName() << "[0] = ~" << "G_" << gptr->GetName() << "[1] ;"<< endl;
        outfile << "G_" << gptr->GetName() << "[1] = " << "~temp ;"<< endl;
    }
    if (gptr->GetValue1() != new_value1 || gptr->GetValue2() != new_value2) {
        gptr->SetValue1(new_value1);
        gptr->SetValue2(new_value2);
        ScheduleFanout(gptr);
    }
    gate_eval++;
    return;
}




//Evaluate parallel value of gptr
void CIRCUIT::ParallelEvaluate(GATEPTR gptr)
{
    register unsigned i;
    bitset<PatternNum> new_value1(gptr->Fanin(0)->GetValue1());
    bitset<PatternNum> new_value2(gptr->Fanin(0)->GetValue2());
    switch(gptr->GetFunction()) {
        case G_AND:
        case G_NAND:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 &= gptr->Fanin(i)->GetValue1();
                new_value2 &= gptr->Fanin(i)->GetValue2();
            }
            break;
        case G_OR:
        case G_NOR:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 |= gptr->Fanin(i)->GetValue1();
                new_value2 |= gptr->Fanin(i)->GetValue2();
            }
            break;
        default: break;
    } 
    //swap new_value1 and new_value2 to avoid unknown value masked
    if (gptr->Is_Inversion()) {
        new_value1.flip(); new_value2.flip();
        bitset<PatternNum> value(new_value1);
        new_value1 = new_value2; new_value2 = value;
    }
    if (gptr->GetValue1() != new_value1 || gptr->GetValue2() != new_value2) {
        gptr->SetValue1(new_value1);
        gptr->SetValue2(new_value2);
        ScheduleFanout(gptr);
    }
    gate_eval++;
    return;
}

void CIRCUIT::PrintParallelIOs(unsigned idx)
{
    register unsigned i;
    for (unsigned j=0; j<idx; j++){
	    for (i = 0;i<No_PI();++i) { 
		    if(PIGate(i)->GetWireValue(0, j)==0){ 
			   if(PIGate(i)->GetWireValue(1, j)==1){
	    			cout << "F";
			   }
			   else cout << "0";
		    }
		    else{
			   if(PIGate(i)->GetWireValue(1, j)==1){
	    			cout << "1";
			   }
			   else cout << "X";
		    }

	    }
	    cout << " ";
	    for (i = 0;i<No_PO();++i) { 
		    if(POGate(i)->GetWireValue(0, j)==0){ 
			   if(POGate(i)->GetWireValue(1, j)==1){
	    			cout << "F";
			   }
			   else cout << "0";
		    }
		    else{
			   if(POGate(i)->GetWireValue(1, j)==1){
	    			cout << "1";
			   }
			   else cout << "X";
		    }
	    }
	    cout << endl;
    }
    return;
}

void CIRCUIT::ScheduleAllPIs()
{
    for (unsigned i = 0;i < No_PI();i++) {
        ScheduleFanout(PIGate(i));
    }
    return;
}

void CIRCUIT::Simulator(string filename)
{
    fstream outfile;
    outfile.open(filename.c_str(), ios::out);
    outfile<<"#include <iostream>"<<endl;
    outfile<<"#include <ctime>"<<endl;
    outfile<<"#include <bitset>"<<endl;
    outfile<<"#include <string>"<<endl;
    outfile<<"#include <cstdlib>"<<endl;
    outfile<<"#include <fstream>"<<endl<<endl;
    outfile<<"using namespace std;"<<endl;
    outfile<<"const unsigned PatternNum = 16;"<<endl<<endl;
    outfile<<"void evaluate();"<<endl;
    outfile<<"void printIO(unsigned idx);"<<endl<<endl;
    for (unsigned i=0;i<No_Gate();i++)
    {
        outfile<<"bitset<PatternNum> G_"<<Gate(i)->GetName()<<"[2];"<<endl;
    }
    outfile<<"bitset<PatternNum> temp;" << endl;
    outfile<< "ofstream fout(\"" << GetName() << ".out" << "\",ios::out);" <<endl<<endl;
    outfile<<"int main()"<<endl<<"{"<<endl;
    outfile<<"clock_t time_init, time_end;"<<endl;
    outfile<<"time_init = clock();"<<endl;
    unsigned pattern_num(0);
    unsigned pattern_idx(0);
    while(!Pattern.eof()){ 
        for(pattern_idx=0; pattern_idx<PatternNum; pattern_idx++){
            if(!Pattern.eof()){ 
                ++pattern_num;
                Pattern.ReadNextPattern(pattern_idx);
            }
            else break;
	    }
        for (unsigned j=0;j<No_PI();j++)
        {
            outfile<<"G_"<<PIGate(j)->GetName() << "[0] = " << (int)(PIGate(j)->GetValue1().to_ulong()) << " ;" << endl;
            outfile<<"G_"<<PIGate(j)->GetName() << "[1] = " << (int)(PIGate(j)->GetValue2().to_ulong()) << " ;" << endl;
        }
        outfile<<endl;
        outfile<<"evaluate();"<<endl;
        if((pattern_num)%16==0) 
        {
            outfile<<"printIO("<<16<<");"<<endl<<endl;
        }
        else 
        {
            outfile<<"printIO("<<pattern_num%16<<");"<<endl<<endl;
        }
    }
    outfile<<"time_end = clock();"<<endl;
    outfile<<"cout << \"Total CPU Time = \" << double(time_end - time_init)/CLOCKS_PER_SEC << endl;"<<endl;
    outfile<<"system(\"ps aux | grep a.out \");"<<endl;
    outfile<<"return 0;"<<endl<<"}"<<endl;
    ScheduleAllPIs();
    outfile<<"void evaluate()"<<endl<<"{"<<endl;
    MyParallelLogicSim(outfile);
    outfile<<"}"<<endl;
    
    outfile << "void printIO(unsigned idx)"<<endl<<"{"<<endl;
    outfile << "for (unsigned j=0; j<idx; j++)"<<endl<<"{";
    for(unsigned i=0; i<No_PI(); i++) {

        outfile << "        if(" << "G_" << PIGate(i)->GetName() << "[0][j]==0)"<<endl;
        outfile << "        {"<<endl;
        outfile << "            if(" << "G_" << PIGate(i)->GetName() << "[1][j]==1)"<<endl;
        outfile << "                fout<<\"F\";"<<endl;
        outfile << "            else"<<endl;
        outfile << "                fout<<\"0\";"<<endl;
        outfile << "        }"<<endl;
        outfile << "        else"<<endl;
        outfile << "        {"<<endl;
        outfile << "            if(" << "G_" << PIGate(i)->GetName() << "[1][j]==1)"<<endl;
        outfile << "                fout<<\"1\";"<<endl;
        outfile << "            else"<<endl;
        outfile << "                fout<<\"2\";"<<endl;
        outfile << "        }"<<endl;
    }
    outfile << "fout<<\" \";"<<endl;
    for(unsigned i=0; i<No_PO(); i++) {
        outfile << "        if(" << "G_" << POGate(i)->GetName() << "[0][j]==0)"<<endl;
        outfile << "        {"<<endl;
        outfile << "            if(" << "G_" << POGate(i)->GetName() << "[1][j]==1)"<<endl;
        outfile << "                fout<<\"F\";"<<endl;
        outfile << "            else"<<endl;
        outfile << "                fout<<\"0\";"<<endl;
        outfile << "        }"<<endl;
        outfile << "        else"<<endl;
        outfile << "        {"<<endl;
        outfile << "            if(" << "G_" << POGate(i)->GetName() << "[1][j]==1)"<<endl;
        outfile << "                fout<<\"1\";"<<endl;
        outfile << "            else"<<endl;
        outfile << "                fout<<\"2\";"<<endl;
        outfile << "        }"<<endl;
    }
    outfile << "fout << endl;"<<endl;
    outfile << "}"<<endl;
    outfile << "}"<<endl;
    outfile.close();
}


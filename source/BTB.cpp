
#include <BTB.h>

void BTB::InitPrecditor()
{
    State S00("00", NOT_TAKEN);
    State S01("01", NOT_TAKEN);
    State S10("10", TAKEN);
    State S11("11", TAKEN);

    /* add S00 next state */
    S00.AddNextState ("1", "01");
    S00.AddNextState ("0", "00");

    /* add S01 next state */
    S01.AddNextState ("1", "11");
    S01.AddNextState ("0", "00");

    /* add S10 next state */
    S10.AddNextState ("1", "11");
    S10.AddNextState ("0", "00");

    /* add S11 next state */
    S11.AddNextState ("1", "11");
    S11.AddNextState ("0", "10");


    m_Predictor.AddState (S00);
    m_Predictor.AddState (S01);
    m_Predictor.AddState (S10);
    m_Predictor.AddState (S11);

    return;    
}


void BTB::InitSlector()
{
    State S00("00", CORRELATOR);
    State S01("01", CORRELATOR);
    State S10("10", NON_CORRELATOR);
    State S11("11", NON_CORRELATOR);

    /* add S00 next state */
    S00.AddNextState ("00", "00");
    S00.AddNextState ("01", "00");
    S00.AddNextState ("11", "00");
    S00.AddNextState ("10", "01");

    /* add S01 next state */
    S01.AddNextState ("00", "01");
    S01.AddNextState ("11", "01");
    S01.AddNextState ("01", "00");
    S01.AddNextState ("10", "10");

    /* add S10 next state */
    S10.AddNextState ("00", "10");
    S10.AddNextState ("11", "10");    
    S10.AddNextState ("10", "11");
    S10.AddNextState ("01", "01");

    /* add S11 next state */
    S11.AddNextState ("00", "11");
    S11.AddNextState ("10", "11");
    S11.AddNextState ("01", "10");
    S11.AddNextState ("11", "11");


    /* init state machine */
    m_Slector.AddState (S00);
    m_Slector.AddState (S01);
    m_Slector.AddState (S10);
    m_Slector.AddState (S11);

    return;    
}


BtbLine* BTB::GetLineById(unsigned Id)
{
    auto it = m_BtbBuffer.find(Id);
    assert (it != m_BtbBuffer.end());

    return &(it->second);
}


bool BTB::IsCollision (unsigned PC)
{
    unsigned Id = PC2Id(PC);

    BtbLine* Line = GetLineById(Id);
    assert (Line != NULL);

    if (Line->GetPc() != PC && Line->GetPc() != 0)
    {
        //printf ("Collision: %u - [%x, %x] \r\n", Id, PC, Line->GetPc());
        return true;
    }

    return false;
}


unsigned BTB::Query(unsigned PC, unsigned &Taken)
{
    unsigned Id = PC2Id(PC);

    BtbLine* Line = GetLineById(Id);
    assert (Line != NULL);

    if (Line->GetPc() != PC)
    {
        Taken = NOT_TAKEN;       
        return 0;        
    }
    else
    {
        //printf("\tQuery  => BTB[%-4u]PC:%#x, hit, ", Id, Line->GetPc());
        IncreaseStat(E_STAT_HIT);

        switch (m_PType)
        {
            case P_LOCAL:
            {
                Taken = m_Predictor.GetStateValue(Line->GetLocalPredict ());
                break;
            }
            case P_GLOBAL:
            {
                Taken = m_Predictor.GetStateValue (Line->GetGlobalPredict (m_Correlator));
                break;
            }
            case P_CORRELATOR:
            {
                unsigned SL = m_Slector.GetStateValue (Line->GetSelector ());
                //printf("Slector:[%s][", Line->GetSelector ().c_str());
                
                if (SL == CORRELATOR)
                {
                    Taken = m_Predictor.GetStateValue (Line->GetGlobalPredict (m_Correlator));
                    //printf("CORRELATOR]:");
                }
                else
                {
                    Taken = m_Predictor.GetStateValue(Line->GetLocalPredict ());
                    //printf("NON_CORRELATOR]:");
                }
                break;
            }
            default:
            {
                assert(0 && "unknown predictor type!!!");
            }
        }
         

        if (Taken == TAKEN)
        {
            //printf("TAKEN: target=%#x\r\n", Line->GetTarget ());
        }
        else
        {
            //printf("NOT_TAKEN\r\n");
        }

        return Line->GetTarget ();
    }
}

void BTB::Update(unsigned PC, unsigned Target, unsigned ActualTaken)
{
    unsigned Id = PC2Id(PC);

    BtbLine* Line = GetLineById(Id);
    assert (Line != NULL);
      
    if (Line->GetPc() != PC)
    {
        /* not in BTB, replace it and initiate line */
        Line->SetPc (PC);
        Line->SetTarget (Target);
        
        Line->SetSelector ("10");
        
        Line->SetLocalPredict ("11");
        
        Line->SetGlobalPredict ("00", "11");
        Line->SetGlobalPredict ("01", "11");
        Line->SetGlobalPredict ("10", "11");
        Line->SetGlobalPredict ("11", "11");
 
        /* init correlator */
        m_Correlator = m_Correlator[1] + to_string(ActualTaken); 
    }
    else
    {
        /* in BTB:check the precdictor */
        unsigned SL = m_Slector.GetStateValue (Line->GetSelector ());       
        unsigned GP = m_Predictor.GetStateValue (Line->GetGlobalPredict (m_Correlator));
        unsigned LP = m_Predictor.GetStateValue (Line->GetLocalPredict ());
		
        string NextSelector = m_Slector.RunMachine (Line->GetSelector (),
                                                 to_string(LP==ActualTaken) + to_string(GP==ActualTaken));  
         
        string NextLState = m_Predictor.RunMachine (Line->GetLocalPredict (), to_string(ActualTaken)); 
        
        string NextGState     = m_Predictor.RunMachine (Line->GetGlobalPredict (m_Correlator), to_string(ActualTaken));
        string NextCorrelator = m_Correlator[1] + to_string(ActualTaken); 

        Line->SetGlobalPredict (m_Correlator, NextGState);
		m_Correlator = NextCorrelator;
		
        Line->SetLocalPredict (NextLState);
        Line->SetSelector (NextSelector);

        if (ActualTaken == TAKEN)
        {
            Line->SetTarget (Target);
        }
    }
    
#if 0
    printf("\tUpdate => BTB[%-4u]PC:%#x TARGET:%#x, Correlator:%s, Global:[00]%s [01]%s [10]%s [11]%s, Local:%s, Slector:%s\r\n",
           Id, Line->GetPc (), Line->GetTarget(), m_Correlator.c_str(),
           Line->GetGlobalPredict("00").c_str(), Line->GetGlobalPredict("01").c_str(),  Line->GetGlobalPredict("10").c_str(),  Line->GetGlobalPredict("11").c_str(),
           Line->GetLocalPredict ().c_str(), Line->GetSelector ().c_str());
#endif
    return;
}

void BTB::IncreaseStat(string Name)
{
    m_StatInfo.AddStat (Name);
}

float BTB::GetHitRate()
{
    unsigned Hit  = m_StatInfo.GetStat (E_STAT_HIT);
    unsigned Miss = m_StatInfo.GetStat (E_STAT_MISS);
    if (Hit+Miss == 0)
    {
        return 0;
    }
    
    return (Hit*1.0/(Miss+Hit));
}

float BTB::GetAccuracy()
{
    unsigned Hit  = m_StatInfo.GetStat (E_STAT_HIT);
    if (Hit == 0)
    {
        return 0;
    }

    unsigned Right  = m_StatInfo.GetStat (E_STAT_R_PREDICT);
    return (Right*1.0)/Hit;
}

void BTB::PrintLocal()
{
    unsigned Id;
    FILE *f = fopen("Local_Predict.txt", "w");
    assert(f != NULL);
    
    fprintf(f, "%-6s\t%-6s\t%-6s\t%-10s\r\n", "Entry", "PC", "Target", "Local_Pred");
    for (Id = 0; Id < m_Size; Id++)
    {
        BtbLine &Line = m_BtbBuffer[Id];
        if (Line.GetPc () == 0)
        {
            continue;
        }
        
        fprintf(f, "%-6u\t%-6x\t%-6x\t%-10s\r\n", 
               Id, Line.GetPc (), Line.GetTarget (), Line.GetLocalPredict ().c_str());
    }

    fclose(f);
}

void BTB::PrintGlobal()
{
    unsigned Id;
    FILE *f = fopen("Global_Predict.txt", "w");
    assert(f != NULL);
    
    fprintf(f, "%-6s\t%-6s\t%-6s\t%-3s %-3s %-3s %-3s\r\n", 
           "Entry", "PC", "Target", "Global[00", "01", "10", "11]");
    for (Id = 0; Id < m_Size; Id++)
    {
        BtbLine &Line = m_BtbBuffer[Id];
        if (Line.GetPc () == 0)
        {
            continue;
        }
        
        fprintf(f, "%-6u\t%-6x\t%-6x\t %-6s%-3s%-3s %-3s %-3s\r\n", 
               Id, Line.GetPc (), Line.GetTarget (), "",
               Line.GetGlobalPredict ("00").c_str(), Line.GetGlobalPredict ("01").c_str(),
               Line.GetGlobalPredict ("10").c_str(), Line.GetGlobalPredict ("11").c_str());
    }

    fclose(f);
}

void BTB::PrintCorrelator()
{
    unsigned Id;
    FILE *f = fopen("Correlator_Predict.txt", "w");
    assert(f != NULL);
    
    fprintf(f, "%-6s\t%-6s\t%-6s\t%-6s\t%-3s %-3s %-3s %-3s\t%-8s\r\n", 
           "Entry", "PC", "Target", "Local", "Global[00", "01", "10", "11]", "Selector");
    for (Id = 0; Id < m_Size; Id++)
    {
        BtbLine &Line = m_BtbBuffer[Id];
        if (Line.GetPc () == 0)
        {
            continue;
        }
        
        fprintf(f, "%-6u\t%-6x\t%-6x\t%-6s\t %-6s%-3s%-3s %-3s %-3s\t%-8s\r\n", 
               Id, Line.GetPc (), Line.GetTarget (), Line.GetLocalPredict ().c_str(),"",
               Line.GetGlobalPredict ("00").c_str(), Line.GetGlobalPredict ("01").c_str(),
               Line.GetGlobalPredict ("10").c_str(), Line.GetGlobalPredict ("11").c_str(),
               Line.GetSelector ().c_str());
    }

    fclose(f);
}



void BTB::ShowStat ()
{
    printf("********************************************\r\n");
    printf("*                  result                  *\r\n");
    printf("********************************************\r\n");
    for (auto it = m_StatInfo.begin(); it != m_StatInfo.end(); it++)
    {
        printf("%-16s:%-4u \r\n", it->first.c_str(), it->second);
    }

    printf("%-16s:%-4.2f%% \r\n", "HitRate", 100*GetHitRate ());
    printf("%-16s:%-4.2f%% \r\n", "Accuracy", 100*GetAccuracy ());

    switch (m_PType)
    {
        case P_LOCAL:
        {
            PrintLocal();
            break;
        }
        case P_GLOBAL:
        {
            PrintGlobal ();
            break;
        }
        case P_CORRELATOR:
        {
            PrintCorrelator ();
            break;            
        }
    } 

    printf ("BTB size = %u \r\n", GetSize ());

    return;
}



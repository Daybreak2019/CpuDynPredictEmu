#include "Execute.h"

void Execute::ReadInst(string &InstPath)
{
    FILE *F = fopen (InstPath.c_str(), "r");
    assert (F != NULL);

    unsigned Inst;
	unsigned ReadNum = 0;
    while (!feof(F))
    {		
        if (fscanf(F, "%x", &Inst) != 1)
        {
            continue;
        }
        m_InstVector.push_back(Inst);
        ReadNum++;
        //printf("read inst:%x \r\n", Inst);
    }
    
	printf("read %u instructions\r\n", ReadNum);
    fclose(F);
}

void Execute::Run(vector<unsigned> &InstVector)
{
    unsigned CurPC;
    unsigned NextPC;
    unsigned PredictPC;
    unsigned PredictTaken;
    unsigned ActualTaken;
    
    unsigned InstNum = InstVector.size();
    
    for (unsigned Index = 0; Index < InstNum-1; Index++)
    {
        CurPC  = InstVector[Index];
        NextPC = InstVector[Index+1];
        
        printf("process inst:[%-6u/%-6u]\r", Index, InstNum);
        m_BTB->IncreaseStat (E_STAT_INST);
        
        PredictPC = m_BTB->Query(CurPC, PredictTaken);
        if (PredictPC == 0 && NextPC == CurPC+4)
        {
            continue;
        }

        /* it is a branch */
        m_BTB->IncreaseStat (E_STAT_BRANCH);

        /* Actual Branch */
        if (NextPC == CurPC+4)
        {
            ActualTaken = NOT_TAKEN;
            //printf("\tActual:NOT_TAKEN, NextPC:%#x, ", NextPC);
        }
        else
        {
            ActualTaken = TAKEN;
            //printf("\tActual:TAKEN, NextPC:%#x, ", NextPC);
            m_BTB->IncreaseStat (E_STAT_TAKEN_BRANCH);
        }
        
        if (PredictPC == 0)
        {
            /* not hit */
            m_BTB->IncreaseStat (E_STAT_MISS);
            if (m_BTB->IsCollision(CurPC))
            {
                m_BTB->IncreaseStat(E_STAT_COLLISION);
            }
            
            //printf("\r\n\tQuery  => PC:%#x, hit miss \r\n", CurPC);
        }
        else
        {       
            /* hit: check the prediction */
            if (PredictTaken == TAKEN)
            {
                if (PredictPC == NextPC)
                {
                    m_BTB->IncreaseStat (E_STAT_R_PREDICT);
                    //printf("Predict Right");
                }
                else
                {
                    m_BTB->IncreaseStat (E_STAT_W_PREDICT);
                    //printf("Predict Wrong");

                    if (PredictTaken == ActualTaken)
                    {
                        m_BTB->IncreaseStat (E_STAT_W_ADDRESS);
                        //printf("(Address)");
                    }
                }
            }
            else
            {
                if (NextPC == CurPC+4)
                {
                    m_BTB->IncreaseStat (E_STAT_R_PREDICT);
                    //printf("Predict Right");
                }
                else
                {
                    m_BTB->IncreaseStat (E_STAT_W_PREDICT);
                    //printf("Predict Wrong");
                }
            }  
            //printf("\r\n");
        }

        m_BTB->Update (CurPC, NextPC, ActualTaken);
        //printf("\r\n");
    }

    m_BTB->IncreaseStat (E_STAT_INST);
    printf("process inst:[%-6u/%-6u]\r\n", InstNum, InstNum);

    return;
}


void Execute::Run(string &InstPath)
{
    ReadInst(InstPath);

    Run(m_InstVector);

    return;
}


void Execute::Result()
{
    m_BTB->ShowStat ();    
}




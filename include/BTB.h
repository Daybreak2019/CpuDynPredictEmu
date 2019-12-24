
#ifndef _BTB_H_
#define _BTB_H_
#include <StateMachine.h>

enum
{
    NOT_TAKEN = 0,
    TAKEN,   
};

enum
{
    CORRELATOR = 0,
    NON_CORRELATOR
};

enum
{
    P_LOCAL = 0,
    P_GLOBAL,
    P_CORRELATOR
};


#define E_STAT_HIT        ("Hit")
#define E_STAT_MISS       ("Miss")
#define E_STAT_R_PREDICT  ("Right Prediction")
#define E_STAT_W_PREDICT  ("Wrong Prediction")
#define E_STAT_W_ADDRESS  ("Wrong Address")
#define E_STAT_COLLISION  ("Collisions")
#define E_STAT_BRANCH     ("Branchs")
#define E_STAT_INST       ("Instructions")
#define E_STAT_TAKEN_BRANCH       ("Taken Branch")


class Stat
{
private:
    map<string, unsigned> m_StatInfo;

public:
    Stat ()
    {
        m_StatInfo[E_STAT_HIT]       = 0;
        m_StatInfo[E_STAT_MISS]      = 0;
        m_StatInfo[E_STAT_R_PREDICT] = 0;
        m_StatInfo[E_STAT_W_PREDICT] = 0;
        m_StatInfo[E_STAT_W_ADDRESS] = 0;
        m_StatInfo[E_STAT_COLLISION] = 0;
        m_StatInfo[E_STAT_BRANCH]    = 0;
    }

    inline void AddStat(string Name)
    {
        m_StatInfo[Name]++;
    }

    inline unsigned GetStat(string Name)
    {
        auto it = m_StatInfo.find(Name);
        assert(it != m_StatInfo.end());

        return it->second;
    }

    map<string, unsigned>::iterator begin()
    {
        return m_StatInfo.begin();
    }

    map<string, unsigned>::iterator end()
    {
        return m_StatInfo.end();
    }
};


class BtbLine
{
private:
    unsigned m_Pc;
    unsigned m_Target;
    
    map<string, string> m_GlobalPredict;
    string m_LocalPredict;
    
    string m_Slector;

public:
    BtbLine ()
    {
        m_Pc = 0;
        m_Target = 0;

        m_GlobalPredict["00"] = "00";
        m_GlobalPredict["01"] = "00";
        m_GlobalPredict["10"] = "00";
        m_GlobalPredict["11"] = "00";
        
        m_LocalPredict = "";
        m_Slector      = "";
    }

    inline void SetPc(unsigned Pc)
    {
        m_Pc = Pc;
    }

    inline unsigned GetPc()
    {
        return m_Pc;
    }

    inline void SetTarget(unsigned Target)
    {
        m_Target = Target;
    }

    inline unsigned GetTarget()
    {
        return m_Target;
    }

    inline void SetSelector(string Selc)
    {
        m_Slector = Selc;
    }

    inline string GetSelector()
    {
        return m_Slector;
    }

    inline void SetLocalPredict(string LocalPredict)
    {
        m_LocalPredict = LocalPredict;
    }

    inline string GetLocalPredict()
    {
        return m_LocalPredict;
    }

    inline void SetGlobalPredict(string Correlator, string Precdict)
    {
        m_GlobalPredict[Correlator] = Precdict;
    }

    inline string GetGlobalPredict(string Correlator)
    {
        auto it  = m_GlobalPredict.find(Correlator);
        assert (it != m_GlobalPredict.end());

        return it->second;
    }
};

class BTB
{
private:
    map<unsigned, BtbLine> m_BtbBuffer;

    StateMachine m_Predictor;   
    StateMachine m_Slector;

    string m_Correlator;

    Stat m_StatInfo;
    
    unsigned m_PType;
    unsigned m_Size;

private:

    inline unsigned PC2Id(unsigned PC)
    {
        if (m_Size == 1024)
        {
            return ((PC & 0xfff) >> 2);
        }
        else if (m_Size == 2048)
        {
            return ((PC & 0xfff) >> 1);
        }
        else if (m_Size == 4096)
        {
            return (PC & 0xfff);
        }
        else if (m_Size == 8192)
        {
            return ((PC & 0xffff) >> 3);
        }
    }
    
    BtbLine* GetLineById(unsigned Id);
    
    void InitPrecditor();
    void InitSlector();

    float GetHitRate();
    float GetAccuracy();
    
public:
    BTB (unsigned Size, unsigned PType)
    {
        for (unsigned Id = 0; Id < Size; Id++)
        {
            BtbLine Btb;
            m_BtbBuffer[Id] = Btb;
        }

        InitPrecditor();
        InitSlector();

        m_Correlator = "00";

        m_PType = PType;
        m_Size  = Size;

        string Predict[3] = {"Local", "Global", "Correlator"};
        printf ("Apply %s predictor [%u].\r\n", Predict[m_PType].c_str(), m_Size);
    }

    void IncreaseStat(string Name);

    unsigned Query(unsigned PC, unsigned &Taken);

    void Update(unsigned PC, unsigned Target, unsigned ActualTaken);

    bool IsCollision (unsigned PC);

    void ShowStat ();

    map<string, unsigned>::iterator begin()
    {
        return m_StatInfo.begin();
    }

    map<string, unsigned>::iterator end()
    {
        return m_StatInfo.end();
    }

    void PrintLocal();
    void PrintGlobal();
    void PrintCorrelator();

    inline unsigned GetSize ()
    {
        switch (m_PType)
        {
            case P_LOCAL:
            {
                return m_Size * (4 + 4 + 2);
            }
            case P_GLOBAL:
            {
                return m_Size * (4 + 4 + 2*4);
            }
            case P_CORRELATOR:
            {
                return m_Size * (4 + 4 + 2 + 2*4 + 2);
            }
            default:
            {
                return 0;
            }
         }
    }
      
};




#endif 

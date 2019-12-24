
#ifndef _EXECUTE_H_
#define _EXECUTE_H_
#include <BTB.h>


class Execute
{
private:
    BTB *m_BTB;
    vector<unsigned> m_InstVector;

private:
    
    void ReadInst(string &InstPath);   
public:   
    Execute (unsigned BitSize, unsigned PType)
    {
        m_BTB = new BTB(BitSize, PType);
        assert (m_BTB != NULL);        
    }

    ~Execute ()
    {
        delete m_BTB;
        m_BTB = NULL;        
    }

    void Run(vector<unsigned> &InstVector);
    void Run(string &InstPath);

    void Result();
    
};




#endif 

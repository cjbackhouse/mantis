#ifndef MANTISDUMMYSOURCE_HPP_
#define MANTISDUMMYSOURCE_HPP_

#include "MantisCallable.hpp"

#include "MantisCondition.hpp"

#include <sys/time.h>
#include "MantisStatus.hpp"
#include "MantisBuffer.hpp"
#include "MantisEnv.hpp"

class MantisDummySource :
    public MantisCallable
{
    public:
        static MantisDummySource* digFromEnv(safeEnvPtr& env, 
					     MantisStatus* sts,
					     MantisBuffer* buf);
        virtual ~MantisDummySource();
        
        void SetStatus( MantisStatus* aStatus );
        void SetBuffer( MantisBuffer* aBuffer );
        void SetDigitizationRate( const double& aRate );
        
        void Initialize();
        void Execute();
        void Finalize();
        
    private:
        MantisDummySource();
        
        MantisCondition fCondition;
        
        unsigned long fAcquisitionCount;
        unsigned long fRecordCount;
        unsigned long long fLiveMicroseconds;
        unsigned long long fDeadMicroseconds;
                
        MantisStatus* fStatus;
        MantisBuffer* fBuffer;
        double fDigitizationRate;
};

#endif

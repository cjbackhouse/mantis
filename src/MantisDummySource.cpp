#include "MantisDummySource.hpp"

#include <cstdlib>
#include <unistd.h>

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

MantisDummySource::MantisDummySource() :
     fCondition(), fAcquisitionCount( 0 ), fRecordCount( 0 ), fLiveMicroseconds( 0 ), fDeadMicroseconds( 0 ), fStatus( NULL ), fBuffer( NULL ), fDigitizationRate( 0. )
{
}
MantisDummySource::~MantisDummySource()
{
}

MantisDummySource* MantisDummySource::digFromEnv(safeEnvPtr& env, 
				       MantisStatus* sts,
				       MantisBuffer* buf)
{
  MantisDummySource* res = new MantisDummySource();
  res->SetDigitizationRate((env.get())->getClockRate());
  res->SetStatus(sts);
  res->SetBuffer(buf);

  return res;
}

void MantisDummySource::SetStatus( MantisStatus* aStatus )
{
    fStatus = aStatus;
    return;
}
void MantisDummySource::SetBuffer( MantisBuffer* aBuffer )
{
    fBuffer = aBuffer;
    return;
}
void MantisDummySource::SetDigitizationRate( const double& aRate )
{
    fDigitizationRate = aRate;
    return;
}

void MantisDummySource::Initialize()
{
    fStatus->SetWriterCondition( &fCondition );

    MantisBufferIterator* Iterator = fBuffer->CreateIterator();
    for( size_t Count = 0; Count < fBuffer->GetBufferLength(); Count++ )
    {
      //        Result = AllocateDmaBufferPX4( fHandle, fBuffer->GetDataLength(), &Iterator->Data()->fDataPtr );
      Iterator->Data()->fDataPtr = new MantisData::DataType[fBuffer->GetDataLength()];
        Iterator->Increment();
    }
    delete Iterator;
    
    return;
}

void MantisDummySource::Execute()
{   
    //allocate some local variables
    timeval StartTime;
    timeval EndTime;
    timeval DeadTime;
    
    //grab an iterator
    MantisBufferIterator* Iterator = fBuffer->CreateIterator();
    
    //wait for run to release me
    fCondition.Wait();
    if( fStatus->IsRunning() == false )
    {
        delete Iterator;
        return;
    }

    fAcquisitionCount++;
    
    //start timing
    gettimeofday( &StartTime, NULL );
    
    //go go go go
    while( true )
    {
        if( fStatus->IsRunning() == false )
        {
            //this exit occurs during active data acquisition
            
            //get the time and update the number of live microseconds
            gettimeofday( &EndTime, NULL );
            fLiveMicroseconds += (1000000 * EndTime.tv_sec + EndTime.tv_usec) - (1000000 * StartTime.tv_sec + StartTime.tv_usec);
            
            delete Iterator;
            return;
        }
        
        Iterator->SetWriting();
        
        Iterator->Data()->fId = fAcquisitionCount;
        Iterator->Data()->fTick = clock();

	// Right arbitrary letters (this way can tell it didn't get corrupted)
	for(unsigned int n = 0; n < fBuffer->GetDataLength(); ++n)
	  Iterator->Data()->fDataPtr[n] = 'a'+rand()%26;
        
        fRecordCount++;
        
        Iterator->SetWritten();
        
        if( Iterator->TryIncrement() == false )
        {
            //get the time and update the number of live microseconds
            gettimeofday( &EndTime, NULL );
            fLiveMicroseconds += (1000000 * EndTime.tv_sec + EndTime.tv_usec) - (1000000 * StartTime.tv_sec + StartTime.tv_usec);
  
            //wait
            fCondition.Wait();
            
            //get the time and update the number of dead microseconds
            gettimeofday( &DeadTime, NULL );
            fDeadMicroseconds += (1000000 * DeadTime.tv_sec + DeadTime.tv_usec) - (1000000 * EndTime.tv_sec + EndTime.tv_usec);
            
            if( fStatus->IsRunning() == false )
            {
                delete Iterator;
                return;
            }

            fAcquisitionCount++;
            
            //start timing
            gettimeofday( &StartTime, NULL );
            
            Iterator->Increment();
        }              
    }
    return;
}  

void MantisDummySource::Finalize()
{  
    MantisBufferIterator* Iterator = fBuffer->CreateIterator();
    for( size_t Count = 0; Count < fBuffer->GetBufferLength(); Count++ )
    {
        delete[] Iterator->Data()->fDataPtr;
        Iterator->Increment();
    }
    delete Iterator;
    
    double LiveTime = fLiveMicroseconds / 1000000.;
    double DeadTime = fDeadMicroseconds / 1000000.;
    double MegabytesRead = fRecordCount * ( ((double)(fBuffer->GetDataLength())) / (1048576.) );;
    double ReadRate = MegabytesRead / LiveTime;
    
    cout << "\nreader statistics:\n";
    cout << "  * records taken: " << fRecordCount << "\n";
    cout << "  * aquisitions taken: " << fAcquisitionCount << "\n";
    cout << "  * live time: " << LiveTime << "(sec)\n";
    cout << "  * dead time: " << DeadTime << "(sec)\n";
    cout << "  * total data read: " << MegabytesRead << "(Mb)\n";
    cout << "  * average read rate: " << ReadRate << "(Mb/sec)\n";
    cout.flush();

    return;
}

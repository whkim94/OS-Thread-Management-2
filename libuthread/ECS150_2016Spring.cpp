#include "VirtualMachine.h"
#include "Machine.h"
#include <unistd.h>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <vector>
#include <map>
#include <string.h>

#define VM_TIMEOUT_INFINITE ((TVMTick)0)
#define VM_TIMEOUT_IMMEDIATE ((TVMTick)-1)
#define SMALL_BUFFER_SIZE       256




using namespace std;

//////////////////////////////////////////////////////////////
//////////////// GLOBALS /////////////////////////////////////
/////////////////////////////////////////////////////////////

//int* time;
//TVMTick count;
volatile int timeticks;
volatile unsigned int  totalticks;
volatile int g_result; // global result
volatile unsigned int threadCount = (volatile unsigned int)2;
volatile int semiticks;

void* shared_loc;
void* base;
void* data;
TVMMemorySize length;
int bytes;
//volatile bool flag = false;
volatile int flag = 1;
volatile int totalLeft = 256;
const TVMMemoryPoolID VM_MEMORY_POOL_ID_SYSTEM = 0;

//void UniversalCallback(void *param, int result);

//typedef void (*TMachineAlarmCallback)(void *calldata);

/*****************NEEDED**********************/



//////////////////////////////////////////////////////////////
//////////////// TCB/MCCANNOUNCED HERE /////////////////////////
/////////////////////////////////////////////////////////////

//memoety pool class
class mp {
public: 

    //when to use uint8_T versus int
    //use size(int) to detect if program needs int, long, or uint8_t

    TVMMemorySize memsize;
    uint8_t* poolbase;
    TVMMemoryPoolID poolID;
    int sizeleft;
    int sizeused;


    //need stack to manage free space

    mp(void *base, TVMMemorySize size, TVMMemoryPoolIDRef refID)
    {
        poolbase= (uint8_t*)base;
        memsize= size;
        poolID= *refID;
        sizeleft = size;
        sizeused = 0;

	poolbase = new uint8_t[size];
        //use stack here
    }
};

class aSpace { //allocation space
public:
    void *base;
    TVMMemorySize length;
    TVMMemoryPoolID memID;

//    aSpace(void *b,int l) {
//        length = l;
//        base = b;
//    }
};

//Thread Control Block
class TCB // Needs ID and priority 
{
public:

    TCB(TVMThreadIDRef ID, TVMThreadID threadID, TVMThreadState state, TVMThreadPriority priority, 
        TVMMemorySize memsize, TVMThreadEntry entry, void *params, TVMTick TCB_Ticks) : ID(ID), threadID(threadID), state(state),priority(priority), memsize(memsize), entry(entry),params(params),TCB_Ticks(TCB_Ticks)
        {
            stack = new int[memsize]; //dont forget new
        }

    TVMThreadIDRef ID;
    TVMThreadID threadID;
    TVMThreadState state;
    TVMThreadPriority priority;
    TVMMemorySize memsize;
    TVMThreadEntry entry;
    void *params;
    TVMTick TCB_Ticks;

    int* stack;

    //int state;
    //int fucked = 0; // deleted
    int result1; // make public
    SMachineContext context;  //static SMachineContext MachineContextCaller;

}; // end TCB class

////////////////////////////////////////////////////////////////
////////////////// Vectors go here /////////////////////////////
////////////////////////////////////////////////////////////////


//Do I want queues instead of vectors
vector<TCB*> threads; // threads doe
vector<TCB*> rekt; // sleep vector
vector<TCB*> low; // low priority vector
vector<TCB*> high; // high priority vector
vector<TCB*> medium; // medium priority vector

vector <TCB*> dead; //dead queue
vector <TCB*> waiting; //waitng to be ready queue

TCB* current; // current thread
TCB* stored; //thread that is stored or idled
//TCB* temp;

TMachineSignalStateRef save;
//ID of next thread
TVMThreadID nextID;

vector<mp*> memorypools; // stack for mem pools
vector<aSpace> allocatedSpace;
vector<aSpace> unAllocatedSpace;


//////////////////////////////////////////////////////////////
//////////////// DAS SCHEDULER and etc/////////////////////////
/////////////////////////////////////////////////////////////
//load ready
//void readyqueue(TCB* thread)
void readyqueue(TVMThreadID thread)
{
    threads[thread]->state = VM_THREAD_STATE_READY;

    if (threads[thread]->priority == VM_THREAD_PRIORITY_HIGH)
    {
        high.push_back(threads[thread]);
    }

    else if (threads[thread]->priority == VM_THREAD_PRIORITY_NORMAL)
    {
        medium.push_back(threads[thread]);
    }

    else if(threads[thread]->priority == VM_THREAD_PRIORITY_LOW)
    {
        low.push_back(threads[thread]);
    }
}


void readyqueue2(TCB* thread)
{
    thread->state = VM_THREAD_STATE_READY;

    if (thread-> priority == VM_THREAD_PRIORITY_HIGH)
    {
        high.push_back(thread);
    }

    else if (thread-> priority == VM_THREAD_PRIORITY_NORMAL)
    {
        medium.push_back(thread);
    }

    else if(thread->priority == VM_THREAD_PRIORITY_LOW)
    {
        low.push_back(thread);
    }
}



// Context Swictcher

void switcher(vector<TCB*> &queue)
{

    if (current->state == VM_THREAD_STATE_RUNNING)
    {
        current->state = VM_THREAD_STATE_READY;
    }


    else if(current->state == VM_THREAD_STATE_READY)
    {
       // readyqueue(threads[current]);
    }

    else if (current->state == VM_THREAD_STATE_WAITING && current->TCB_Ticks !=0)
    {
        rekt.push_back(current);
    }

    //MachineContextSwitch();
    TCB* wut = current;
    //current = stored;


    current = queue.front();
    queue.erase(queue.begin());
    current->state= VM_THREAD_STATE_RUNNING;

    MachineContextSwitch(&(wut->context), &(current->context));
}
//scheduler constructor goes here
void scheduler()
{
    /*
        Description: Stores thread priority from highest to lowest
        so what happens is that it checks if highest priority is full.
        Next it checks the medium, if that is full, it then sends the thread
        to the lowest vector.
    */

   // if (high.empty() !=1
    if(!high.empty())
    {
        //store in high queue
        switcher(high);
    }
    else if(!medium.empty())
    //else if (medium.empty() !=1)
    {
        //store in medium queue
        switcher(medium);
    }
    else if(!low.empty())
    //else if (low.empty() !=1)
    {
        //store in low queue
        switcher(low);
    }

    else
    {
        //put in sleep
        if (current->state == VM_THREAD_STATE_WAITING && current->TCB_Ticks !=0)
        {
                rekt.push_back(current);
           
        }
        TCB* no= current;
        current = stored;
        current->state = VM_THREAD_STATE_RUNNING;

        MachineContextSwitch(&(no->context), &(stored->context));

    }
} // scheduler()


extern "C"
{
    /*************VirtualMachineUtils.c****************/
    //void *VMLibraryHandle = NULL;

    TVMMainEntry VMLoadModule(const char *module);
    TVMMemorySize heapsize;
  //void MachineInitialize(size_t shared_size);
    void MachineEnableSignals(void);

}

  ///////////////////////////////////////////////////////////////////////
 ////////////// Callback Functions here ////////////////////////////////
///////////////////////////////////////////////////////////////////////


// DEFINE OWN CALLBACK FUNCTION HERE
void  SleepAlarmCallback(void *calldata) // Callback thread for sleep
{       

    //TMachineSignalState save;
    //MachineSuspendSignals(&save);
    
    /*int i;
    int size= rekt.size();

    for(i=0; i<size; i++)
    {

        rekt[i]-> TCB_Ticks --; //decrement the ticks
        //cout<< "coundwon"<<endl;
        totalticks++;

     if(rekt[i]->TCB_Ticks == 0)
     {
        //cout<<"sleepcallback"<<endl;
        rekt[i]->state=VM_THREAD_STATE_READY; //switch to ready
        readyqueue2(rekt[i]);
        rekt.erase(rekt.begin() +i);
        //scheduler();
     }
    }
     //singled threaded version below
     //while (lols != 0)
    */
    //cerr << "In sleepAlarmCall" << endl;
    flag=1;
    if (semiticks !=0)
    {
       semiticks--;
       cout <<"THATS THE SOUND OF THE POLICE" << endl;
       totalticks++;    
    }
    /*if (lols == 0)
    {
        break;
    }
   
    }
    else 
    {
        cout<< "end, iterations:" << semiticks<< endl;
    }*/
   
    //MachineResumeSignals(&save);
    return;
}// end SleepAlarmCallback

// Callback Function for fileopen
void UniversalCallback(void *param, int result)
{
    //cout << "In Universal call back" << endl;
    //TCB *arbiter = (TCB*)  param;
    
    //cout<< "Callback"<<endl;
    //arbiter->state= VM_THREAD_STATE_READY; //ready?
    //readyqueue(arbiter->threadID);
    //threads[arbiter->threadID]->result1= result; // set results
    g_result = result;
    current->result1 = result;
    //cerr<<"result passed in: "<<result<<endl;
    flag = 1;
    //scheduler();
    return;


}// end callback

void newcallback(void *param, int result)
{
    bytes=result;
    cout<<"lols"<<endl;
}


void lols(void *param) //idle function
{
    MachineResumeSignals(save);
    while(1); //loop
}

void lols2(void *param) //another idle function
// kill function
{
    //MachineResumeSignals(save);
    MachineEnableSignals();
    /*
    (TCB*)param->entry((TCB*)param->params);
    VMThreadTerminate(*(TCB*)param->ID);

    /*/

    TCB *arbiter = (TCB*) param;

    //arbiter->entry(arbiter->params);
    arbiter->entry(arbiter->params);
    //VMThreadTerminate((arbiter->threadID));

}

    TVMStatus VMStart(int tickms, TVMMemorySize heapsize, TVMMemorySize sharedsize, int argc, char *argv[]) 
    //http://stackoverflow.com/questions/19404040/void-is-not-a-pointer-to-object-type
    //argv[] is module
    {

        TVMMainEntry funcName = VMLoadModule(argv[0]);
        //void* temp_loc = MachineInitialize((size_t) sharedsize);

        timeticks = tickms;

        //cout<<"hello"<<endl;

        TVMThreadID thread;

        //int* base = new int[heapsize]; //nothing until memory pools
        shared_loc = MachineInitialize((size_t) sharedsize);
        MachineRequestAlarm(tickms*1000, SleepAlarmCallback, NULL);
        MachineEnableSignals();        


        TCB* main = new TCB((unsigned int*)0, thread, VM_THREAD_STATE_RUNNING, VM_THREAD_PRIORITY_NORMAL, 0, NULL, NULL, 0);
        threads.push_back(main);
        current = main;

        //stored = new TCB((unsigned int*)1, thread, VM_THREAD_STATE_RUNNING, VM_THREAD_PRIORITY_NORMAL, 0, NULL, NULL, 0);
        //stored->state= VM_THREAD_STATE_READY;
        //readyqueue2(stored);

        /*void* stack;
        stack = malloc(0x100000);
        threads.push_back(stored);*/
        //MachineContextCreate(&(stored->context), lols, NULL, stored->stack, stored->memsize);
        nextID=1;


        //TVMMemoryPoolID main1;
        //TVMMemoryPoolID shared;

        //VMMemoryPoolCreate(base, heapsize, &main);
        //VMMemoryPoolCreate(sharedbase, sharedsize, &shared);

        //mp *systemmemorypool = new mp(heapsize,sharedsize,0);
        //memorypools.push_back(systemmemorypool);
    

        if( funcName !=NULL)
        {


                //cout<< "VMStart complete"<<endl;

               funcName(argc, argv);

        // store heapsize in memory pool id system

        // sharedsize specifiedn between virtual and machine

               MachineTerminate();
               return VM_STATUS_SUCCESS;


         }

         else {
             return VM_STATUS_FAILURE;
     }

    }



    //////////////////////////////////////////////////////////
    /////////////////// POOL Functions ///////////////////////
    //////////////////////////////////////////////////////////

    TVMStatus VMMemoryPoolCreate( void *base, TVMMemorySize size, TVMMemoryPoolIDRef memory)
    {
        //cerr << "In VMCreate" << endl;
        if (base == NULL || memory == NULL || size == 0)
        {
            cerr << "Invalid parameter" <<endl;
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        }
        //if ()


        //TMachineSignalState save;
        //MachineSuspendSignals(&save);

	
	TVMMemorySize tempsize = ((size+63)/64) * 64;
        mp *temp = new mp(base,tempsize, memory);
        //totalLeft -= tempsize;
        memorypools.push_back(temp);
        //cerr << "Created new memory pool, now have total of " << memorypools.size() << " memory pools" << endl;
        //MachineResumeSignals(&save);

    //things


    return VM_STATUS_SUCCESS;
    }

    TVMStatus VMMemoryPoolDelete(TVMMemoryPoolID memory) {
        int pool_loc = -1;
        for (unsigned int i = 0; i<memorypools.size();i++) {
          if (memorypools[i]->poolID == memory) {
              pool_loc = i;
          }
        }
        if (pool_loc == -1) { //memory pool is not valid)
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (0) { //memory allocated from pool but not deallocated
            return VM_STATUS_ERROR_INVALID_STATE;
        }


        //memorypools.erase(memorypools.begin()+memory); //just added-Kent
        memorypools.erase(memorypools.begin()+pool_loc);

        //erase from unallocatedSpace
        int loc = -1;
        for (unsigned int i = 0; i<unAllocatedSpace.size();i++) {
          if (unAllocatedSpace[i].memID == memory) {
              pool_loc = i;
          }
        }
        if (loc!=-1) {
            unAllocatedSpace.erase(unAllocatedSpace.begin()+loc);
        }

        //erase from allocatedSpace
        loc = -1;
        for (unsigned int i = 0; i<allocatedSpace.size();i++) {
          if (allocatedSpace[i].memID == memory) {
              pool_loc = i;
          }
        }
        if (loc!=-1) {
            allocatedSpace.erase(allocatedSpace.begin()+loc);
        }

        return VM_STATUS_SUCCESS;
    }

    TVMStatus VMMemoryPoolQuery(TVMMemoryPoolID memory,TVMMemorySizeRef bytesleft) {
      if ( bytesleft == NULL) {
        return VM_STATUS_ERROR_INVALID_PARAMETER;}

      int pool_loc = -1;
      for (unsigned int i = 0; i<memorypools.size();i++) {
        if (memorypools[i]->poolID == memory) {
            pool_loc = i;
        }
      }
      if (pool_loc == -1) {
          return VM_STATUS_ERROR_INVALID_PARAMETER;
      }

      *bytesleft = memorypools[pool_loc]->sizeleft;

      return VM_STATUS_SUCCESS;
    }

    TVMStatus VMMemoryPoolAllocate(TVMMemoryPoolID memory,TVMMemorySize size, void **pointer) {

        //special case where we allocate before creating memory pool
        //cerr << "In VMAllocate" <<endl;
        if (memorypools.size() <1) {
            //cerr <<"No pool creaeted, creating system memory pool"<<endl;
            VMMemoryPoolCreate(&pointer,256,(TVMMemoryPoolIDRef)&VM_MEMORY_POOL_ID_SYSTEM);
            cerr << "Created system memory pool" <<endl;

            aSpace *temp = new aSpace;
            temp->base = pointer;
            temp->length = 256;
            temp->memID = memory;
            unAllocatedSpace.push_back(*temp);
        }

        //return error for invalid parameter
        if ( size == 0 || pointer==NULL) {
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        }
        //return error if ID is not found
        int pool_loc = -1;
        for (unsigned int i = 0; i<memorypools.size();i++) {
          if (memorypools[i]->poolID == memory) {
              pool_loc = i;
          }
        }
        if (pool_loc == -1) {
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        }

        //return error if not enough space
        if (memorypools[pool_loc]->sizeleft < (unsigned int)size) { //memory pool can't allocate less than [size] bytes
            return VM_STATUS_ERROR_INSUFFICIENT_RESOURCES;
        }

        //allocation code

        //remove from unallocated vector
        int unAlloc = -1;
        for (int i=0;i<unAllocatedSpace.size();i++) {
            if (unAllocatedSpace[i].length < size)
                unAlloc = i;
        }
        if (unAlloc != -1) {
            unAllocatedSpace.erase(unAllocatedSpace.begin()+unAlloc);
        }

        //have chucnks of memory
        aSpace *temp = new aSpace;
        temp->base = ((memorypools[pool_loc]->poolbase)+(memorypools[pool_loc]->sizeused));
        temp->length = size;
        temp->memID = memorypools[pool_loc]->poolID;
        allocatedSpace.push_back(*temp);
        memorypools[pool_loc]->sizeleft -= size;
        memorypools[pool_loc]->sizeused += size;

        *pointer = temp->base;

        return VM_STATUS_SUCCESS;
    }

    TVMStatus VMMemoryPoolDeallocate(TVMMemoryPoolID memory,void *pointer) {
        //return error if ID is not found
        int pool_loc = -1;
        for (unsigned int i = 0; i<memorypools.size();i++) {
          if (memorypools[i]->poolID == memory) {
              pool_loc = i;
          }
        }
        if (pool_loc == -1) {
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        }

        //pointer is nor previously allocated
        int alloc = -1;
        for (int i=0;i<allocatedSpace.size();i++) {
            if (allocatedSpace[i].base == pointer) {
                alloc = i;
            }
        }
        if (alloc == -1) {
            cerr << "pointer not previously allocated" << endl;
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        }

        //deallocation
        memorypools[pool_loc]->sizeleft += allocatedSpace[alloc].length;
        memorypools[pool_loc]->sizeused -= allocatedSpace[alloc].length;

        aSpace *temp = new aSpace;
        temp->base = ((memorypools[pool_loc]->poolbase)+(memorypools[pool_loc]->sizeused));
        temp->length = memorypools[pool_loc]->sizeleft;
        temp->memID = (memorypools[pool_loc]->poolID);
        unAllocatedSpace.push_back(*temp);

        allocatedSpace.erase(allocatedSpace.begin()+alloc);

        pointer = temp->base;

        return VM_STATUS_SUCCESS;
    }

    //////////////////////////////////////////////////////////
    /////////////////// File Functions ///////////////////////
    //////////////////////////////////////////////////////////

    TVMStatus VMFileOpen(const char *filename, int flags, int mode, int *filedescriptor) 
    {

        //MachineSuspendSignals(save);
        //call machinefileopen
        if (filename == NULL || filedescriptor == NULL) {
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        }

        flag = 0;
        MachineFileOpen(filename,flags, mode, UniversalCallback,current);
        //cerr << "==> MachineFileOpen completed" <<endl;
        //resume the signals
        //MachineResumeSignals(save);
        //change current thread to waiting
        //current->state =VM_THREAD_STATE_WAITING;
        //scheduler();
        while (!flag) {}
        *filedescriptor = current->result1;
        return VM_STATUS_SUCCESS;


        // else
        // {
        //     return VM_STATUS_FAILURE;
        // }

    }
    TVMStatus VMFileClose(int filedescriptor) 
    {
        MachineFileClose( filedescriptor,  UniversalCallback, current);
        //scheduler();

        return VM_STATUS_SUCCESS;
    }

    TVMStatus VMFileRead(int filedescriptor, void *data, int *length) 
    {
        //cerr << "==> In VMFileRead" <<endl;
        if (data==NULL || length == NULL) {
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        }

        //current->state= VM_THREAD_STATE_WAITING;
        flag = 0;
        int splits = (*length + 511) / 512;
        int temp = *length;
        //cerr << "==>number of READ splits: "<<splits << endl;
        for (int i=0;i<splits;i++) {
	  //cerr << "temp for FileRead"<<temp <<endl;
	  //memcpy(shared_loc, shared_loc, temp);
            if (temp >=512) {
                MachineFileRead(filedescriptor, shared_loc , 512, UniversalCallback, current);
            }
            else {
                MachineFileRead(filedescriptor, shared_loc , temp, UniversalCallback, current);
            }
            //&shared_loc += 512;
            while(!flag) {}    
            flag = 0;
            temp -= 512;
        }

        *length = g_result;
	 memcpy(data, shared_loc, *length);
       
        //cerr<<"==> MachineFileRead completed"<<endl;
        //while (!flag) {}



        // length = result

        //scheduler();

        //*length = current-> result1;


        return VM_STATUS_SUCCESS;
    }

    TVMStatus VMFileWrite( int filedescriptor, void *data, int *length) 
    {
        //write(filedescriptor,&data,(size_t)length);

        //TMachineSignalState save;
       //MachineSuspendSignals(&save);

        if (data == NULL || length == NULL)
        {
             return VM_STATUS_ERROR_INVALID_PARAMETER;           
        }

        //cout<<"memcpy finished"<<endl;
        int splits = (*length + 511) / 512;
        int temp = *length;
        //cerr << "number of splits"<<splits << endl;
        for (int i=0;i<splits;i++) {
            //cerr << "temp for FileWrite"<<temp <<endl;
            flag = 0;
            memcpy(shared_loc, data+(i*512), temp);
            if (temp >=512) {
                MachineFileWrite(filedescriptor, shared_loc , 512, UniversalCallback, current);
            }
            else {
                MachineFileWrite(filedescriptor, shared_loc , temp, UniversalCallback, current);
            }
            //&shared_loc += 512;
            while(!flag){    }    
            temp -=512;
            
        }

        
        //cout<<"MachineFileWrite done"<<endl;

        //Base pointer is wrong for *length. Obviously pointing to the wrong thing.
        //Callback isn't working

        //current->state = VM_THREAD_STATE_WAITING;
        //allocate pools
        // scheduler();
        //MachineResumeSignals(&save);
        //deallocate pools

        return VM_STATUS_SUCCESS;
    }

    TVMStatus VMFileSeek(int filedescriptor, int offset, int whence, int *newoffset) 
    {
        flag = 0;
        MachineFileSeek(filedescriptor, offset, whence, UniversalCallback, current);
        //current->state= VM_THREAD_STATE_WAITING;
        *newoffset = offset;
        //scheduler();
        while (!flag) {}

        return VM_STATUS_SUCCESS;
    }

    TVMStatus VMThreadSleep(TVMTick tick)
    {

        if (tick==VM_TIMEOUT_INFINITE) {
            return VM_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (tick == VM_TIMEOUT_IMMEDIATE) {
            //give  the remaining time to the next process
            return VM_STATUS_SUCCESS;
        }

        semiticks = tick;

        while (semiticks>0);

        return VM_STATUS_SUCCESS;
    }

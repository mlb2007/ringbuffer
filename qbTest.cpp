#include "qb.h"

const int stop_value = 5;

// a simple minded object that is put in queue buffer
// and is optionally observed by the queue buffer
class SimpleObj{
  BufferObserver<SimpleObj>* myObserver;
  int val;
public:
  SimpleObj(BufferObserver<SimpleObj>* _ob = NULL,
	    const int s = -1)
    :myObserver(_ob),
     val(s){}
  
  SimpleObj(const SimpleObj& other)
    :myObserver(other.myObserver),
     val(other.val){}
  
  SimpleObj& operator=(const SimpleObj& other){
    if(this !=&other){
      myObserver = other.myObserver;
      val = other.val;
    }
    return *this;
  }

  ~SimpleObj() {
    myObserver = NULL;
    val = -1;
  }

  void print(const char* msg) const {
    PRINT(std::cout << "***(" << msg << ")value is:" << value() << "\n";);
  }

  void value(const int _v){val = _v;}
  int value() const { return val;}
  
  void signalFull() {
    // signal my observer
    if(myObserver != NULL){
      myObserver->signalFull(this);
    }
  }
  void signalEmpty() {
    // signal my observer
    if(myObserver != NULL){
      myObserver->signalEmpty(this);
    }
  }
  
};


// thread routines
void* read_function(void* sobj)
{
  while(true){
    QBUF<SimpleObj>* qbuff = static_cast<QBUF<SimpleObj>*>(sobj);
    //PRINT(std::cout << "** --> read function \n";);
    
    SimpleObj* sbuff = qbuff->pop();
  
    // print the read obj, do something, release
    sbuff->print(":read_function() print:");
    int val = sbuff->value();
    if(val == stop_value)
      break;
    
    // work
    for(int ii=0; ii < 1000000;++ii);
  
    //release & signal thereby
    sbuff->signalEmpty();
  }
  // some exit code
  pthread_exit(NULL);
}


void* write_function(void* sobj)
{
  while(true) {
    QBUF<SimpleObj>* qbuff = static_cast<QBUF<SimpleObj>*>(sobj);
    SimpleObj* sbuff = qbuff->acquire();
    // write some value, release
    int val = sbuff->value();
    sbuff->value(++val);
    val = sbuff->value();
    sbuff->print(":write_function() print:");
    if(val == stop_value)
      break;
    
    // work
    for(int ii=0; ii < 1000000;++ii);

    //release & signal thereby
    sbuff->signalFull();
  }
  
  // some exit code
  pthread_exit(NULL);
  
}


int main(int argc, char** argv)
{
  int ret_code = 0;

  // create a queue buffer & use data owned by it
  // to read and write
  QBUF<SimpleObj> qb;

  // create threads
  pthread_t write_thread;
  pthread_t read_thread;
  
  // Initialize and set thread detached attribute   
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  
  // wite thread create
  std::cout << "Create write_thread\n";
  ret_code = pthread_create(&write_thread, &attr, write_function,(void*)&qb); 
  if (ret_code) {
    PRINT(std::cout << "ERROR; return code from write_thread pthread_create() " 
	  << ret_code << "\n";);
    return ret_code;
  }
  // read thread create
  PRINT(std::cout << "Create read_thread\n";);
  ret_code = pthread_create(&read_thread, &attr, read_function,(void*)&qb); 
  if (ret_code) {
    PRINT(std::cout << "ERROR; return code from read_thread pthread_create() "
	  << ret_code << "\n";);
    return ret_code;
  }

  // Free attribute and wait for the other threads
   pthread_attr_destroy(&attr);

   // join threads
   void* status;
   ret_code = pthread_join(write_thread, &status);
   if (ret_code) {
     PRINT(std::cout << "ERROR: write_thread: return code from pthread_join():" 
	   << ret_code << "\n";);
     return ret_code;
   }
   PRINT(std::cout << "Main: completed join with write_thread having a status:" 
	 << (long)status << "\n";);
   
   ret_code = pthread_join(read_thread, &status);
   if (ret_code) {
     PRINT(std::cout << "ERROR: read_thread: return code from pthread_join():" 
	   << ret_code << "\n";);
     return ret_code;
   }
   PRINT(std::cout << "Main: completed join with read_thread having a status:" 
	 << (long)status << "\n";);
   
   PRINT(std::cout << "Main: program completed...\n";);
   pthread_exit(NULL);  
   
   return ret_code;
}

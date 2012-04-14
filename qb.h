#ifndef _QBUF_H_
#define _QBUF_H_

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>

#include <pthread.h>

const size_t size = 10;

pthread_mutex_t p_mutex;
#define PRINT(arg) {pthread_mutex_lock(&p_mutex); \
  arg; \
  pthread_mutex_unlock(&p_mutex);}

template<typename _type>
class BufferObserver {
 public:
  BufferObserver() {}
  virtual ~BufferObserver(){}
  //virtual void signal(_type* typeobj) = 0;
  virtual void signalFull(_type* typeobj) = 0;
  virtual void signalEmpty(_type* typeobj) = 0;
};

template<typename _type>
class QBUF:public BufferObserver<_type>
{
  _type* buf[size];
  int head,tail,curr;
  bool hd;
  bool empty;
  size_t cnt;
  //
  pthread_mutex_t _mutex;
  pthread_cond_t cond_full;
  pthread_cond_t cond_empty;

  //void inc_cnt(){ ++cnt%size;}
  //void dec_cnt(){--cnt;}
  //int gap() {return (abs(head-tail)+1);}

  void inc_head(){
    head = (head+1)%size;
  }

  void inc_tail(){
    tail = (tail+1)%size;
  }
  /*
  // get from head or tail of the queue
  _type* acquire_data(bool hd_) {
    // acquire lock
    pthread_mutex_lock(&_mutex);
    hd = hd_;

    PRINT(std::cout << "head:" << head << " tail:" << tail << "\n";); 
    _type* retbuf = NULL;   

    // if head, tail coincide, wait for 
    // some thread to release this current 
    // buffer
    for(;;){
      if(head == tail){
	PRINT(std::cout << "*** " << "hd=: "
	      << ((hd==true)? "true":"false")
	      << " going to wait state\n";);
	pthread_cond_wait(&cond_value, &_mutex);
      } else {
	if(hd){
	  retbuf = buf[head];
	  curr = head;
	  PRINT(std::cout << "->head:buf[" << curr << "] returned\n";);
	  // wrap around
	  //head = (head+1)%size;
	} else {
	  retbuf = buf[tail];
	  curr = tail;
	  PRINT(std::cout << "->tail:buf[" << curr << "] returned\n";);
	  // wrap around
	  //tail = (tail+1)%size;
	}
	break;
      }
    } // for ever
    pthread_mutex_unlock(&_mutex);
    return retbuf;
  }
  */
  //
  QBUF(const QBUF&);
  QBUF& operator=(const QBUF&);


 public:
  QBUF(){
    for(size_t i = 0; i < size; ++i)
      buf[i] = new _type(this,i); // constructor takes second arg 
    // to test QBUF with "SimpleObj" datatype
    head = 0;
    tail = 0;
    curr = -1;
    hd = false;
    empty = true;
    cnt = size;
  }

  virtual ~QBUF(){
    for(size_t i = 0; i < size; ++i)
      if(buf[i]){
	delete buf[i];
	buf[i] = NULL;
      }
    head = 0;
    tail = 0;
    curr = -1;
    empty = true;
    hd = false;
    cnt = 0;
  }

  // get from the tail end of the queue 
  // to fill with data, put back and release
  //it explicitly
  _type* acquire() {
    //return acquire_data(false);
    pthread_mutex_lock(&_mutex);
    hd = false;
    _type* retbuf = NULL;   
    
    PRINT(std::cout << "acquire():head:" << head << " tail:" << tail << "\n";); 
    
    // if head, tail coincide, wait for 
    // some thread to release this current 
    // buffer
    for(;;){
      if(!empty && (head == tail)){
	PRINT(std::cout << "*** tail going to wait state\n";);
	pthread_cond_wait(&cond_empty, &_mutex);
	continue;
      }
      inc_head();
      retbuf = buf[tail];
      curr = tail;
      PRINT(std::cout << "->tail:buf[" << curr << "] will be acquired\n";);
      empty = false;
      break;    
    } // for ever
    pthread_mutex_unlock(&_mutex);
    return retbuf;
  }
  
  // get from head of the queue
  // use it and release it explicitly
  _type* pop() {
    //return acquire_data(true);
    // acquire lock
    pthread_mutex_lock(&_mutex);
    hd = true;
    _type* retbuf = NULL;   

    PRINT(std::cout << "pop():head:" << head << " tail:" << tail << "\n";); 
    
    // if head, tail coincide, wait for 
    // some thread to release this current 
    // buffer
    for(;;){
      if((head == tail)){
	PRINT(std::cout << "*** head going to wait state\n";);
	pthread_cond_wait(&cond_full, &_mutex);
	continue;
      }
      inc_tail();
      retbuf = buf[head];
      curr = head;
      PRINT(std::cout << "->head:buf[" << curr << "] will be popped\n";);
      break;

    } // for ever

    pthread_mutex_unlock(&_mutex);
    return retbuf;
  }
  
  // release of a buffer lock using signal
  //virtual void signal(_type* typeobj) {
  //pthread_mutex_lock(&_mutex);
  //pthread_cond_signal(&cond_value);
  //pthread_mutex_unlock(&_mutex);
  //}

  virtual void signalFull(_type* typeobj) {
    pthread_mutex_lock(&_mutex);
    pthread_cond_signal(&cond_full);
    PRINT(std::cout << "-- signal Full by tail\n";);
    //inc_tail();
    pthread_mutex_unlock(&_mutex);
  }

  virtual void signalEmpty(_type* typeobj) {
    pthread_mutex_lock(&_mutex);
    pthread_cond_signal(&cond_empty);
    PRINT(std::cout << "-- signal Empty by head\n";);
    //inc_head();
    pthread_mutex_unlock(&_mutex);
  }
  

}; 

#endif //_QBUF_H_

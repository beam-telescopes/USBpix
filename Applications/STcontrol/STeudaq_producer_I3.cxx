#include <eudaq/RawEvent.hh>
#include <eudaq/Logger.hh>

#include "STeudaq_producer.h"
#include <definesI3.h>

unsigned int encodeTriggerNumber(unsigned int trigger_number) {
  unsigned int word;
  word=(TRIGGER_WORD_HEADER_MASK | EXT_TRIGGER_MODE_MASK | (TRIGGER_NUMBER_MASK & ((trigger_number) << 8)));
  if (TRIGGER_NUMBER_MACRO(word) != trigger_number)
    EUDAQ_WARN((QString("Corrected Trigger missmatch: ") + QString::number(TRIGGER_NUMBER_MACRO(word))).toStdString());
  
  EUDAQ_INFO((QString("Corrected Trigger: ") + QString::number(TRIGGER_NUMBER_MACRO(word))).toStdString());
  
  return word;
}

void EUDAQProducer::dataPendingI3(std::vector<unsigned int *>* data_vec, int boardid)
{
  all_events_recieved=false;

  if(STEP_DEBUG) std::cout << "Get data list from board: " << boardid << " - Current Trigger ID: " << lastTriggerNumber << std::endl;
  
  int cur_BoardIndex;
  
  // find cur_BoardIndex that corresponds to boardid
  for (cur_BoardIndex=0; cur_BoardIndex<board_count; cur_BoardIndex++)
    if (board_ids[cur_BoardIndex] == boardid) break;

  if(data_vec->size()!=1){ // vector depth is for #FE, of which we have just one
    EUDAQ_WARN((QString("Wrong size of data array: ") + QString::number(data_vec->size())).toStdString());
    if(data_vec->size()==0){
      data_pending_running=false;
      return; // no data, can't do anything
    }
  }
  unsigned int *data = data_vec->at(0);
  
  if(STEP_DEBUG) std::cout << "Searching for last trigger" << std::endl;
  // eudaq::Timer t;
  // search new data for last trigger number:
  int t_trigger_number=0;
  int multiplier=0;
  int lastDataIndex = -1;
  
  unsigned int nDataEl=0;
  for (int it = 0; it<(SRAM_BYTESIZE/4); it++) {
    if (data[it] == 0) {
      break;
    } else {
      nDataEl++;
      if ( (TRIGGER_WORD_HEADER_MACRO(data[it])) == 1 ) {
	// INIT lastTriggerNumber
	if (lastTriggerNumber==-1) lastTriggerNumber = TRIGGER_NUMBER_MACRO(data[it])-1;
	
	t_trigger_number=TRIGGER_NUMBER_MACRO(data[it]);
	
	// The TLU returns only 15 bit numbers, so we have to be aware of a restart
	if (recieved_trigger[cur_BoardIndex]==-1) multiplier=0;
	else multiplier=recieved_trigger[cur_BoardIndex]/TLU_TRIGGER_AMOUNT;
	
	if (t_trigger_number < (recieved_trigger[cur_BoardIndex]%TLU_TRIGGER_AMOUNT)) {
	  multiplier++;
	  if(STEP_DEBUG) std::cout << "INCREASED MULTIPLIER TO " << multiplier << " AT TRIGGER NUMBER " << t_trigger_number << " resulting trigger: " << (multiplier*TLU_TRIGGER_AMOUNT)+t_trigger_number << std::endl;
	}
	recieved_trigger[cur_BoardIndex]=(multiplier*TLU_TRIGGER_AMOUNT)+t_trigger_number;
	lastDataIndex = it;
     }
    }
  }
  if(lastDataIndex==-1) {
    if(STEP_DEBUG) std::cout << "LAST DATA INDEX is -1, nothing to do" << std::endl;
    delete[] data;
    data = NULL;    
    data_pending_running=false;
    return;
  }

  // if(STEP_DEBUG) std::cout << "Searching for last trigger ended after" << t.Seconds() << std::endl;
  
  // check for remaining events:
  if ((*std::max_element (recieved_trigger.begin(), recieved_trigger.end())) == (*std::min_element (recieved_trigger.begin(), recieved_trigger.end()))) {
    all_events_recieved = true;
  }
  
  if (nDataEl==0) {
    data_pending_running=false;
    return;
  }
  
  if(STEP_DEBUG) std::cout << "MAX TRIGGER NUMBER FOR BOARD " << cur_BoardIndex << ": " << recieved_trigger[cur_BoardIndex] << " : " << recieved_trigger[cur_BoardIndex] % TLU_TRIGGER_AMOUNT << std::endl;
  if(STEP_DEBUG) std::cout << "Number of elements in data list: " << nDataEl << std::endl;
  
  // Insert data in EventData[cur_BoardIndex]
  //  EventData[cur_BoardIndex]->insert(EventData[cur_BoardIndex]->end(),data.begin(),data.end());
  
  // JGK: code adapted according to FE-I4 version
  if(STEP_DEBUG) std::cout << "Inserting data into EventData[" << cur_BoardIndex << "]" << std::endl;
  //Instert data in EventData
  for (int it = 0; it <=lastDataIndex ; it++)
    {
      EventData[cur_BoardIndex].push_back(data[it]);
    }
  
  //Clean up data
  delete[] data;
  data = NULL;

  return;
}
void EUDAQProducer::sendEventsI3(bool endrun) 
{
  // Check if there are completed events
  int min_trigger_number;
  int cur_BoardIndex;
  
  // if endrun then send all the data that is currently stored
  if (endrun) {
    min_trigger_number = *std::max_element (recieved_trigger.begin(), recieved_trigger.end());
    if(STEP_DEBUG) std::cout << "sendEvents: ENDRUN!" << std::endl;
  } else {
    min_trigger_number = *std::min_element (recieved_trigger.begin(), recieved_trigger.end());
  }
  
  if(STEP_DEBUG) std::cout << "MIN TRIGGER NUMBER: " << min_trigger_number << std::endl;
  
  // Return if there are no events to be send to Run Control
  if (min_trigger_number==-1) {
    data_pending_running=false;
    return;
  }
  
  if(STEP_DEBUG) std::cout << "LAST TRIGGER NUMBER: " << lastTriggerNumber << std::endl;
  
  int triggerCounter = lastTriggerNumber+1;
  int triggerNumber;
  bool isTriggerWord, proceedLoop;
  unsigned int Word;
  
  int hitcounter=0;
  
  std::vector<unsigned int> one_event;
  while (triggerCounter <= min_trigger_number) {
    triggerNumber = triggerCounter % TLU_TRIGGER_AMOUNT;
    
    if (endrun && STEP_DEBUG) std::cout << "Sending Event " << triggerNumber << std::endl;
    
    auto evpt = eudaq::Event::MakeUnique(EUDAQProducer::EVENT_TYPE);
    auto &ev = *(evpt.get());    
    
    // Now search all boards for events with this trigger number
    // and check for data consistency
    for (cur_BoardIndex=0; cur_BoardIndex<board_count; cur_BoardIndex++) {
      proceedLoop=true;
      
      // loop for next event
      while (proceedLoop) {
	isTriggerWord=false;
	one_event.clear();
	unsigned int eoe_counter=0;
	
	while ( !isTriggerWord && (EventData[cur_BoardIndex].size()>0)) {
	  Word = EventData[cur_BoardIndex].front();
	  EventData[cur_BoardIndex].pop_front();
	  
	  if ( (TRIGGER_WORD_HEADER_MACRO(Word)) == 1 ) {
	    isTriggerWord = true;
	  } else if ((TRIGGER_WORD_HEADER_MACRO(Word) == 0) && (HEADER_MACRO(Word) == 1) && ((FLAG_MACRO(Word) & FLAG_WO_STATUS) == FLAG_WO_STATUS)) {
	    // good EOE WORD:
	    eoe_counter++;
	  } else if ((TRIGGER_WORD_HEADER_MACRO(Word) == 0) && (HEADER_MACRO(Word) == 1) && (ROW_MACRO(Word) < 160) && (COL_MACRO(Word) < 18)) {
	    // good hit word
	    hitcounter++;
	    if (hitcounter%100==0) std::cout << "send hitword: " << hitcounter << std::endl;
	  } else {
	    // warning: unknown data word
	    EUDAQ_WARN((QString("Unknown data word recieved - board: ") + QString::number(board_ids[cur_BoardIndex]) + QString(" at trigger ") + QString::number(triggerNumber)).toStdString());
	  }
	  
	  
	  one_event.push_back(Word);
	}
	// if we got here, this should be trigger word, but let's check again
	if (isTriggerWord) {
	  // check if it is the desired trigger number
	  if (((int) TRIGGER_NUMBER_MACRO(Word))<triggerNumber) {
	    // Events should have already been send in an earlier loop
	    // process loop once more
	    //if(STEP_DEBUG) std::cout << "ERROR: Trigger Number to small" << std::endl;
	    EUDAQ_WARN((QString("Trigger Number to small. recieved: ") + QString::number(TRIGGER_NUMBER_MACRO(Word)) + QString(" for board ") + QString::number(board_ids[cur_BoardIndex]) + QString(" - expected: ") + QString::number(triggerNumber)).toStdString());
	    one_event.clear();
	  } else {
	    if (((int) TRIGGER_NUMBER_MACRO(Word))>triggerNumber) {
	      // Trigger Number missing
	      EUDAQ_WARN((QString("Trigger Number ") + QString::number(triggerNumber) + QString(" is missing for board ") + QString::number(board_ids[cur_BoardIndex])).toStdString());
	      // Put Data back in list:
	      EventData[cur_BoardIndex].insert(EventData[cur_BoardIndex].begin(), one_event.begin(), one_event.end());
	      one_event.clear();
	      one_event.push_back(encodeTriggerNumber(triggerNumber));
	      ev.AddBlock(cur_BoardIndex, one_event);
	      ev.SetTriggerN(triggerNumber);
	      proceedLoop=false;
	    } else {
	      // Trigger Number fits
	      // check for consistency
	      if (TRIGGER_WORD_ERROR_MACRO(Word) != 0) {
		EUDAQ_WARN((QString("FE-Error ") + QString::number(TRIGGER_WORD_ERROR_MACRO(Word)) + QString(" board ") + QString::number(board_ids[cur_BoardIndex]) + QString(" at trigger ") + QString::number(TRIGGER_NUMBER_MACRO(Word))).toStdString());
	      } else if (eoe_counter != consecutive_lvl1) {
		EUDAQ_WARN((QString("Wrong number of EOE words for board ") + QString::number(board_ids[cur_BoardIndex]) + QString(" at trigger ") + QString::number(TRIGGER_NUMBER_MACRO(Word))).toStdString());
	      }
	      
	      // add data (hit, EoE words and Trigger word to event)
	      ev.AddBlock(cur_BoardIndex, one_event);
	      ev.SetTriggerN(triggerNumber);
	      proceedLoop=false;
	    }
	  }
	} else {
	  if(STEP_DEBUG) std::cout << "No Event: " << triggerNumber << " board: " << cur_BoardIndex << std::endl;
	  one_event.clear();
	  one_event.push_back(encodeTriggerNumber(triggerNumber));
	  ev.AddBlock(cur_BoardIndex, one_event);
	  ev.SetTriggerN(triggerNumber);
	  proceedLoop=false;
	}
      }
    }
    
    try {
      SendEvent(std::move(evpt));
    } catch (...){
      if(STEP_DEBUG) std::cout << "ERROR: unable to send event " << m_ev << std::endl;
    }
    
    triggerCounter++;
    m_ev++;
  }
  
  std::cout << "sent hitwords: " << hitcounter << std::endl;
  
  lastTriggerNumber = triggerCounter-1; //triggerNumber;
  data_pending_running=false;
  if(STEP_DEBUG) std::cout << "Data send to Trigger: " << lastTriggerNumber << std::endl;
}

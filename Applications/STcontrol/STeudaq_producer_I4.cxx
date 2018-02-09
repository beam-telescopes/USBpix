#include <eudaq/RawEvent.hh>
#include <eudaq/Logger.hh>

#include "STeudaq_producer.h"
#include <defines.h>

#define STEPDEBUG true

unsigned int encodeTriggerNumber(unsigned int trigger_number, bool first) {
  // this trigger is good => save result!
  unsigned int word1, word2;
  word1=(TRIGGER_WORD_HEADER | (TRIGGER_NUMBER_31_24_MASK & ((trigger_number) >> 24)));
  word2=(TRIGGER_NUMBER_23_0_MASK & (trigger_number));
  if (TRIGGER_NUMBER_MACRO2(word1, word2) != trigger_number)
    EUDAQ_WARN((QString("Corrected Trigger missmatch: ") + QString::number(TRIGGER_NUMBER_MACRO2(word1, word2))).toStdString());
  
  if (first) return word1;
  else return word2;
}

void EUDAQProducer::dataPendingI4(std::vector<unsigned int *>* data_vec, int boardid)
{
  for(size_t chip = 0; chip < data_vec->size(); chip++)
    {
      
      if(STEP_DEBUG) std::cout << "dataPending(...): processing chip: " << chip << std::endl;
      
      //the data for this chip
      unsigned int* data = data_vec->at(chip);
      
      if(STEP_DEBUG)
	{
	  std::cout << "First 10 words of data[] from chip: " << chip << std::endl;
	  for(int i = 0; i <= 10; i++)
	    {
	      std::cout << data[i] << std::endl;
	    }
	}
      
      all_events_recieved = false;
      
      if(STEP_DEBUG) std::cout << "got data from board: " << boardid << " - Current Trigger ID: " << lastTriggerNumber << std::endl;
      
      int lastDataIndex = -1;
      
      //index of board and chip in EventData and recieved_trigger
      int index = boardOffset[boardid]+chip;
      
      if(STEP_DEBUG) std::cout << "Searching for last trigger" << std::endl;
      
      //incremented if data word is zero
      int zero_counter = 0;
      //break after ten zero words
      const int max_zero = 10;
      
      //current trigger no
      int t_trigger_number = 0;
      //TLU multiplier due to 15bit rollover
      int multiplier = 0;
      
      //this is the loop over all data words!
      //checking data[i+1], so max. index is one lower than array size
      for (int i = 0; i < (SRAM_WORDSIZE_PER_CHIP-1) ; i++)
	{	
	  if (data[i] == 0) 
	    {
	      zero_counter++;
	    } 
	  else 
	    {
	      //reset zero counter
	      zero_counter = 0;
	      
	      if( TRIGGER_WORD_MACRO(data[i]) ) 
		{
		  //decode Triggernumber
		  t_trigger_number = TRIGGER_NUMBER_MACRO2(data[i], data[i+1])%TLU_TRIGGER_AMOUNT;
		  
		  //INIT lastTriggerNumber
		  if(lastTriggerNumber == -1)
		    {
		      lastTriggerNumber = t_trigger_number-1;
		    }
		  
		  // The TLU returns only 15 bit numbers, so we hfave to be aware of a restart
		  if (recieved_trigger[index] == -1)
		    {
		      multiplier = 0;
		    }
		  else 
		    {
		      multiplier = recieved_trigger[index]/TLU_TRIGGER_AMOUNT;
		    }
		  
		  //increase multiplier only when close to the end of the trigger range
		  if(
		     (t_trigger_number < (recieved_trigger[index]%TLU_TRIGGER_AMOUNT)) && 
		     ((recieved_trigger[index]%TLU_TRIGGER_AMOUNT)>(TLU_TRIGGER_AMOUNT - 50))
		     ) 
		    {
		      multiplier++;
		      recieved_trigger[index] = (multiplier*TLU_TRIGGER_AMOUNT)+t_trigger_number;
		    } 
		  else if (t_trigger_number > (recieved_trigger[index]%TLU_TRIGGER_AMOUNT))
		    {
		      recieved_trigger[index] = (multiplier*TLU_TRIGGER_AMOUNT)+t_trigger_number;
		    }
		  
		  //increase i by 1, because trigger number is split into two words
		  i++;
		  lastDataIndex = i;
		}
	    }
	  
	  // break loop if there are sufficient zeros
	  if (zero_counter>max_zero)
	    {
	      //TODO: Give out error
	      if(STEP_DEBUG) std::cout << "zero_counter>" << max_zero<<" -> exiting trigger finder loop, halting at: word index i=" << i << std::endl;
	      break;
	    }
	} //loop over data words
      
      if(lastDataIndex==-1)
	{
	  if(STEP_DEBUG) std::cout << "LAST DATA INDEX is -1, nothing to do" << std::endl;
	  delete[] data;
	  data = NULL;
	  
	  //if we processed the last chip on the board, and this happens, there is no data pending anymore
	  if(chip == data_vec->size()-1)
	    {
	      data_pending_running=false;
	    }
	  
	  continue;
	}
      
      if(STEP_DEBUG) std::cout << "MAX TRIGGER NUMBER FOR BOARD : " << index << " : " << recieved_trigger[index] << 
		       " : " << recieved_trigger[index] % TLU_TRIGGER_AMOUNT << "\nLAST DATA INDEX: " << lastDataIndex << std::endl;
      
      if(STEP_DEBUG) std::cout << "Inserting data into EventData[" << index << "]" << std::endl;
      
      //Instert data in EventData
      for (int i = 0; i <=lastDataIndex ; i++)
	{
	  EventData[index].push_back(data[i]);
	}
      
      //Clean up data
      delete[] data;
      data = NULL;
    }//loop over all chips on board
  
  //check if all events have been received, i.e. if all chips have processed the same triggers:
  if(
     *std::max_element(recieved_trigger.begin(), recieved_trigger.end()) 
     == *std::min_element(recieved_trigger.begin(), recieved_trigger.end())
     ) 
    {
      all_events_recieved = true;
    }
}

void EUDAQProducer::sendEventsI4(bool endrun) 
{
  // Check if there are completed events
  int min_trigger_number;
  //int cur_BoardIndex;
  unsigned int Word, TriggerWord2;
  std::vector<unsigned int> one_event;
  bool isTriggerWord, proceedLoop;
  
  uint32_t tg_h17 = 0;
  uint16_t last_tg_l15 = 0;
	
  //if endrun then send all the data that is currently stored
  if(endrun) 
    {
      if(STEP_DEBUG) std::cout << "sendEvents: ENDRUN!" << std::endl;
      min_trigger_number = *std::max_element(recieved_trigger.begin(), recieved_trigger.end());
    } 
  else 
    {
      min_trigger_number = *std::min_element(recieved_trigger.begin(), recieved_trigger.end());
    }
  
  if(STEP_DEBUG) std::cout << "MIN TRIGGER NUMBER: " << min_trigger_number << std::endl;
  
  // Return if there are no events to be send to Run Control
  if(min_trigger_number == -1) 
    {
      data_pending_running=false;
      return;
    }
  
  if(STEP_DEBUG) std::cout << "LAST TRIGGER NUMBER: " << lastTriggerNumber << std::endl;
  
  int triggerCounter = lastTriggerNumber+1;
  int triggerNumber;
  
  if(STEP_DEBUG) std::cout << "creating RawDataEvent for event type " << EUDAQProducer::EVENT_TYPE << std::endl;
  
  while(triggerCounter <= min_trigger_number) 
    {
      triggerNumber = triggerCounter % TLU_TRIGGER_AMOUNT;
      
      if(STEP_DEBUG && triggerNumber%100 == 0 ) std::cout << "Sending Event " << triggerNumber <<  "... ";
      
      auto evpt = eudaq::Event::MakeUnique(EUDAQProducer::EVENT_TYPE);
      auto &ev = *(evpt.get());    

      //Now search all chips for events with this trigger number
      for (size_t chip = 0; chip < boardChips.size(); chip++) 
	{
	  
	  //if(STEP_DEBUG) std::cout << "sendEvent() for chip: " << chip << std::endl;
	  proceedLoop=true;
	  
	  while(proceedLoop)
	    {
	      
	      //std::cout << "In proceedLoop!" << std::endl;
	      isTriggerWord=false;
	      one_event.clear();
	      unsigned int dh_counter=0;
	      
	      while( !isTriggerWord && !EventData[chip].empty())
		{
		  //std::cout << "In !isTriggerWorld loop" << std::endl;
		  
		  //std::cout << "Front access!" << std::endl;
		  Word = EventData[chip].front();
		  //std::cout << "Front delete!" << std::endl;
		  EventData[chip].pop_front();
		  
		  //std::cout << "trying TRIGGER_WORD_MACRO or DATA_HEADER_MACRO: "; 
		  
		  if(TRIGGER_WORD_MACRO(Word))
		    {
		      //std::cout << "success!" << std::endl;
		      isTriggerWord = true;
		    }
		  else if(DATA_HEADER_MACRO(Word))
		    {
		      //std::cout << "success!" << std::endl;
		      dh_counter++;
		    }
		  
		  one_event.push_back(Word);
		}
	      
	      // if we got here, this should be trigger word, but let's check again
	      if(isTriggerWord)
		{
		      uint16_t tg_l15 = 0x7fff & (TriggerWord[6] + (TriggerWord[7]<<8));
		      if(tg_l15 < last_tg_l15 && last_tg_l15>0x6000 && tg_l15<0x2000){
			tg_h17++;
			EUDAQ_INFO("increase high 17bits of trigger number, last_tg_l15("+ std::to_string(last_tg_l15)+") tg_l15("+ std::to_string(tg_l15)+")" );
		      }
			uint32_t tg_n = (tg_h17<<15) + tg_l15;
			//evup->SetTriggerN(tg_n);
		        triggerNumber = tg_n;
			last_tg_l15 = tg_l15;  
		  //std::cout << "Found trigger word!" << std::endl;
		  
		  // get 2nd trigger word
		  TriggerWord2 = EventData[chip].front();
		  EventData[chip].pop_front();
		  one_event.push_back(TriggerWord2);
		  
		  //check if it is the desired trigger number
		  if(( (int)(TRIGGER_NUMBER_MACRO2(Word, TriggerWord2))%TLU_TRIGGER_AMOUNT) < triggerNumber)
		    {
		      std::cout << "Failed! Expected trigger " << triggerNumber << ", found " << (int)(TRIGGER_NUMBER_MACRO2(Word, TriggerWord2))%TLU_TRIGGER_AMOUNT << std::endl;
		      //Events should have already been send in an earlier loop, process loop once more
		      EUDAQ_WARN(QString("Trigger Number to small. recieved: " + QString::number(TRIGGER_NUMBER_MACRO2(Word, TriggerWord2)) + 
					 " for board " + /*String::number(board_ids[cur_BoardIndex]) + */" - expected: " + QString::number(triggerNumber)).toStdString().c_str());
		      
		      one_event.clear();
		    }
		  else
		    {
		      if( ((int)(TRIGGER_NUMBER_MACRO2(Word, TriggerWord2))%TLU_TRIGGER_AMOUNT) > triggerNumber)
			{
			  std::cout << "Trigger missing! Expected trigger " << triggerNumber << ", found " << (int)(TRIGGER_NUMBER_MACRO2(Word, TriggerWord2))%TLU_TRIGGER_AMOUNT << std::endl;
			  
			  // Trigger Number missing
			  EUDAQ_INFO(QString("Trigger Number " + QString::number(triggerNumber) + " is missing for board ").toStdString()) ;// + 
			  //QString::number(board_ids[cur_BoardIndex]) ).toStdString().c_str());
			  
			  //Put Data back in list:
			  EventData[chip].insert(EventData[chip].begin(), one_event.begin(), one_event.end());
			  one_event.clear();
			  
			  //Add two dummy trigger blocks (no data!)!
			  one_event.push_back(encodeTriggerNumber(triggerNumber, true));
			  one_event.push_back(encodeTriggerNumber(triggerNumber, false));
			  ev.AddBlock(chip, one_event);
			  ev.SetTriggerN(triggerCounter);
			  proceedLoop = false;
			} 
		      else
			{
			  //Trigger Number fits, check for consistency
			  if(dh_counter != consecutive_lvl1)
			    {
			      //SINCE WITH CURRENT 4CHIP MODULES IT OFTEN HAPPENS THAT A MODULE IS D 
			      //std::cout << "On chip: " << chip<< ", DataHeader count is: " << dh_counter << " whereas " << 
			      //consecutive_lvl1 << "consecutive lvl1 triggers were expected!" << std::endl; 
			      
			      //EUDAQ_WARN((QString("Wrong number of data header (") + QString::number(dh_counter) + 
			      //QString(") for chip ") + chip +*/ QString(" at trigger ") + 
			      //QString::number(TRIGGER_NUMBER_MACRO2(Word, TriggerWord2))).toLatin1().data());
			    }
			  
			  if(STEP_DEBUG && triggerNumber%100 == 0 ) std::cout << "Success!" << std::endl;
			  //add data (hit, EoE words and Trigger word to event)
			  ev.AddBlock(chip, one_event);
			  ev.SetTriggerN(triggerNumber);
			  proceedLoop = false;
			}
		    }
		} 
	      else 
		{
		  if(STEP_DEBUG) std::cout << "No Event: " << triggerNumber << " chip: " << chip << std::endl;
		  one_event.clear();
		  
		  //Add two dummy trigger blocks (no data!)!
		  one_event.push_back(encodeTriggerNumber(triggerNumber, true));
		  one_event.push_back(encodeTriggerNumber(triggerNumber, false));
		  ev.AddBlock(chip, one_event);
		  ev.SetTriggerN(triggerNumber);
		  proceedLoop=false;
		}
	    }
	}
      
      try 
	{
	  SendEvent(std::move(evpt));
	} 
      catch (...)
	{
	  if(STEP_DEBUG) std::cout << "ERROR: unable to send event " << m_ev << std::endl;
	  break;
	}
      
      triggerCounter++;
      m_ev++;
    }
  
  lastTriggerNumber = triggerCounter-1; //triggerNumber;
  data_pending_running = false;
  
  if(STEP_DEBUG) std::cout << "Data send to Trigger: " << lastTriggerNumber << std::endl;
}

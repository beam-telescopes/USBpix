#include "LogRoot.h"

ClassImp(LogRoot)

LogRoot::LogRoot() : TLogFile(){
}
LogRoot::LogRoot(TLogFile &log) : TLogFile(log){
}
LogRoot::~LogRoot(){
}

//
// Created by croemheld on 06.01.2023.
//

#include <icarus/Logger/Logger.h>

namespace icarus::logger {

/*
 * Logger implementations
 */

struct LogTypeInfo {
  std::string LTMessage;
  std::string ColorCode;
};

static std::array<LogTypeInfo, 4> LogTypes = {{{" ERROR ", COLOR_CODE(RED, true)},
                                               {"WARNING", COLOR_CODE(YELLOW, false)},
                                               {"SUCCESS", COLOR_CODE(GREEN, false)},
                                               {"MESSAGE", COLOR_CODE(CYAN, false)}}};

/*
 * LoggerImpl methods
 */

void LoggerImpl::worker(LogMessageQueue *Queue) {
  MessageQueue = Queue;
  while (true) {
    LogMessage Message;
    if (MessageQueue->pop(Message)) {
      if (Message.LogType == LogTypeEnum::LOGTYPETERM)
        break;
      logs(Message.LogType, Message.Message);
    }
  }
}

std::string LoggerImpl::getPreamble(LogTypeEnum LogType) const {
  return "[" + LogTypes[LogType].LTMessage + "]";
}

std::string LoggerImpl::getAppendix() const {
  return "\n";
}

void LoggerImpl::logs(LogTypeEnum LogType, std::string &Message) {
  FOStream << getPreamble(LogType) << Message << getAppendix();
  FOStream.flush();
}

std::string TermLogger::getPreamble(LogTypeEnum LogType) const {
  return "[" + LogTypes[LogType].ColorCode + LogTypes[LogType].LTMessage + COLOR_CODE(WHITE, false) + "]";
}

/*
 * LogThread methods
 */

void LogThread::logs(LogTypeEnum LogType, std::string &Message) {
  MessageQueue->push({LogType, Message});
}

void LogThread::shutdown() {
  LoggerThread.join();
  delete MessageQueue;
  delete Logger;
}

/*
 * Logger methods
 */

std::vector<LogThread *> Loggers;
std::vector<LogMessage> EarlyMessages;

void logMessage(LogMessage &&Message) {
  for (LogThread *L : Loggers) {
    L->logs(Message.LogType, Message.Message);
  }
}

void addEarlyLoggerMessage(LogMessage &&Message) {
  EarlyMessages.emplace_back(std::move(Message));
}

void logEarlyMessages() {
  for (LogMessage &Message : EarlyMessages) {
    logMessage(std::move(Message));
  }
  EarlyMessages.clear();
}

void addLogger(LoggerImpl *Logger) {
  Loggers.push_back(new LogThread(Logger));
}

void waitFinished() {
  for (LogThread *L : Loggers) {
    logMessage({LogTypeEnum::LOGTYPETERM, ""});
    L->shutdown();
  }
}

static std::set<std::string> DebugTypes;

/**
 * Add this debug type to the set of all types allowed to be printed.
 * @param DebugType The debug type to add to the set.
 */
void setDebugType(llvm::StringRef DebugType) {
  DebugTypes.insert(std::string(DebugType));
}

/**
 * Add all provided debug types in the comma separated string to the
 * set of all types allowed to be printed.
 * @param DebugOnly The comma separated string of debug types to add to the set.
 */
void setDebugTypes(llvm::StringRef DebugOnly) {
  if (!llvm::DebugFlag) {
    llvm::SmallVector<llvm::StringRef, 8> DTList;
    DebugOnly.split(DTList, ',', -1, false);
    for (auto DebugType : DTList)
      setDebugType(DebugType);
  }
}

#ifndef NDEBUG

bool isDebugType(llvm::StringRef DebugType) {
  return DebugTypes.count(std::string(DebugType)) > 0;
}

#endif

void initLoggerOptions(llvm::StringRef DebugOnly, llvm::StringRef DebugFile) {
  addLogger(new TermLogger());
  if (!DebugFile.empty())
    addLogger(new FileLogger(DebugFile));
  logEarlyMessages();
  setDebugTypes(DebugOnly);
}

} // namespace icarus::logger
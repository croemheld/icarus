//
// Created by croemheld on 05.01.2023.
//

#ifndef ICARUS_LOGGER_LOGGER_H
#define ICARUS_LOGGER_LOGGER_H

#include <llvm/ADT/APInt.h>

#include <llvm/Support/Debug.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/raw_os_ostream.h>

#include <icarus/Support/Clang.h>
#include <icarus/Support/Math.h>
#include <icarus/Support/String.h>

#include <icarus/Threads/ThreadPool.h>

#include <fstream>
#include <iomanip>
#include <iostream>

namespace icarus {

enum LogTypeEnum { LOGTYPEFAIL, LOGTYPEWARN, LOGTYPECONF, LOGTYPEINFO, LOGTYPETERM };

/**
 * Preprocessor macro to get the string for a specific color and font weight. The code is taken from
 * the LLVM method llvm::raw_ostream::changeColor.
 */
#define COLOR_CODE(COLOR, BOLD)                                                                                        \
  llvm::sys::Process::OutputColor(static_cast<char>(llvm::raw_ostream::Colors::COLOR), BOLD, false)

/**
 * The structure represents an entry in the LogMessageQueue consisting of the LogTypeEnum and string
 * of the message. Each logger is responsible for formatting the desired output.
 */
struct LogMessage {
  LogTypeEnum LogType;
  std::string Message;
};

/**
 * Full template specialization of the thread safe queue used for propagating logging messages to an
 * instance of a logger. Each logger thread automatically allocates a new queue.
 */
class LogMessageQueue : public ThreadSafeQueue<LogMessageQueue, LogMessage> {};

/**
 * Base class for all loggers. Each subclass might to implement the two virtual methods for prepending
 * a message type information at the beginning of the log message. The base class already contains the
 * default preamble and a new line character appendix which work best for most use cases.
 */
class LoggerImpl {

  LogMessageQueue *MessageQueue;
  llvm::raw_os_ostream ROStream;
  llvm::formatted_raw_ostream FOStream;

protected:
  /**
   * Initialize a new LoggerImpl instance. The base class cannot be instantiated directly and needs to
   * be derived by a subclass for different devices to log to.
   * @param Stream The output stream to write the messages to.
   * @param hasColors True, if the logger supports colors.
   */
  LoggerImpl(std::ostream &Stream, bool hasColors) : MessageQueue(nullptr), ROStream(Stream), FOStream(ROStream) {
#if ICARUS_CLANG_VERSION > 9
    FOStream.enable_colors(hasColors);
#endif
    FOStream.changeColor(llvm::raw_ostream::Colors::WHITE, false, false);
  }

public:
  /**
   * The logger thread method that runs indefinitely long. In the case a LogTypeEnum::LOGTYPETERM (terminate)
   * message is sent via the message queue, the logger thread terminates.
   * @param MessageQueue The message queue that stores all messages to log.
   */
  void worker(LogMessageQueue *MessageQueue);

  /**
   * The string to prepend to the log message. By default, the message already contains the local time
   * and the normalized thread number on which the message was issued. This might change if we want to
   * have more flexibility in arranging the log message format.
   * @param LogType The message type for which to prepend a string to the message.
   * @return A string that indicates the type of the log message.
   */
  [[nodiscard]] virtual std::string getPreamble(LogTypeEnum LogType) const;

  /**
   * The string to append to the log message. By default, this only returns a string that contains the
   * new line character. If there is never anything different here, we can directly append it and then
   * drop this member function if it is not needed.
   * @return A string containing the new line character.
   */
  [[nodiscard]] virtual std::string getAppendix() const;

  /**
   * Directs the formatted message including preamble and appendix to the output stream. Currently, we
   * are always flushing the stream after every message, which might slow down the process.
   * @param LogType The message type to create a preamble for.
   * @param Message The message string to log.
   */
  void logs(LogTypeEnum LogType, std::string &Message);
};

/**
 * A generic file logger which writes the output to a file. The location of the output file can be set
 * with the -debug-file CLI option in icarus.
 */
class FileLogger : public LoggerImpl {

  std::ofstream FileStream;

public:
  /**
   * Creates a new file logger instance. The class does not need to override the virtual methods as it
   * does not support colors: The default implementation of the base LoggerImpl class suffices.
   * @param File The path to the output file, which will be created if it does not exist yet.
   */
  explicit FileLogger(llvm::StringRef File) : FileStream(File.str()), LoggerImpl(FileStream, false) {}
};

/**
 * A generic terminal logger with color support. If the terminal does not support colors then the text
 * might include raw ANSI color codes. If this happens, we might have to separate the code for a color
 * terminal and a non-color terminal and control the selection via the CLI in icarus (--color?).
 */
struct TermLogger : public LoggerImpl {

  /**
   * Creates a new terminal logger instance. The preamble is overridden to print colored output on the
   * terminal for the different message types.
   */
  TermLogger() : LoggerImpl(std::cout, true) {}

  [[nodiscard]] std::string getPreamble(LogTypeEnum LogType) const override;
};

/**
 * The logger thread that contains a std::thread instance used to run a LoggerImpl object. On creation
 * this class will automatically allocate a new message queue which directs messages to its worker.
 */
class LogThread {

  LoggerImpl *Logger;
  LogMessageQueue *MessageQueue;
  std::thread LoggerThread;

public:
  /**
   * Create a new logger thread instance together with a dynamically allocated message queue. They are
   * destroyed by the Logger singleton instance via LogThread::waitFinished and do not need a destructor.
   * @param Logger The logger instance to launch in a new thread.
   */
  explicit LogThread(LoggerImpl *Logger)
      : Logger(Logger), MessageQueue(new LogMessageQueue()), LoggerThread(&LoggerImpl::worker, Logger, MessageQueue) {}

  /**
   * Stores the log type and the associated message in the message queue. The queue is thread safe and
   * does not need to rely on other external mutexes in the LogThread class.
   * @param LogType The message type to store in the queue.
   * @param Message The message string to log.
   */
  void logs(LogTypeEnum LogType, std::string &Message);

  /**
   * Destroys the logger thread by waiting for the thread to finish the remaining messages, before the
   * message queue is being deleted by the Logger class.
   */
  void shutdown();
};

/**
 * The logger frontend class and its methods to call when printing various information in icarus. Only
 * its static methods are publicly accessible from the outside to ensure this class stays a singleton.
 */
class Logger {

  std::vector<LogThread *> Loggers;

  Logger() = default;

  /**
   * @return Return a reference to the single static instance of the logger.
   */
  static Logger &get();

  /**
   * Stores any early messages in a single static vector. This method is only used for printing before
   * the actual logger threads have been setup. The messages stored here are only shown as soon as the
   * logger has initialized at least one logger thread.
   * @return A reference to the list of early messages to print later.
   */
  static std::vector<LogMessage> &getEarlyMessages();

  /**
   * Formats the log message and adds more information such as the timestamp (default: H:m:s), and the
   * normalized thread ID from which the log message originates from.
   * @tparam Args The variadic types of the message arguments.
   * @param ThreadID The normalized thread ID from which the message originates from.
   * @param args The different arguments of the message string.
   * @return A formatted string of the log message including timestamp and normalized thread ID.
   */
  template <typename... Args> static std::string formatMessage(unsigned ThreadID, Args &&...args) {
    std::stringstream Buffer;
    auto Clock = std::chrono::system_clock::now();
    auto Time = std::chrono::system_clock::to_time_t(Clock);
    ((Buffer << "[" << std::put_time(std::localtime(&Time), "%T") << "]"));
    if (ThreadPool::getThreadNum()) {
      unsigned Num = numDigits(ThreadPool::getThreadNum() + 1);
      ((Buffer << format("[%0*d]", Num, ThreadID)));
    } else if (ThreadID == UINT32_MAX) {
      ((Buffer << "[" << COLOR_CODE(MAGENTA, false) << "INIT" << COLOR_CODE(WHITE, false) << "]"));
    }
    ((Buffer << " "));
    ((Buffer << std::forward<Args>(args)), ...);
    return Buffer.str();
  }

  /**
   * Formats and propagates a log message to all loggers.
   * @tparam Args The variadic types of the message arguments.
   * @param LogType The type of the message to log.
   * @param args The different arguments of the message string.
   */
  template <typename... Args> void doLogs(enum LogTypeEnum LogType, Args &&...args) {
    unsigned ThreadID = ThreadPool::getThreadID();
    std::string Message = formatMessage(ThreadID, std::forward<Args>(args)...);
    for (LogThread *L : Loggers) {
      L->logs(LogType, Message);
    }
  }

  /**
   * Prints all early stored messages that were saved before the logger was initialized. After that we
   * clear the list of early messages. Never called again after that one time.
   */
  void doPrintEarlyLogs();

  /**
   * Add a logger interface, that is a subclass of LoggerImpl, to the logger frontend. The logger will
   * be deleted automatically in Logger::doWaitFinished before icarus ends.
   * @param Logger The LoggerImpl subclass to add to the logger frontend.
   */
  void doAddLogger(LoggerImpl *Logger);

  /**
   * Send a LogTypeEnum::LOGTYPETERM message to each registered logger thread and wait until the thread joins
   * before we delete logger threads, the shared message queue and the LoggerImpl subclass instances.
   */
  void doWaitFinished();

public:
  /**
   * Static method with call to singleton Logger::doLogs method.
   * @tparam Args The variadic types of the message arguments.
   * @param LogType The type of the message to log.
   * @param args The different arguments of the message string.
   */
  template <typename... Args> static void logs(enum LogTypeEnum LogType, Args &&...args) {
    get().doLogs(LogType, std::forward<Args>(args)...);
  }

  /**
   * Static method which stores early log messages in the early message list.
   * @tparam Args The variadic types of the message arguments.
   * @param LogType The type of the message to log.
   * @param args The different arguments of the message string.
   */
  template <typename... Args> static void earlyLogs(enum LogTypeEnum LogType, Args &&...args) {
    std::string Message = formatMessage(UINT32_MAX, std::forward<Args>(args)...);
    getEarlyMessages().push_back({LogType, Message});
  }

  /**
   * Static method with call to singleton Logger::doPrintEarlyLogs method.
   */
  static void printEarlyLogs();

  /**
   * Static method with call to singleton Logger::doAddLogger method.
   * @param Logger The LoggerImpl subclass to add to the logger frontend.
   */
  static void addLogger(LoggerImpl *Logger);

  /**
   * Static method with call to singleton Logger::doWaitFinished method.
   */
  static void waitFinished();
};

#define LOGS(Enum, ...) Logger::logs(Enum, __VA_ARGS__)

#define EARLY_LOGS(Enum, ...) Logger::earlyLogs(Enum, __VA_ARGS__)

#define INFO(...) LOGS(LOGTYPEINFO, __VA_ARGS__)
#define CONF(...) LOGS(LOGTYPECONF, __VA_ARGS__)
#define WARN(...) LOGS(LOGTYPEWARN, __VA_ARGS__)
#define FAIL(...) LOGS(LOGTYPEFAIL, __VA_ARGS__)

#define EARLY_INFO(...) EARLY_LOGS(LOGTYPEINFO, __VA_ARGS__)
#define EARLY_CONF(...) EARLY_LOGS(LOGTYPECONF, __VA_ARGS__)
#define EARLY_WARN(...) EARLY_LOGS(LOGTYPEWARN, __VA_ARGS__)
#define EARLY_FAIL(...) EARLY_LOGS(LOGTYPEFAIL, __VA_ARGS__)

#define INFO_COND(Cond, ...)                                                                                           \
  do {                                                                                                                 \
    if (Cond)                                                                                                          \
      INFO(__VA_ARGS__);                                                                                               \
  } while (0)
#define CONF_COND(Cond, ...)                                                                                           \
  do {                                                                                                                 \
    if (Cond)                                                                                                          \
      CONF(__VA_ARGS__);                                                                                               \
  } while (0)
#define WARN_COND(Cond, ...)                                                                                           \
  do {                                                                                                                 \
    if (Cond)                                                                                                          \
      WARN(__VA_ARGS__);                                                                                               \
  } while (0)
#define FAIL_COND(Cond, ...)                                                                                           \
  do {                                                                                                                 \
    if (Cond)                                                                                                          \
      FAIL(__VA_ARGS__);                                                                                               \
  } while (0)

#define EARLY_INFO_COND(Cond, ...)                                                                                     \
  do {                                                                                                                 \
    if (Cond)                                                                                                          \
      EARLY_INFO(__VA_ARGS__);                                                                                         \
  } while (0)
#define EARLY_CONF_COND(Cond, ...)                                                                                     \
  do {                                                                                                                 \
    if (Cond)                                                                                                          \
      EARLY_CONF(__VA_ARGS__);                                                                                         \
  } while (0)
#define EARLY_WARN_COND(Cond, ...)                                                                                     \
  do {                                                                                                                 \
    if (Cond)                                                                                                          \
      EARLY_WARN(__VA_ARGS__);                                                                                         \
  } while (0)
#define EARLY_FAIL_COND(Cond, ...)                                                                                     \
  do {                                                                                                                 \
    if (Cond)                                                                                                          \
      EARLY_FAIL(__VA_ARGS__);                                                                                         \
  } while (0)

#ifndef NDEBUG

/**
 * Check if the specified type is registered in the set of debug types.
 * @param DebugType The debug type to check.
 * @return True, if the debug type is registered in the set.
 */
bool isDebugType(llvm::StringRef DebugType);

#define INFO_WITH(Type, ...) INFO_COND(llvm::DebugFlag || isDebugType(Type), __VA_ARGS__)
#define CONF_WITH(Type, ...) CONF_COND(llvm::DebugFlag || isDebugType(Type), __VA_ARGS__)
#define WARN_WITH(Type, ...) WARN_COND(llvm::DebugFlag || isDebugType(Type), __VA_ARGS__)
#define FAIL_WITH(Type, ...) FAIL_COND(llvm::DebugFlag || isDebugType(Type), __VA_ARGS__)

#else

#define isDebugType(Type) (false)

#define INFO_WITH(Type, ...)                                                                                           \
  do {                                                                                                                 \
  } while (0)
#define CONF_WITH(Type, ...)                                                                                           \
  do {                                                                                                                 \
  } while (0)
#define WARN_WITH(Type, ...)                                                                                           \
  do {                                                                                                                 \
  } while (0)
#define FAIL_WITH(Type, ...)                                                                                           \
  do {                                                                                                                 \
  } while (0)

#endif /* !NDEBUG */

/**
 * Initialize logger component with options passed via the CLI.
 * @param DebugOnly The comma separated string of debug types to add to the set.
 * @param DebugFile Absolute path to the logger file where to store the output.
 */
void initLoggerOptions(llvm::StringRef DebugOnly, llvm::StringRef DebugFile);

} // namespace icarus

#endif // ICARUS_LOGGER_LOGGER_H

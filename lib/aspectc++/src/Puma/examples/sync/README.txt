

                          Example

                       "synchronized"
                    statement extension




INTRODUCTION
------------

The intention of this example is to demonstrate how to extend
the C++ parser. Thus it will be mainly shown how to extend the
lexical, syntactical, and semantical analysis. Additionally
two ways of manipulating parsed source code will be presented.


THE EXTENSION
-------------

The extension to the C++ parser to realize is a new statement
protecting the enclosed sub-statements from being modified by
more than one thread. The syntax of the new statement looks as
follows:

  sync_stmt ->   'synchronized' '{' sub-statements '}'
               | 'synchronized' '{'                '}'

It may be used for instance like this:

  void Log::write(const std::string &message) {
    std::string entry = getCurrentTime();
    entry += ": "+getUserName();
    entry += ": "+message;

    synchronized {
      std::ofstream logfile(m_File, std::ios::app);
      logfile << entry << std::endl;
      logfile.close();
    }
  }

The "synchronized" statement shall not open a new local scope,
i.e. the object "logfile" is to be introduced into the same scope
as the object "entry". Furthermore "synchronized" statements shall
not be nested. Otherwise an error message shall be generated. For
instance the following code:

  synchronized {
    ...
    synchronized {
      ...
    }
  }

shall cause the parser to show a message like this:

  file:3: error: synchronized statements cannot be nested


REALIZATION
-----------

The protection of the enclosed statements shall be realized using
a pthread's mutex implementation. More precisely, the statement's
head shall be replaced by:

  pthread_mutex_t __mutex_n;
  pthread_mutex_init(&__mutex_n, 0);
  pthread_mutex_lock(&__mutex_n);

and its tail by:

  pthread_mutex_unlock(&__mutex_n);
  pthread_mutex_destroy(&__mutex_n);

For instance the above code shall be transformed into:

  void Log::write(const std::string &message) {
    std::string entry = getCurrentTime();
    entry += ": "+getUserName();
    entry += ": "+message;

    pthread_mutex_t __mutex_1;
    pthread_mutex_init(&__mutex_1, 0);
    pthread_mutex_lock(&__mutex_1);

    std::ofstream logfile(m_File, std::ios::app);
    logfile << entry << std::endl;
    logfile.close();

    pthread_mutex_unlock(&__mutex_1);
    pthread_mutex_destroy(&__mutex_1);
  }

Additionally an include directive for the system header pthread.h
has to be added to the input file.


IMPLEMENTATION
--------------

The following steps has to be performed to realize this extension:

1. Implement a scanner extension for the "synchronized" keyword.
   For this purpose a lexer class has to be written. The lexer
   has to be set as lexer to use in the C++ scanner.

2. Implement a grammar extension for the "synchronized" statement.
   For this purpose a new syntactical analysis class has to be
   derived from the C++ syntax class. A new syntactical rule has
   to be implemented for the "synchronized" statement. Additionally
   a new syntax tree node for the "synchronized" statement is needed
   as well as a new syntax tree builder class supporting the new
   syntax tree node.

3. Implement a semantic analysis extension that is able to decide
   whether a "synchronized" statement is nested. For this purpose
   a new semantical analysis class has to be derived from the C++
   semantics class. A new semantic state has to be introduced used
   to decide whether a "synchronized" statement is nested or not.

4. Implement a transformer that replaces the "synchronized" statement
   with the pthread function calls. For this purpose a syntax tree
   visitor has to be derived to find all "synchronized" statements
   in the syntax tree. Each "synchronized" statement has to be
   transformed as described above using special code manipulation
   classes.

5. Implement the main application putting all the components together.
   This includes opening the input file, performing scanning, parsing,
   and transforming in the correct order, and saving the result.

#!/bin/bash

# path to ag++
AGCC="../../AspectC++/bin/${TARGET}/ag++"

# summary printed in case at least one testcase failed
ERR_SUM="+-----------------+\n";
ERR_SUM="${ERR_SUM}| Error Messages: |\n";
ERR_SUM="${ERR_SUM}+-----------------+\n";
ERR_OCCURED=0;

# files for storing output from stdout and stderr
STDOUT_FILE=stdout.tmp;
STDERR_FILE=stderr.tmp;

# check if TARGET variable is set
if [ -z ${TARGET} ];then
  echo -en "\n\nThis script has to be invoked by 'make test' from the toplevel directory\n\n"
  exit
fi

# check if the name of a test was passed
if [ $# -gt 0 ];then
  if [ -f $1 ];then
    TEST_FILES="$1";
  else
    echo "Test file '$1' not found";
    exit;
  fi
else
  # if no test was execute all test in this folder
  TEST_FILES=$(find . -maxdepth 1 -name 'test_??' | sort | sed ':a;N;$!ba;s/\n/ /g');
fi


# loop through all test files
for testfile in ${TEST_FILES};
do
  rm -f puma.config;
  ERR_MSG="";

  # include testfile file
  source $testfile;

  # redirect stdout and stderr
  exec 6>&1;
  exec 7>&2;
  exec > ${STDOUT_FILE};
  exec 2> ${STDERR_FILE};

  # execute main function of current test
  main ;
  RET=$?;

  # execute main function of current test

   #restore stdout and stderr
  exec 1>&6 6>&-;
  exec 2>&7 7>&-;

  # perform cleanup of test
  cleanup ;

  # check if test succeeded
  if [ $RET == 0 ]; then
    # test succeded
    echo -n ".";
  else
    # test failed
    echo -n "[${testfile/*_/}:F]";
    ERR_OCCURED=1;

    # get stdout and stderr
    STDOUT=`cat ${STDOUT_FILE}`;
    STDERR=`cat ${STDERR_FILE}`;

    # add information to error summary
    ERR_SUM="${ERR_SUM}\n\n-----------------------------------------------------------------------------------------\n";
    ERR_SUM="${ERR_SUM}TESTFILE: ${testfile}\n";
    ERR_SUM="${ERR_SUM}-----------------------------------------------------------------------------------------\n\n";
    ERR_SUM="${ERR_SUM}TEST DESCRIPTION: ${DESC//'\\'/\\\\}\n";
    ERR_SUM="${ERR_SUM}-----------------\n\n";
    ERR_SUM="${ERR_SUM}ERROR: ${ERR_MSG//'\\'/\\\\}\n";
    ERR_SUM="${ERR_SUM}------\n\n";
    ERR_SUM="${ERR_SUM}STDOUT:\n";
    ERR_SUM="${ERR_SUM}-------\n${STDOUT//'\\'/\\\\}\n";
    ERR_SUM="${ERR_SUM}STDERR:\n";
    ERR_SUM="${ERR_SUM}-------\n${STDERR//'\\'/\\\\}\n";
  fi
  rm -f puma.config;

done

# print summery of all errors occured
echo;
echo;
if [ $ERR_OCCURED == 1 ];then
  echo -e "$ERR_SUM";
else
  echo "All tests OK";
fi

# remove files containing output from stdout and stderr
rm $STDERR_FILE $STDOUT_FILE;

